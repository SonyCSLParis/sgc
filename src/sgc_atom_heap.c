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
#include "sgc_atom_heap.h"
#include "sgc_sys.h"


/**************************************************************************
 *
 *   atom heap
 *
 */

#define SGC_BLOCK_SIZE     2048

typedef struct _sgc_atom_block_t sgc_atom_block_t;

struct _sgc_atom_block_t
{
  sgc_atom_block_t *next;        /* the next free block */
  sgc_atom_t *first;
  sgc_atom_t *last;
  sgc_atom_t a[SGC_BLOCK_SIZE];
};

struct _sgc_atom_heap_t
{
  pthread_mutex_t mutex;
  sgc_atom_block_t *block; 
  int index;
};

int sgc_atom_heap_expand(sgc_atom_heap_t *heap);



sgc_atom_heap_t *sgc_new_atom_heap(void)
{
  sgc_atom_heap_t *heap;

  heap = sgc_new(sgc_atom_heap_t);
  if (heap == NULL) {
    sgc_panic("Couldn't allocate the atom heap");
    return NULL;
  }

  pthread_mutex_init(&heap->mutex, NULL);

  heap->block = NULL;
  heap->index = 0;

  /* preallocate one block */
  if (sgc_atom_heap_expand(heap) != 0) {
    sgc_panic("Couldn't allocate the atom heap");
    return NULL;
  }

  return heap;
}

void sgc_delete_atom_heap(sgc_atom_heap_t *heap)
{
  sgc_atom_block_t *block; 
  sgc_atom_block_t *next; 

  if (heap == NULL) {
    return;
  }

  block = heap->block;

  while (block) {
    next = block->next;
    sgc_free(block);
    block = next;
  }
    
  pthread_mutex_destroy(&heap->mutex);

  sgc_free(heap);
}

int sgc_atom_heap_expand(sgc_atom_heap_t *heap)
{
  sgc_atom_block_t *block; 
  int i;

  block = sgc_new(sgc_atom_block_t);
  if (block == NULL) {
    sgc_panic("Couldn't expand the atom heap");
    return -1;
  }

  block->next = heap->block;
  heap->block = block;
  heap->index = 0;

  block->first = &block->a[0];
  block->last = &block->a[SGC_BLOCK_SIZE - 1];
  
  return 0;
}

sgc_atom_t *sgc_atom_heap_alloc(sgc_atom_heap_t *heap)
{
  sgc_atom_t *a = NULL;

  pthread_mutex_lock(&heap->mutex);

  if ((heap->index == SGC_BLOCK_SIZE) && !sgc_atom_heap_expand(heap)) {
    sgc_panic("Out of memory");
    return NULL;
  }

  a = &(heap->block->a[heap->index++]);

  pthread_mutex_unlock(&heap->mutex);  

  return a;
}

int sgc_atom_heap_is_valid_atom(sgc_atom_heap_t *heap, sgc_atom_t *atom)
{
  sgc_atom_block_t *block = heap->block;

  while (block) {
    if ((block->first <= atom) && (atom <= block->last)) {
      return 1;
    }
    block = block->next;
  }

  return 0;
}
