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

#ifndef _SGC_LOG_H
#define _SGC_LOG_H

/**********************************************************
 *
 * Logging
 *
 */

enum {
        SGC_DBG_0 = 0,
        SGC_DBG_1,
        SGC_DBG_2,
        SGC_DBG_3,
        SGC_DBG_4,
        SGC_DBG_5,
        SGC_DBG_6,
        SGC_DBG_7,
        SGC_DBG_LAST
};

enum {
        SGC_ERR_PANIC = 0,
        SGC_ERR_ERROR,
        SGC_ERR_WARN,
        SGC_ERR_LAST
};

enum {
        SGC_PROF_0 = 0,
        SGC_PROF_1,
        SGC_PROF_2,
        SGC_PROF_3,
        SGC_PROF_4,
        SGC_PROF_5,
        SGC_PROF_6,
        SGC_PROF_7,
        SGC_PROF_LAST
};

int sgc_log_init(void);
void sgc_log_cleanup(void);

void sgc_log(char* file, int line, int type, int filter, char* fmt, ...);

void sgc_trace(int type, int filter, char* s);
void sgc_close_stream(void);


#define sgc_panic(_format, _args...)  \
  sgc_log(__FILE__, __LINE__, SGC_LOG_ERR, SGC_ERR_PANIC, _format, ## _args)

#define sgc_err(_format, _args...)    \
  sgc_log(__FILE__, __LINE__, SGC_LOG_ERR, SGC_ERR_ERROR, _format, ## _args)

#define sgc_warn(_format, _args...)   \
  sgc_log(__FILE__, __LINE__, SGC_LOG_ERR, SGC_ERR_WARN, _format, ## _args)



#ifdef DEBUG
#define sgc_dbg(_format, _args...)    \
  sgc_log(__FILE__, __LINE__, SGC_LOG_DBG, SGC_DBG_0, _format, ## _args)

#define sgc_trace_alloc(_mem, _num)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "alloc\t%p\t%d\t%d", _mem, (_mem)->size, _num)

#define sgc_trace_free(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "free\t%p\t%d", _mem, (_mem)->size)

#define sgc_trace_expand(_size_index, _nobj, _mem_size, _obj_size)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "expand\t%d\t%d\t%d\t%d", \
	  _size_index, _nobj, _mem_size, _obj_size)

#define sgc_trace_area(_size)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "area\t%d", _size)

#define sgc_trace_areablob(_size)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "areablob\t%d", _size)

#define sgc_trace_memblob(_size)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "memblob\t%d", _size)

#define sgc_trace_sweep(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "sweep\t%p\t%d", _mem, (_mem)->size)

#define sgc_trace_root(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "root\t%p\t%d", _mem, (_mem)->size)

#define sgc_trace_stack(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "stack\t%p\t%d", _mem, (_mem)->size)

#define sgc_trace_reached(_mem, _ref)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, \
	  "reached\t%p\t%d\t%p", _mem, _mem->size, _ref)

#define sgc_trace_inner(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "inner\t%p", _mem)

#define sgc_trace_start()    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "starting")

#define sgc_trace_clearing()    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "clearing")

#define sgc_trace_cleared(_count)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "cleared\t%d", _count)

#define sgc_trace_marking()    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "marking")

#define sgc_trace_sweeping()    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "sweeping")

#define sgc_trace_finished()    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "finished")

#define sgc_trace_waiting(_th)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "waiting")

#define sgc_trace_stackstart(_th)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "stackstart")

#define sgc_trace_stackend(_th)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "stackstart")

#define sgc_trace_sync(_th)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "sync")

#define sgc_trace_cont(_th)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_GC, "cont")

#define sgc_trace_memerr(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "memerr\t%p", _mem)

#define sgc_trace_leaked(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "leaked\t%p", _mem)

#define sgc_trace_freed(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "freed\t%p", _mem)

#define sgc_trace_leaked_total(_count,_size)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "leaked_total\t%d\t%d", _count, _size)

#define sgc_trace_freed_total(_count,_size)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_SCAN, "freed_total\t%d\t%d", _count, _size)

#define sgc_trace_rcvcti(_mem1, _mem2)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "rcv_cti\t%p\t%p", _mem1, _mem2)

#define sgc_trace_sndcti(_mem1, _mem2)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "snd_cti\t%p\t%p", _mem1, _mem2)

#define sgc_trace_rcvnew(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "rcv_new\t%p", _mem)

#define sgc_trace_sndnew(_mem)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "snd_new\t%p", _mem)

#define sgc_trace_rcvfree(_mem1)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "rcv_free\t%p", _mem1)

#define sgc_trace_sndfree(_mem1)    \
  sgc_log(0, 0, SGC_LOG_TRACE, SGC_TRACE_ALLOC, "snd_free\t%p", _mem1)


#else
#define sgc_dbg(_format, _args...)   
#define sgc_trace_alloc(_mem, _num)    
#define sgc_trace_free(_mem)    
#define sgc_trace_sweep(_mem)   
#define sgc_trace_memblob(_size)
#define sgc_trace_expand(_size_index, _nobj, _mem_size, _obj_size) 
#define sgc_trace_areablob(_size)  
#define sgc_trace_area(_size)
#define sgc_trace_root(_mem)   
#define sgc_trace_stack(_mem)   
#define sgc_trace_reached(_mem, _ref)   
#define sgc_trace_inner(_mem)   
#define sgc_trace_start()   
#define sgc_trace_clearing()   
#define sgc_trace_cleared(_count)   
#define sgc_trace_marking()   
#define sgc_trace_sweeping()   
#define sgc_trace_finished()   
#define sgc_trace_waiting(_th)   
#define sgc_trace_stackstart(_th)   
#define sgc_trace_stackend(_th)   
#define sgc_trace_sync(_th)   
#define sgc_trace_cont(_th)   
#define sgc_trace_memerr(_mem)   
#define sgc_trace_leaked(_mem)   
#define sgc_trace_freed(_mem)   
#define sgc_trace_leaked_total(_count,_size)   
#define sgc_trace_freed_total(_count,_size)   
#define sgc_trace_rcvcti(_mem1, _mem2)   
#define sgc_trace_sndcti(_mem1, _mem2)   
#define sgc_trace_rcvnew(_mem)   
#define sgc_trace_sndnew(_mem)   
#define sgc_trace_rcvfree(_mem1)   
#define sgc_trace_sndfree(_mem1)   

#endif

#endif /* _SGC_LOG_H */
