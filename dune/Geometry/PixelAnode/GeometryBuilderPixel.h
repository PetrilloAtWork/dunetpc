/**
 * @file   dune/Geometry/PixelAnode/GeometryBuilderPixel.h
 * @brief  Pixel-based geometry extractor for DUNE.
 * @author Gianluca Petrillo (petrillo@slac.stanford.edu)
 * @date   November 5, 2019
 * @see    `dune/Geometry/PixelAnode/GeometryBuilderPixel.cxx`
 */

#ifndef DUNE_GEOMETRY_PIXELANODE_GEOMETRYBUILDERPIXEL_H
#define DUNE_GEOMETRY_PIXELANODE_GEOMETRYBUILDERPIXEL_H

// LArSoft libraries
#include "larcorealg/Geometry/GeometryBuilderStandard.h"

// C/C++ standard libraries
#include <vector>
#include <memory> // std::unique_ptr<>



//------------------------------------------------------------------------------
namespace dune { class GeometryBuilderPixel; }
  
/**
 * @brief Geometry builder using `dune::PixelPlaneGeo` for sensitive planes.
 * 
 * This builder works like `geo::GeometryBuilderSquarePixel`, with the exception
 * that it uses `dune::PixelPlaneGeo` instead of `geo::PixelPlaneGeo` objects
 * to represent the pixel readout anode planes.
 */
class dune::GeometryBuilderPixel: public geo::GeometryBuilderStandard {
  
    public:
  
  struct Config: public geo::GeometryBuilderStandard::Config {
    
    using Name = fhicl::Name;
    using Comment = fhicl::Comment;
    
    // so far, only the standard configuration is needed
    
  }; // struct Config
  
  
  /// Constructor: uses the specified configuration.
  GeometryBuilderPixel(Config const& config);
  
  //
  // we don't expand the public interface here
  //
  
    protected:
  
  // --- BEGIN Plane information ---------------------------------------------
  /// @name Pixel plane information
  /// @{

  /// Create a `geo::PixelPlane` for each plane volume in the geometry.
  virtual PlanePtr_t doMakePlane(Path_t& path) override;

  /// @}
  // --- END Plane information -----------------------------------------------


  // --- BEGIN Wire information ----------------------------------------------
  /// @name Wire information
  /// @{
  
  /// Core implementation of `extractWires()`: no wires returned whatsoever.
  virtual Wires_t doExtractWires(Path_t&) override { return {}; }
  
  /// @}
  // --- END Wire information ------------------------------------------------
  
  
}; // class dune::GeometryBuilderPixel


// -----------------------------------------------------------------------------

#endif // DUNE_GEOMETRY_PIXELANODE_GEOMETRYBUILDERPIXEL_H
