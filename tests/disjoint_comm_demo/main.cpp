#include "enrico/comm.h"
#include "enrico/message_passing.h"

#include <iostream>
#include <map>
#include <mpi.h>
#include <string>
#include <unistd.h>
#include <utility>

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  MPI_Comm super_comm;
  MPI_Comm_dup(MPI_COMM_WORLD, &super_comm);

  std::pair<int, int> n_subcomm_nodes(1, 1);
  std::pair<int, int> n_procs_per_node(2, 4);
  std::pair<MPI_Comm, MPI_Comm> subcomms;
  MPI_Comm coupling_comm, intranode_comm;

  enrico::get_comms(super_comm,
                    n_subcomm_nodes,
                    n_procs_per_node,
                    &subcomms,
                    &coupling_comm,
                    &intranode_comm);

  //===========================================================================
  // Print comms, nodes, and ranks
  //===========================================================================

  char hostname[_SC_HOST_NAME_MAX];
  gethostname(hostname, _SC_HOST_NAME_MAX);

  enrico::Comm super(super_comm);
  std::map<std::string, enrico::Comm> others{{"sub0", enrico::Comm(subcomms.first)},
                                             {"sub1", enrico::Comm(subcomms.second)},
                                             {"intra", enrico::Comm(intranode_comm)},
                                             {"coup", enrico::Comm(coupling_comm)}};

  if (super.rank == 0) {
    std::cout << "subcomm,subcomm_rank,host,supercomm_rank" << std::endl;
  }
  MPI_Barrier(super.comm);

  for (const auto& p : others) {
    auto& comm = p.second;
    auto& comm_name = p.first;
    if (comm.comm != MPI_COMM_NULL) {
      for (int j = 0; j < comm.size; ++j) {
        if (comm.rank == j) {
          std::cout << comm_name << "," << comm.rank << "," << hostname << ","
                    << super.rank << std::endl;
        }
        MPI_Barrier(comm.comm);
      }
    }
    MPI_Barrier(super.comm);
  }

  MPI_Finalize();
  return 0;
}
