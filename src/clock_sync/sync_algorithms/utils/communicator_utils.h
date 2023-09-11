#ifndef MPITS_COMMUNICATOR_HELPER_FUNCS_H_
#define MPITS_COMMUNICATOR_HELPER_FUNCS_H_

#include <mpi.h>


void create_intranode_communicator(MPI_Comm old_comm, MPI_Comm *new_comm);

void create_intrasocket_communicator(MPI_Comm old_comm, int socket_id, MPI_Comm *new_comm);

void create_interlevel_communicator(MPI_Comm old_comm, MPI_Comm local_comm,
    const int local_rank_per_node, MPI_Comm *new_comm);

void print_comm_debug_info(const char *tag, MPI_Comm comm1, MPI_Comm comm2);

#endif

