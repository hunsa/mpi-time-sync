// avoid getsubopt bug
//#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#include <getopt.h>

#include "mpits.h"
#include "clock_drift_utils.h"
#include "time_provider/clocks/Clock.hpp"
#include "clock_sync/clock_sync_common.h"
#include "clock_sync/clock_offset_algs/SKaMPIClockOffsetAlg.hpp"

typedef struct opt {
  int npp;   /* number of ping-pongs */
  int steps;  /* --steps */
  char testname[256];
  double print_procs_ratio;  /* --print-procs-ratio */
} drift_test_opts_t;

static const struct option default_long_options[] = {
    { "npp", required_argument, 0, 'n' },
    { "steps", required_argument, 0, 's' },
    { "print-procs-ratio", required_argument, 0, 'p' },
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
};

int parse_drift_test_options(drift_test_opts_t* opts_p, int argc, char **argv);

void print_time_parameters(FILE* f) {
  char clock[25];

  strcpy(clock, "MPI_Wtime");
#if ENABLE_GETTIME_REALTIME
  strcpy(clock, "clock_gettime_REALTIME");
#elif ENABLE_GETTIME_MONOTONIC
  strcpy(clock, "clock_gettime_MONOTONIC");
#endif
  fprintf(f, "#@clock=%s\n", clock);
#if !defined(ENABLE_GETTIME_REALTIME) && !defined(ENABLE_GETTIME_MONOTONIC)
  fprintf(f, "#@clock_tick=%1.10f\n", MPI_Wtick());
#endif
}

void print_help(char* testname) {
    int my_rank;
    int root_proc = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == root_proc) {

        printf("\nUSAGE: %s [options]\n", testname);

        printf("options:\n");
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "--steps",
                    "set the number of 1s steps to wait after sync (default: 0)");
        printf("%-40s %-40s\n", "--npp=<int>",
                    "set the number of ping-pong rounds between two processes to measure offset");
        printf("%-40s %-40s\n", "--print-procs-ratio",
        "set the fraction of the total processes to be tested for clock drift. If print-procs-ratio=0, only the last rank and the rank with the largest power of two are tested (default: 0)");

        printf("\nEXAMPLES: mpirun -np 4 %s --npp=10 --clock-sync=HCA2 --print-procs-ratio=0.1\n", testname);
        printf("\nEXAMPLES: mpirun -np 4 %s --npp=10 --clock-sync=HCA2 --steps=5 --print-procs-ratio=0.1\n", testname);
        printf("\n\n");
    }
}


void init_parameters(drift_test_opts_t* opts_p, char* name) {
  opts_p->npp = 10;
  opts_p->steps = 0;
  opts_p->print_procs_ratio = 0;
  strcpy(opts_p->testname,name);
}


int parse_drift_test_options(drift_test_opts_t* opts_p, int argc, char **argv) {
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
        case 's': /* here only two steps, the first after sync and the second after 's' seconds */
            opts_p->steps = atoi(optarg);
            break;
        case 'p': /* fraction of processes for which to measure the drift (normal distribution)
                   if print_procs_ratio==0, print only the largest power of two and the last rank
                   */
            opts_p->print_procs_ratio = atof(optarg);
            break;
        case 'n': /* number of repetitions (pingpongs) */
            opts_p->npp = atol(optarg);
            break;
        case 'h':
            print_help(opts_p->testname);
            break;
        case '?':
        default:
            break;
        }
    }

    if (opts_p->steps < 0) {
      fprintf(stderr, "Invalid number of steps (should be >=0)");
      exit(1);
    }
    if (opts_p->print_procs_ratio < 0 || opts_p->print_procs_ratio > 1) {
      fprintf(stderr, "Invalid process ratio (should be a number between 0 and 1)");
      exit(1);
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options

    return 0;
}

void print_initial_settings(int argc, char* argv[], drift_test_opts_t opts,
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
        fprintf(f, "#@npp=%d\n", opts.npp);
        fprintf(f, "#@steps=%d\n", opts.steps);
        fprintf(f, "#@timerres=%.9f\n", MPI_Wtick());

        print_time_parameters(f);
        print_sync_info(f);
    }

}

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    drift_test_opts_t opts;
    int master_rank;
    FILE* f;
    //mpits_clocksync_t clock_sync;

    double  min_drift;
    double *all_global_times = NULL;

    //int step;
    int n_wait_steps = 0;
    //double wait_time_s = 1;
    //struct timespec sleep_time;
    double runtime_s;
    int ntestprocs;
    int* testprocs_list;
    int index;
    mpits_clocksync_t  cs;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    parse_drift_test_options(&opts, argc, argv);

    n_wait_steps = 2;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    generate_test_process_list(opts.print_procs_ratio, &testprocs_list, &ntestprocs);

    if (my_rank == master_rank) {
      all_global_times = (double*) calloc(ntestprocs * n_wait_steps, sizeof(double));
    }

    int number_ping_pongs = opts.npp;

    Clock *clock = initialize_local_clock();
    SKaMPIClockOffsetAlg offset_alg(10,number_ping_pongs);

    if( MPITS_Init(MPI_COMM_WORLD, &cs) != 0  ) {
      exit(1);
    }

    print_initial_settings(argc, argv, opts, cs.print_sync_info);

    runtime_s = MPITS_get_time();

    MPITS_Clocksync_sync(&cs);
    runtime_s = MPITS_get_time() - runtime_s;


    if (my_rank == master_rank) {
      double target_time = MPITS_Clocksync_get_time(&cs) + ((double)opts.steps);
      int first_iter = 1;

      printf ("#@sync_duration=%.9f\n", runtime_s);

      int offset = 0;

      // measure once
      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];    // select the process to exchange pingpongs with
        if (p != master_rank) {
          all_global_times[offset * ntestprocs + index] =
                  offset_alg.measure_offset(MPI_COMM_WORLD, master_rank, p, *clock)->get_offset();
 //                 SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank,
 //                                                                                          p, &cs);
        }
      }

      //printf("target_time=%14.9f, cur_time=%14.9f\n", target_time, MPITS_get_time());
      // wait until 's' seconds are done
      do {
        double cur_time = MPITS_Clocksync_get_time(&cs);
        if( cur_time >= target_time ) {
          if( first_iter == 1 ) {
            // we were late.. report a warning
            printf ("#@LATE_WARNING=%d\n", opts.steps);
          }
          break;
        } else {
          first_iter = 0;
        }
      } while( 1 );

      // measure again
      offset = 1;
      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];    // select the process to exchange pingpongs with
        if (p != master_rank) {
          all_global_times[offset * ntestprocs + index] =
                  offset_alg.measure_offset(MPI_COMM_WORLD, master_rank, p, *clock)->get_offset();
                  //SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &cs);
        }
      }


    } else {
      int i;

      for(i=0; i<n_wait_steps; i++) {
        // measure twice (n_wait_steps should be 2)
        for (index = 0; index < ntestprocs; index++) {
          p = testprocs_list[index];    // make sure the current rank is in the test list
          if (my_rank == p) {
              offset_alg.measure_offset(MPI_COMM_WORLD, master_rank, p, *clock);
            //SKaMPIClockOffset_measure_offset(MPI_COMM_WORLD, master_rank, p, &cs);
          }
        }
      }
    }
    MPITS_Clocksync_finalize(&cs);

    f = stdout;
    if (my_rank == master_rank) {
      int offset = 0;

      fprintf(f,"%14s %7s %14s\n", "wait_time_s", "p", "min_diff");

      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];
        min_drift = all_global_times[offset * ntestprocs + index];
        fprintf(f, "%14.9f %7d %14.9f\n", 0.0f, p, fabs(min_drift));
      }

      offset = 1;
      for (index = 0; index < ntestprocs; index++) {
        p = testprocs_list[index];
        min_drift = all_global_times[offset * ntestprocs + index];
        fprintf(f, "%14.9f %7d %14.9f\n", (double)opts.steps, p, fabs(min_drift));
      }

      free(all_global_times);
    }

    free(testprocs_list);
    MPITS_Finalize();
    MPI_Finalize();
    return 0;
}

