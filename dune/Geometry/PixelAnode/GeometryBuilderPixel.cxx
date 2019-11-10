/**
 * @file   dune/Geometry/PixelAnode/GeometryBuilderPixel.cxx
 * @brief  Pixel-based geometry extractor for DUNE.
 * @author Gianluca Petrillo (petrillo@slac.stanford.edu)
 * @date   November 5, 2019
 * @see    `dune/Geometry/PixelAnode/GeometryBuilderPixel.h`
 */

// library header
#include "dune/Geometry/PixelAnode/GeometryBuilderPixel.h"

// DUNE libraries
#include "dune/Geometry/PixelAnode/PixelPlaneGeo.h"

// support libraries
#include "messagefacility/MessageLogger/MessageLogger.h"


//------------------------------------------------------------------------------
dune::GeometryBuilderPixel::GeometryBuilderPixel(Config const& config)
  : geo::GeometryBuilderStandard(config)
{
  MF_LOG_TRACE("GeometryBuilder")
    << "Loading geometry builder: dune::GeometryBuilderPixel";
} // dune::GeometryBuilderPixel::GeometryBuilderPixel()


//------------------------------------------------------------------------------
auto dune::GeometryBuilderPixel::doMakePlane(Path_t& path) -> PlanePtr_t {
  return std::make_unique<dune::PixelPlaneGeo>(
    path.current(), path.currentTransformation<geo::TransformationMatrix>()
    );
} // dune::GeometryBuilderPixel::doMakePlane()


//------------------------------------------------------------------------------

