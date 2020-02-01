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

#include "sgc_mem_heap.h"
#include "sgc_sys.h"
#include "sgc_priv.h"
#include "sgc_list.h"
#include "sgc_area_heap.h"


/**************************************************************************
 *
 *   Mem heap
 *
 */

int sgc_mem_heap_init(sgc_mem_heap_t *heap, uint8 size_index, uint32 mem_size)
{
	uint32 nmems;

	heap->obj_size = mem_size;

	mem_size += sizeof(sgc_mem_t) - SGC_MEM_DATA_SIZE;

	if (mem_size % SGC_MEM_ALIGN)
		mem_size += SGC_MEM_ALIGN - (mem_size % SGC_MEM_ALIGN);

	nmems = sgc_area_heap_area_size() / mem_size;

	heap->mem_size = mem_size;
	heap->area_size = nmems * mem_size;
	heap->num_mems = nmems;
	heap->mem_areas = NULL;
	heap->size_index = size_index;

	return 0;
}

void sgc_mem_heap_destroy(sgc_mem_heap_t *heap)
{
	sgc_area_t *area, *next;

	area = heap->mem_areas;
	while (area) {
		next = area->next;
		sgc_area_heap_free(area);
		area = next;
	}
}

sgc_mem_t *sgc_mem_heap_expand(sgc_mem_heap_t *heap)
{
	char *p;
	sgc_area_t *area;
	sgc_mem_t *mem;
	uint32 i;
	sgc_mem_t *list = 0;

	area = sgc_area_heap_alloc();
	if (area == NULL)
		return NULL;

	area->next = heap->mem_areas;
	heap->mem_areas = area;

	p = &area->data[0];

	for (i = 0; i < heap->num_mems; i++) {

		mem = (sgc_mem_t*) p;
		mem->list.next = (sgc_dlist_t*) list;
		mem->list.prev = NULL;
		mem->size = heap->size_index;
                list = mem;

		p += heap->mem_size;
	}

	sgc_trace_expand(heap->size_index, heap->num_mems, heap->mem_size, heap->obj_size);

	return list;
}

int sgc_mem_heap_valid(sgc_mem_heap_t *heap, sgc_mem_t *mem)
{
	sgc_area_t *mem_area;
	char *p0, *p1, *p;

	p = (char*) mem;
	mem_area = heap->mem_areas;

	while (mem_area) {

		p0 = (char*) &mem_area->data[0];
		p1 = p0 + heap->area_size;

		if ((p0 <= p) && (p < p1)) { 
			if ((p - p0) % heap->mem_size == 0)
				return 1;
			else
				sgc_trace_inner(mem);
		}

		mem_area = mem_area->next;
	}

	return 0;
}
