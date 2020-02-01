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

#include "sgc_mess.h"
#include "sgc_sys.h"
#include "sgc_lifo.h"
#include "sgc_fifo.h"


#if SGC_USE_THREADS

static sgc_lifo_t sgc_message_free_list;
static sgc_fifo_t sgc_message_queue;

int sgc_message_module_init(int count)
{
	sgc_lifo_init(&sgc_message_free_list);
	sgc_fifo_init(&sgc_message_queue);

	while (count-- > 0) {
		sgc_message_t* m = sgc_new(sgc_message_t);
		if (m == NULL) {
			sgc_panic("Out of memory");
			return -1;
		}
		sgc_lifo_push(&sgc_message_free_list, (sgc_cell_t*) m);
	}
	return 0;
}

int sgc_message_module_cleanup()
{
	sgc_message_t* m;

	m = (sgc_message_t*) sgc_fifo_get(&sgc_message_queue);
	while (m != NULL) {
		sgc_free(m);
		m = (sgc_message_t*) sgc_fifo_get(&sgc_message_queue);
	}

	m = (sgc_message_t*) sgc_lifo_pop(&sgc_message_free_list);
	while (m != NULL) {
		sgc_free(m);
		m = (sgc_message_t*) sgc_lifo_pop(&sgc_message_free_list);
	}

	return 0;
}

sgc_message_t* sgc_message_new(int message, void* arg1, void* arg2)
{
	sgc_message_t* m;

	m = (sgc_message_t*) sgc_lifo_pop(&sgc_message_free_list);
	if (m == NULL) {
		sgc_warn("No more GC messages available, allocating new one");
		m = sgc_new(sgc_message_t);
		if (m == NULL) {
			sgc_panic("Out of memory");
			return NULL;
		}
	}
	
	m->message = message;
	m->arg1 = arg1;
	m->arg2 = arg2;

	return m;
}

void sgc_message_delete(sgc_message_t* m)
{
	sgc_lifo_push(&sgc_message_free_list, (sgc_cell_t*) m);
}

void sgc_message_put(sgc_message_t* m)
{
	sgc_fifo_put(&sgc_message_queue, (sgc_cell_t*) m);	
}

sgc_message_t* sgc_message_get()
{
	return (sgc_message_t*) sgc_fifo_get(&sgc_message_queue);	
}

void sgc_message_send(int message, void* arg1, void* arg2)
{
	sgc_message_t* m;

	m = sgc_message_new(message, arg1, arg2);
	if (m == NULL)
		// FIXME: crash or send error message
		return;

	switch (message) {
	case SGC_NEW_MESSAGE:
		sgc_trace_sndnew(arg1);
		break;
			
	case SGC_CTI_MESSAGE: 
		sgc_trace_sndcti(((arg1)? sgc_ptr_to_mem(arg1) : 0), sgc_ptr_to_mem(arg2));
		break;

	case SGC_FREE_MESSAGE: 
		sgc_trace_sndfree(sgc_ptr_to_mem(arg1));
		break;
	}
	
	sgc_message_put(m);
}

#else



int sgc_message_module_init(int count)
{
	return 0;
}

int sgc_message_module_cleanup()
{
        return 0;
}

sgc_message_t* sgc_message_new(int message, void* arg1, void* arg2)
{
        return NULL;
}

void sgc_message_delete(sgc_message_t* m)
{
}

void sgc_message_put(sgc_message_t* m)
{
}

sgc_message_t* sgc_message_get()
{
	return NULL;
}

void sgc_handle_message(sgc_message_t* m);

void sgc_message_send(int message, void* arg1, void* arg2)
{
        sgc_message_t m;
	m.message = message;
	m.arg1 = arg1;
	m.arg2 = arg2;
        sgc_handle_message(&m); // FIXME: the messages are supposed to be treated in a separate thread
}

#endif


