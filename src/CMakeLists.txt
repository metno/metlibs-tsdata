
SET(tsdata_core_SOURCES
  ptAsciiStream.cc
  ptDataStream.cc
  ptDiagramData.cc
  ptError.cc
  ptParameter.cc
  ptMIROS22File.cc
  ptParameterDefinition.cc
  ptPrimitiveType.cc
  ptSHCinfo.cc
  ptWeatherParameter.cc
  DynamicFunction.cc
)

METNO_HEADERS (tsdata_core_HEADERS tsdata_core_SOURCES ".cc" ".h")
LIST(APPEND tsdata_core_HEADERS
  tsDataVersion.h
  PetsExceptions.h
  ptTimeLine.h
)

########################################################################

ADD_LIBRARY(tsdata-core SHARED
  ${tsdata_core_SOURCES}
)

SET_TARGET_PROPERTIES(tsdata-core PROPERTIES
  VERSION ${TSDATA_PVERSION_FULL}
  SOVERSION "${TSDATA_VERSION_MAJOR}"
  OUTPUT_NAME "${lib_name}-core"
)

TARGET_LINK_LIBRARIES(tsdata-core
  ${metlibs_PACKAGE}
  ${curl_PACKAGE}
  ${boost_PACKAGE}
)

INSTALL(TARGETS tsdata-core
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

########################################################################

IF (ENABLE_FIMEX)
  SET(tsdata_fimex_SOURCES
    FimexStream.cc
    FimexTools.cc
  )
  METNO_HEADERS (tsdata_fimex_HEADERS tsdata_fimex_SOURCES ".cc" ".h")

  SET(ENABLE_FIMEX_FORK TRUE CACHE BOOL "Enable fork to fetch fimex data in parallel")
  IF (NOT ENABLE_FIMEX_FORK)
    SET(CXXFLAG_FIMEX_NO_FORK "-DTSDATA_FIMEX_DISABLE_FORK=1")
  ENDIF ()

  ADD_LIBRARY(tsdata-fimex SHARED ${tsdata_fimex_SOURCES})
  TARGET_COMPILE_DEFINITIONS(tsdata-fimex PUBLIC ${CXXFLAG_FIMEX_NO_FORK})
  SET_TARGET_PROPERTIES(tsdata-fimex PROPERTIES
    VERSION ${TSDATA_PVERSION_FULL}
    SOVERSION "${TSDATA_VERSION_MAJOR}"
    OUTPUT_NAME "${lib_name}-fimex"
  )
  TARGET_LINK_LIBRARIES(tsdata-fimex
    tsdata-core
    ${fimex_PACKAGE}
  )
  INSTALL(TARGETS tsdata-fimex
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
ENDIF ()

########################################################################

IF (ENABLE_SMHI_MORA)
  SET(tsdata_mora_SOURCES
    SMHIMoraStream.cc
  )
  METNO_HEADERS (tsdata_mora_HEADERS tsdata_mora_SOURCES ".cc" ".h")

  ADD_LIBRARY(tsdata-mora SHARED ${tsdata_mora_SOURCES})
  SET_TARGET_PROPERTIES(tsdata-mora PROPERTIES
    VERSION ${TSDATA_PVERSION_FULL}
    SOVERSION "${TSDATA_VERSION_MAJOR}"
    OUTPUT_NAME "${lib_name}-mora${METNO_QT_SUFFIX}"
  )
  TARGET_LINK_LIBRARIES(tsdata-mora
    tsdata-core
    ${curl_PACKAGE}
    ${QT_LIBRARIES}
  )
  INSTALL(TARGETS tsdata-mora
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
ENDIF ()

########################################################################

INSTALL(FILES ${tsdata_core_HEADERS} ${tsdata_fimex_HEADERS} ${tsdata_mora_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/metlibs/tsData
)
