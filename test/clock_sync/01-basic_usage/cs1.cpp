
#include "mpi.h"
#include "mpits.h"

int main(int argc, char *argv[]) {

  mpits_clocksync_t cs;
  int rank;
  int size;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  printf("rank %2d: %2d\n", rank, size);

  MPITS_Init(MPI_COMM_WORLD, &cs);


  MPITS_Clocksync_init(&cs);
  for(int i=0; i<5; i++) {
    MPITS_Clocksync_sync(&cs);
  }
  MPITS_Clocksync_finalize(&cs);

  MPITS_Finalize();

  MPI_Finalize();

  return 0;
}
