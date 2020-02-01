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

#ifndef _SGC_PRIV_H
#define _SGC_PRIV_H

#include "sgc_types.h"


void sgc_scan_mem(sgc_mem_t* mem);

void sgc_from_list_remove(sgc_mem_t* mem);
void sgc_new_list_append(sgc_mem_t* mem);
void sgc_scan_list_append(sgc_mem_t* mem);


#endif /* _SGC_PRIV_H */
