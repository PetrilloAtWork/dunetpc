art_make(
    MODULE_LIBRARIES 
	  dune_Geometry
    larcore_Geometry_Geometry_service
    larsim_Simulation nug4_ParticleNavigation lardataobj_Simulation
	  larsim_MCCheater_BackTrackerService_service
    larsim_MCCheater_ParticleInventoryService_service
    lardata_Utilities
    lardataobj_RawData
    lardataobj_RecoBase
    lardata_RecoObjects
	  larana_OpticalDetector
	  larana_OpticalDetector_OpHitFinder
	  larcorealg_Geometry
#	  LArProperties_service
	  RunHistoryDUNE
	  lbne-raw-data_Overlays
          nusimdata_SimulationBase
  	  artdaq-core_Utilities
          ${LARRECO_LIB}
          ${LARDATA_LIB}
          ${ART_FRAMEWORK_CORE}
          ${ART_FRAMEWORK_PRINCIPAL}
          ${ART_FRAMEWORK_SERVICES_REGISTRY}
          ${ART_ROOT_IO_TFILE_SUPPORT} ${ROOT_CORE}
          ${ART_ROOT_IO_TFILESERVICE_SERVICE}
          art_Persistency_Common canvas
          art_Persistency_Provenance canvas
          art_Utilities canvas
          ${MF_MESSAGELOGGER}
          
          ${FHICLCPP}
          cetlib cetlib_except
          ${ROOT_BASIC_LIB_LIST}
        )

# install_headers()
install_fhicl()
install_source()
install_scripts()
