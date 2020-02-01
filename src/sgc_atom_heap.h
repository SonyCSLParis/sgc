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

#ifndef _SGC_ATOM_HEAP_H
#define _SGC_ATOM_HEAP_H

#include "sgc.h"
#include "sgc_types.h"

typedef struct _sgc_atom_heap_t sgc_atom_heap_t;

sgc_atom_heap_t* sgc_new_atom_heap(void);
void sgc_delete_atom_heap(sgc_atom_heap_t* heap);

/* Retuns -1 on error, 0 otherwise */
sgc_atom_t* sgc_atom_heap_alloc(sgc_atom_heap_t* heap);


int sgc_atom_heap_is_valid_atom(sgc_atom_heap_t* heap, sgc_atom_t* atom);


#endif /* _SGC_ATOM_HEAP_H */
