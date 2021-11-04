#ifndef ENRICO_COMM_SPLIT_H
#define ENRICO_COMM_SPLIT_H

#include "enrico/comm.h"

#include <array>
#include <mpi.h>

namespace enrico {

//! Splits a given MPI communicator into new communicators for each single-physics driver
//!
//! \param[in] super_comm An existing communicator that will be split
//! \param[in] num_nodes The desired number of nodes for the each single-physics driver's
//!            new communicator.  If a value is <= 0, then the respective driver's
//!            communicator will contain all nodes of super_comm
//! \param[in] procs_per_node The desired number of procs/node for each single-physics
//!            driver's new communicator.  If a value is <=, then the respective driver's
//!            communicator will contain the maximum number of available procs/node
//! \param[out] driver_comms The newly-created communicators, one for each driver.
//!             Each is comm active on the calling rank if it's contained by the respective
//!             driver; and null if not.
//! \param[out] intranode_comm A new comm that spans the node that the calling rank is in
//! \param[out] coupling_comm A new comm containing one proc per node.
void get_driver_comms(Comm super_comm,
                      std::array<int, 2> num_nodes,
                      std::array<int, 2> procs_per_node,
                      std::array<Comm, 2>& driver_comms,
                      Comm& intranode_comm,
                      Comm& coupling_comm);

//! Computes a balanced decomposition of a 1D array.
//!
//! This is based on MPE_Decomp1D from MPICH, slightly modified for C++.
//!
//! In ENRICO, this is used to statically assign heat/fluids ranks to neutronics ranks
//! for heat source coupling.  In this context, n := the number of heat/fluids ranks,
//! size := the number of neutronics ranks, rank := the sending neutron rank, and
//! return value := the range of receiving heat/fluid ranks
//!
//! \param[in] n Length of the array
//! \param[in] size Number of processers in decomposition
//! \param[in] rank Rank of this processor in the decomposition
//! \return The starting and ending indices assigned to this rank.
//!         Start is inclusive, end is non-inclusive.
std::pair<int, int> decomp_1d(int n, int size, int rank);

//! Discovers map from each rank ID in comm1 to a (possibly NULL) rank ID in comm2
//!
//! If comm1 is a supercomm or disjoint from comm2, then the rank IDs from comm1
//! that are not in comm2 will be mapped to MPI_PROC_NULL.
//!
//! \param comm1 The comm whose rank IDs are the keys of the mapping
//! \param comm2  The comm whose rank IDS are the values of the mapping
//! \return Vector element i maps rank i from comm1 to a rank ID in comm2.  The size
//!         is comm1.size().  Valid only on ranks in comm1.
std::vector<int> map_comm_ranks(const Comm& comm1, const Comm& comm2);

} // namespace enrico

#endif // ENRICO_COMM_SPLIT_H
