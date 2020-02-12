//! \file openmc_nek_driver.h
//! Driver for coupled Nek5000/OpenMC simulations
#ifndef ENRICO_OPENMC_NEK_DRIVER_H
#define ENRICO_OPENMC_NEK_DRIVER_H

#include "enrico/coupled_driver.h"
#include "enrico/heat_fluids_driver.h"
#include "enrico/mpi_types.h"
#include "enrico/neutronics_driver.h"

#include <mpi.h>

#include <unordered_map>

namespace enrico {

//! Driver for coupled Nek5000/OpenMC simulations
class OpenmcNekDriver : public CoupledDriver {

public:
  //! Initializes coupled OpenMC-Nek5000 driver with the given MPI communicator and
  //! sets up geometry mappings.
  //!
  //! Currently, openmc_comm and nek_comm must be subsets of comm.  The function
  //! enrico::get_node_comms() can be used to split a coupled_comm into suitable subcomms.
  //!
  //! \param comm The MPI communicator used for the coupled driver
  //! \param node XML node containing settings
  explicit OpenmcNekDriver(Comm comm, pugi::xml_node node);

  //! Whether the calling rank has access to global coupling fields. Because the OpenMC
  //! and Nek communicators are assumed to overlap (though they are not the same), and
  //! Nek broadcasts its solution onto the OpenMC ranks, we need to check that both
  //! communicators are active.
  //!
  //! TODO: This won't work if the OpenMC and Nek communicators are disjoint
  bool has_global_coupling_data() const override;

  void set_heat_source() override;

  void set_temperature() override;

  void set_density() override;

  NeutronicsDriver& get_neutronics_driver() const override;

  HeatFluidsDriver& get_heat_driver() const override;

  Comm intranode_comm_;       //!< The communicator representing intranode ranks
  int openmc_procs_per_node_; //!< Number of MPI ranks per (shared-memory) node in OpenMC
                              //!< comm

protected:
  //! Initialize global temperature buffers on all OpenMC ranks.
  //!
  //! These arrays store the dimensionless temperatures of Nek's global elements. These
  //! are **not** ordered by Nek's global element indices. Rather, these are ordered
  //! according to an MPI_Gatherv operation on Nek5000's local elements.
  void init_temperatures() override;

  //! Initialize global source buffers on all OpenMC ranks.
  //!
  //! These arrays store the dimensionless source of Nek's global elements. These are
  //! **not** ordered by Nek's global element indices. Rather, these are ordered according
  //! to an MPI_Gatherv operation on Nek5000's local elements.
  void init_heat_source() override;

  //! Initialize global density buffers on all OpenMC ranks.
  //!
  //! These arrays store the ensity of Nek's global elements. These are **not**
  //! ordered by Nek's global element indices. Rather, these are ordered according
  //! to an MPI_Gatherv operation on Nek5000's local elements.
  void init_densities() override;

  //! Initialize global fluid masks on all OpenMC ranks.
  //!
  //! These arrays store the dimensionless source of Nek's global elements. These are
  //! **not** ordered by Nek's global element indices. Rather, these are ordered according
  //! to an MPI_Gatherv operation on Nek5000's local elements.
  void init_elem_fluid_mask();

  //! Initialize fluid masks for OpenMC cells on all OpenMC ranks.
  void init_cell_fluid_mask();

private:
  //! Create bidirectional mappings from OpenMC cell instances to/from Nek5000 elements
  void init_mappings();

  //! Initialize the tallies for all OpenMC materials
  void init_tallies();

  //! Initialize global volume buffers for OpenMC ranks
  void init_volumes();

  std::unique_ptr<NeutronicsDriver> neutronics_driver_;  //!< The neutronics driver
  std::unique_ptr<HeatFluidsDriver> heat_fluids_driver_; //!< The heat-fluids driver

  //! States whether a global element is in the fluid region
  //! These are **not** ordered by Nek's global element indices.  Rather, these are
  //! ordered according to an MPI_Gatherv operation on Nek5000's local elements.
  std::vector<int> elem_fluid_mask_;

  //! States whether an OpenMC cell in the fluid region
  xt::xtensor<int, 1> cell_fluid_mask_;

  //! The dimensionless volumes of Nek's global elements
  //! These are **not** ordered by Nek's global element indices.  Rather, these are
  //! ordered according to an MPI_Gatherv operation on Nek5000's local elements.
  std::vector<double> elem_volumes_;

  //! Map that gives a list of Nek element global indices for a given neutronics
  //! cell handle. The Nek global element indices refer to indices defined by
  //! the MPI_Gatherv operation, and do not reflect Nek's internal global
  //! element indexing.
  std::unordered_map<CellHandle, std::vector<int32_t>> cell_to_elems_;

  //! Map that gives the neutronics cell handle for a given Nek global element
  //! index. The Nek global element indices refer to indices defined by the
  //! MPI_Gatherv operation, and do not reflect Nek's internal global element
  //! indexing.
  std::vector<CellHandle> elem_to_cell_;

  //! Number of cell instances in OpenMC model
  int32_t n_cells_;
};

} // namespace enrico

#endif // ENRICO_OPENMC_NEK_DRIVER_H
