#include <stdio.h>
#include <stdlib.h>
#include "comb.h"
#include "graph.h"

double comb_weight(comb *C);
int comps_to_combs(graph *C, int ncomps, int *comps, int *ncombs, comb ***p_clist);
int comb_int(const void *a, const void *b);
int valid_comb (comb *C);
void destroy_comb(comb *C);
void print_comb(comb *C, int *original_indices);

int comp_int(const void *a, const void *b) {
  if(*(int *) a < *(int *)b) return -1;
  if(*(int *) a > *(int *)b) return 1;
  return 0;
}

int comps_to_combs(graph *G, int ncomps, int *comps, int *ncombs,
    comb ***p_clist)
{
  int rval = 0, i, j, k, l, m, nteeth, best_t;
  double best_wt, node_wt;
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

        best_t = -1; best_wt = -1.0; node_wt = 0.0;
        node = G->nodelist[k];
        for (m = 0; m < node.deg; m++) {
          if (comps[node.adj[m].n] != i) {
            node_wt += G->elist[node.adj[m].e].wt;
            if(G->elist[node.adj[m].e].wt > best_wt) {
              best_t = node.adj[m].e;
              best_wt = G->elist[best_t].wt;
            }
          }
        }
        if (best_t != -1 && 1.0 - best_wt < node_wt) tmp_ts[nteeth++] = best_t;
        if (l == c_sizes[i]) break;
      }
    }
    clist[j]->nteeth = nteeth;
    clist[j]->teethedges = malloc (clist[j]->nteeth * sizeof (int));
    if (!clist[j]->teethedges) {
      fprintf(stderr, "not enough memory for clist[%d]->teethedges\n", j);
      rval = 1; goto CLEANUP;
    }
    for (k = 0; k < clist[j]->nteeth; k++) clist[j]->teethedges[k] = tmp_ts[k];
    qsort(clist[j]->handlenodes, clist[j]->nhandle, sizeof(int), comp_int);
    qsort(clist[j]->teethedges, clist[j]->nteeth, sizeof(int), comp_int);
    j++;
  }
  *p_clist = clist;
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
    a = C->G->elist[C->teethedges[i]].end1; b = C->G->elist[C->teethedges[i]].end2;
    if (C->G->nodelist[a].mark != 0 || C->G->nodelist[b].mark != 0) return 0;
    C->G->nodelist[a].mark = j; C->G->nodelist[b].mark = j;
  }
  return 1;
}

double comb_weight (comb *C)
{
  int i, j;
  node node;
  double wt = 0.0;
  /* Mark the nodes appearing in the handle */
  for (i = 0; i < C->G->ncount; i++) C->G->nodelist[i].mark = 0;
  for (i = 0; i < C->nhandle; i++) C->G->nodelist[C->handlenodes[i]].mark = 1;

  /* Contributions from handle */
  for (i = 0; i < C->nhandle; i++) {
    node = C->G->nodelist[C->handlenodes[i]];
    for (j = 0; j < node.deg; j++) {
      if (!C->G->nodelist[node.adj[j].n].mark)
        wt += C->G->elist[node.adj[j].e].wt;
    }
  }
  /* Contributions from teeth */
  for (i = 0; i < C->nteeth; i++)
    wt += 4.0 - 2*C->G->elist[C->teethedges[i]].wt; // Using degree constraints

  return wt;
}

void destroy_comb (comb *C)
{
  if(C->handlenodes) free (C->handlenodes);
  if(C->teethedges) free (C->teethedges);
}

int equal_combs(comb *C, comb *D)
{
  int i;
  if(C->nteeth != D->nteeth || C->nhandle != D->nhandle)
    return 0;
  for (i = 0; i < C->nhandle; i++) {
    if(C->handlenodes[i] != D->handlenodes[i]) return 0;
  }
  for (i = 0; i < C->nteeth; i++) {
    if(C->teethedges[i] != D->teethedges[i]) return 0;
  }
  return 1;
}

void print_comb(comb *C, int *original_indices)
{
  int i, a, b;
  for (i = 0; i < C->G->ncount; i++)
    C->G->nodelist[i].mark = 0;
  for (i = 0; i < C->nhandle; i++)
    C->G->nodelist[C->handlenodes[i]].mark = 1;

  printf("[Handle Nodes]\n");
  for (i = 0; i < C->nhandle; i++) printf("%d ", original_indices[C->handlenodes[i]]);
  printf("\n");

  printf("[Handle Edges]\n");
  for (i = 0; i < C->G->ecount; i++) {
    a = C->G->elist[i].end1; b = C->G->elist[i].end2;
    if (C->G->nodelist[a].mark + C->G->nodelist[b].mark == 2)
      printf("%d %d %.2f\n", original_indices[a], original_indices[b], C->G->elist[i].wt);
  }

  printf("[Teeth]\n");
  for (i = 0; i < C->nteeth; i++) {
    int k = C->teethedges[i];
    printf("%d %d %.6f\n", original_indices[C->G->elist[k].end1], original_indices[C->G->elist[k].end2], C->G->elist[k].wt);
  }
  printf("\n");
}
