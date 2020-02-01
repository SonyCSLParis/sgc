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


/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include "sgc_list.h"
#include "sgc_sys.h"

sgc_list_t *new_sgc_list(void)
{
        sgc_list_t *list;
        list = sgc_new(sgc_list_t);
        list->data = NULL;
        list->next = NULL;
        return list;
}

void delete_sgc_list(sgc_list_t *list)
{
        sgc_list_t *next;
        while (list) {
                next = list->next;
                sgc_free(list);
                list = next;
        }
}

void delete1_sgc_list(sgc_list_t *list)
{
        if (list)
                sgc_free(list);
}

sgc_list_t *sgc_list_last(sgc_list_t *list)
{
        if (list) {
                while (list->next)
                        list = list->next;
        }

        return list;
}

sgc_list_t *sgc_list_append(sgc_list_t *list, void* data)
{
        sgc_list_t *new_list;
        sgc_list_t *last;

        new_list = new_sgc_list();
        new_list->data = data;

        if (list) {
                last = sgc_list_last(list);
                /* g_assert (last != NULL); */
                last->next = new_list;
    
                return list;
        } else {
                return new_list;
        }
}

sgc_list_t *sgc_list_prepend(sgc_list_t *list, void* data)
{
        sgc_list_t *new_list;

        new_list = new_sgc_list();
        new_list->data = data;
        new_list->next = list;

        return new_list;
}

sgc_list_t *sgc_list_remove(sgc_list_t *list, void* data)
{
        sgc_list_t *tmp;
        sgc_list_t *prev;

        prev = NULL;
        tmp = list;

        while (tmp) {
                if (tmp->data == data) {
                        if (prev)
                                prev->next = tmp->next;
                        if (list == tmp)
                                list = list->next;
                        tmp->next = NULL;
                        delete_sgc_list(tmp);
      
                        break;
                }
    
                prev = tmp;
                tmp = tmp->next;
        }

        return list;
}

sgc_list_t *sgc_list_remove_link(sgc_list_t *list, sgc_list_t *link)
{
        sgc_list_t *tmp;
        sgc_list_t *prev;

        prev = NULL;
        tmp = list;

        while (tmp) {
                if (tmp == link) {
                        if (prev)
                                prev->next = tmp->next;
                        if (list == tmp)
                                list = list->next;
                        tmp->next = NULL;
                        break;
                }
    
                prev = tmp;
                tmp = tmp->next;
        }
  
        return list;
}
