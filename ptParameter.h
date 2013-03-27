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


#ifndef _ptParameter_h
#define _ptParameter_h

#include <parameter/parameter.h>
#include <string>

class Parameter {
  parameter* par;

public:
  Parameter() {
    par=0;
  }

  Parameter(const parameter& p);
  Parameter(const Parameter& p);

  ~Parameter();

  Parameter& operator=(const Parameter& p);
  Parameter& operator=(const parameter& p);

  std::string alias() const {
    return par->alias;
  }
  std::string name() const {
    return par->name;
  }
  std::string unit() const {
    return par->unit;
  }
  std::string robsname() const {
    return par->robsname;
  }
  int num() const {
    return par->num;
  }
  int scale() const {
    return par->scale;
  }
  int size() const {
    return par->size;
  }
  int order() const {
    return par->order;
  }
  int datatype() const {
    return par->datatype;
  }
  int plottype() const {
    return par->plottype;
  }

  void printName() const;
};

#endif
