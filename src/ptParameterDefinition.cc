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

#include <ptParameterDefinition.h>

#include <fstream>
#include <algorithm>

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

miString ParameterDefinition::ParId2Str(ParId p){
  miString tmp;
  miString alias("x"),level("x"),model("x"),
    run("x"),submodel("x");

  if (p.alias!=A_UNDEF) 
    alias= p.alias;
  if (p.level!=L_UNDEF)
    level= miString(p.level);
  if (p.model!=M_UNDEF)
    model= p.model;
  if (p.run!=R_UNDEF)
    run= miString(p.run);
  if (p.submodel!=S_UNDEF)
    run= miString(p.submodel);

  tmp= alias + "," + level + "," + model + "," + run + "," + submodel;

  return tmp;
}
		
		
ParId ParameterDefinition::Str2ParId(miString buffer)
{
  ParId parid = ID_UNDEF;
  vector<miString> parts= buffer.split(',');
  int n= parts.size();
  if (n > 0){
    parts[0].trim();
    if (parts[0]!="x")
      parid.alias = parts[0];
    if (n > 1){
      if (parts[1].isNumber())
	parid.level = atoi(parts[1].cStr());
      if (n > 2){
	parts[2].trim();
	if (parts[2]!="x")
	  parid.model = parts[2];
	if (n > 3){
	  if (parts[3].isNumber())
	    parid.run = atoi(parts[3].cStr());
	  if (n > 4){
	    parts[4].trim();
	    if (parts[4]!="x")
	      parid.submodel = parts[4];
	  }
	}
      }
    }
  }
  return parid;
}
		
