#ifndef __GRAPH_H
#define __GRAPH_H

typedef struct ajdobj {
    int n;   /* index of neighbor node */
    int e;   /* index of adj joining neighbor */
} adjobj;

typedef struct node {
    int deg;
    adjobj *adj;
    int mark;
} node;

typedef struct graph {
    int ncount;
    int ecount;
    node *nodelist;
    int *elist;
    double *ewts;
    adjobj *adjspace;
} graph;

void init_graph(graph*);
void free_graph(graph*);
int build_graph(int, int, int*, double*, graph*);
int build_contracted_graph(int, int, int*, double*, double*, graph*);
void get_delta(int, int*, int*, int*, graph*);
void get_comps(graph*, int*, int*, double, double);
void comp_sizes(int, int, int*, int *);

#endif  /* __GRAPH_H */
