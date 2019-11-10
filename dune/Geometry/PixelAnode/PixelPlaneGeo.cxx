/**
 * @file    dune/Geometry/PixelAnode/PixelPlaneGeo.cxx
 * @brief   Readout plane for a DUNE pixel detector.
 * @author  Gianluca Petrillo (petrillo@slac.stanford.edu)
 * @date    November 5, 2019
 * @see     `dune/Geometry/PixelAnode/PixelPlaneGeo.h`
*/

// class header
#include "dune/Geometry/PixelAnode/PixelPlaneGeo.h"

// C/C++ standard library
#include <utility> // std::move()


// -----------------------------------------------------------------------------
// --- dune::PixelPlaneGeo
// -----------------------------------------------------------------------------
std::regex const dune::PixelPlaneGeo::DefaultPixelPattern {
  ".*pixel.*",
  std::regex::basic | std::regex::icase | std::regex::optimize
  };


// -----------------------------------------------------------------------------
dune::PixelPlaneGeo::PixelPlaneGeo(
  TGeoNode const& node, geo::TransformationMatrix&& trans,
  std::regex const& pixelNamePattern /* = DefaultPixelPattern */
  )
  : geo::PixelPlaneGeoBase(node, std::move(trans))
{

  /*
   * REMINDER: do not rely on virtual methods from derived classes here, as
   *           they might not be available yet (the derived class constructor
   *           hasn't been run yet at this point)
   */
  
  RectPixelGeometry_t pixelGeometry
    = geo::PixelPlaneGeoBase::ReadPixelGeometryFromMetadata(node);
  pixelGeometry = geo::PixelPlaneGeoBase::ExtractPixelGeometry
    (node, pixelNamePattern, pixelGeometry);
  
  // use the base class standard initialization for pixel geometry:
  geo::PixelPlaneGeoBase::InitializePixelGeometry(pixelGeometry);
  
} // dune::PixelPlaneGeo::PixelPlaneGeo()


// -----------------------------------------------------------------------------
