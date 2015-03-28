#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "util.h"

void get_delta (int comp, int *complist, int *deltacount, int *delta, graph *G);
void init_graph (graph *G);
void free_graph (graph *G);
int build_graph (int ncount, int ecount, int *elist, double *x, graph *G);
void get_comps(graph *G, int *comps, int *ncomps, double lower, double upper);
void comp_sizes(int ncount, int ncomps, int *comps, int *comp_sizes);
void dfs (int n, graph *G, int comp, int *comps, double lower, double upper);

void comp_sizes(int ncount, int ncomps, int *comps, int *comp_sizes)
{
  int i, j, n;
  for (i = 0; i < ncomps; i++) {
    n = 0;
    for(j = 0; j < ncount; j++) {
      if(comps[j] == i) n++;
    }
    comp_sizes[i] = n;
  }
}

void get_comps(graph *G, int *comps, int *ncomps, double lower, double upper)
{
  int i;
  *ncomps = 0;
  for (i = 0; i < G->ncount; i++) G->nodelist[i].mark = 0;
  for (i = 0; i < G->ncount; i++) {
    if (G->nodelist[i].mark == 0)
      dfs(i, G, (*ncomps)++, comps, lower, upper);
  }
}

void dfs (int n, graph *G, int comp, int *comps, double lower, double upper)
{
  int i, neighbor;
  node *pn;

  comps[n] = comp;

  pn = &G->nodelist[n];
  pn->mark = 1;

  for (i = 0; i < pn->deg; i++) {
    if (lower < G->ewts[pn->adj[i].e] && G->ewts[pn->adj[i].e] < upper) {
      neighbor = pn->adj[i].n;
      if (G->nodelist[neighbor].mark == 0) {
        dfs (neighbor, G, comp, comps, lower, upper);
      }
    }
  }
}

void init_graph (graph *G)
{
  if (G) {
    G->nodelist = NULL;
    G->elist = NULL;
    G->ewts = NULL;
    G->adjspace = NULL;
    G->ncount = 0;
    G->ecount = 0;
  }
}

void free_graph (graph *G)
{
  if (G) {
    if (G->nodelist) free (G->nodelist);
    if (G->adjspace) free (G->adjspace);
  }
}

int build_graph (int ncount, int ecount, int *elist, double *ewts, graph *G)
{
  int rval = 0, i, a, b;
  node *n;
  adjobj *p;

  G->nodelist = malloc (ncount * sizeof (node));
  G->adjspace = malloc (2 * ecount * sizeof (node));
  if (!G->nodelist || !G->adjspace) {
    fprintf (stderr, "out of memory for nodelist or adjspace\n");
    rval = 1; goto CLEANUP;
  }

  G->elist = elist;
  G->ewts  = ewts;
  for (i = 0; i < ncount; i++) G->nodelist[i].deg = 0;
  for (i = 0; i < ecount; i++) {
    a = elist[2*i];  b = elist[2*i+1];
    G->nodelist[a].deg++;
    G->nodelist[b].deg++;
  }

  p = G->adjspace;
  for (i = 0; i < ncount; i++) {
    G->nodelist[i].adj = p;
    p += G->nodelist[i].deg;
    G->nodelist[i].deg = 0;
  }

  for (i = 0; i < ecount; i++) {
    a = elist[2*i];  b = elist[2*i+1];
    n = &G->nodelist[a];
    n->adj[n->deg].n = b;
    n->adj[n->deg].e = i;
    n->deg++;
    n = &G->nodelist[b];
    n->adj[n->deg].n = a;
    n->adj[n->deg].e = i;
    n->deg++;
  }

  G->ncount = ncount;
  G->ecount = ecount;

CLEANUP:
  return rval;
}

int build_contracted_graph(int ncount, int ecount, int *elist, double *ewts,
    double *y, graph *G)
{
  int rval = 0, i = 0, j = 0, a, b, n_c = 0, n_f = 0, n = 0;
  double *c_ewts = NULL, *f_ewts = NULL;
  int *c_elist = NULL;

  int *nodes = malloc (ncount * sizeof (int)),
      *c_edges = malloc (2 * ecount * sizeof (int)),
      *f_edges = malloc (2 * ecount * sizeof (int));
  if(!c_edges || !f_edges || !nodes) {
    fprintf(stderr, "out of memory for c_edges or f_edges or nodes\n");
    rval = 1; goto CLEANUP;
  }
  c_ewts = malloc (ecount * sizeof (double));
  f_ewts = malloc (ecount * sizeof (double));
  if(!c_ewts || !f_ewts) {
    fprintf(stderr, "out of memory for c_ewts or f_ewts\n");
    rval = 1; goto CLEANUP;
  }

  for(i = 0; i < ncount; i++) nodes[i] = 0;
  for(i = 0; i < ecount; i++) {
    a = elist[2*i]; b = elist[2*i+1];
    if(1 - ewts[i] > LP_EPSILON) {
      for(j = 0; j < n_c; j++) {
        if(a == c_edges[2*j]) {
          c_edges[2*j] = b; break;
        } else if(a == c_edges[2*j+1]) {
          c_edges[2*j+1] = b; break;
        } else if(b == c_edges[2*j]) {
          c_edges[2*j] = a; break;
        } else if(b == c_edges[2*j+1]) {
          c_edges[2*j+1] = a; break;
        }
      }
      if(j == n_c) {
        c_edges[2*n_c] = a; c_edges[2*n_c+1] = b;
        c_ewts[n_c] = ewts[i];
        n_c++;
      }
    } else {
      f_edges[2*n_f] = a; f_edges[2*n_f+1] = b;
      f_ewts[n_f] = ewts[i];
      n_f++;
      if(!nodes[a]) { nodes[a] = 1; n++; }
      if(!nodes[b]) { nodes[b] = 1; n++; }
    }
  }

  /* Find then reindex the nodes used */
  for(i = 0; i < n_c; i++) {
    a = c_edges[2*i]; b = c_edges[2*i+1];
    if(!nodes[a]) { nodes[a] = 1; n++; }
    if(!nodes[b]) { nodes[b] = 1; n++; }
  }
  for(i = 0; i < n_f; i++) {
    a = f_edges[2*i]; b = c_edges[2*i+1];
    if(!nodes[a]) { nodes[a] = 1; n++; }
    if(!nodes[b]) { nodes[b] = 1; n++; }
  }

  for(i = 0, j = 0; i < ncount; i++) {
    if(nodes[i]) { nodes[i] = j++; }
  }

  /* Construct new edges list and edge weight vector */
  y = malloc ((n_c + n_f) * sizeof (double));
  if(!y) {
    fprintf(stderr, "not enough memory for y\n");
    rval = 1; goto CLEANUP;
  }
  c_elist = malloc (2*(n_c + n_f) * sizeof (int));
  if(!c_elist) {
    fprintf(stderr, "not enough memory for c_elist\n");
    rval = 1; goto CLEANUP;
  }
  for(i = 0, j = 0; i < n_c; i++, j++) {
    c_elist[2*j] = nodes[c_edges[2*i]];
    c_elist[2*j+1] = nodes[c_edges[2*i+1]];
    y[j] = c_ewts[i];
  }
  for(i = 0; i < n_f; i++, j++) {
    c_elist[2*j] = nodes[f_edges[2*i]];
    c_elist[2*j+1] = nodes[f_edges[2*i+1]];
    y[j] = f_ewts[i];
  }

  rval = build_graph(n, n_c + n_f, c_elist, y, G);
  if(rval) {
    fprintf(stderr, "build_graph failed\n");
    goto CLEANUP;
  }
CLEANUP:
  if(c_edges) free(c_edges);
  if(f_edges) free(f_edges);
  if(c_ewts) free(c_ewts);
  if(f_ewts) free(f_ewts);
  if(nodes) free(nodes);
  if(c_elist) free(c_elist);
  return rval;
}

void get_delta (int comp, int *complist, int *deltacount, int *delta, graph *G)
{
  int i, k = 0;
  int *elist = G->elist;

  for (i = 0; i < G->ncount; i++) G->nodelist[i].mark = (complist[i] == comp);

  for (i = 0; i < G->ecount; i++) {
    if (G->nodelist[elist[2*i]].mark + G->nodelist[elist[2*i+1]].mark == 1) {
      delta[k++] = i;
    }
  }
  *deltacount = k;

  for (i = 0; i < G->ncount; i++) G->nodelist[i].mark = 0;
}
