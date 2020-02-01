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

#ifndef _SGC_ALLOCATOR_H
#define _SGC_ALLOCATOR_H

#include "sgc.h"
#include "sgc_types.h"

int sgc_allocator_init(void);
void sgc_allocator_cleanup(void);

sgc_mem_t* sgc_allocator_alloc(uint32 size);
void sgc_allocator_free(sgc_mem_t* mem);
int sgc_allocator_is_valid(sgc_mem_t* mem);
uint32 sgc_allocator_byte_size(sgc_mem_t* mem);
uint32 sgc_allocator_mem_size(uint8 size_index);

#endif /* _SGC_ALLOCATOR_H */
