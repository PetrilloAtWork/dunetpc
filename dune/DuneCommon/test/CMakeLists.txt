# dunetpc/dune/DuneCommon/test/CMakeLists.txt

include(CetTest)

cet_enable_asserts()

cet_test(test_LineColors SOURCES test_LineColors.cxx
  LIBRARIES
    ${ROOT_BASIC_LIB_LIST}
)


cet_test(test_DuneTimeConverter SOURCES test_DuneTimeConverter.cxx
  LIBRARIES
    lbne_daqinput35t
    art_Persistency_Provenance canvas
)

cet_test(test_StringManipulator SOURCES test_StringManipulator.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
  )

cet_test(test_TPadManipulator SOURCES test_TPadManipulator.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_RootPalette SOURCES test_RootPalette.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_CompactRealDftData SOURCES test_CompactRealDftData.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_DuneFFT SOURCES test_DuneFFT.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_coldelecResponse SOURCES test_coldelecResponse.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_gausTF1 SOURCES test_gausTF1.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_offsetLine SOURCES test_offsetLine.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_GausStepFitter SOURCES test_GausStepFitter.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_GausRmsFitter SOURCES test_GausRmsFitter.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)

cet_test(test_SampleTailer SOURCES test_SampleTailer.cxx
  LIBRARIES
    dune_DuneCommon
    ${ROOT_BASIC_LIB_LIST}
)
