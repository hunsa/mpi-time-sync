// avoid getsubopt bug
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <time.h>
#include <math.h>
#include "mpi.h"
#include "mpits.h"
#include <getopt.h>

#include "clock_drift_utils.h"

//#define ZF_LOG_LEVEL ZF_LOG_VERBOSE
#define ZF_LOG_LEVEL ZF_LOG_WARN
#include "log/zf_log.h"


typedef struct opt {
  int reps;   /* --reps number of clock sync tests */
  char testname[256];
  double print_procs_ratio;  /* --print-procs-ratio */
} clock_drift_test_opts_t;

static const struct option default_long_options[] = {
    { "nrep", required_argument, 0, 'r' },
    { "print-procs-ratio", required_argument, 0, 'p' },
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
};

int parse_drift_test_options(clock_drift_test_opts_t* opts_p, int argc, char **argv);

void print_help(char* testname) {
    int my_rank;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == root_proc) {

        printf("\nUSAGE: %s [options]\n", testname);

        printf("options:\n");
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "--nrep",
                    "number of repetitions (default: 1)");
        printf("%-40s %-40s\n", "--print-procs-ratio",
        "set the fraction of the total processes to be tested for clock drift. If print-procs-ratio=0, only the last rank and the rank with the largest power of two are tested (default: 0)");

        printf("\nEXAMPLE: MPITS_PARAMS=\"--clock-sync=None\" mpirun -np 8 %s --print-procs-ratio=1.0 --nrep=10\n", testname);
        printf("\n\n");
    }
}


void init_parameters(clock_drift_test_opts_t* opts_p, char* name) {
  opts_p->reps = 1;
  opts_p->print_procs_ratio = 0;
  strcpy(opts_p->testname,name);
}


int parse_drift_test_options(clock_drift_test_opts_t* opts_p, int argc, char **argv) {
    int c;

    init_parameters(opts_p, argv[0]);

    opterr = 0;

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "h", default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 'r':
            opts_p->reps = atoi(optarg);
            break;
        case 'p': /* fraction of processes for which to measure the drift (normal distribution)
                   if print_procs_ratio==0, print only the largest power of two and the last rank
                   */
            opts_p->print_procs_ratio = atof(optarg);
            break;
        case 'h':
            print_help(opts_p->testname);
            break;
        case '?':
            break;
        }
    }

    if (opts_p->print_procs_ratio < 0 || opts_p->print_procs_ratio > 1) {
      fprintf(stderr, "Invalid process ratio (should be a number between 0 and 1)");
      exit(1);
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options

    return 0;
}

void print_initial_settings(int argc, char* argv[], clock_drift_test_opts_t opts,
        print_sync_info_t print_sync_info) {
    int my_rank, np;
    FILE * f;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;

        fprintf(f, "#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            fprintf(f, " %s", argv[i]);
        }
        fprintf(f, "\n");
        fprintf(f, "#@nrep=%d\n", opts.reps);
        fprintf(f, "#@timerres=%.9f\n", MPI_Wtick());

        print_sync_info(f);
    }

}

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    clock_drift_test_opts_t opts;
    int master_rank;
    FILE* f;
    mpits_clocksync_t cs;

    double  min_drift;
    double *all_global_times = NULL;

    int ntestprocs;
    int* testprocs_list;
    int index;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;


    parse_drift_test_options(&opts, argc, argv);

    MPITS_Init(MPI_COMM_WORLD, &cs);
    MPITS_Clocksync_init(&cs);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    generate_test_process_list(opts.print_procs_ratio, &testprocs_list, &ntestprocs);

    if (my_rank == master_rank) {
      all_global_times = (double*) calloc(ntestprocs * opts.reps, sizeof(double));
    }

    print_initial_settings(argc, argv, opts, cs.print_sync_info);

    for (int i = 0; i < opts.reps; i++) {
      MPITS_Clocksync_sync(&cs);

      if (my_rank == master_rank) {

        // measure once
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // select the process to exchange pingpongs with
          if (p != master_rank) {
            all_global_times[i * ntestprocs + index] = SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p,
                                                                                        &cs);
          }
        }

      } else {
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // make sure the current rank is in the test list
          if (my_rank == p) {
            SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &cs);
          }
        }
      }

    }

    MPITS_Clocksync_finalize(&cs);

    f = stdout;
    if (my_rank == master_rank) {

      fprintf(f,"%4s %7s %14s\n", "nrep", "p", "min_diff");

      for(int i=0; i<opts.reps; i++) {
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];
          min_drift = all_global_times[i * ntestprocs + index];
          fprintf(f, "%4d %7d %14.9f\n", i, p, fabs(min_drift));
        }
      }

      free(all_global_times);
    }

    free(testprocs_list);
    MPITS_Finalize();
    MPI_Finalize();
    return 0;
}
