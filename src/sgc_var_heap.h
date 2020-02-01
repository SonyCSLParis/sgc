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

#ifndef _SGC_VAR_HEAP_H
#define _SGC_VAR_HEAP_H


#include "sgc.h"
#include "sgc_types.h"

/*

     Variable size heap

 */

typedef struct _sgc_var_heap_t
{
	sgc_dlist_t* free_list;
} sgc_var_heap_t;

int sgc_var_heap_init(sgc_var_heap_t* heap);
void sgc_var_heap_destroy(sgc_var_heap_t* heap);

sgc_mem_t* sgc_var_heap_alloc(sgc_var_heap_t* heap, uint8 size_index, uint32 obj_size);
void sgc_var_heap_free(sgc_var_heap_t* heap, sgc_mem_t* mem);
int sgc_var_heap_valid(sgc_var_heap_t* heap, sgc_mem_t* mem);

#endif /* _SGC_VAR_HEAP_H */
