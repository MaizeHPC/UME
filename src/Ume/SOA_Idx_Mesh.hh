/*!
  \file Ume/SOA_Idx_Mesh.hh
*/
#ifndef SOA_IDX_MESH_HH
#define SOA_IDX_MESH_HH 1

#include "Ume/Mesh_Base.hh"
#include "Ume/SOA_Entity.hh"
#include "Ume/SOA_Idx_Corners.hh"
#include "Ume/SOA_Idx_Edges.hh"
#include "Ume/SOA_Idx_Faces.hh"
#include "Ume/SOA_Idx_Points.hh"
#include "Ume/SOA_Idx_Sides.hh"
#include "Ume/SOA_Idx_Zones.hh"
#include <iosfwd>

namespace Ume {

//! A struct-of-arrays implementation using integer indexing
namespace SOA_Idx {

using Types = Ume::DS_Types::Types;

//! An Struct-Of-Arrays Mesh
struct Mesh : public Mesh_Base {
  enum Geometry_Type { CARTESIAN, CYLINDRICAL, SPHERICAL };
  int mype;
  int numpe;
  Geometry_Type geo;
  Corners corners;
  Edges edges;
  Faces faces;
  Points points;
  Sides sides;
  Zones zones;
  Mesh();
  void write(std::ostream &os) const;
  void read(std::istream &is);
  constexpr size_t ndims() const { return 3; }
  bool operator==(Mesh const &rhs) const;
  void print_stats(std::ostream &os) const;
};

} // namespace SOA_Idx
} // namespace Ume

#endif
