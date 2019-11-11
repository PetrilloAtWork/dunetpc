#
# art-independent library
#
cet_make(
  EXCLUDE
    "GeometryBuilderDUNEPixelTool_tool.cc"
  LIBRARIES
    larcorealg_Geometry
    larcorealg_CoreUtils
    ${MF_MESSAGELOGGER}
    ${FHICLCPP}
    ${CETLIB}
    ${CETLIB_EXCEPT}
    ${ROOT_CORE}
    ${ROOT_PHYSICS}
    ${ROOT_GEOM}
    ${ROOT_GENVECTOR}
  )

#
# art plugins
#
simple_plugin(GeometryBuilderDUNEPixelTool "tool"
  dune_Geometry_PixelAnode
  larcorealg_Geometry
  )

#
# all the rest
#
install_headers()
install_fhicl()
install_source()
