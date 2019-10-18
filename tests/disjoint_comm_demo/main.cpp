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

  enrico::Comm super(super_comm);
  std::map<std::string, enrico::Comm> others{{"sub0", enrico::Comm(subcomms.first)},
                                             {"sub1", enrico::Comm(subcomms.second)},
                                             {"intra", enrico::Comm(intranode_comm)},
                                             {"coup", enrico::Comm(coupling_comm)}};

  enrico::print_comm_layout(super, others);

  MPI_Finalize();
  return 0;
}
