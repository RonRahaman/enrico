#include "enrico/settings.h"
#include <gsl.hpp>
#include <pugixml.hpp>

namespace enrico {

namespace settings {

void parse_settings(int argc, char* argv[], std::string xml_file)
{
  parse_xml(xml_file);
  // Command line args take precedence, hence call second
  parse_cmd_args(argc, argv);
}

void parse_cmd_args(int argc, char* argv[])
{
  for (int i = 1; i < argc; ++i) {
    std::string arg{argv[i]};
    if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    }
  }
}

void parse_xml(std::string& xml_file)
{
  pugi::xml_document doc;
  auto result = doc.load_file(xml_file.c_str());
  if (!result) {
    throw std::runtime_error{"Can't parse ENRICO settings from " + xml_file};
  }
  auto root = doc.document_element();

  // -----------------
  // Coupling settings
  // -----------------

  auto coup_node = root.child("coupling");
  coupling::power = coup_node.child("power").text().as_double();
  coupling::max_timesteps = coup_node.child("max_timesteps").text().as_int();
  coupling::max_picard_iter = coup_node.child("max_picard_iter").text().as_int();

  if (coup_node.child("epsilon"))
    coupling::epsilon = coup_node.child("epsilon").text().as_double();

  // Determine relaxation parameters for heat source, temperature, and density
  auto set_alpha = [](pugi::xml_node node, double& alpha) {
    if (node) {
      std::string s{node.child_value()};
      if (s == "robbins-monro") {
        alpha = coupling::ROBBINS_MONRO;
      } else {
        alpha = node.text().as_double();
      }
    }
  };
  set_alpha(coup_node.child("alpha"), coupling::alpha);
  set_alpha(coup_node.child("alpha_T"), coupling::alpha_T);
  set_alpha(coup_node.child("alpha_rho"), coupling::alpha_rho);

  // check for convergence norm
  if (coup_node.child("convergence_norm")) {
    std::string s = coup_node.child_value("convergence_norm");
    if (s == "L1") {
      coupling::norm = coupling::Norm::L1;
    } else if (s == "L2") {
      coupling::norm = coupling::Norm::L2;
    } else if (s == "Linf") {
      coupling::norm = coupling::Norm::LINF;
    } else {
      throw std::runtime_error{"Invalid value for <convergence_norm>"};
    }
  }

  if (coup_node.child("temperature_ic")) {
    std::string s = coup_node.child_value("temperature_ic");

    if (s == "neutronics") {
      coupling::temperature_ic = coupling::Initial::neutronics;
    } else if (s == "heat_fluids") {
      coupling::temperature_ic = coupling::Initial::heat;
    } else {
      throw std::runtime_error{"Invalid value for <temperature_ic>"};
    }
  }

  if (coup_node.child("density_ic")) {
    std::string s = coup_node.child_value("density_ic");

    if (s == "neutronics") {
      coupling::density_ic = coupling::Initial::neutronics;
    } else if (s == "heat_fluids") {
      coupling::density_ic = coupling::Initial::heat;
    } else {
      throw std::runtime_error{"Invalid value for <density_ic>"};
    }
  }

  // -------------------
  // Neutronics settings
  // -------------------
  auto neut_node = root.child("neutronics");
  neutronics::nodes = neut_node.child("nodes").text().as_int();
  neutronics::procs_per_node = neut_node.child("procs_per_node").text().as_int();

  std::string nd = neut_node.child_value("driver");
  if (nd == "openmc") {
    neutronics::driver = neutronics::Driver::openmc;
  } else if (nd == "shift") {
    neutronics::driver = neutronics::Driver::shift;
  } else {
    throw std::runtime_error{"Invalid value for <neutronics><driver>"};
  }

  // --------------------
  // Heat-fluids settings
  // --------------------
  auto hf_node = root.child("heat_fluids");
  heat_fluids::nodes = hf_node.child("nodes").text().as_int();
  heat_fluids::procs_per_node = hf_node.child("procs_per_node").text().as_int();
  heat_fluids::pressure_bc = hf_node.child("pressure_bc").text().as_double();

  std::string hfd = hf_node.child_value("driver");
  if (hfd == "nek5000") {
    heat_fluids::driver = heat_fluids::Driver::nek5000;
  } else if (hfd == "nekrs") {
    heat_fluids::driver = heat_fluids::Driver::nekrs;
  } else if (hfd == "surrogate") {
    heat_fluids::driver = heat_fluids::Driver::surrogate;
  } else {
    throw std::runtime_error{"Invalid value for <heat_fluids><driver>"};
  }

  // Nek-specific
  if (heat_fluids::driver == heat_fluids::Driver::nek5000 ||
      heat_fluids::driver == heat_fluids::Driver::nekrs) {
    heat_fluids::casename = hf_node.child_value("casename");
    if (hf_node.child("output_heat_source")) {
      heat_fluids::output_heat_source =
        hf_node.child("output_heat_source").text().as_bool();
    }
  }

  // Surrogate-specific
  if (heat_fluids::driver == heat_fluids::Driver::surrogate)
}

}
}
