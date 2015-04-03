#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include "graph.h"
#include "util.h"
#include "comb.h"

static int getprob (char *fname, int *p_ncount, int *p_ecount, edge **p_elist);
static int parseargs (int ac, char **av);
static void usage (char *f);

static int find_combs(int ncount, int ecount, edge *elist);

static char *fname = (char *) NULL;
static int seed = 0;
static int verbose = 0;
static double EPSILON = 0.001;

int main (int ac, char **av)
{
  int rval  = 0, ncount = 0, ecount = 0;
  edge *elist = NULL;

  seed = (int) CO759_real_zeit ();

  rval = parseargs (ac, av);
  printf("Epsilon: %.6f\n", EPSILON);
  if (rval) goto CLEANUP;

  if (!fname) {
    printf ("Must specify a problem file\n");
    rval = 1; goto CLEANUP;
  }
  srandom (seed);

  if (fname) printf ("Problem name: %s\n", fname);

  rval = getprob (fname, &ncount, &ecount, &elist);
  if (rval) { fprintf (stderr, "getprob failed\n"); goto CLEANUP; }

  qsort(elist, ecount, sizeof(edge), comp_edgewt);

  rval = find_combs(ncount, ecount, elist);
  if (rval) { fprintf (stderr, "find_combs failed\n"); goto CLEANUP; }
  fflush (stdout);

CLEANUP:
  if (elist) free (elist);
  return rval;
}

static int find_combs(int ncount, int ecount, edge *elist)
{
  int rval = 0, i, j, a, b, l_idx, u_idx, violating;
  int ncomps, ncombs = 0, nvcombs = 0, vclist_size = 2;
  int *comps = NULL;
  double l = -EPSILON, u = 1.0 + EPSILON, lhs, rhs;
  graph G;
  comb **clist = NULL, **vclist = NULL;
  init_graph(&G);
  rval = build_contracted_graph(ncount, ecount, elist, &G);

  if (rval) {
    fprintf(stderr, "build_contracted_graph failed\n");
    rval = 1; goto CLEANUP;
  }

  comps = malloc (ncount * sizeof (int));
  vclist = malloc (vclist_size * sizeof (comb*));
  if (!comps || !vclist) {
    fprintf(stderr, "not enough memory for comps or vclist\n");
    rval = 1; goto CLEANUP;
  }

  for(l_idx = 0, l = -EPSILON; l_idx < G.ecount - 1; l_idx++) {
    if (G.elist[l_idx].wt - l < EPSILON) continue;
    l = G.elist[l_idx].wt;
    for(u_idx = G.ecount - 1, u = 1 + EPSILON; u_idx > l_idx && u > l; u_idx--) {
      if (u - G.elist[u_idx].wt < EPSILON) continue;
      u = G.elist[u_idx].wt;
      get_comps(&G, comps, &ncomps, l, u);
      comps_to_combs(&G, ncomps, comps, &ncombs, &clist);
      for (i = 0; i < ncombs; i++) {
        violating = 0;
        if (valid_comb(clist[i])) {
          lhs = comb_weight(clist[i]);
          rhs = 3 * clist[i]->nteeth + 1;
          if(lhs < rhs) {
            for (j = 0; j < nvcombs; j++) {
              if(equal_combs(clist[i], vclist[j])) break;
            }
            if (j == nvcombs) {
              violating = 1;
              if(nvcombs + 1 > vclist_size) {
                vclist_size *= 2;
                vclist = realloc (vclist, vclist_size * sizeof (comb*));
                if (!vclist) {
                  fprintf(stderr, "out of memory for vclist\n");
                  rval = 1; goto CLEANUP;
                }
              }
              vclist[nvcombs++] = clist[i];
              printf("Found violating comb #%d with |H| = %d and %d teeth with violation %.6f\n", nvcombs, clist[i]->nhandle, clist[i]->nteeth, rhs - lhs);

              if (verbose) {
                for (j = 0; j < clist[i]->G->ncount; j++)
                  clist[i]->G->nodelist[j].mark = 0;
                for (j = 0; j < clist[i]->nhandle; j++)
                  clist[i]->G->nodelist[clist[i]->handlenodes[j]].mark = 1;

                printf("[Handle Nodes]\n");
                for (j = 0; j < clist[i]->nhandle; j++) printf("%d ", clist[i]->handlenodes[j]);
                printf("\n");

                printf("[Handle Edges]\n");
                for (j = 0; j < clist[i]->G->ecount; j++) {
                  a = clist[i]->G->elist[j].end1; b = clist[i]->G->elist[j].end2;
                  if (clist[i]->G->nodelist[a].mark + clist[i]->G->nodelist[b].mark == 2)
                    printf("%d %d %.2f\n", a, b, clist[i]->G->elist[j].wt);
                }

                printf("[Teeth]\n");
                for (j = 0; j < clist[i]->nteeth; j++) {
                  int k = clist[i]->teethedges[j];
                  printf("%d %d %.6f\n", clist[i]->G->elist[k].end1, clist[i]->G->elist[k].end2, clist[i]->G->elist[k].wt);
                }
                printf("\n");
              }
            }
          }
        }
        if (!violating && clist[i]) { destroy_comb(clist[i]); free (clist[i]); }
      }
      if (clist) {
        free (clist);
      }
    }
  }
  printf("Found %d violating combs\n", nvcombs);

CLEANUP:
  free_graph(&G);
  if(comps) free (comps);
  if(vclist) {
    for (i = 0; i < nvcombs; i++) {
      if(vclist[i]) {
        destroy_comb(vclist[i]);
        free (vclist[i]);
      }
    }
  }
  return rval;
}

static int getprob (char *filename, int *p_ncount, int *p_ecount, edge **p_elist)
{
  FILE *f = NULL;
  int i, end1, end2, rval = 0, ncount, ecount;
  edge *elist = NULL;
  double w;

  if (filename) {
    if ((f = fopen (filename, "r")) == NULL) {
      fprintf (stderr, "Unable to open %s for input\n",filename);
      rval = 1;  goto CLEANUP;
    }

    if (fscanf (f, "%d %d", &ncount, &ecount) != 2) {
      fprintf (stderr, "Input file %s has invalid format\n",filename);
      rval = 1;  goto CLEANUP;
    }

    printf ("Nodes: %d  Edges: %d\n", ncount, ecount);
    fflush (stdout);

    elist = malloc (ecount * sizeof (edge));
    if (!elist) {
      fprintf (stderr, "out of memory for elist\n");
      rval = 1;  goto CLEANUP;
    }

    for (i = 0; i < ecount; i++) {
        if (fscanf(f,"%d %d %lf",&end1, &end2, &w) != 3) {
        fprintf (stderr, "%s has invalid input format\n", filename);
        rval = 1;  goto CLEANUP;
      }
      elist[i] = (edge) { .end1 = end1, .end2 = end2, .wt = w};
    }
  } else {
    fprintf(stderr, "A filename is required\n");
    rval = 1; goto CLEANUP;
  }
  *p_ncount = ncount;
  *p_ecount = ecount;
  *p_elist = elist;

CLEANUP:
  if (f) fclose (f);
  return rval;
}

static int parseargs (int ac, char **av)
{
  int c;

  if (ac == 1) {
    usage (av[0]);
    return 1;
  }

  while ((c = getopt (ac, av, "e:s:v:")) != EOF) {
    switch (c) {
      case 's':
        seed = atoi (optarg);
        break;
      case 'e':
        EPSILON = atof (optarg);
        break;
      case 'v':
        verbose = 1;
        break;
      case '?':
      default:
        usage (av[0]);
        return 1;
    }
  }

  if (optind < ac) fname = av[optind++];

  if (optind != ac) {
    usage (av[0]);
    return 1;
  }

  return 0;
}

static void usage (char *f)
{
  fprintf (stderr, "Usage: %s [-see below-] [prob_file]\n", f);
  fprintf (stderr, "   -s d  random seed\n");
  fprintf (stderr, "   -v    verbose\n");
  fprintf (stderr, "   -e f  set epsilon for threshold steps\n");
}
