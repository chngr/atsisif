#ifndef __HEAP_H
#define __HEAP_H

typedef struct Heap
{
  int size;
  int *keys;
  int *vals;
} Heap;


int heap_init(Heap*, int);
void heap_destroy(Heap*);
void heap_push(Heap*, int, int);
void heap_pop(Heap*, int*, int*);

#endif
