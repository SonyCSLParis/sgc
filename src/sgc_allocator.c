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

#include "sgc_allocator.h"
#include "sgc_sys.h"
#include "sgc_mem_heap.h"
#include "sgc_var_heap.h"
#include "sgc_area_heap.h"
#include "sgc_interval.h"


#if USE_LITTLEMEMORY
#define SGC_AREA_SIZE               (8 * 1024)
#define SGC_AREA_PREALLOC           4
#define SGC_MAX_MEMHEAP_SIZE_INDEX  9
#else
#define SGC_AREA_SIZE               (64 * 1024)
#define SGC_AREA_PREALLOC           32
#define SGC_MAX_MEMHEAP_SIZE_INDEX  11
#endif

#define SGC_SIZE_INDEX_COUNT        29

uint32 sgc_mem_sizes[] = {
	4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536,	
	131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216,
	33554432, 67108864, 134217728, 268435456, 536870912, 1073741824
};


static sgc_mem_heap_t sgc_heaps[SGC_MAX_MEMHEAP_SIZE_INDEX];
static sgc_mem_t* sgc_free_list[SGC_MAX_MEMHEAP_SIZE_INDEX];
static sgc_var_heap_t sgc_var_heap;

static int sgc_get_size_index(int nbytes);

int sgc_allocator_init()
{
	int i;

	if (sgc_area_heap_init(SGC_AREA_SIZE, SGC_AREA_PREALLOC, 0) != 0) 
		return -1;

	for (i = 0; i < SGC_MAX_MEMHEAP_SIZE_INDEX; i++) 
		sgc_free_list[i] = NULL;

	for (i = 0; i < SGC_MAX_MEMHEAP_SIZE_INDEX; i++)
                sgc_mem_heap_init(&sgc_heaps[i], i, sgc_mem_sizes[i]);

        sgc_var_heap_init(&sgc_var_heap);
	
	return 0;
}

void sgc_allocator_cleanup()
{
	int i;

	for (i = 0; i < SGC_MAX_MEMHEAP_SIZE_INDEX; i++) 
		sgc_mem_heap_destroy(&sgc_heaps[i]);
	
        sgc_var_heap_destroy(&sgc_var_heap);

	sgc_area_heap_cleanup();
}

sgc_mem_t *sgc_allocator_alloc(uint32 size)
{
	sgc_mem_t *mem = 0;
	int index;

	index = sgc_get_size_index(size);

        if (index < SGC_MAX_MEMHEAP_SIZE_INDEX) {

                if (sgc_free_list[index] == NULL) {
                        sgc_free_list[index] = sgc_mem_heap_expand(&sgc_heaps[index]);
                        if (sgc_free_list[index] == NULL) 
                                return NULL;
                }

                mem = sgc_free_list[index];
                sgc_free_list[index] = (sgc_mem_t*) mem->list.next;
                mem->list.next = NULL;

        } else {
                mem = (sgc_mem_t*) sgc_var_heap_alloc(&sgc_var_heap, index,
                                                      sgc_mem_sizes[index]);
        }

	return mem;
}

void sgc_allocator_free(sgc_mem_t *mem)
{
        if (mem->size < SGC_MAX_MEMHEAP_SIZE_INDEX) {
                mem->list.next = (void*) sgc_free_list[mem->size];
                sgc_free_list[mem->size] = mem;
        } else {
                sgc_var_heap_free(&sgc_var_heap, mem);                
        }
}

int sgc_allocator_is_valid(sgc_mem_t *mem)
{
	sgc_ptr_t ptr = (sgc_ptr_t) mem;

	/* Check for a valid size */
	if (mem->size > SGC_SIZE_INDEX_COUNT) 
		return 0;

	/* Check for a valid color */
	if (mem->color >= SGC_COLORMAX) 
		return 0;

	/* Check for a valid flags */
	if (mem->color >= SGC_FLAGSMAX) 
		return 0;

	/* Check for a valid dummy */
	if (mem->dummy != SGC_DUMMY_VALUE) 
		return 0;

	if (sgc_inside_interval(ptr) == 0) 
		return 0;

	if (mem->size >= SGC_MAX_MEMHEAP_SIZE_INDEX) 
                return sgc_var_heap_valid(&sgc_var_heap, mem);
                        
	return sgc_mem_heap_valid(&sgc_heaps[mem->size], mem);
}

uint32 sgc_allocator_mem_size(uint8 size_index)
{
	return sgc_mem_sizes[size_index];
}

int sgc_get_size_index(int nbytes)
{
	int mem_size = 4;
	int size_index = 0;

	while (mem_size < nbytes) {
		size_index++;
		mem_size *= 2;
	}

	return size_index;
}

uint32 sgc_allocator_byte_size(sgc_mem_t *mem)
{
	return sgc_mem_sizes[mem->size];
}
