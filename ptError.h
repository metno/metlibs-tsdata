/*
  libtsData - Time Series Data
  
  $Id$

  Copyright (C) 2006 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: diana@met.no
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef error_h
#define error_h

enum ErrorFlag {
  OK = 0,

  MEMORY_ALLOC_ERROR = 10,

  // numbers 100 .... 199 reserved for file errors
  DF_FILE_OPEN_ERROR = 101,
  DF_STATION_NOT_FOUND = 102,
  DF_MODEL_NOT_FOUND = 103,
  DF_PARAMETER_NOT_FOUND = 104,
  DF_FILE_ACCESS_ERROR = 105,  
  DF_DATA_READING_ERROR = 106,
  DF_PARAMETERLIST_NOT_FOUND = 107,
  DF_POSITIONLIST_NOT_FOUND = 108,
  DF_MODELLIST_NOT_FOUND = 109,

  DF_TIMELINE_NOT_READ = 110,
  DF_DATA_NOT_READ = 111,
  DF_RANGE_ERROR = 112,

  // numbers 200 ..... 299 reserved for DiagramData errors
  DD_RANGE_ERROR = 201,
  DD_NO_PARAMETERS_FOUND = 202,
  DD_SOME_PARAMETERS_NOT_FOUND = 203,
  DD_PARAMETER_NOT_FOUND
};
  


#endif
