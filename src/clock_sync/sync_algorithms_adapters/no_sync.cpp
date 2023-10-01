
#include <cstdio>
#include <string>

#include "no_sync.h"
#include "clock_sync/mpi_clock_sync_internal.h"

static void empty() {
};

static void empty_with_comm(MPI_Comm comm) {
};

static void empty_init_module(MPI_Comm comm, int argc, char** argv) {
}

static double get_normalized_time(double local_time) {
//  int my_rank;
//  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
//
//  if (my_rank == 0) {
//    fprintf(stderr, "WARNING: Global time is not defined.\n");
//  }

  return local_time;
}


static void print_sync_parameters(FILE* f) {
    fprintf(f, "#@clocksync=None\n");
}


extern "C"
void register_no_clock_sync_module(mpits_clocksync_t *sync_mod) {
  sync_mod->name = (char*)std::string("None").c_str();;
  sync_mod->clocksync = MPITS_CLOCKSYNC_NONE;

  sync_mod->init_module = empty_init_module;
  sync_mod->cleanup_module = empty;

  sync_mod->init_sync = empty_with_comm;
  sync_mod->finalize_sync = empty_with_comm;

  sync_mod->sync_clocks = empty_with_comm;

  sync_mod->get_global_time = get_normalized_time;
  sync_mod->print_sync_info = print_sync_parameters;
}


