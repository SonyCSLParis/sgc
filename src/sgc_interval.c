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

#include "sgc_sys.h"
#include "sgc_interval.h"

typedef struct _sgc_interval_t
{
  sgc_ptr_t low;
  sgc_ptr_t high;
} sgc_interval_t;


#if USE_SBRK

static sgc_interval_t interval = { NULL, NULL };

int sgc_register_interval(sgc_ptr_t ptr, int size)
{
        if (interval.low == NULL)
                interval.low = ptr;
        if (interval.high < ptr + size)
                interval.high = ptr + size;
        return 0;
}

int sgc_inside_interval(sgc_ptr_t ptr)
{
	return ((interval.low <= ptr) && (ptr < interval.high));
}

#else

#include "sgc_list.h"
#include "sgc_sys.h"

sgc_interval_t* new_sgc_interval(void);

void sgc_interval_init(sgc_interval_t *interval, 
		       sgc_ptr_t p1, 
		       sgc_ptr_t p2);

int sgc_interval_inside(sgc_interval_t *interval, 
			sgc_ptr_t ptr);

static sgc_list_t* sgc_free_intervals = NULL;        // FIXME: not thread safe
static sgc_list_t* sgc_registered_intervals = NULL;  // FIXME: not thread safe

static int sgc_prealloc_intervals(void)
{
	int i;
	for (i = 0; i < 1000; i++) {
		sgc_interval_t* interval = new_sgc_interval();
		if (interval == NULL)
			return -1;
		sgc_free_intervals = sgc_list_prepend(sgc_free_intervals,
						      interval);
	}
	return 0;
}

int sgc_register_interval(sgc_ptr_t ptr, int size)
{
	sgc_list_t* first_node;
	sgc_interval_t* interval;

	if (sgc_free_intervals == NULL) {
		if (sgc_prealloc_intervals() != 0) {
			sgc_panic("*** sgc_register_interval: Out of memory ***\n");
			return -1;
		}
	}

	first_node = sgc_free_intervals;
	sgc_free_intervals = sgc_list_remove_link(sgc_free_intervals, first_node);

	interval = sgc_list_get(first_node);

	sgc_interval_init(interval, ptr, ptr + size);

	sgc_registered_intervals = sgc_list_prepend(sgc_registered_intervals, 
						    interval);	

	return 0;
}

int sgc_inside_interval(sgc_ptr_t ptr)
{
	sgc_interval_t* interval = NULL;
	sgc_list_t* list = sgc_registered_intervals;
	while (list) {
		interval = (sgc_interval_t*) sgc_list_get(list);
		if (sgc_interval_inside(interval, ptr))
			return 1;
		list = sgc_list_next(list);
	}
	return 0;
}

sgc_interval_t* new_sgc_interval()
{
	sgc_interval_t* interval;
	interval = sgc_new(sgc_interval_t);
	interval->low = NULL;
	interval->high = NULL;

	return interval;
}

void sgc_interval_init(sgc_interval_t *interval,
		       sgc_ptr_t p1, 
		       sgc_ptr_t p2)
{
	if (p1 < p2) {
		interval->low = p1;
		interval->high = p2;
	} else {
		interval->low = p2;
		interval->high = p1;
	}

	//printf("memory interval: low=%p, high=%p\n", p1, p2);
}

int sgc_interval_inside(sgc_interval_t *interval, sgc_ptr_t ptr)
{
	return ((interval->low <= ptr) && (ptr < interval->high));
}

#endif

