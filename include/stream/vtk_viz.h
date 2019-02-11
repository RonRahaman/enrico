#include <string>
#include <iostream>
#include <fstream>

#include "stream/heat_driver.h"

using xt::xtensor;
using std::ofstream;

namespace stream {

class SurrogateVtkWriter {

friend SurrogateHeatDriver;

enum class VizDataType {
  none = 0,
    source = 1,
    temp   = 2,
    all    = 3
};

enum class VizRegionType {
  none = 0,
  fuel = 1,
  clad = 2,
  all  = 3
};

private:
  //! Initializes the surrogate to VTK writer with a surrogate model. Can only be called withing the SurrogateHeatDriver.
  //!
  //! \param surrogate_ptr Pointer to the surrogate to write
  //! \param t_rad         Radial resolution of the generated VTK mesh
SurrogateVtkWriter(const SurrogateHeatDriver *surrogate_ptr,
               int t_res,
               std::string regions_to_write,
               std::string data_to_write);

public:
  //! Write the surrogate model to VTK
  void write_vtk(std::string filename = "magnolia.vtk");

  //! Write a vtk header for an unstructured grid
  void write_header(ofstream& vtk_file);

  //! Write fuel pin points to the vtk file
  void write_points(ofstream& vtk_file);

  //! Write the wedge/hex element connectivity to the vtk file
  void write_element_connectivity(ofstream& vtk_file);

  //! Write the wedge/hex element types to the vtk file
  void write_element_types(ofstream& vtk_file);

  //! Write requested data to the vtk file
  void write_data(ofstream& vtk_file);

  //! Generate fuel mesh points
  //! \return fuel points (axial, radial_rings, xyz)
  xtensor<double, 3> fuel_points();
  //! Generate cladding mesh points
  //! \return cladding points (axial, radial_rings, xyz)
  xtensor<double, 3> clad_points();
  //! Return 1-D array of points for writing (xyz...) (ordered radially, axially)
  //! \return 1-D array of all points in the model
  xtensor<double, 1> points();
  //! Return 1-D array of points, translated to a pin center
  //! \return 1-D array of points translated to a center x,y
  xtensor<double, 1> points_for_pin(double x, double y);
  //! Return connectivity with an offset
  xtensor<int, 1> conn_for_pin(int offset);
  //! Generate fuel connectivity (axial, radial, res, conn)
  //! \return fuel element connectivity (axial, radial, res, conn)
  xtensor<int, 4> fuel_conn();
  //! Generate cladding connectivity
  //! \return cladding element connectivity (axial, radial, res, conn)
  xtensor<int, 4> clad_conn();
  //! Return all connectivity values for writing
  //! \return 1-D array of connectivity, strided by CONN_STRIDE_ per element (ordered radially, axially)
  xtensor<int, 1> conn();
  //! Generate fuel vtk mesh element (cell) types
  //! \return fuel cell types (axia, radial, azimuthal)
  xtensor<int, 3> fuel_types();
  //! Generate cladding vtk mesh element (cell) types
  //! \return cladding cell types (axia, radial, azimuthal)
  xtensor<int, 3> clad_types();
  //! Return all element type values for writing
  //! \return 1-D array of types, one for each element (ordered radially, axially)
  xtensor<int, 1> types();

private:

  // internal variables/parameters
  const SurrogateHeatDriver* sgate_; //!< pointer to surrogate
  int radial_res_;                   //!< radial resolution;
  VizDataType data_out_;
  VizRegionType regions_out_;

  // pin templates
  xtensor<double, 1> points_; //!< template of xyz values for the mesh, centerd on the origin
  xtensor<int, 1> conn_;      //!< template of mesh element connectivity for a single pin
  xtensor<int, 1> types_;     //!< template of mesh element types for a single pin

  // SECTIONS
  // axial
  int n_axial_sections_; //!< number of axial sections
  int n_axial_points_;   //!< number of axial points (planes)
  // radial
  int n_radial_fuel_sections_; //!< number of radial fuel sections
  int n_radial_clad_sections_; //!< number of radial cladding sections
  int n_radial_sections_;      //!< total number of radial SECTIONS

  // POINTS
  unsigned long n_fuel_points_; //!< number of points needed to represent the fuel regions
  unsigned long n_clad_points_; //!< number of points needed to represent the cladding regions
  unsigned long n_points_;      //!< total number of points in the vtk representation

  // PER PLANE VALUES
  int n_sections_per_plane_;  //!< total number of radial sections in a plane
  int fuel_points_per_plane_; //!< number of fuel points in a plane
  int clad_points_per_plane_; //!< number of cladding points in a plane
  int points_per_plane_;      //!< total number of points in a plane

  // MESH ELEMENTS
  unsigned long n_fuel_elements_; //!< number of fuel elements in mesh
  unsigned long n_clad_elements_; //!< number of cladding elements in the mesh
  unsigned long n_mesh_elements_; //!< total number of mesh elements

  // CONNECTIVITY ENTRIES
  unsigned long n_fuel_entries_per_plane_; //!< number of fuel connectivity entries in a plane
  unsigned long n_clad_entries_per_plane_; //!< number of cladding connectivity entries in a plane
  unsigned long n_entries_per_plane_;      //!< total number of connectivity entries in a plane
  unsigned long n_entries_;                //!< total number of connectivity entries in a plane
};

} // stream
