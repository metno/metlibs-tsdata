PROJECT(metlibs-tsdata)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(MetnoUtils)
INCLUDE(MetnoQtUtils)

METNO_PVERSION_DEFINES(TSDATA "src/tsDataVersion.h")

SET(ENABLE_HDF4      TRUE CACHE BOOL "Enable HDF4 support")
SET(ENABLE_FIMEX     TRUE CACHE BOOL "Enable Fimex support")
SET(ENABLE_WDB       TRUE CACHE BOOL "Enable WDB support")
SET(ENABLE_SMHI_MORA TRUE CACHE BOOL "Enable SMHI Mora support")

SET (PKGS_CORE
  metlibs-putools>=8.0.0
  metlibs-pudatatypes>=6.0.0
  metlibs-pumet>=7.0.0
)
PKG_CHECK_MODULES(PC_METLIBS REQUIRED ${PKGS_CORE} metlibs-puctools>=6.0.0)

IF (ENABLE_HDF4)
  #SET (PKGS_HDF4 )
  #PKG_CHECK_MODULES(PC_HDF4 REQUIRED ${PKGS_HDF4})

  FIND_PATH(HDF4_INCLUDE_DIR hdf.h HINTS /usr/include/hdf)
  IF(NOT HDF4_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "HDF4 Support specified, cannot find file hdf.h")
  ELSE()
    SET(HDF4_INCLUDE_DIRS  ${HDF4_INCLUDE_DIR})
  ENDIF()

  FIND_LIBRARY(HDF4_DF_LIB NAMES df libdf hdf)
  IF(NOT HDF4_DF_LIB)
    MESSAGE(FATAL_ERROR "Can't find or link to the hdf4 df library.")
  ENDIF()

  FIND_LIBRARY(HDF4_MFHDF_LIB NAMES mfhdf libmfhdf)
  IF(NOT HDF4_MFHDF_LIB)
    MESSAGE(FATAL_ERROR "Can't find or link to the hdf4 mfhdf library.")
  ENDIF()

  SET(HDF4_LIBRARIES ${HDF4_DF_LIB} ${HDF4_MFHDF_LIB})
  SET(HDF4_PC_LIBRARIES "${HDF4_DF_LIB} ${HDF4_MFHDF_LIB}")

  MESSAGE(STATUS "HDF4 include dirs: ${HDF4_INCLUDE_DIRS}")
  MESSAGE(STATUS "HDF4 libraries: ${HDF4_DF_LIB}, ${HDF4_MFHDF_LIB}")
ENDIF ()

IF (ENABLE_FIMEX)
  SET (PKGS_FIMEX fimex)
  PKG_CHECK_MODULES(PC_FIMEX REQUIRED ${PKGS_FIMEX})
ENDIF ()

IF (ENABLE_WDB)
  SET (PKGS_WDB libpqxx)
  PKG_CHECK_MODULES(PC_WDB REQUIRED ${PKGS_WDB})
ENDIF ()

IF (ENABLE_SMHI_MORA)
  METNO_FIND_QT(BUILDONLY Xml)
ENDIF ()

FIND_PACKAGE(CURL)

FIND_PACKAGE(Boost COMPONENTS date_time system REQUIRED)

SET(lib_name "metlibs-tsdata")

METNO_GEN_PKGCONFIG(metlibs-tsdata-core.pc.in "${lib_name}-core.pc"
  "${PKGS_CORE}" "" "" "")
METNO_GEN_PKGCONFIG(metlibs-tsdata-fimex.pc.in "${lib_name}-fimex.pc"
  "${PKGS_FIMEX}" "" "" "")
METNO_GEN_PKGCONFIG(metlibs-tsdata-hdf4.pc.in "${lib_name}-hdf4.pc"
  "" "" "" "")
METNO_GEN_PKGCONFIG(metlibs-tsdata-mora.pc.in "${lib_name}-mora${METNO_QT_SUFFIX}.pc"
  "" "" "" "")
METNO_GEN_PKGCONFIG(metlibs-tsdata-wdb.pc.in "${lib_name}-wdb.pc"
  "${PKGS_WDB}" "" "" "")

CONFIGURE_FILE(tsData.pc.in tsData.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/tsData.pc DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig")

ADD_SUBDIRECTORY(src)
