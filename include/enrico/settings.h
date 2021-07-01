#ifndef ENRICO_SRC_SETTINGS_H
#define ENRICO_SRC_SETTINGS_H

#include <string>

namespace enrico {

namespace settings {

void parse_settings(int argc, char* argv[], std::string xml_file = "enrico.xml");

void parse_cmd_args(int argc, char* argv[]);

void parse_xml(std::string& xml_file);

bool verbose = false;

namespace coupling {
// Types, aliases
enum class Norm { L1, L2, LINF }; //! Types of norms

//! Enumeration of available temperature initial condition specifications.
//! 'neutronics' sets temperature condition from the neutronics input files,
//! while 'heat' sets temperature based on a thermal-fluids input (or restart) file.
enum class Initial { neutronics, heat };

//! Special alpha value indicating use of Robbins-Monro relaxation
constexpr static double ROBBINS_MONRO = -1.0;

double power;        //!< Power in [W]
int max_timesteps;   //!< Maximum number of time steps
int max_picard_iter; //!< Maximum number of Picard iterations

//! Picard iteration convergence tolerance, defaults to 1e-3 if not set
double epsilon{1e-3};

//! Constant relaxation factor for the heat source, defaults to 1.0 (standard Picard) if
//! not set
double alpha{1.0};

//! Constant relaxation factor for the temperature, defaults to the
//! relaxation aplied to the heat source if not set
double alpha_T{alpha};

//! Constant relaxation factor for the density, defaults to the
//! relaxation applied to the heat source if not set
double alpha_rho{alpha};

//! Where to obtain the temperature initial condition from. Defaults to the
//! temperatures in the neutronics input file.
Initial temperature_ic{Initial::neutronics};

//! Where to obtain the density initial condition from. Defaults to the densities
//! in the neutronics input file.
Initial density_ic{Initial::neutronics};

// Norm to use for convergence checks
Norm norm{Norm::LINF};

} // namespace coupling

namespace neutronics {
enum class Driver { openmc, shift };
Driver driver;
int nodes;
int procs_per_node;
std::string filename; //!< SHIFT filename.  Only used by ShiftDriver.
} // namespace neutronics

namespace heat_fluids {
enum class Driver { nek5000, nekrs, surrogate };
Driver driver;
int nodes;
int procs_per_node;
double pressure_bc; //! System pressure in [MPa]
bool output_heat_source{false};
std::string casename; //! Nek5000 casename.  Only used by Nek5000Driver
} // namespace heat_fluids

} // namespace settings

} // namespace enrico

#endif // ENRICO_SRC_SETTINGS_H
