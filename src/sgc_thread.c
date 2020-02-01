/*  
    sgc: A garbage collector for C/C++.
 
    Copyright (C) 2002-2020 Peter Hanappe, Sony Computer Science
    Laboratory Paris

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*  -*- linux-c -*- */

#if SGC_USE_THREADS

#include "sgc_thread.h"
#include "sgc_types.h"
#include "sgc_list.h"
#include "sgc_sys.h"
#include "sgc_allocator.h"
#include "sgc_priv.h"

typedef struct _sgc_thread_t sgc_thread_t;

struct _sgc_thread_t {
	pthread_t thread;

	pthread_mutex_t scan_mutex;

	pthread_mutex_t sync_mutex;
	pthread_cond_t sync_cond;

	sgc_ptr_t stack_top;
	sgc_ptr_t stack_bottom;
	uint32 stack_size;
};


static sgc_list_t *sgc_threads = NULL;
static pthread_mutex_t sgc_threads_mutex;
static pthread_key_t sgc_thread_key;
static pthread_once_t sgc_thread_key_once = PTHREAD_ONCE_INIT;

static sgc_thread_t *sgc_find_thread(pthread_t thread);
static int sgc_scan_thread(sgc_thread_t *th);
static void sgc_set_gc_waiting(sgc_thread_t *th, int waiting);
static int sgc_is_gc_waiting(sgc_thread_t *th);
static void sgc_wait_gc_finished(sgc_thread_t *th);
static void sgc_set_gc_finished(sgc_thread_t *th);
static sgc_thread_t *sgc_thread_new(pthread_t thread);
static void sgc_thread_key_alloc(void);
static void sgc_thread_key_destroy(void  *buf);

     
static void sgc_thread_key_destroy(void  *buf)
{
}

     
static void sgc_thread_key_alloc()
{
	pthread_key_create(&sgc_thread_key, sgc_thread_key_destroy);
}

int sgc_thread_init()
{
	pthread_mutex_init(&sgc_threads_mutex, NULL);
	pthread_once(&sgc_thread_key_once, sgc_thread_key_alloc);
	return 0;
}


int sgc_thread_cleanup()
{
	pthread_mutex_destroy(&sgc_threads_mutex);	
	return 0;
}


int sgc_thread_attach_main(pthread_t thread, void *ptr)
{
	sgc_thread_t *th = sgc_thread_new(thread);

	if (th == NULL) {
		return -1;
	}

	th->stack_top = ptr;

	
/* 	{ */
/* 		int i; */
/* 		uint32 start = sgc_curtime(); */

/* 		for (i = 0; i < 10000; i++) { */
/* 			th = (sgc_thread_t*) pthread_getspecific(sgc_thread_key); */
/* 		} */
		
/* 		printf("Total time: %d\n", sgc_curtime() - start); */
/* 	} */

	return 0;
}

int sgc_thread_attach(pthread_t thread)
{
	pthread_attr_t attr;
	sgc_thread_t *th = sgc_thread_new(thread);
        size_t size;

	if (th == NULL) {
		return -1;
	}

	pthread_getattr_np(th->thread, &attr);
	pthread_attr_getstack(&attr, (void**) &th->stack_top, &size);
        th->stack_size = size;

	return 0;
}

/*
 * The new thread immediately locks the scan mutex. This lock will
 * cause the GC thread to block when it enter sgc_scan_thread().
 */
sgc_thread_t *sgc_thread_new(pthread_t thread)
{
	sgc_thread_t *th;
	
	th = sgc_new(sgc_thread_t);
	if (th == NULL) {
		sgc_panic("Out of memory");
		return NULL;
	}

	pthread_mutex_lock(&sgc_threads_mutex);	

	th->thread = thread;

	pthread_mutex_init(&th->scan_mutex, NULL);
	//th->scan_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

	pthread_mutex_init(&th->sync_mutex, NULL);
	//th->sync_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
	pthread_cond_init(&th->sync_cond, NULL);

	pthread_mutex_lock(&th->scan_mutex);	

	sgc_threads = sgc_list_append(sgc_threads, th);

	pthread_setspecific(sgc_thread_key, th);

	pthread_mutex_unlock(&sgc_threads_mutex);	

	return th;
}

int sgc_thread_detach(pthread_t thread)
{
	sgc_thread_t *th;

	pthread_mutex_lock(&sgc_threads_mutex);	

	th = sgc_find_thread(pthread_self());

	if (th == NULL) {
		pthread_mutex_unlock(&sgc_threads_mutex);	
		sgc_err("Request to detach an un-attached thread");
		return -1;
	}

	pthread_mutex_unlock(&th->scan_mutex);

	pthread_cond_destroy(&th->sync_cond);
	pthread_mutex_destroy(&th->sync_mutex);
	pthread_mutex_destroy(&th->scan_mutex);

	sgc_threads = sgc_list_remove(sgc_threads, th);

	pthread_mutex_unlock(&sgc_threads_mutex);	

	return 0;
}

int sgc_scan_threads()
{
	sgc_list_t *thread_list;
	sgc_thread_t *th;

	pthread_mutex_lock(&sgc_threads_mutex);	

	thread_list = sgc_threads;

	while (thread_list) {
		th = sgc_list_get(thread_list);
		thread_list = sgc_list_next(thread_list);

		sgc_scan_thread(th);
	}

	pthread_mutex_unlock(&sgc_threads_mutex);	

	return 0;
}

int sgc_thread_lock(void)
{
	sgc_thread_t *th;

	/* FIXME: synchronization on sgc_threads_mutex. We shouldn't
	 * use sgc_find_thread() but instead, attach the sgc_thread_t
	 * structure to the thread using pthread_setspecific(). */
	th = sgc_find_thread(pthread_self());

	/* Acquire the 'scan' lock. This will block until the GC is
	 * finished if it is currently running.*/
	pthread_mutex_lock(&th->scan_mutex);	

	sgc_trace_cont(th);

	return 0;
}

int sgc_thread_unlock(void)
{
	uint32 stack_bottom;
	sgc_thread_t *th;

	/* FIXME: synchronization on sgc_threads_mutex. We shouldn't
	 * use sgc_find_thread() but instead, attach the sgc_thread_t
	 * structure to the thread using pthread_setspecific(). */
	th = sgc_find_thread(pthread_self());

	if (th == NULL) {
		sgc_err("Request to synchronize the GC with a detached thread");
		return -1;
	}

	th->stack_bottom = &stack_bottom;

	/* Release the 'scan' lock so the GC can continue if it's
	 * waiting. As long as we hold the 'scan' lock it will still
	 * be blocked.*/
	pthread_mutex_unlock(&th->scan_mutex);	

	return 0;
}

int sgc_thread_sync(void)
{
	uint32 stack_bottom;
	sgc_thread_t *th;

	/* FIXME: synchronization on sgc_threads_mutex. We shouldn't
	 * use sgc_find_thread() but instead, attach the sgc_thread_t
	 * structure to the thread using pthread_setspecific(). */
	th = sgc_find_thread(pthread_self());

	if (th == NULL) {
		sgc_err("Request to synchronize the GC with a detached thread");
		return -1;
	}

	th->stack_bottom = &stack_bottom;

	/* Release the 'scan' lock so the GC can continue if it's
	 * waiting. As long as we hold the 'scan' lock it will still
	 * be blocked.*/
	pthread_mutex_unlock(&th->scan_mutex);	

	sgc_trace_sync(th);

	/* Acquire the lock of the 'sync' condition. */
	pthread_mutex_lock(&th->sync_mutex);

	
	/* Finished signaling this thread's readyness to sync. */
	pthread_mutex_unlock(&th->sync_mutex);


	/* Re-acquire the 'scan' lock. This will block until the GC is
	 * finished if it is currently running.*/
	pthread_mutex_lock(&th->scan_mutex);	

	sgc_trace_cont(th);

	return 0;
}

int sgc_scan_thread(sgc_thread_t *th)
{
	sgc_ptr_t from, to, ptr;
	sgc_ptr_t *handle;
	sgc_mem_t *referee;

	pthread_mutex_lock(&th->sync_mutex);

	sgc_trace_waiting(th);

	pthread_mutex_lock(&th->scan_mutex);

	pthread_mutex_unlock(&th->sync_mutex);

	sgc_trace_stackstart(th);

	from = th->stack_top;
	to = th->stack_bottom;

	if (from > to) {
		sgc_ptr_t tmp = to;
		to = from;
		from = tmp;
	}

	for (ptr = from; ptr <= to; ptr++) {

		referee = sgc_ptr_to_mem(*ptr);

		if (sgc_allocator_is_valid(referee) && (referee->color == SGC_WHITE)) {

			/* Mark it gray */
			referee->color = SGC_GRAY;
			
			/* Remove from 'from' set */
			sgc_from_list_remove(referee);

			/* Move to 'scan' set */
			sgc_scan_list_append(referee);

			sgc_trace_stack(referee);
		}
	}


	sgc_trace_stackend(th);

	pthread_mutex_unlock(&th->scan_mutex);

	return 0;
}

static sgc_thread_t *sgc_find_thread(pthread_t thread)
{
	sgc_list_t *thread_list = sgc_threads;
	sgc_thread_t *th;

	while (thread_list) {
		th = sgc_list_get(thread_list);
		if (th->thread == thread) {
			return th;
		}
		thread_list = sgc_list_next(thread_list);
	}

	return NULL;
}

#endif
