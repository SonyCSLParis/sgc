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

#ifndef _SGC_MEM_HEAP_H
#define _SGC_MEM_HEAP_H


#include "sgc.h"
#include "sgc_types.h"
#include "sgc_area_heap.h"
#include "sgc_lifo.h"

typedef struct _sgc_mem_heap_t
{
	uint32 obj_size;                   /* the useful size of an mem */
	uint32 mem_size;                   /* the size of an mem, including its header */
	uint32 area_size;                  /* the size of a memory area */
	uint32 num_mems;                   /* the number of mems in the area */
	sgc_area_t* mem_areas;             /* a list of all the mem areas */
	uint8 size_index;
} sgc_mem_heap_t;

int sgc_mem_heap_init(sgc_mem_heap_t* heap, uint8 size_index, uint32 mem_size);
void sgc_mem_heap_destroy(sgc_mem_heap_t* heap);

sgc_mem_t* sgc_mem_heap_expand(sgc_mem_heap_t *heap);
int sgc_mem_heap_valid(sgc_mem_heap_t *heap, sgc_mem_t* mem);

#endif /* _SGC_MEM_HEAP_H */
