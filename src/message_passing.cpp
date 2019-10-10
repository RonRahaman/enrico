#include "enrico/message_passing.h"
#include "gsl.hpp"

#include <mpi.h>

namespace enrico {

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

  MPI_Comm indexing_comm;
  get_node_comms(super_comm, 1, &indexing_comm, intranode_comm);

  int intranode_rank = 0;
  MPI_Comm_rank(*intranode_comm, &intranode_rank);

  int node_index = 0;
  if (indexing_comm != MPI_COMM_NULL) {
    // If we call get_node_comms with procs_per_node == 1, then the indexing_comm
    // should include procs for which intranode_rank == 0.  This must be ensures for
    // the MPI_Bcase (below) to do its intended purpose
    Ensures(intranode_rank == 0);
    MPI_Comm_rank(indexing_comm, &node_index);
  }
  MPI_Bcast(static_cast<void*>(&node_index), 1, MPI_INT, 0, *intranode_comm);
}

MPI_Datatype define_position_mpi_datatype()
{

  Position p;
  int blockcounts[3] = {1, 1, 1};
  MPI_Datatype types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
  MPI_Aint displs[3];

  // Get displacements of struct members
  MPI_Get_address(&p.x, &displs[0]);
  MPI_Get_address(&p.y, &displs[1]);
  MPI_Get_address(&p.z, &displs[2]);

  // Make the displacements relative
  displs[2] -= displs[0];
  displs[1] -= displs[0];
  displs[0] = 0;

  // Make datatype
  MPI_Datatype type;
  MPI_Type_create_struct(3, blockcounts, displs, types, &type);
  MPI_Type_commit(&type);

  return type;
}

} // namespace enrico
