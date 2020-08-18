//! \file base_drivers.h
//! Base classes for single- and coupled-physics drivers
#ifndef ENRICO_DRIVERS_H
#define ENRICO_DRIVERS_H

#include "enrico/comm.h"
#include "enrico/mpi_types.h"

#include <mpi.h>

#include <vector>

namespace enrico {

//! Base class for driver that controls a physics solve
class Driver {
public:
  //! Initializes the solver with the given MPI communicator.
  //! \param comm An existing MPI communicator used to initialize the solver
  explicit Driver(MPI_Comm comm, int write_at_timestep, int write_at_picard_iter)
    : comm_(comm)
    , write_at_timestep_(write_at_timestep)
    , write_at_picard_iter_(write_at_picard_iter)
  {}

  //! Performs the necessary initialization for this solver in one Picard iteration
  virtual void init_step() {}

  //! Performs the solve in one Picard iteration
  //! \param timestep timestep index
  //! \param iteration iteration index
  virtual void solve_step(int timestep, int iteration){};

  //! Performs the solve in one Picard iteration
  void solve_step() { this->solve_step(-1, -1); }

  //! Write results for physics solve for given timestep and iteration
  //! \param timestep timestep index
  //! \param iteration iteration index
  virtual void write_step(int timestep, int iteration) {}

  //! Write results for a physics solve at the end of the coupled simulation
  void write_step() { this->write_step(-1, -1); }

  //! Performs the necessary finalization for this solver in one Picard iteration
  virtual void finalize_step() {}

  //! Queries whether the comm for this solver is active
  //! \return True if this comm's solver is not MPI_COMM_NULL
  bool active() const;

  Comm comm_; //!< The MPI communicator used to run the solver

  const int write_at_timestep_; //!< Write output at this timestep interval

  const int write_at_picard_iter_; //!< Write output at this interval of Picard iterations
};

} // namespace enrico

#endif // ENRICO_DRIVERS_H
