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

#include "sgc_area_heap.h"
#include "sgc_sys.h"
#include "sgc_priv.h"
#include "sgc_interval.h"

typedef struct _sgc_area_heap_t
{
	sgc_area_t* free;
	uint32 area_size;
	uint32 max_areas;
	uint32 num_areas;
	uint32 num_free_areas;
} sgc_area_heap_t;

static sgc_area_heap_t _sgc_area_heap;
static sgc_area_heap_t* sgc_area_heap = NULL;

int sgc_area_heap_expand(void);

int sgc_area_heap_init(uint32 area_size, uint32 prealloc, uint32 max)
{
	sgc_area_heap = &_sgc_area_heap;
	if (sgc_area_heap == NULL) {
		sgc_panic("Out of memory");
		return -1;
	}

	sgc_area_heap->free = NULL;
	sgc_area_heap->area_size = area_size;
	sgc_area_heap->max_areas = max;
	sgc_area_heap->num_areas = 0;
	sgc_area_heap->num_free_areas = 0;

	while (prealloc-- > 0) {
		if (sgc_area_heap_expand() != 0) {
			return -1;
		}
	}

	return 0;
}

void sgc_area_heap_cleanup()
{
}

int sgc_area_heap_expand()
{
        if ((sgc_area_heap->max_areas > 0)
            && (sgc_area_heap->num_areas >= sgc_area_heap->max_areas)) {
		sgc_panic("Out of memory");
		return -1;
        }

	sgc_area_t *area = (sgc_area_t*) sgc_sbrk(sgc_area_heap->area_size);
	if (area == NULL) {
		sgc_panic("Out of memory");
		return -1;
	}

	sgc_register_interval((sgc_ptr_t) area, sgc_area_heap->area_size);
	
        area->next = sgc_area_heap->free;
        sgc_area_heap->free = area;

	sgc_area_heap->num_areas++;
	sgc_area_heap->num_free_areas++;	
	sgc_trace_areablob(sgc_area_heap->area_size);

	return 0;
}

sgc_area_t *sgc_area_heap_alloc()
{
	sgc_area_t* area;

	if ((sgc_area_heap->free == NULL) 
            && (sgc_area_heap_expand() != 0)) 
                return NULL;
        
	area = sgc_area_heap->free;
        sgc_area_heap->free = area->next;
        
	sgc_area_heap->num_free_areas--;
	sgc_trace_area(sgc_area_heap->area_size);
        
	return area;
}

void sgc_area_heap_free(sgc_area_t *area)
{
        area->next = sgc_area_heap->free;
        sgc_area_heap->free = area;
	sgc_area_heap->num_free_areas++;
}

uint32 sgc_area_heap_area_size()
{
	return sgc_area_heap->area_size - sizeof(sgc_area_t) + SGC_MEM_DATA_SIZE;
}
