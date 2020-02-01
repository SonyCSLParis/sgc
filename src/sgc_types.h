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

#ifndef _SGC_TYPES_H
#define _SGC_TYPES_H

typedef char              sint8;
typedef unsigned char     uint8;
typedef short             sint16;
typedef unsigned short    uint16;
typedef long              sint32;
typedef unsigned long     uint32;

typedef uint32 sgc_word_t;
typedef sgc_word_t* sgc_ptr_t;

#endif /* _SGC_TYPES_H */
