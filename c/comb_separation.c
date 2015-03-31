#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include "graph.h"
#include "util.h"
#include "comb.h"

static int getprob (char *fname, int *p_ncount, int *p_ecount, int **p_elist, double **p_ewt);
static int parseargs (int ac, char **av);
static void usage (char *f);

static int find_combs(int ncount, int ecount, int *elist, double *ewts);

static char *fname = (char *) NULL;
static int seed = 0;
static int verbose = 0;
static double lower = 0.05;
static double upper = 0.95;
static double t_thresh = 0.5;

int main (int ac, char **av)
{
  int rval  = 0, ncount = 0, ecount = 0;
  int *elist = NULL;
  double *ewts = NULL;
  double szeit;

  seed = (int) CO759_real_zeit ();

  rval = parseargs (ac, av);
  printf("upper: %.2f lower: %.2f\n", upper, lower);
  if (rval) goto CLEANUP;

  if (!fname) {
    printf ("Must specify a problem file\n");
    rval = 1; goto CLEANUP;
  }
  printf ("Seed = %d\n", seed);
  srandom (seed);

  if (fname) printf ("Problem name: %s\n", fname);

  rval = getprob (fname, &ncount, &ecount, &elist, &ewts);
  if (rval) { fprintf (stderr, "getprob failed\n"); goto CLEANUP; }

  szeit = CO759_zeit ();
  rval = find_combs(ncount, ecount, elist, ewts);
  if (rval) { fprintf (stderr, "find_combs failed\n"); goto CLEANUP; }
  printf ("Running Time: %.2f seconds\n", CO759_zeit() - szeit);
  fflush (stdout);

CLEANUP:
  if (elist) free (elist);
  if (ewts) free (ewts);
  return rval;
}

static int find_combs(int ncount, int ecount, int *elist, double *ewts)
{
  int i, j, rval = 0, *comps = NULL, ncomps, ncombs = 0, a, b;
  double *y = NULL, szeit;
  graph G;
  comb **clist = NULL;
  init_graph(&G);
  szeit = CO759_zeit ();
  rval = build_contracted_graph(ncount, ecount, elist, ewts, y, &G);
  printf ("Running Time for build_contracted_graph: %.2f seconds\n", CO759_zeit() - szeit);
  printf("Graph nodes: %d Graph edges: %d\n", G.ncount, G.ecount);

  if (rval) {
    fprintf(stderr, "build_contracted_graph failed\n");
    rval = 1; goto CLEANUP;
  }

  comps = malloc (ncount * sizeof (int));
  if (!comps) {
    fprintf(stderr, "not enough memory for comps\n");
    rval = 1; goto CLEANUP;
  }
  szeit = CO759_zeit ();
  get_comps(&G, comps, &ncomps, lower, upper);
  printf ("Running Time for get_comps: %.2f seconds\n", CO759_zeit() - szeit);

  szeit = CO759_zeit ();
  comps_to_combs(&G, ncomps, comps, &ncombs, &clist, t_thresh, verbose);
  printf ("Running Time for comps_to_combs: %.2f seconds\n", CO759_zeit() - szeit);
  for (i = 0; i < ncombs; i++) {
    if (valid_comb(clist[i]) && violating_comb(clist[i], verbose)) {
      printf("Found violating comb with |H| = %d and %d teeth\n", clist[i]->nhandle, clist[i]->nteeth);

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
          a = clist[i]->G->elist[2*j]; b = clist[i]->G->elist[2*j+1];
          if (clist[i]->G->nodelist[a].mark + clist[i]->G->nodelist[b].mark == 2)
            printf("%d %d %.2f\n", a, b, clist[i]->G->ewts[j]);
        }

        printf("[Teeth]\n");
        for (j = 0; j < clist[i]->nteeth; j++) {
          int k = clist[i]->teethedges[j];
          printf("%d %d %.2f\n", clist[i]->G->elist[2*k], clist[i]->G->elist[2*k+1], clist[i]->G->ewts[k]);
        }
        printf("\n");
      }
    }
  }

CLEANUP:
  free_graph(&G);
  if(y) free (y);
  if(comps) free (comps);
  if(clist) {
    for (i = 0; i < ncombs; i++) { destroy_comb(clist[i]); }
    free (clist);
  }
  return rval;
}

static int getprob (char *filename, int *p_ncount, int *p_ecount, int **p_elist,
    double **p_ewt)
{
  FILE *f = NULL;
  int i, end1, end2, rval = 0, ncount, ecount, *elist = NULL;
  double w, *ewt = NULL;

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

    elist = (int *) malloc (2 * ecount * sizeof (int));
    if (!elist) {
      fprintf (stderr, "out of memory for elist\n");
      rval = 1;  goto CLEANUP;
    }

    ewt = malloc (ecount * sizeof (double));
    if (!ewt) {
      fprintf (stderr, "out of memory for elen\n");
      rval = 1;  goto CLEANUP;
    }

    for (i = 0; i < ecount; i++) {
      if (fscanf(f,"%d %d %lf",&end1, &end2, &w) != 3) {
      fprintf (stderr, "%s has invalid input format\n", filename);
      rval = 1;  goto CLEANUP;
    }
    elist[2*i] = end1;
    elist[2*i+1] = end2;
    ewt[i] = w;
    }
  } else {
    fprintf(stderr, "A filename is required\n");
    rval = 1; goto CLEANUP;
  }
  *p_ncount = ncount;
  *p_ecount = ecount;
  *p_elist = elist;
  *p_ewt = ewt;

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

  while ((c = getopt (ac, av, "l:u:t:s:v")) != EOF) {
    switch (c) {
      case 's':
        seed = atoi (optarg);
        break;
      case 'l':
        lower = atof (optarg);
        break;
      case 'u':
        upper = atof (optarg);
        break;
      case 't':
        t_thresh = atof (optarg);
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
  fprintf (stderr, "   -l f  set lower threshold for graph components\n");
  fprintf (stderr, "   -u f  set upper threshold for graph components\n");
  fprintf (stderr, "   -t f  set teeth threshold for comb candidates\n");
}
