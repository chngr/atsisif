#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "util.h"

void get_delta (int comp, int *complist, int *deltacount, int *delta, graph *G);
void init_graph (graph *G);
void free_graph (graph *G);
int build_graph (int ncount, int ecount, edge *elist, graph *G);
int build_contrated_graph(int ncount, int ecount, edge *elist, graph *G);
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
    if (lower <= G->elist[pn->adj[i].e].wt && G->elist[pn->adj[i].e].wt <= upper) {
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
    G->adjspace = NULL;
    G->ncount = 0;
    G->ecount = 0;
  }
}

void free_graph (graph *G)
{
  if (G) {
    if (G->nodelist) free (G->nodelist);
    if (G->elist)    free (G->elist);
    if (G->adjspace) free (G->adjspace);
  }
}

int build_graph (int ncount, int ecount, edge *elist, graph *G)
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

  G->elist = malloc (ecount * sizeof (edge));
  if (!G->elist) {
    fprintf (stderr, "out of memory for elist\n");
    rval = 1; goto CLEANUP;
  }
  G->elist = elist;
  for (i = 0; i < ncount; i++) G->nodelist[i].deg = 0;
  for (i = 0; i < ecount; i++) {
    G->nodelist[elist[i].end1].deg++;
    G->nodelist[elist[i].end2].deg++;
  }

  p = G->adjspace;
  for (i = 0; i < ncount; i++) {
    G->nodelist[i].adj = p;
    p += G->nodelist[i].deg;
    G->nodelist[i].deg = 0;
  }

  for (i = 0; i < ecount; i++) {
    a = elist[i].end1;  b = elist[i].end2;
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

int build_contracted_graph(int ncount, int ecount, edge *elist, graph *G)
{
  int rval = 0, i = 0, j = 0, a, b, n_c = 0, n_f = 0, n = 0, in_path = 0;
  double *c_ewts = NULL, *f_ewts = NULL, w;
  edge *n_elist = NULL;

  int *nodes = malloc (ncount * sizeof (int));
  edge *c_edges = malloc (ecount * sizeof (edge)),
       *f_edges = malloc (ecount * sizeof (edge));
  if(!c_edges || !f_edges || !nodes) {
    fprintf(stderr, "out of memory for c_edges or f_edges or nodes\n");
    rval = 1; goto CLEANUP;
  }

  for(i = 0; i < ncount; i++) nodes[i] = 0;
  for(i = 0; i < ecount; i++) {
    a = elist[i].end1; b = elist[i].end2; w = elist[i].wt;
    if(1.0 - w < LP_EPSILON) {
      in_path = 0;
      for(j = 0; j < n_c; j++) {
        if(a == c_edges[j].end1) {
          c_edges[j].end1 = b; in_path = 1;
        } else if(a == c_edges[j].end2) {
          c_edges[j].end2 = b; in_path = 1;
        } else if(b == c_edges[j].end1) {
          c_edges[j].end1 = a; in_path = 1;
        } else if(b == c_edges[j].end2) {
          c_edges[j].end2 = a; in_path = 1;
        }
        if(in_path) break;
      }
      if(!in_path)
        c_edges[n_c++] = (edge) { .end1 = a, .end2 = b, .wt = w };
    } else {
      f_edges[n_f++] = (edge) { .end1 = a, .end2 = b, .wt = w };
      if(!nodes[a]) { nodes[a] = 1; n++; }
      if(!nodes[b]) { nodes[b] = 1; n++; }
    }
  }

  /* Find then reindex the nodes used */
  for(i = 0; i < n_c; i++) {
    a = c_edges[i].end1; b = c_edges[i].end2;
    if(!nodes[a]) { nodes[a] = 1; n++; }
    if(!nodes[b]) { nodes[b] = 1; n++; }
  }

  for(i = 0, j = 0; i < ncount; i++) {
    if(nodes[i]) { nodes[i] = j++; }
  }

  /* Construct new edges list and edge weight vector */
  n_elist = malloc ((n_c + n_f) * sizeof (edge));
  if(!n_elist) {
    fprintf(stderr, "not enough memory for c_elist\n");
    rval = 1; goto CLEANUP;
  }
  for(i = 0, j = 0; i < n_f; i++, j++) {
    n_elist[j] = (edge) { .end1 = nodes[f_edges[i].end1],
                          .end2 = nodes[f_edges[i].end2],
                          .wt = f_edges[i].wt };
  }
  for(i = 0; i < n_c; i++, j++) {
    n_elist[j] = (edge) { .end1 = nodes[c_edges[i].end1],
                          .end2 = nodes[c_edges[i].end2],
                          .wt = c_edges[i].wt };
  }

  rval = build_graph(n, n_c + n_f, n_elist, G);
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
  return rval;
}

void get_delta (int comp, int *complist, int *deltacount, int *delta, graph *G)
{
  int i, k = 0;
  edge *elist = G->elist;

  for (i = 0; i < G->ncount; i++) G->nodelist[i].mark = (complist[i] == comp);

  for (i = 0; i < G->ecount; i++) {
    if (G->nodelist[elist[i].end1].mark + G->nodelist[elist[i].end2].mark == 1) {
      delta[k++] = i;
    }
  }
  *deltacount = k;

  for (i = 0; i < G->ncount; i++) G->nodelist[i].mark = 0;
}

int comp_edgewt(const void * edge1, const void * edge2)
{
  edge *a = (edge *) edge1,
       *b = (edge *) edge2;
  if (a->wt > b->wt) return 1;
  if (a->wt < b->wt) return -1;
  return 0;
}
