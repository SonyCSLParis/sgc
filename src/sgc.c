/*  
    sgc: A garbage collector for C/C++.
 
    Copyright (C) 2002-2008 Peter Hanappe, Sony Computer Science
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

#include "sgc.h"
#include "sgc_priv.h"
#include "sgc_sys.h"
#include "sgc_types.h"
#include "sgc_list.h"
#include "sgc_allocator.h"
#include "sgc_thread.h"
#include "sgc_mess.h"
#include "sgc_dlist.h"
#include "sgc_log.h"

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>

static pthread_t sgc_thread;
static pthread_t sgc_continue = 1;
static int sgc_initialized = 0;
static int sgc_concurrent = 0;

#define SGC_MAX_ROOTS 128
static void** sgc_roots[SGC_MAX_ROOTS];

static int sgc_counter = 0;
static int sgc_stop_counter = -1;
static sgc_counter_callback_t sgc_counter_callback = NULL;
static void* sgc_stop_addr = 0;

static sgc_dlist_t* sgc_new_list = NULL;
static sgc_dlist_t* sgc_scan_list = NULL;
static sgc_dlist_t* sgc_from_list = NULL;

typedef struct _sgc_type_t {
        sgc_finalise_t finalise;
} sgc_type_t;

static sgc_type_t sgc_typetab[256];

static void* sgc_main(void *);
static void sgc_handle_messages(void);
static void sgc_clear(void);
static void sgc_mark(int scanthreads);
static void sgc_sweep(int findleaks);
static void sgc_print_list(sgc_dlist_t* list, char* label);

/* static void sgc_lib_init(void) ; */
/* static void sgc_lib_fini(void); */

//static void __attribute__((constructor)) sgc_lib_init(void)
static void sgc_lib_init(void)
{
	int i;

        printf("*** sgc_lib_init ***\n");

	if (sgc_initialized == 1) {
		sgc_err("Garbage collector already initialized");
		return;
	} else if (sgc_initialized == -1) {
		sgc_err("Garbage collector failed to initialize");
		return;
	}

	sgc_initialized = -1;

	if (sgc_log_init() != 0)
		return;
        
	if (sgc_allocator_init() != 0)
		return;

	if (sgc_message_module_init(2048) != 0)
		return;

        for (i = 0; i < 256; i++)
                sgc_typetab[i].finalise = NULL;

        for (i = 0; i < SGC_MAX_ROOTS; i++)
                sgc_roots[i] = NULL;

#if SGC_USE_THREADS
	sgc_thread_init();
#endif

	sgc_initialized = 1;
}

//static void __attribute__((destructor)) sgc_lib_fini(void)
static void sgc_lib_fini(void)
{
        printf("*** sgc_lib_fini ***\n");
}

int sgc_init(void* ptr, int concurrent)
{
        printf("*** sgc_init 1 ***\n");

	if (sgc_initialized == 0)
                sgc_lib_init();
                        
        printf("*** sgc_init 2 ***\n");

        if (sgc_initialized != 1) {
		sgc_err("Garbage collector failed to initialize");
		return -1;
        }

        printf("*** sgc_init 3 ***\n");

#if SGC_USE_THREADS
	sgc_thread_attach_main(pthread_self(), ptr);

	sgc_concurrent = concurrent;
	if (sgc_concurrent)
		pthread_create(&sgc_thread, NULL, sgc_main, NULL);
#endif
        printf("*** sgc_init 4 ***\n");

	return 0;
}

void sgc_cleanup()
{
#if SGC_USE_THREADS
	sgc_thread_detach(pthread_self());
#endif

	sgc_continue = 0;
	
#if SGC_USE_THREADS
	if (sgc_concurrent)
		pthread_join(sgc_thread, NULL);
	sgc_thread_cleanup();
#endif

	sgc_message_module_cleanup();
	sgc_allocator_cleanup();
	sgc_log_cleanup();
        sgc_close_stream();
}

int sgc_get_counter()
{
	return sgc_counter;
}

void sgc_call_at_counter(sgc_counter_callback_t callback, int counter)
{
	sgc_stop_counter = counter;
        sgc_counter_callback = callback;
}

void sgc_stop_scan_at(void* ptr)
{
	sgc_stop_addr = ptr;
}

#if SGC_USE_THREADS
void* sgc_main(void *arg)
{
	uint32 count = 0;

	while (sgc_continue) {
		count++;
		sgc_run();
		pthread_yield();
	}

	return NULL;
}
#endif

void* sgc_new_object(int nbytes, int flags, unsigned char type)
{
	sgc_mem_t* mem;

	
	/* if (0) { */
	/* 	void *array[10]; */
	/* 	size_t size; */
	/* 	char **strings; */
	/* 	size_t i; */
		
	/* 	size = backtrace(array, 10); */
	/* 	strings = backtrace_symbols(array, size); */
		
	/* 	printf ("Obtained %zd stack frames.\n", size); */
		
	/* 	for (i = 0; i < size; i++) { */
	/* 		printf ("%s\n", strings[i]); */
	/* 	} */
		
	/* 	free (strings); */
	/* } */

	if (nbytes <= 0)
		return NULL;

	mem = sgc_allocator_alloc(nbytes);
	if (mem == NULL)
		return NULL;

	mem->list.next = NULL;
	mem->list.prev = NULL;
	mem->color = SGC_BLACK;
	mem->flags = flags;
	mem->dummy = SGC_DUMMY_VALUE;
	mem->type = type;
	mem->counter = sgc_counter++;
	mem->bytesize = nbytes;

#if SGC_USE_BACKTRACE
	mem->backtrace_size = backtrace(mem->backtrace, SGC_MEM_BACKTRACE_SIZE);
#endif

	sgc_message_send(SGC_NEW_MESSAGE, mem, 0);

	if ((mem->counter == sgc_stop_counter) && (sgc_counter_callback != NULL))
                sgc_counter_callback();

	sgc_trace_alloc(mem, nbytes);

	if (flags & SGC_ZERO)
		sgc_memset(&mem->data[0], 0, nbytes);

	return (void*) &mem->data[0];
}

void sgc_free_object(void* ptr)
{
	sgc_mem_t *mem1;

	if (ptr == 0) {
		// FIXME: send error message
		return;
	}
	
	mem1 = sgc_ptr_to_mem(ptr);
	if (mem1->color == SGC_RED) {
		//printf("sgc:    ***  object freed twice!   ***\n");
	}

	sgc_message_send(SGC_FREE_MESSAGE, mem1, 0);	
}

void* sgc_resize_object(void* ptr, int nbytes, int flags, unsigned char type)
{
	sgc_mem_t *mem;

	if (nbytes < 0)
                return NULL;

	if (ptr == NULL)
                return sgc_new_object(nbytes, flags, type);
	
	mem = sgc_ptr_to_mem(ptr);
	/* if (mem->color == SGC_RED) { */
		//printf("sgc:    ***  object alreadt freed!   ***\n");
	/* } */
        
        int reuse = 1;        
        uint32 size = sgc_allocator_byte_size(mem);

        if (((unsigned int) nbytes < size) && (mem->size > 0)) {
                uint32 smaller = sgc_allocator_mem_size(mem->size - 1);
                if ((unsigned int) nbytes <= smaller) 
                        reuse = 0; // a smaller one will do
        } else if ((unsigned int) nbytes > size) {
                reuse = 0; // need a bigger one
        }

        if (reuse) {
                mem->bytesize = nbytes;
                return ptr;
        }

        void* newptr = sgc_new_object(nbytes, mem->flags, mem->type);
        if (newptr == NULL) {
                sgc_free_object(ptr);
                return NULL;
        }

        int old_nbytes = mem->bytesize;
        int copy = (old_nbytes < nbytes)? old_nbytes : nbytes;
        sgc_memcpy(newptr, ptr, copy);

        sgc_free_object(ptr);

        return newptr;
}

void sgc_root(void** handle)
{
        int i;

#ifdef DEBUG
        int count = 0;
        for (i = 0; i < SGC_MAX_ROOTS; i++)
                if (sgc_roots[i] != NULL)
                        count++;
        printf("SGC: Number of roots: %d\n", count);
#endif

        for (i = 0; i < SGC_MAX_ROOTS; i++) {
                if (sgc_roots[i] == NULL) {
                        sgc_roots[i] = handle;
                        return;
                }
        }
	sgc_panic("*** Root overflow!  ***");
}

void sgc_unroot(void** handle)
{
        int i;
        for (i = 0; i < SGC_MAX_ROOTS; i++) {
                if (sgc_roots[i] == handle) {
                        sgc_roots[i] = NULL;
                        return;
                }
        }
}

void sgc_new_list_append(sgc_mem_t* mem)
{
	sgc_new_list = sgc_dlist_append(sgc_new_list, (sgc_dlist_t*) mem);
}

void sgc_scan_list_append(sgc_mem_t* mem)
{
	sgc_scan_list = sgc_dlist_append(sgc_scan_list, (sgc_dlist_t*) mem);
}

void sgc_from_list_remove(sgc_mem_t* mem)
{
	sgc_dlist_t* list = (sgc_dlist_t*) mem;
	if (list == sgc_from_list)
		sgc_from_list = sgc_dlist_remove_node(list);
	else
		sgc_dlist_remove_node(list);

/* 	sgc_from_list = sgc_dlist_remove(sgc_from_list, (sgc_dlist_t*) mem); */
}

void sgc_run() 
{
	sgc_trace_start();
	
	sgc_handle_messages();

	/* Add the 'new' set to the current 'scan' set and empty the
	 * 'new' set. */
	sgc_scan_list = sgc_dlist_join(sgc_scan_list, sgc_new_list);
	sgc_from_list = sgc_scan_list;
	sgc_new_list = NULL;
	sgc_scan_list = NULL;

	sgc_clear();	
	sgc_mark(1);
	sgc_sweep(0);

	sgc_trace_finished();
}

static int default_memory_leak_fun(int op, void* ptr, 
                                   unsigned char type, 
				   int counter, int size)
{
        return 0;
}

void sgc_search_memory_leaks(memory_leak_fun_t callback)
{
	int count_allocated = 0;
	int count_scanned = 0;
	int count_freed = 0;
	int count_leaked = 0;
	sgc_dlist_t* node;

	sgc_trace_start();

	sgc_handle_messages();

	/* Add the 'new' set to the current 'scan' set and empty the
	 * 'new' set. */
	sgc_scan_list = sgc_dlist_join(sgc_scan_list, sgc_new_list);
	sgc_from_list = sgc_scan_list;
	sgc_new_list = NULL;
	sgc_scan_list = NULL;

	/* printf("clearing\n"); */

	sgc_clear();


	/* printf("counting allocated objects\n"); */

	if (callback == NULL)
		callback = default_memory_leak_fun;

	node = sgc_dlist_first(sgc_from_list);
	while (node) {
		count_allocated++;
		sgc_mem_t* mem = (sgc_mem_t*) node;
		(*callback)(0, (void*) &mem->data[0], 
			    mem->type, mem->counter, mem->bytesize);
		node = sgc_dlist_next(node);
		if (node == sgc_dlist_first(sgc_from_list)) {
			break;
		}
	}

	/* printf("marking\n"); */

	sgc_mark(0);

	/* printf("counting scanned objects\n"); */

	node = sgc_dlist_first(sgc_scan_list);
	while (node) {
		count_scanned++;
		sgc_mem_t* mem = (sgc_mem_t*) node;
		
		(*callback)(1, (void*) &mem->data[0], 
			    mem->type, mem->counter, mem->bytesize);
		
		node = sgc_dlist_next(node);
		if (node == sgc_dlist_first(sgc_scan_list)) {
			break;
		}
	}

	/* printf("counting freed/leaked objects\n"); */

	node = sgc_dlist_first(sgc_from_list);
	while (node) {
		sgc_mem_t* mem = (sgc_mem_t*) node;
		
		if (mem->color == SGC_RED) {
			count_freed++;
			(*callback)(2, (void*) &mem->data[0], 
				    mem->type, mem->counter, mem->bytesize);
			
		} else if (mem->color == SGC_WHITE) {
			char **strings;
			int i;
			
			count_leaked++;
			
			/* printf("Memory leak: ----------------\n"); */
			
			/* printf("counter=%d, size=%d, type=%d\n", mem->counter, mem->bytesize, mem->type); */

			/* printf("stack trace at allocation:\n"); */
			(*callback)(3, (void*) &mem->data[0], 
				    mem->type, mem->counter, mem->bytesize);
			
			strings = backtrace_symbols(mem->backtrace, mem->backtrace_size);
			
			for (i = 1; i < mem->backtrace_size; i++) {
                                
                                printf("--\n");
                                        
                                // DIRTY
                                int j;
                                char cmd[256];
                                snprintf(cmd, 255, "addr2line -e %s", strings[i]);
                                for (j = 0; cmd[j] != 0; j++) {
                                        if ((cmd[j] == '[') || (cmd[j] == ']')
                                            || (cmd[j] == '(') || (cmd[j] == ')'))
                                                cmd[j] = ' ';
                                } 

                                printf("%s\n", strings[i]);
                                
                                FILE * file = popen(cmd, "r");
                                if (file == NULL) {
                                        printf("%s\n", strings[i]);
                                        continue;
                                }
                                
                                char res[256];
                                size_t read = fread(res, sizeof(char), 255, file);
                                if (read > 0) {
                                        res[read] = 0;
                                        printf("%s", res);
                                } else {
                                        printf("%s\n", strings[i]);
                                }

                                fclose(file);
                                // DIRTY
			}

			free (strings);
			
			printf("------------------------------\n");
			
			
		} else {
			printf("Invalid color in 'from' list\n");
		}
		
		node = sgc_dlist_next(node);
		if (node == sgc_dlist_first(sgc_from_list)) {
			break;
		}
	}

	printf("count_allocated = %d\n", count_allocated);
	printf("count_scanned   = %d\n", count_scanned);
	printf("count_freed     = %d\n", count_freed);
	printf("count_leaked    = %d\n", count_leaked);

	printf("sweeping\n");

	sgc_sweep(1);

	sgc_trace_finished();
}

void sgc_clear() 
{
	int count = 0;
	sgc_dlist_t* node;
	
	sgc_trace_clearing();

	node = sgc_dlist_first(sgc_from_list);
	while (node) {
		sgc_mem_t* mem = (sgc_mem_t*) node;
		
		// Keep the red ones: they are used for debugging
		// memory leaks
		if (mem->color != SGC_RED)
			mem->color = SGC_WHITE;
		
		count++;

		node = sgc_dlist_next(node);
		if (node == sgc_dlist_first(sgc_from_list))
			break;
	}

	sgc_trace_cleared(count);
}

void sgc_mark(int scanthreads) 
{
	void** handle;
	void* ptr;
	sgc_dlist_t* scan_ptr;
	sgc_mem_t* mem;
	int count = 0;
        int i;

	sgc_trace_marking();

	/* Add all the roots to the 'scan' set */
        for (i = 0; i < SGC_MAX_ROOTS; i++) {

                if (sgc_roots[i] == NULL)
                        continue;

		handle = sgc_roots[i];
		ptr = *handle;
		mem = sgc_ptr_to_mem(ptr);

		if ((ptr != 0) 
		    && (mem->color != SGC_RED)
		    && sgc_allocator_is_valid(mem)) {

			/* Mark it gray */
			mem->color = SGC_GRAY;

			/* Remove from 'from' set */
			sgc_from_list_remove(mem);

			/* Move to 'scan' set */
			sgc_scan_list_append(mem);

			sgc_trace_root(mem);
		}
	}

	/* Scan the stacks for roots */
#if SGC_USE_THREADS
	if (scanthreads) {
		sgc_scan_threads();
	}
#endif

	scan_ptr = sgc_scan_list;

	while (scan_ptr) {
		mem = (sgc_mem_t*) scan_ptr;

		ptr = (void*) &mem->data[0];
		if (ptr == sgc_stop_addr) {
			//printf("sgc: reached stop addr: %p\n", ptr);
		}

		/* Mark it black */
		mem->color = SGC_BLACK;

		/* Scan for object references */
		if ((mem->flags & SGC_POINTERLESS) == 0)
			sgc_scan_mem(mem);

		sgc_handle_messages();

		scan_ptr = sgc_dlist_next(scan_ptr);
		if (scan_ptr == sgc_dlist_first(sgc_scan_list))
			break;
	}
}

void sgc_scan_mem(sgc_mem_t* mem)
{
	uint32 size = sgc_allocator_byte_size(mem);
	uint32 index = 0;
	uint32 bytes = 0;
	sgc_ptr_t ptr;
	sgc_ptr_t* hdle;
	sgc_mem_t* referee;

	hdle = (sgc_ptr_t*) &mem->data[0];
	while (bytes < size) {

		ptr = hdle[index];
		referee = sgc_ptr_to_mem(ptr);

		/* Test for ptr==0 because it's a common case */
		if ((ptr != 0) && sgc_allocator_is_valid(referee)) {

			if (referee->color == SGC_RED) {
				/* Red objects should never be in the
				 * 'scan' list. */
				sgc_trace_memerr(referee);
				
			} else if (referee->color == SGC_WHITE) {

				sgc_trace_reached(referee, mem);
				
				mem->color = SGC_GRAY;
				sgc_from_list_remove(referee);
				sgc_scan_list_append(referee);
			}
		}

		index++;
		bytes += sizeof(sgc_ptr_t);
	}	
}

void sgc_sweep(int findleaks) 
{
	sgc_dlist_t* node;
	sgc_dlist_t* next;
	sgc_mem_t* mem;

	sgc_trace_sweeping();

	node = sgc_dlist_first(sgc_from_list);
	while (node) {

		next = sgc_dlist_next(node);

		mem = (sgc_mem_t*) node;

		if (findleaks) {
			if (mem->color == SGC_WHITE) {
				sgc_trace_leaked(mem);

			} else if (mem->color == SGC_RED) {
				sgc_trace_freed(mem);
				sgc_allocator_free(mem);
			}
		} else {
			sgc_trace_sweep(mem);
                        if (sgc_typetab[mem->type].finalise)
                                sgc_typetab[mem->type].finalise(&mem->data[0]);
			sgc_allocator_free(mem);
		}

		node = next;
		if (node == sgc_dlist_first(sgc_from_list))
			break;
	}
}

void sgc_handle_message(sgc_message_t* m)
{
	sgc_mem_t *mem1, *mem2;

        switch (m->message) {
        case SGC_NEW_MESSAGE:
                mem1 = (sgc_mem_t *) m->arg1;
                sgc_new_list_append(mem1);		
                sgc_trace_rcvnew(mem1);
                break;
			
        case SGC_CTI_MESSAGE: 
                
                if (m->arg1 == NULL) {
                        mem2 = sgc_ptr_to_mem(m->arg2);
                        if (mem2->color == SGC_WHITE) { 
                                sgc_from_list_remove(mem2);
                                sgc_scan_list_append(mem2);
                        }
                        sgc_trace_rcvcti(0, mem2);
                } else {
                        mem1 = sgc_ptr_to_mem(m->arg1);
                        mem2 = sgc_ptr_to_mem(m->arg2);
                        if ((mem2->color == SGC_WHITE) 
                            && (mem2->color == SGC_WHITE)) { 
                                sgc_from_list_remove(mem2);
                                sgc_scan_list_append(mem2);
                        }
                        sgc_trace_rcvcti(mem1, mem2);
                }
                break;

        case SGC_FREE_MESSAGE:
                mem1 = (sgc_mem_t *) m->arg1;
                mem1->color = SGC_RED;		
                sgc_trace_rcvfree(mem1);
                break;
			
        }
}

static void sgc_handle_messages()
{
	sgc_message_t* m = sgc_message_get();
	while (m != NULL) {
                sgc_handle_message(m);
		sgc_message_delete(m);
		m = sgc_message_get();
	}
}

/* void sgc_print_list(sgc_dlist_t* list, char* label) */
/* { */
/* 	sgc_dlist_t* cur = list; */

/* 	printf("%s: ", label); */

/* 	while (cur) { */
		
/* 		printf("%p ", cur); */

/* 		cur = sgc_dlist_next(cur); */
/* 		if (cur == sgc_dlist_first(list)) { */
/* 			break; */
/* 		} */
/* 	} */

/* 	printf("\n"); */
/* } */

void sgc_register_type(unsigned char type, sgc_finalise_t finalise)
{
        sgc_typetab[type].finalise = finalise;
}

void sgc_unregister_type(unsigned char type)
{
        sgc_typetab[type].finalise = NULL;
}
