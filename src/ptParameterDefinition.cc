/*
  libtsData - Time Series Data

  Copyright (C) 2006-2016 met.no

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


// ptParameterDefinition.cc

#include "ptParameterDefinition.h"

#include <puTools/miStringFunctions.h>

#include <algorithm>
#include <fstream>

using namespace std;
using namespace miutil;

const Alias    A_UNDEF= "x";
const Level    L_UNDEF=  INT_MIN;
const Model    M_UNDEF= "x";
const Run      R_UNDEF=  INT_MIN;
const SubModel S_UNDEF= "x";

vector<Parameter> ParameterDefinition::paramList;

std::ostream& operator<<(std::ostream& out, const ParId& pi)
{
    return out << " alias: " << pi.alias << " level: " << pi.level
               << " model: " << pi.model << " run: " << pi.run
               << " submodel: " << pi.submodel;
}

bool operator==(const ParId& lhs, const ParId& rhs) {
  if (lhs.submodel != S_UNDEF && lhs.submodel != rhs.submodel)
    return false;
  if (lhs.level != L_UNDEF && lhs.level != rhs.level)
    return false;
  if (lhs.run != R_UNDEF && lhs.run != rhs.run)
    return false;
  if (lhs.model != M_UNDEF && lhs.model != rhs.model)
    return false;
  if (lhs.alias != A_UNDEF && lhs.alias != rhs.alias)
    return false;
  return true;
}

bool operator<(const ParId& lhs, const ParId& rhs)
{
  if (lhs.submodel < rhs.submodel)
    return true;
  if (lhs.level < rhs.level)
    return true;
  if (lhs.run < rhs.run)
    return true;
  if (lhs.model < rhs.model)
    return true;
  if (lhs.alias < rhs.alias)
    return true;
  return false;
}

bool operator>(const ParId& lhs, const ParId& rhs)
{
  if (lhs.submodel > rhs.submodel)
    return true;
  if (lhs.level > rhs.level)
    return true;
  if (lhs.run > rhs.run)
    return true;
  if (lhs.model > rhs.model)
    return true;
  if (lhs.alias > rhs.alias)
    return true;
  return false;
}

std::string ParId::toString()
{
  const std::string palias    =  ( alias    != A_UNDEF ? alias    : "x");
  const std::string plevel    =  ( level    != L_UNDEF ? miutil::from_number(level) : "x");
  const std::string pmodel    =  ( model    != M_UNDEF ? model    : "x");
  const std::string prun      =  ( run      != R_UNDEF ? miutil::from_number(run) : "x");
  const std::string psubmodel =  ( submodel != S_UNDEF ? submodel : "x");
  return palias + "," + plevel + "," + pmodel + "," + prun + "," + psubmodel;
}

void  ParId::reset()
{
  alias    = A_UNDEF;
  level    = L_UNDEF;
  model    = M_UNDEF;
  run      = R_UNDEF;
  submodel = S_UNDEF;

}
void ParId::setFromString(const std::string& buffer)
{
  reset();

  std::vector<std::string> parts = miutil::split(buffer, ",", true);
  int n= parts.size();

  if ( n < 1 ) return;
  if ( parts[0] != "x"    ){
    alias   = parts[0];
  }
  if ( n < 2 ) return;
  if (miutil::is_number(parts[1])){
    level   = miutil::to_int(parts[1]);
  }
  if ( n < 3 ) return;
  if ( parts[2] != "x"    ){
    model    = parts[2];
  }
  if ( n < 4 ) return;
  if (miutil::is_number(parts[3])) {
    run      = miutil::to_int(parts[3]);
  }
  if ( n < 5 ) return;
  if ( parts[4] != "x"    ){
    submodel = parts[4];
  }
}


bool ParameterDefinition::getParameter(const Alias& id, Parameter& p) const
{
  for (int i=0; i<(int)paramList.size(); i++)
    if (paramList[i].alias() == id ){
      p =  paramList[i];
      return true;
    }
  return false;
}

std::string ParameterDefinition::ParId2Str(ParId p)
{
  return p.toString();
}

ParId ParameterDefinition::Str2ParId(const std::string& buffer)
{
  ParId parid;
  parid.setFromString(buffer);
  return parid;
}

const ParId ID_UNDEF;
