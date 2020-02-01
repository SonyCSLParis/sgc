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

#ifndef _SGC_FIFO_H
#define _SGC_FIFO_H

#include "sgc_lifo.h"


typedef struct sgc_fifo_t {
        sgc_lifo_t in;
        sgc_lifo_t out;       
} sgc_fifo_t;

static inline unsigned long sgc_fifo_size (sgc_fifo_t* ff) 
{
        return sgc_lifo_size(&ff->in) + sgc_lifo_size(&ff->out);
}

static inline void sgc_fifo_init(sgc_fifo_t* ff)
{
        sgc_lifo_init(&ff->in);                
        sgc_lifo_init(&ff->out);       
}

static inline void sgc_fifo_put(sgc_fifo_t* ff, sgc_cell_t * cl) 
{
        sgc_lifo_push(&ff->in, cl);
}

static inline sgc_cell_t* sgc_fifo_get(sgc_fifo_t* ff) 
{
        sgc_cell_t * v1, * v2;
        sgc_lifo_t * in = &ff->in;
        sgc_lifo_t * out = &ff->out;
        
        v1 = sgc_lifo_pop(out);
        
        if (!v1) {
                v1 = sgc_lifo_pop(in);
                if (v1) {
                        while ((v2 = sgc_lifo_pop(in))) { 
                                sgc_lifo_push(out, v1); 
                                v1 = v2;
                        }
                }
        }
        return v1;
}

static inline sgc_cell_t* sgc_fifo_avail(sgc_fifo_t* ff) 
{
        sgc_cell_t * v1, * v2;
        sgc_lifo_t * in = &ff->in;
        sgc_lifo_t * out = &ff->out;
        
        v1 = sgc_lifo_avail(out);
        
        if (v1) {
                return v1;
        } else {
                while ((v2 = sgc_lifo_pop(in))) { 
			sgc_lifo_push(out, v2); 
		}
                return sgc_lifo_avail(out);
        }
}


static inline sgc_cell_t* sgc_fifo_clear(sgc_fifo_t* ff) 
{
        sgc_cell_t* next, *cur;
        sgc_cell_t* first = sgc_fifo_get(ff);
        
        if (first==0) 
		return 0;
        
        cur = first;
        
        while ((next = sgc_fifo_get(ff))) {
                cur->link = next;
                cur = next;
        }
        cur->link = 0;
        
        sgc_fifo_init (ff);
        return first;
}

#endif /* _SGC_FIFO_H */
