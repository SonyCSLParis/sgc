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
/*  -*- linux-c -*- */

#include "sgc_dlist.h"

sgc_dlist_t *sgc_dlist_append(sgc_dlist_t *list, sgc_dlist_t *node)
{
	if (list == NULL) {
		node->next = node;
		node->prev = node;
		return node;
	}

	if ((list->next == NULL)
	    || (list->prev == NULL)) {
		//sgc_panic(" *** sgc_dlist_append: invalid list ***\n"); // FIXME
	}

	node->next = list;
	node->prev = list->prev;

	list->prev->next = node;
	list->prev = node;

	return list;
}

/* sgc_dlist_t *sgc_dlist_remove(sgc_dlist_t *list, sgc_dlist_t *node) */
/* { */
/* FIXME FIXME FIXME FIXME FIXME: this is wrong!  */
/* 	sgc_dlist_t *cur = list; */

/* 	while (cur) { */
		
/* 		if (cur == node) { */
/* 			return sgc_dlist_remove_node(cur); */
/* 		} */

/* 		cur = sgc_dlist_next(cur); */
/* 		if (cur == sgc_dlist_first(list)) { */
/* 			break; */
/* 		} */
/* 	} */

/* 	return list; */
/* } */

sgc_dlist_t *sgc_dlist_remove_node(sgc_dlist_t *list)
{
	sgc_dlist_t *next = NULL;

	if (list == NULL) {
		return NULL;
	}
	if (list->next == list) {
		return NULL;
	}

	next = list->next;
	list->prev->next = list->next;
	list->next->prev = list->prev;

	list->next = 0;
	list->prev = 0;

	return next;
}

sgc_dlist_t *sgc_dlist_join(sgc_dlist_t *list1, sgc_dlist_t *list2)
{
	sgc_dlist_t *list2_prev;

	if (list2 == NULL) {
		return list1;
	}
	if (list1 == NULL) {
		return list2;
	}

	if ((list1->next == NULL)
	    || (list1->prev == NULL)) {
		//sgc_panic(" *** sgc_dlist_join: invalid list1 ***\n"); // FIXME
	}

	if ((list2->next == NULL)
	    || (list2->prev == NULL)) {
		//sgc_panic(" *** sgc_dlist_append: invalid list2 ***\n"); // FIXME
	}

	list2_prev = list2->prev;

	list2->prev = list1->prev;
	list2->prev->next = list2;

	list1->prev = list2_prev;
	list2_prev->next = list1;

	return list1;
}

