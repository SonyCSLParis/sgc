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

#ifndef _SGC_H
#define _SGC_H

#if SGC_USE_THREADS
#define _GNU_SOURCE
#include <pthread.h>
#endif 

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SGC_USE_BACKTRACE 1

int sgc_init(void* ptr, int concurrent);

void sgc_cleanup(void);

#define _rt(_p) sgc_root((void**) &_p)
#define _urt(_p) sgc_unroot((void**) &_p)

void sgc_root(void** handle);
void sgc_unroot(void** handle);


typedef enum {
        SGC_ZERO = 1,
        SGC_POINTERLESS = 2,
        SGC_FLAGSMAX = 4,
} sgc_flags_t;


void* sgc_new_object(int nbytes, int flags, unsigned char type);
void sgc_free_object(void* obj);
void* sgc_resize_object(void* ptr, int nbytes, int flags, unsigned char type);

typedef void (*sgc_counter_callback_t)(void);

int sgc_get_counter(void);
void sgc_call_at_counter(sgc_counter_callback_t callback, int counter);

void sgc_stop_scan_at(void* ptr);

typedef void (*sgc_finalise_t)(void* ptr);

void sgc_register_type(unsigned char type, 
                       sgc_finalise_t finalise);

void sgc_unregister_type(unsigned char type);

typedef enum {
        SGC_WHITE = 0,
        SGC_GRAY = 1,
        SGC_BLACK = 2,
        SGC_RED = 3,
        SGC_COLORMAX = 4
} sgc_color_t;

#define SGC_DUMMY_VALUE 0x88

#define SGC_MEM_BACKTRACE_SIZE 16
#define SGC_MEM_DATA_SIZE 0
#define SGC_MEM_ALIGN     sizeof(void*)


typedef struct _sgc_dlist_t sgc_dlist_t;

struct _sgc_dlist_t {
        sgc_dlist_t* next;
        sgc_dlist_t* prev;
};


typedef struct _sgc_mem_t sgc_mem_t;

struct _sgc_mem_t {
        sgc_dlist_t list;
        unsigned char color;
        unsigned char flags;
        unsigned char size;
        unsigned char dummy;
        int type;
        int counter;
        int bytesize;
#if SGC_USE_BACKTRACE
        int backtrace_size;
        void *backtrace[SGC_MEM_BACKTRACE_SIZE];
#endif
        char data[SGC_MEM_DATA_SIZE];
};


enum {
        SGC_NEW_MESSAGE = 1,	// New object 
        SGC_CTI_MESSAGE,	// Color changed
        SGC_FREE_MESSAGE,	// Free object
};

typedef struct _sgc_message_t sgc_message_t;
	
void sgc_message_send(int message, void* arg1, void* arg2);


#define sgc_ptr_to_mem(_p) ((sgc_mem_t* )(((char*)_p)-sizeof(sgc_mem_t)+SGC_MEM_DATA_SIZE))

static inline int sgc_bytesize(void* p)
{
        return sgc_ptr_to_mem(p)->bytesize;
}

static inline void sgc_wb2(void* p1, void* p2)
{
        if ((p2 != NULL) 
            && (sgc_ptr_to_mem(p1)->color == SGC_BLACK) 
            && (sgc_ptr_to_mem(p2)->color == SGC_WHITE)) { 
                sgc_message_send(SGC_CTI_MESSAGE, p1, p2); 
        } 
}

static inline void sgc_wb1(void* p2)
{
        if ((p2 != NULL) && (sgc_ptr_to_mem(p2)->color == SGC_WHITE)) { 
                sgc_message_send(SGC_CTI_MESSAGE, NULL, p2); 
        } 
}

#define _stf(_p1, _f, _p2) { _p1->_f = _p2; sgc_wb2((void*) _p1, (void*) _p1->_f); }
#define _stt(_f, _p2)      { _f = _p2; sgc_wb2((void*) this, (void*) _f); }
#define _str(_p1, _p2)     { _p1 = _p2; sgc_wb1((void*) _p2); }


void sgc_run(void);

int sgc_create_stream(int port);

#if SGC_USE_THREADS
int sgc_thread_attach(pthread_t thread);
int sgc_thread_detach(pthread_t thread);

int sgc_thread_lock(void);
int sgc_thread_unlock(void);

int sgc_thread_sync(void);
#endif

typedef int (*memory_leak_fun_t)(int op, void* ptr, 
                                 unsigned char type, 
                                 int counter, int size);

void sgc_search_memory_leaks(memory_leak_fun_t callback);

// log type
enum {
        SGC_LOG_DBG = 0,
        SGC_LOG_ERR,
        SGC_LOG_PROF,
        SGC_LOG_TRACE,
        SGC_LOG_LAST
};

// log activity
enum {
        SGC_TRACE_ALLOC = 0,
        SGC_TRACE_SCAN,
        SGC_TRACE_GC,
        SGC_TRACE_3,
        SGC_TRACE_4,
        SGC_TRACE_5,
        SGC_TRACE_6,
        SGC_TRACE_7,
        SGC_TRACE_LAST
};

typedef void (*sgc_log_callback)(int, int, char*);

void sgc_add_log_listener(int logtype, int activity, sgc_log_callback write);
void sgc_remove_log_listener(int logtype, int activity, sgc_log_callback write);



/*

  int sgc_object_id(void* ptr);

  void* sgc_new_proxy(int id);
  int sgc_is_proxy(void* ptr);

  void sgc_set_local_id(int local_id);
  void sgc_register_remote_gc(int remote_id);
  void sgc_unregister_remote_gc(int remote_id);
  void sgc_mark_objects(int remote_id, void* atom_ids);

*/


#ifdef __cplusplus
}
#endif

#endif /* _SGC_H */
