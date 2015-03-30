#include <stdio.h>
#include <stdlib.h>
#include "comb.h"
#include "graph.h"

int valid_comb (comb *C);
int violating_comb (comb *C);
int comps_to_combs(graph *C, int ncomps, int *comps, int *ncombs, comb ***p_clist, double t_thresh);
void destroy_comb(comb *C);

int comps_to_combs(graph *G, int ncomps, int *comps, int *ncombs, comb ***p_clist, double t_thresh)
{
  int rval = 0, i, j, k, l, m, nteeth;
  node node;
  comb **clist = NULL;
  int *c_sizes = malloc (ncomps * sizeof (int)),
      *tmp_ts  = malloc (G->ecount * sizeof (int));
  if (!c_sizes || !tmp_ts) {
    fprintf(stderr, "not enough memory for c_sizes or tmp_ts\n");
    rval = 1; goto CLEANUP;
  }

  *ncombs = 0;
  comp_sizes(G->ncount, ncomps, comps, c_sizes);
  for (i = 0; i < ncomps; i++) {
    if (c_sizes[i] > 2) (*ncombs)++;
  }

  clist = malloc ((*ncombs) * sizeof (comb *));
  if (!clist) {
    fprintf(stderr, "not enough memory for clist\n");
    rval = 1; goto CLEANUP;
  }
  for (i = 0; i < *ncombs; i++) {
    clist[i] = malloc (sizeof (comb));
    if (!clist[i]) {
      fprintf (stderr, "not enough memory for clist[%d]\n", i);
      rval = 1; goto CLEANUP;
    }
  }
  for (i = 0; i < *ncombs; i++) clist[i]->G = G;

  for (i = 0, j = 0; i < ncomps; i++) {
    if (c_sizes[i] < 3) continue;

    nteeth = 0;
    clist[j]->nhandle = c_sizes[i];
    clist[j]->handlenodes = malloc (c_sizes[i] * sizeof (int));
    if (!clist[j]->handlenodes) {
      fprintf(stderr, "not enough memory for clist[%d]->handlenodes\n", j);
      rval = 1; goto CLEANUP;
    }
    for (k = 0, l = 0; k < G->ncount; k++) {
      if (comps[k] == i) {
        clist[j]->handlenodes[l++] = k;

        node = G->nodelist[k];
        for (m = 0; m < node.deg; m++) {
          if (comps[node.adj[m].n] != i && G->ewts[node.adj[m].e] > t_thresh)
            tmp_ts[nteeth++] = node.adj[m].e;
        }
        if (l == c_sizes[i]) break;
      }
    }
    clist[j]->nteeth = nteeth;
    clist[j]->teethedges = malloc (nteeth * sizeof (int));
    if (!clist[j]->teethedges) {
      fprintf(stderr, "not enough memory for clist[%d]->teethedges\n", j);
      rval = 1; goto CLEANUP;
    }
    for (k = 0; k < nteeth; k++) clist[j]->teethedges[k] = tmp_ts[k] ;
    j++;
  }
  *p_clist = clist;
  for (i = 0; i < *ncombs; i++) {
    printf("[comb candidate %d] nhandle: %d nteeth: %d\n", i, clist[i]->nhandle, clist[i]->nteeth);
  }
CLEANUP:
  if (c_sizes) free (c_sizes);
  if (tmp_ts) free (tmp_ts);
  return rval;
}

int valid_comb(comb *C)
{
  int i, j, a, b;
  if (C->nhandle < 3 || C->nteeth < 3) return 0;
  if (2 * (C->nteeth/2) == C->nteeth) return 0;

  /* Check disjointness of teeth */
  for (i = 0; i < C->G->ncount; i++) C->G->nodelist[i].mark = 0;
  for (i = 0, j = 1; i < C->nteeth; i++) {
    a = C->G->elist[2*C->teethedges[i]]; b = C->G->elist[2*C->teethedges[i]+1];
    if (C->G->nodelist[a].mark != 0 || C->G->nodelist[b].mark != 0) return 0;
    C->G->nodelist[a].mark = j; C->G->nodelist[b].mark = j;
  }
  return 1;
}

int violating_comb (comb *C)
{
  int i, j;
  node node;
  double rhs = 3.0 * ((double) C->nteeth) + 1.0,
         lhs = 0.0;

  /* Mark the nodes appearing in the handle */
  for (i = 0; i < C->G->ncount; i++) C->G->nodelist[i].mark = 0;
  for (i = 0; i < C->nhandle; i++) C->G->nodelist[C->handlenodes[i]].mark = 1;

  /* Contributions from handle */
  for (i = 0; i < C->nhandle; i++) {
    node = C->G->nodelist[C->handlenodes[i]];
    for (j = 0; j < node.deg; j++) {
      if (!C->G->nodelist[node.adj[j].n].mark)
        lhs += C->G->ewts[node.adj[j].e];
    }
  }

  /* Contributions from teeth */
  for (i = 0; i < C->nteeth; i++) {
    lhs += 4.0 - 2*C->G->ewts[C->teethedges[i]]; // Using degree constraints
  }
  if (lhs < rhs) printf("lhs: %.2f rhs: %.2f\n", lhs, rhs);
  return lhs < rhs;
}

void destroy_comb (comb *C)
{
  if(C->handlenodes) free (C->handlenodes);
  if(C->teethedges) free (C->teethedges);
}
