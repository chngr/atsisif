#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

#define LEFT(i) 2*i+1
#define RIGHT(i) 2*(i+1)
#define PARENT(i) (i - 1)/2
#define SWAP(x,y) { int _tmp = x; x = y; y = _tmp; }
#define SWAPA(i,j,xs) SWAP(xs[i], xs[j])
#define SWAPH(i,j,H) SWAPA(i,j,H->keys); SWAPA(i,j,H->vals)

int heap_init(Heap *H, int size);
void heap_bubble_up(Heap *H, int i);
void heap_destroy(Heap *H);
void heap_pop(Heap *H, int *key, int *val);
void heap_push(Heap *H, int key, int val);

int heap_init(Heap *H, int size)
{
  int rval = 0;
  if(H) {
    H->size = 0;
    H->keys = malloc (size * sizeof(int));
    H->vals = malloc (size * sizeof(int));
    if(!H->keys || !H->vals) {
      rval = 1;
      fprintf(stderr, "not enough memory for heap\n");
    }
  } else {
    rval = 1;
    fprintf(stderr, "heap pointer is null\n");
  }
  return rval;
}

void heap_destroy(Heap *H)
{
  if(H) {
    if(H->keys) free(H->keys);
    if(H->vals) free(H->vals);
  }
}

void heap_push(Heap *H, int key, int val)
{
  if(H) {
    H->keys[H->size] = key; H->vals[H->size] = val;
    heap_bubble_up(H, H->size);
    H->size++;
  }
}

void heap_pop(Heap *H, int *key, int *val)
{
  if(H && H->size > 0) {
    int i = 0, j;
    *key = H->keys[0]; *val = H->vals[0]; H->size--;
    while(RIGHT(i) < H->size) {
      j = H->keys[LEFT(i)] <= H->keys[RIGHT(i)] ? LEFT(i) : RIGHT(i);
      H->keys[i] = H->keys[j];
      H->vals[i] = H->vals[j];
      i = j;
    }
    if(LEFT(i) < H->size) {
      j = LEFT(i);
      H->keys[i] = H->keys[j];
      H->vals[i] = H->vals[j];
      i = j;
    }
    H->keys[i] = H->keys[H->size];
    H->vals[i] = H->vals[H->size];
    heap_bubble_up(H,i);
  } else {
    key = NULL; val = NULL;
  }
}

void heap_bubble_up(Heap *H, int i)
{
  if(H) {
    int j;
    while(i) {
      j = PARENT(i);
      if(H->keys[i] < H->keys[j]) {
        SWAPH(i,j,H);
        i = j;
      } else { break; }
    }
  }
}
