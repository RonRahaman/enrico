#ifndef MAGNOLIA_HEAT_XFER_BACKEND_H
#define MAGNOLIA_HEAT_XFER_BACKEND_H

void solve_steady_nonlin(double *source, double T_co, double *r_grid_fuel,
  double *r_grid_clad, std::size_t n_fuel_rings, std::size_t n_clad_rings,
  double tol, double *T);

#endif // MAGNOLIA_HEAT_XFER_BACKEND_H
