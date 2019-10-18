#include "enrico/message_passing.h"
#include "enrico/comm.h"

#include <iostream>
#include <map>
#include <string>
#include <unistd.h>

namespace enrico {

void get_comms(MPI_Comm super_comm,
               std::pair<int, int> n_subcomm_nodes,
               std::pair<int, int> n_subcomm_procs_per_node,
               std::pair<MPI_Comm, MPI_Comm>* sub_comms,
               MPI_Comm* coupling_comm,
               MPI_Comm* intranode_comm)
{

  // super_rank is used as the "key" to retain ordering in the comm splits.
  // This can sometimes allow the sub_comms to retain some intent from the super_comm's
  // proc layout
  int super_rank;
  MPI_Comm_rank(super_comm, &super_rank);

  // Each intranode_comm contains all ranks in one node
  MPI_Comm_split_type(
    super_comm, MPI_COMM_TYPE_SHARED, super_rank, MPI_INFO_NULL, intranode_comm);
  int intranode_rank;
  MPI_Comm_rank(*intranode_comm, &intranode_rank);

  // Internode comm spans all nodes and contains one rank in every node
  int color = (intranode_rank == 0) ? 0 : 1;
  MPI_Comm_split(super_comm, color, super_rank, coupling_comm);
  if (color != 0)
    MPI_Comm_free(coupling_comm);

  // The node index is different for each node.  All ranks in a given node have the same
  // node_index
  int node_index, num_nodes;
  if (*coupling_comm != MPI_COMM_NULL) {
    MPI_Comm_rank(*coupling_comm, &node_index);
    MPI_Comm_size(*coupling_comm, &num_nodes);
  }
  MPI_Bcast(static_cast<void*>(&node_index), 1, MPI_INT, 0, *intranode_comm);
  MPI_Bcast(static_cast<void*>(&num_nodes), 1, MPI_INT, 0, *intranode_comm);

  // For the first subcomm, get the left nodes:
  if (node_index < n_subcomm_nodes.first &&
      intranode_rank < n_subcomm_procs_per_node.first)
    color = 0;
  else
    color = 1;
  MPI_Comm_split(super_comm, color, super_rank, &sub_comms->first);
  if (color != 0)
    MPI_Comm_free(&sub_comms->first);

  // For the second subcomm, get the right nodes:
  if (node_index >= num_nodes - n_subcomm_nodes.second &&
      intranode_rank < n_subcomm_procs_per_node.second)
    color = 0;
  else
    color = 1;
  MPI_Comm_split(super_comm, color, super_rank, &sub_comms->second);
  if (color != 0)
    MPI_Comm_free(&sub_comms->first);
}

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
    MPI_Comm_rank(indexing_comm, &node_index);
  }
  MPI_Bcast(static_cast<void*>(&node_index), 1, MPI_INT, 0, *intranode_comm);
}

void print_comm_layout(Comm& supercomm, std::map<std::string, Comm>& subcomms)
{

  char hostname[_SC_HOST_NAME_MAX];
  gethostname(hostname, _SC_HOST_NAME_MAX);

  if (supercomm.rank == 0) {
    std::cout << "subcomm_name,subcomm_rank,hostname,supercomm_rank" << std::endl;
  }
  MPI_Barrier(supercomm.comm);

  for (const auto& p : subcomms) {
    auto& comm = p.second;
    auto& comm_name = p.first;
    if (comm.comm != MPI_COMM_NULL) {
      for (int j = 0; j < comm.size; ++j) {
        if (comm.rank == j) {
          std::cout << comm_name << "," << comm.rank << "," << hostname << ","
                    << supercomm.rank << std::endl;
        }
        MPI_Barrier(comm.comm);
      }
    }
    MPI_Barrier(supercomm.comm);
  }
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
