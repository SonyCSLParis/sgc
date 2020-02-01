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

#ifndef _SGC_LIFO_H
#define _SGC_LIFO_H

typedef struct sgc_cell_t {
	struct sgc_cell_t* link;              /*+ next cell in the list       +*/
	                                      /*+ any data here               +*/
} sgc_cell_t;


typedef struct sgc_lifo_t {
        unsigned long n;
        sgc_cell_t* top;                /*+ top of the stack    +*/
} sgc_lifo_t;

static inline void sgc_lifo_init(sgc_lifo_t* lf)
{
	lf->n = 0;
	lf->top = 0;
}

static inline sgc_cell_t* sgc_lifo_avail(sgc_lifo_t* lf) 
{ 
	return (sgc_cell_t*) lf->top;
}

static inline void sgc_lifo_push(sgc_lifo_t* lf, sgc_cell_t * cl)
{
        cl->link = lf->top;
        lf->top = cl;
        lf->n++;
}

static inline sgc_cell_t* sgc_lifo_pop(sgc_lifo_t* lf)
{
	sgc_cell_t* v = lf->top;
        if (lf->top) {
                lf->top = lf->top->link;
                lf->n--;
        }
        return v;
}

static inline unsigned long sgc_lifo_size(sgc_lifo_t* lf)
{
        return lf->n;
}

#endif
