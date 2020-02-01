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

#ifndef _SGC_AREA_HEAP_H
#define _SGC_AREA_HEAP_H

#include "sgc.h"
#include "sgc_types.h"

/*

     Area heap

 */

typedef struct _sgc_area_t sgc_area_t;

struct _sgc_area_t
{
	sgc_area_t* next;
	char data[SGC_MEM_DATA_SIZE];	
};

int sgc_area_heap_init(uint32 area_size, uint32 prealloc, uint32 max);
void sgc_area_heap_cleanup(void);

sgc_area_t* sgc_area_heap_alloc(void);
void sgc_area_heap_free(sgc_area_t* area);

uint32 sgc_area_heap_area_size(void);

#endif /* _SGC_AREA_HEAP_H */
