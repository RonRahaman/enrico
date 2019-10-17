#include "gsl.hpp"

#include <iostream>
#include <mpi.h>
#include <unistd.h>

void get_node_comms(MPI_Comm super_comm,
                    int procs_per_node,
                    MPI_Comm* sub_comm,
                    MPI_Comm* intranode_comm)
{

  // super_comm_rank is used as the "key" to retain ordering in the comm splits.
  // This can allow the sub_comm to retain some intent from the super_comm's proc layout
  int super_comm_rank;
  MPI_Comm_rank(super_comm, &super_comm_rank);

  // intranode_comm is an intermediate object.  It is only used to get an
  // intranode_comm_rank, which is used as the "color" in the final comm split.
  MPI_Comm_split_type(
    super_comm, MPI_COMM_TYPE_SHARED, super_comm_rank, MPI_INFO_NULL, intranode_comm);
  int intranode_comm_rank;
  MPI_Comm_rank(*intranode_comm, &intranode_comm_rank);

  // Finally, split the specified number of procs_per_node from the super_comm
  // We only want the comm where color == 0.  The second comm is destroyed.
  int color = intranode_comm_rank < procs_per_node ? 0 : 1;
  MPI_Comm_split(super_comm, color, super_comm_rank, sub_comm);
  if (color != 0)
    MPI_Comm_free(sub_comm);
}

void get_disjoint_comms(MPI_Comm super_comm,
                        int num_nodes,
                        MPI_Comm* sub_comm,
                        MPI_Comm* intranode_comm)
{

  int intranode_rank, intranode_size;
  MPI_Comm_rank(*intranode_comm, &intranode_rank);
  MPI_Comm_size(*intranode_comm, &intranode_size);

  // internode_comm will get 1 rank per node
  MPI_Comm internode_comm;
  get_node_comms(super_comm, 1, &internode_comm, intranode_comm);

  int internode_rank, internode_size;
  MPI_Comm_rank(internode_comm, &internode_rank);
  MPI_Comm_size(internode_comm, &internode_size);

  // For absolute clarity, require that node

  int node_index = 0;
  if (internode_comm != MPI_COMM_NULL) {
    // If we call get_node_comms with procs_per_node == 1, then the internode_comm
    // should include procs for which intranode_rank == 0.  This must be ensures for
    // the MPI_Bcase (below) to do its intended purpose
    Ensures(intranode_rank == 0);
    MPI_Comm_rank(internode_comm, &node_index);
  }
  MPI_Bcast(static_cast<void*>(&node_index), 1, MPI_INT, 0, *intranode_comm);

  //===========================================================================
  // Debug: Print node indices
  //===========================================================================

  int super_comm_rank = 0, super_comm_size = 0;
  if (super_comm != MPI_COMM_NULL) {
    MPI_Comm_rank(super_comm, &super_comm_rank);
    MPI_Comm_size(super_comm, &super_comm_size);

    char myHostName[_SC_HOST_NAME_MAX];
    gethostname(myHostName, _SC_HOST_NAME_MAX);

    for (int i = 0; i < super_comm_size; i++) {
      if (super_comm_rank == i)
        std::cout << myHostName << "\t: " << node_index << std::endl;
      MPI_Barrier(super_comm);
    }
  }
}

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  MPI_Comm sub_comm, intranode_comm;
  int num_nodes = 1;

  get_disjoint_comms(MPI_COMM_WORLD, num_nodes, &sub_comm, &intranode_comm);

  MPI_Finalize();
  return 0;
}
