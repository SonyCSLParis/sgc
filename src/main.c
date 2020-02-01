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
#include <stdlib.h>
#include <stdio.h>

void sgc_scan_mem(sgc_mem_t* mem);

typedef struct _sgc_list_t sgc_list_t;

struct _sgc_list_t {
	void* data;
	sgc_list_t* next;
};

void trace(int type, int filter, char* s)
{
	printf("%s\n", s);
}

int main(int argc, char** argv)
{
	sgc_list_t* l;
	sgc_list_t* list = 0;
	void* ptr;
	pthread_t thread;
	int i;

	sgc_add_log_listener(SGC_LOG_TRACE, SGC_TRACE_ALLOC, trace);
	sgc_add_log_listener(SGC_LOG_TRACE, SGC_TRACE_SCAN, trace);
	sgc_add_log_listener(SGC_LOG_TRACE, SGC_TRACE_GC, trace);

	if (sgc_init(&argc, 1) != 0) {
		printf("Failed to initialize the garbage collector\n");
		exit(-1);
	}

	for (i = 0; i < 1000; i++) {

		l = sgc_new_object(sizeof(sgc_list_t), 0, 0);
		_stf(l, next, list);
		_stf(l, data, NULL);
		_str(list, l);
		
		l = sgc_new_object(sizeof(sgc_list_t), 0, 0);
		_stf(l, next, list);
		_stf(l, data, NULL);
		_str(list, l);
		
		ptr = sgc_new_object(12, 0, 0);
		_stf(l, data, ptr);

		pthread_yield();
		sgc_thread_sync();
		
		_str(list, NULL);
	}

	sgc_cleanup();

	return 0;
}
