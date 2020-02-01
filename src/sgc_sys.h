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

#ifndef _SGC_SYS_H
#define _SGC_SYS_H

#define USE_SBRK 1
#define USE_LOGGING 0
#define USE_LITTLEMEMORY 1

#include <stdlib.h>
#include <string.h>
#include "sgc.h"
#include "sgc_types.h"
#include "sgc_log.h"

/**********************************************************
 *
 * Macros for standard function
 *
 */

/* #if !USE_SBRK */
#define sgc_sbrk(_n)                malloc(_n)
#define sgc_new(_type)              ((_type*)malloc(sizeof(_type)))
#define sgc_free(_p)                free((void*) _p)
#define sgc_memset(_s, _c, _n)      memset(_s, _c, _n) 
#define sgc_memcpy(_dst, _src, _n)  memcpy(_dst, _src, _n)
/* #endif */

/* void* sgc_sbrk(int incr); */
/* void *sgc_memset(void *s, int c, unsigned int n); */
/* void *sgc_memcpy(void *dest, const void *src, unsigned int n); */

/* void* sgc_sbrk(int incr); */
/* void *sgc_memset(void *s, int c, unsigned int n); */
/* void *sgc_memcpy(void *dest, const void *src, unsigned int n); */

#endif /* _SGC_SYS_H */
