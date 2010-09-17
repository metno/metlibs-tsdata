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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ptPrimitiveType.h"

using namespace std;
using namespace miutil;


ptPrimitiveType Str2Primitive(miString buffer){
  miString prim = buffer.upcase();
  prim.trim(true,true);

  if (prim=="DUM_PRIMITIVE") return DUM_PRIMITIVE;
  else if (prim=="LINE") return LINE;
  else if (prim=="DOUBLE_LINE") return DOUBLE_LINE;
  else if (prim=="MULTI_LINE") return MULTI_LINE;
  else if (prim=="EDITLINE") return EDITLINE;
  else if (prim=="VECTOR") return VECTOR;
  else if (prim=="WIND_VECTOR") return WIND_VECTOR;
  else if (prim=="HIST") return HIST;
  else if (prim=="AXISHIST") return AXISHIST;
  else if (prim=="CLOUD") return CLOUDBOX;
  else if (prim=="TABLE") return TABLE;
  else if (prim=="GRIDLINE") return GRIDLINE;
  else if (prim=="TEXT") return TEXT;
  else if (prim=="STAT") return STAT;
  else if (prim=="XAXIS") return XAXIS;
  else if (prim=="YAXIS") return YAXIS;
  else if (prim=="YAXIS_STATIC") return YAXIS_STATIC;
  else if (prim=="PROG") return PROG;
  else if (prim=="DATE") return DATE;
  else if (prim=="UTC") return UTC;
  else if (prim=="DAY") return DAY;
  else if (prim=="SYMBOL") return SYMBOL;
  else if (prim=="TIMEMARKER") return TIMEMARKER;
  else if (prim=="TIMEBOX") return TIMEBOX;
  else if (prim=="INTERVAL") return INTERVAL;
  else if (prim=="QBOX") return QBOX;
  else if (prim=="LOGO") return LOGO;
  else if (prim=="DONT_PLOT") return DONT_PLOT;
  else {
    //cout << "OOPS..ukjent plottetype:"<<prim<<endl;
    return DUM_PRIMITIVE;
  }
}

miString Primitive2Str(ptPrimitiveType prim){
  if (prim==DUM_PRIMITIVE) return "DUM_PRIMITIVE";
  else if (prim==LINE) return "LINE";
  else if (prim==DOUBLE_LINE) return "DOUBLE_LINE";
  else if (prim==MULTI_LINE) return "MULTI_LINE";
  else if (prim==EDITLINE) return "EDITLINE";
  else if (prim==VECTOR) return "VECTOR";
  else if (prim==WIND_VECTOR) return "WIND_VECTOR";
  else if (prim==HIST) return "HIST";
  else if (prim==AXISHIST) return "AXISHIST";
  else if (prim==CLOUDBOX) return "CLOUD";
  else if (prim==TABLE) return "TABLE";
  else if (prim==GRIDLINE) return "GRIDLINE";
  else if (prim==TEXT) return "TEXT";
  else if (prim==STAT) return "STAT";
  else if (prim==XAXIS) return "XAXIS";
  else if (prim==YAXIS) return "YAXIS";
  else if (prim==YAXIS_STATIC) return "YAXIS_STATIC";
  else if (prim==PROG) return "PROG";
  else if (prim==DATE) return "DATE";
  else if (prim==UTC) return "UTC";
  else if (prim==DAY) return "DAY";
  else if (prim==SYMBOL) return "SYMBOL";
  else if (prim==TIMEMARKER) return "TIMEMARKER";
  else if (prim==TIMEBOX) return "TIMEBOX";
  else if (prim==INTERVAL) return "INTERVAL";
  else if (prim==QBOX) return "QBOX";
  else if (prim==LOGO) return "LOGO";
  else if (prim==DONT_PLOT) return "DONT_PLOT";
  else {
    //cout << "OOPS..ukjent plottetype:"<<prim<<endl;
    return "DUM_PRIMITIVE";
  }
}

bool filePrimitive(ptPrimitiveType prim){
  if (prim==DUM_PRIMITIVE)     return false;
  else if (prim==LINE)         return true;
  else if (prim==DOUBLE_LINE)  return true;
  else if (prim==MULTI_LINE)   return true;
  else if (prim==EDITLINE)     return true;
  else if (prim==VECTOR)       return true;
  else if (prim==WIND_VECTOR)  return true;
  else if (prim==HIST)         return true;
  else if (prim==AXISHIST)     return true;
  else if (prim==CLOUDBOX)     return true;
  else if (prim==TABLE)        return true;
  else if (prim==GRIDLINE)     return false;
  else if (prim==TEXT)         return false;
  else if (prim==STAT)         return false;
  else if (prim==XAXIS)        return false;
  else if (prim==YAXIS)        return false;
  else if (prim==YAXIS_STATIC) return false;
  else if (prim==PROG)         return false;
  else if (prim==DATE)         return false;
  else if (prim==UTC)          return false;
  else if (prim==DAY)          return false;
  else if (prim==SYMBOL)       return true;
  else if (prim==TIMEMARKER)   return false;
  else if (prim==TIMEBOX)      return true;
  else if (prim==INTERVAL)     return false;
  else if (prim==QBOX)         return true;
  else if (prim==DONT_PLOT)    return false;
  else return false;
}

int zOrder(const ptPrimitiveType p){
  int zorder= 10;
  switch (p) {
  case DUM_PRIMITIVE:zorder=10; break;
  case GRIDLINE:     zorder= 0; break;
  case TIMEMARKER:   zorder= 1; break;
  case YAXIS:        zorder= 2; break;
  case YAXIS_STATIC: zorder= 2; break;
  case DOUBLE_LINE:  zorder= 3; break;
  case MULTI_LINE:   zorder= 3; break;
  case HIST:         zorder= 4; break;
  case AXISHIST:     zorder= 4; break;
  case LINE:         zorder= 5; break;
  case EDITLINE:     zorder= 6; break;
  case VECTOR:       zorder= 0; break;
  case WIND_VECTOR:  zorder= 0; break;
  case CLOUDBOX:     zorder= 0; break;
  case TABLE:        zorder= 0; break;
  case TEXT:         zorder=10; break;
  case STAT:         zorder= 0; break;
  case XAXIS:        zorder= 2; break;
  case PROG:         zorder= 0; break;
  case DATE:         zorder= 0; break;
  case UTC:          zorder= 0; break;
  case DAY:          zorder= 0; break;
  case SYMBOL:       zorder= 0; break;
  case TIMEBOX:      zorder= 0; break;
  case INTERVAL:     zorder= 0; break;
  case QBOX:         zorder= 5; break;
  default: break;
  }
  return zorder;
}

