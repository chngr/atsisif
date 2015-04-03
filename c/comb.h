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

double comb_weight(comb *);
int comps_to_combs(graph*, int, int*, int*, comb***);
int equal_combs(comb *, comb *);
int valid_comb(comb *);
void destroy_comb(comb *);
void print_comb(comb *, int *);

#endif
