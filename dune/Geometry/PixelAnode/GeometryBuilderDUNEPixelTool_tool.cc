/**
 * @file   dune/Geometry/PixelAnode/GeometryBuilderDUNEPixelTool_tool.cc
 * @brief  Tool to create a `dune::GeometryBuilderPixel` geometry builder.
 * @author Gianluca Petrillo (petrillo@slac.stanford.edu)
 * @date   November 5, 2019
 * @see    `dune/Geometry/PixelAnode/GeometryBuilderPixel.h`
 */

// DUNE libraries
#include "dune/Geometry/PixelAnode/GeometryBuilderPixel.h"

// LArSoft libraries
#include "larcore/Geometry/GeometryBuilderTool.h"

// framework libraries
#include "art/Utilities/ToolConfigTable.h"
#include "art/Utilities/ToolMacros.h"


// -----------------------------------------------------------------------------
namespace dune { class GeometryBuilderDUNEPixelTool; }

/**
 * @brief Tool to create a builder for DUNE pixel detectors.
 * 
 * This tool creates a `dune::GeometryBuilderPixel` geometry builder.
 * 
 * It can be used in the configuration of `geo::Geometry` service.
 * 
 */
class dune::GeometryBuilderDUNEPixelTool: public geo::GeometryBuilderTool {
  
  /// The type of builder being created.
  using Builder_t = dune::GeometryBuilderPixel;
  
    public:
  
  using Parameters = art::ToolConfigTable<Builder_t::Config>;
  
  
  /// Constructor: passes all parameters to the geometry algorithm.
  GeometryBuilderDUNEPixelTool(Parameters const& config);
  
  
    protected:
  
  std::unique_ptr<Builder_t> fBuilder;
  
  
  // --- BEGIN -- Virtual interface implementation ---------------------------
  /// @name Virtual interface
  /// @{
  
  /// Returns a pointer to the geometry builder object.
  virtual std::unique_ptr<geo::GeometryBuilder> doMakeBuilder() override
    { return std::move(fBuilder); }
  
  /// @}
  // --- END -- Virtual interface implementation -----------------------------
  
  
  /// Creates and returns the geometry builder.
  std::unique_ptr<Builder_t> createBuilder
    (Builder_t::Config const& config) const;
  
}; // class dune::GeometryBuilderDUNEPixelTool


// -----------------------------------------------------------------------------
// ---  Implementation
// -----------------------------------------------------------------------------
dune::GeometryBuilderDUNEPixelTool::GeometryBuilderDUNEPixelTool
  (Parameters const& config)
  : fBuilder(createBuilder(config()))
{}


// -----------------------------------------------------------------------------
auto dune::GeometryBuilderDUNEPixelTool::createBuilder
  (Builder_t::Config const& config) const -> std::unique_ptr<Builder_t>
{
  return std::make_unique<Builder_t>(config);
} // dune::GeometryBuilderDUNEPixelTool::createBuilder()



// -----------------------------------------------------------------------------
DEFINE_ART_CLASS_TOOL(dune::GeometryBuilderDUNEPixelTool)


// -----------------------------------------------------------------------------
