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


#ifndef _ptPrimitiveType_h
#define _ptPrimitiveType_h

#include <puTools/miString.h>

enum ptPrimitiveType {
  DUM_PRIMITIVE,// 0
  LINE,         // 1
  DOUBLE_LINE,  // 2
  MULTI_LINE,   // 3
  EDITLINE,     // 4
  VECTOR,       // 5
  WIND_VECTOR,  // 6
  HIST,         // 7
  CLOUDBOX,     // 8
  TABLE,        // 9
  GRIDLINE,     //10
  TEXT,         //11
  STAT,         //12
  XAXIS,        //13
  YAXIS,        //14
  YAXIS_STATIC, //15
  PROG,         //16
  DATE,         //17
  UTC,          //18
  DAY,          //19
  SYMBOL,       //20
  TIMEMARKER,   //21
  AXISHIST,     //22
  LOGO,         //23
  TIMEBOX,      //24
  INTERVAL,     //25
  QBOX,         //26
  DONT_PLOT     //27
};


extern bool filePrimitive(ptPrimitiveType);
extern int zOrder(const ptPrimitiveType);
extern ptPrimitiveType Str2Primitive(miutil::miString);
extern miutil::miString Primitive2Str(ptPrimitiveType);

#endif
