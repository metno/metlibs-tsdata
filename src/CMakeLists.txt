
INCLUDE_DIRECTORIES(${PC_METLIBS_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})
ADD_DEFINITIONS(-W -Wall ${PC_METLIBS_CFLAGS_OTHER})
LINK_DIRECTORIES(${PC_METLIBS_LIBRARY_DIRS} ${BOOST_LIBRARY_DIRS}
  ${PC_FIMEX_LIBRARY_DIRS}
  ${PC_HDF4_LIBRARY_DIRS}
  ${PC_MORA_LIBRARY_DIRS}
  ${PC_WDB_LIBRARY_DIRS}
)

########################################################################

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
  KlimaStream.cc
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
  ${PC_METLIBS_LIBRARIES}
  ${CURL_LIBRARIES}
  ${BOOST_LIBRARIES}
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
  TARGET_INCLUDE_DIRECTORIES(tsdata-fimex PUBLIC ${PC_FIMEX_INCLUDE_DIRS})
  TARGET_COMPILE_DEFINITIONS(tsdata-fimex PUBLIC ${PC_FIMEX_CFLAGS_OTHER} ${CXXFLAG_FIMEX_NO_FORK})
  SET_TARGET_PROPERTIES(tsdata-fimex PROPERTIES
    VERSION ${TSDATA_PVERSION_FULL}
    SOVERSION "${TSDATA_VERSION_MAJOR}"
    OUTPUT_NAME "${lib_name}-fimex"
  )
  TARGET_LINK_LIBRARIES(tsdata-fimex
    tsdata-core
    ${PC_FIMEX_LIBRARIES}
  )
  INSTALL(TARGETS tsdata-fimex
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
ENDIF ()

########################################################################

IF (ENABLE_HDF4)
  SET(tsdata_hdf4_SOURCES
    ptHDFFile.cc
    ptHDFUtils.cc
  )
  METNO_HEADERS (tsdata_hdf4_HEADERS tsdata_hdf4_SOURCES ".cc" ".h")

  ADD_LIBRARY(tsdata-hdf4 SHARED ${tsdata_hdf4_SOURCES})
  TARGET_INCLUDE_DIRECTORIES(tsdata-hdf4 PUBLIC ${HDF4_INCLUDE_DIRS})
  TARGET_COMPILE_DEFINITIONS(tsdata-hdf4 PUBLIC ${HDF4_CFLAGS_OTHER})
  SET_TARGET_PROPERTIES(tsdata-hdf4 PROPERTIES
    VERSION ${TSDATA_PVERSION_FULL}
    SOVERSION "${TSDATA_VERSION_MAJOR}"
    OUTPUT_NAME "${lib_name}-hdf4"
  )
  TARGET_LINK_LIBRARIES(tsdata-hdf4
    tsdata-core
    ${HDF4_LIBRARIES}
  )
  INSTALL(TARGETS tsdata-hdf4
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
  TARGET_INCLUDE_DIRECTORIES(tsdata-mora PUBLIC ${PC_MORA_INCLUDE_DIRS})
  TARGET_COMPILE_DEFINITIONS(tsdata-mora PUBLIC ${PC_MORA_CFLAGS_OTHER})
  SET_TARGET_PROPERTIES(tsdata-mora PROPERTIES
    VERSION ${TSDATA_PVERSION_FULL}
    SOVERSION "${TSDATA_VERSION_MAJOR}"
    OUTPUT_NAME "${lib_name}-mora${METNO_QT_SUFFIX}"
  )
  TARGET_LINK_LIBRARIES(tsdata-mora
    tsdata-core
    ${PC_MORA_LIBRARIES}
    ${CURL_LIBRARIES}
    ${QT_LIBRARIES}
  )
  INSTALL(TARGETS tsdata-mora
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
ENDIF ()

########################################################################

IF (ENABLE_WDB)
  SET(tsdata_wdb_SOURCES
    WdbStream.cc
    WdbQueries.cc
    WdbCacheQuery.cc
  )
  METNO_HEADERS (tsdata_wdb_HEADERS tsdata_wdb_SOURCES ".cc" ".h")

  ADD_LIBRARY(tsdata-wdb SHARED ${tsdata_wdb_SOURCES})
  TARGET_INCLUDE_DIRECTORIES(tsdata-wdb PUBLIC ${PC_WDB_INCLUDE_DIRS})
  TARGET_COMPILE_DEFINITIONS(tsdata-wdb PUBLIC ${PC_WDB_CFLAGS_OTHER})
  SET_TARGET_PROPERTIES(tsdata-wdb PROPERTIES
    VERSION ${TSDATA_PVERSION_FULL}
    SOVERSION "${TSDATA_VERSION_MAJOR}"
    OUTPUT_NAME "${lib_name}-wdb"
  )
  TARGET_LINK_LIBRARIES(tsdata-wdb
    tsdata-core
    ${PC_WDB_LIBRARIES}
  )
  INSTALL(TARGETS tsdata-wdb
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
ENDIF ()

########################################################################

INSTALL(FILES ${tsdata_core_HEADERS} ${tsdata_fimex_HEADERS} ${tsdata_hdf4_HEADERS} ${tsdata_mora_HEADERS} ${tsdata_wdb_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/metlibs/tsData
)
