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


// ptParameterDefinition.cc

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ptParameterDefinition.h"

#include <fstream>
#include <algorithm>

using namespace std;
using namespace miutil;


vector<Parameter> ParameterDefinition::paramList;

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

bool ParId::wdbCompare(const ParId& lhs)
{

  if (lhs.alias != alias)
    return false;

  if(lhs.submodel != submodel ) {
    // special case! the raw submodel (dataversion 0 ) has to match exactly
    if (lhs.submodel ==  "RAW" || submodel == "RAW" )
      return false;
    // in any other case, the UNDEF
    if (lhs.submodel !=  M_UNDEF && submodel != M_UNDEF )
      return false;
  }

  if (lhs.model !=  M_UNDEF && model != M_UNDEF  && lhs.model != model )
    return false;
  return true;

}

miutil::miString ParId::toString()
{
  miString palias    =  ( alias    != A_UNDEF ? alias           : "x");
  miString plevel    =  ( level    != L_UNDEF ? miString(level) : "x");
  miString pmodel    =  ( model    != M_UNDEF ? model           : "x");
  miString prun      =  ( run      != R_UNDEF ? miString(run)   : "x");
  miString psubmodel =  ( submodel != S_UNDEF ? submodel        : "x");
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
void ParId::setFromString(miutil::miString buffer)
{
  reset();
  vector<miString> parts= buffer.split(',',true);
  int n= parts.size();
  if ( n < 1 ) return;
  if ( parts[0] != "x"    ) alias   = parts[0];
  if ( n < 2 ) return;
  if ( parts[1].isNumber()) level   = atoi(parts[1].cStr());
  if ( n < 3 ) return;
  if ( parts[2] != "x"    ) model    = parts[2];
  if ( n < 4 ) return;
  if ( parts[3].isNumber()) run      = atoi(parts[3].cStr());
  if ( n < 5 ) return;
  if ( parts[4] != "x"    ) submodel = parts[4];
}


bool ParameterDefinition::getParameter(const Alias& id, Parameter& p) const
{
  for (int i=0; i<paramList.size(); i++)
    if (strcmp(paramList[i].alias().cStr(),id.cStr())==0){
      p =  paramList[i];
      return true;
    }
  return false;
}

bool ParameterDefinition::readParameters(const miString paramdef)
{
  // Experimental reading of parameter file
  paramList.clear();
  parameter **parlist;
  int npar;

  parsepardefs(paramdef.cStr(),&parlist,&npar);
  for (int i=0; i<npar; i++) {
    paramList.push_back(Parameter(*parlist[i]));
    free(parlist[i]);
  }
  free(parlist);
  return true;
}

miString ParameterDefinition::ParId2Str(ParId p)
{
  return p.toString();
}


ParId ParameterDefinition::Str2ParId(miString buffer)
{
  ParId parid;
  parid.setFromString(buffer);
  return parid;
}

