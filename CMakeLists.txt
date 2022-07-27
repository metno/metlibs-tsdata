PROJECT(metlibs-tsdata)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(MetnoUtils)
INCLUDE(MetnoQtUtils)

METNO_PVERSION_DEFINES(TSDATA "src/tsDataVersion.h")

SET(ENABLE_FIMEX     TRUE CACHE BOOL "Enable Fimex support")
SET(ENABLE_SMHI_MORA TRUE CACHE BOOL "Enable SMHI Mora support")

SET (PKGS_CORE
  metlibs-putools>=8.0.0
  metlibs-pudatatypes>=6.0.0
  metlibs-pumet>=7.0.0
  metlibs-puctools>=6.0.0
  metlibs-milogger>=6.0.0
)
PKG_CHECK_MODULES(PC_METLIBS REQUIRED ${PKGS_CORE})

IF (ENABLE_FIMEX)
  SET (PKGS_FIMEX fimex>=1.7)
  PKG_CHECK_MODULES(PC_FIMEX REQUIRED ${PKGS_FIMEX})
ENDIF ()

IF (ENABLE_SMHI_MORA)
  METNO_FIND_QT(BUILDONLY Xml)
ENDIF ()

FIND_PACKAGE(CURL)

FIND_PACKAGE(Boost COMPONENTS date_time system REQUIRED)

SET(lib_name "metlibs-tsdata")

METNO_GEN_PKGCONFIG(metlibs-tsdata-core.pc.in "${lib_name}-core.pc"
  "${PKGS_CORE}" "" "" "")
IF (ENABLE_FIMEX)
  METNO_GEN_PKGCONFIG(metlibs-tsdata-fimex.pc.in "${lib_name}-fimex.pc"
  "${PKGS_FIMEX}" "" "" "")
  SET (TSDATA_PC_FIMEX ", metlibs-tsdata-fimex = ${TSDATA_PVERSION_FULL}")
ENDIF()
IF (ENABLE_SMHI_MORA)
  METNO_GEN_PKGCONFIG(metlibs-tsdata-mora.pc.in "${lib_name}-mora${METNO_QT_SUFFIX}.pc"
    "" "" "" "")
  SET (TSDATA_PC_SMHI_MORA ", metlibs-tsdata-mora${METNO_QT_SUFFIX} = ${TSDATA_PVERSION_FULL}")
ENDIF ()

CONFIGURE_FILE(tsData.pc.in tsData.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/tsData.pc DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig")

ADD_SUBDIRECTORY(src)
