
#include "mpi.h"
#include "mpi_clock_sync.h"

int main(int argc, char *argv[]) {

  mpits_clocksync_t cs;
  int rank;
  int size;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  printf("rank %2d: %2d\n", rank, size);

  MPITS_Init(&argc, &argv, &cs, MPI_COMM_WORLD);

  MPITS_Clocksync_init(&cs);

  for(int i=0; i<5; i++) {
    MPITS_Clocksync_sync(&cs);
  }

  MPITS_Finalize();

  MPI_Finalize();

  return 0;
}
