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

#ifndef _SGC_DLIST_H
#define _SGC_DLIST_H

#include "sgc.h"

sgc_dlist_t* sgc_dlist_append(sgc_dlist_t* list, sgc_dlist_t* node);
/* sgc_dlist_t* sgc_dlist_remove(sgc_dlist_t* list, sgc_dlist_t* node); */

sgc_dlist_t* sgc_dlist_remove_node(sgc_dlist_t* list);

sgc_dlist_t* sgc_dlist_join(sgc_dlist_t* list, sgc_dlist_t* node);

#define sgc_dlist_next(_list)  ((_list != NULL)? (_list)->next : NULL)
#define sgc_dlist_prev(_list)  ((_list != NULL)? (_list)->prev : NULL)
#define sgc_dlist_last(_list)  ((_list != NULL)? (_list)->prev : NULL)
#define sgc_dlist_first(_list) (_list)


#endif /* _SGC_DLIST_H */

