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


// This class is an old wrapper around parameter.h from libparameter.
// It is assumed that libparameter and the lex-based setup parser
// is not in use anymore. libparameter reads input from parameter.def files.
// This is part of the old pose system which is replaced by metgdata and 
// should only be usefull for the metdat->hdf translation process.

#ifndef _ptParameter_h
#define _ptParameter_h

#include <string>

#define PARNSZ 32  /* Parameter name */
#define UNITSZ 10  /* Unit */
#define ALIASZ 16  /* Short parameter name */
#define ROBSSZ  6  /* Name for observations */

class Parameter {
  int num_;
  std::string name_;
  std::string alias_;
  std::string unit_;
  int  scale_;
  int  size_;
  int  order_;
  unsigned datatype_;
  unsigned plottype_;

public:
  Parameter() {}

  std::string alias() const;
  std::string name() const;
  std::string unit() const;
  int num() const;
  int scale() const;
  int size() const;
  int order() const;
  int datatype() const;
  int plottype() const;
  void printName() const;
};

#endif
