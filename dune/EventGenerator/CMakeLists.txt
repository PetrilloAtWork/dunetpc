include_directories( $ENV{IFDHC_FQ_DIR}/inc )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
art_make( BASENAME_ONLY
          MODULE_LIBRARIES 
                        larcoreobj_SummaryData
                        larcorealg_Geometry
                        larcore_Geometry_Geometry_service
                        nurandom_RandomUtils_NuRandomService_service
                        larsim_Simulation lardataobj_Simulation
                        nusimdata_SimulationBase
                        ${ART_FRAMEWORK_CORE}
                        ${ART_FRAMEWORK_PRINCIPAL}
                        ${ART_FRAMEWORK_SERVICES_REGISTRY}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
                        ${ART_FRAMEWORK_SERVICES_OPTIONAL_RANDOMNUMBERGENERATOR_SERVICE}
                        art_Persistency_Common
                        art_Persistency_Provenance
                        art_Utilities canvas
                        ${MF_MESSAGELOGGER}
                        hep_concurrency
                        ${FHICLCPP}
                        cetlib cetlib_except
                        ${CLHEP}
                        ${ROOT_BASIC_LIB_LIST}
                        ${SQLITE3}
                        ${ROOT_EG}
                        ${IFDH}
                        dune_EventGenerator_ProtoDUNEbeamDataProducts
 )


install_headers()
install_fhicl()
install_source()

add_subdirectory(CRY)
add_subdirectory(GENIE)
add_subdirectory(Radiological)
add_subdirectory(CORSIKAprotodunedp)
add_subdirectory(NEUT)
add_subdirectory(MUSUN)
add_subdirectory(NuE)
add_subdirectory(ProtoDUNEbeamDataProducts)
