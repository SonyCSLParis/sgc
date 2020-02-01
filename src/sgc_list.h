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
#ifndef _SGC_LIST_H
#define _SGC_LIST_H

#include "sgc.h"

typedef struct _sgc_list_t sgc_list_t;

struct _sgc_list_t
{
  void* data;
  sgc_list_t *next;
};

sgc_list_t* new_sgc_list(void);
void delete_sgc_list(sgc_list_t *list);
void delete1_sgc_list(sgc_list_t *list);
sgc_list_t* sgc_list_append(sgc_list_t *list, void* data);
sgc_list_t* sgc_list_prepend(sgc_list_t *list, void* data);
sgc_list_t* sgc_list_remove(sgc_list_t *list, void* data);
sgc_list_t* sgc_list_remove_link(sgc_list_t *list, sgc_list_t *llink);
sgc_list_t* sgc_list_last(sgc_list_t *list);

#define sgc_list_next(slist)	((slist) ? (((sgc_list_t *)(slist))->next) : NULL)
#define sgc_list_get(slist)	((slist) ? ((slist)->data) : NULL)


#endif  /* _SGC_LIST_H */
