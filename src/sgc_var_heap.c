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

#include "sgc_var_heap.h"
#include "sgc_sys.h"
#include "sgc_priv.h"
#include "sgc_dlist.h"
#include "sgc_interval.h"

/**************************************************************************
 *
 *   Mem heap
 *
 */

int sgc_var_heap_init(sgc_var_heap_t *heap)
{
	heap->free_list = NULL;
	return 0;
}

void sgc_var_heap_destroy(sgc_var_heap_t *heap)
{
#if !USE_SBRK
	while (heap->free_list) {
                sgc_mem_t *mem = (sgc_mem_t *) heap->free_list;
                heap->free_list = sgc_dlist_remove_node(heap->free_list);
		sgc_free(mem);
	}
#endif
}

sgc_mem_t *sgc_var_heap_alloc(sgc_var_heap_t *heap, uint8 index, uint32 size)
{
	uint32 mem_size;
        sgc_mem_t *mem;

        if (heap->free_list) {
                sgc_dlist_t *cur = heap->free_list;
                sgc_dlist_t *last = sgc_dlist_prev(heap->free_list);
                while (1) {
                        mem = (sgc_mem_t *) cur;
                        if (mem->size == index) {
                                heap->free_list = sgc_dlist_remove_node(cur);
                                return mem;
                        }
                        
                        if (cur == last) 
                                break;
                        
                        cur = sgc_dlist_next(cur);
                } 
        }

        mem_size = sizeof(sgc_mem_t) - SGC_MEM_DATA_SIZE + size;

	mem = (sgc_mem_t*) sgc_sbrk(mem_size);
	if (mem == NULL) {
		sgc_panic("Out of memory");
		return NULL;
	}

	sgc_register_interval((sgc_ptr_t) mem, mem_size);

        mem->list.next = NULL;
        mem->list.prev = NULL;
	mem->size = index;

	sgc_trace_memblob(mem_size);
	sgc_trace_expand(index, 1, mem_size, size);

	return mem;
}

void sgc_var_heap_free(sgc_var_heap_t *heap, sgc_mem_t *mem)
{
#if USE_SBRK
	heap->free_list = sgc_dlist_append(heap->free_list, (sgc_dlist_t*) mem);
#else
        sgc_free(mem);
#endif
}

int sgc_var_heap_valid(sgc_var_heap_t *heap, sgc_mem_t *mem)
{
        // FIMXE!!!
	return 1;
}
