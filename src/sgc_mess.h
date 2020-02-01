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

#ifndef _SGC_MESS_H
#define _SGC_MESS_H


#include "sgc.h"
#include "sgc_lifo.h"

struct _sgc_message_t {
	sgc_cell_t cell;
	int message;
	void* arg1;
	void* arg2;
};

int sgc_message_module_init(int count);
int sgc_message_module_cleanup(void);

sgc_message_t* sgc_message_new(int message, void* arg1, void* arg2);
void sgc_message_delete(sgc_message_t* m);
sgc_message_t* sgc_message_get(void);
void sgc_message_put(sgc_message_t* m);


#endif /* _SGC_MESS_H */
