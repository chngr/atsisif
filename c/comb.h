#ifndef __COMB_H
#define __COMB_H
#include "graph.h"

typedef struct comb {
  graph *G;         /* Supergraph containing comb */
  int nhandle;
  int nteeth;
  int *handlenodes; /* Indices for nodes handle wrt. G */
  int *teethedges;  /* Indices for edges for teeth */
} comb;

int comps_to_combs(graph*, int, int*, comb*, double);
int valid_comb(comb*);
int violating_comb(comb*);
void destory_comb(comb *);

#endif
