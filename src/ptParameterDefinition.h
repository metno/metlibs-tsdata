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


// ptParameterDefinition.h

#ifndef _parameterdefinition_h
#define _parameterdefinition_h

#include "ptParameter.h"
#include <iosfwd>
#include <vector>
#include <string>

typedef std::string Alias;
typedef int Level;
typedef std::string Model;
typedef int Run;
typedef std::string SubModel;

extern const Alias    A_UNDEF;
extern const Level    L_UNDEF;
extern const Model    M_UNDEF;
extern const Run      R_UNDEF;
extern const SubModel S_UNDEF;

struct ParId {
  Alias alias;
  Level level;
  Model model;
  Run run;
  SubModel submodel;

  ParId(Alias a =A_UNDEF, Level l =L_UNDEF,
	Model m =M_UNDEF, Run r =R_UNDEF, SubModel s =S_UNDEF)
    : alias(a),
      level(l),
      model(m),
      run(r),
      submodel(s)
  {}

  bool wdbCompare(const ParId& lhs);

  void reset();
  friend bool operator<(const ParId& lhs, const ParId& rhs);
  friend bool operator>(const ParId& lhs, const ParId& rhs);
  friend bool operator==(const ParId& lhs, const ParId& rhs);
  friend bool operator!=(const ParId& lhs, const ParId& rhs)
  { return !(lhs==rhs);}


  friend std::ostream& operator<<(std::ostream& out, const ParId& pi);

  std::string toString();
  void setFromString(const std::string&);
};

extern const ParId ID_UNDEF;


// ParameterDefinition is a class defining a mapping between the ParId enum
// values and the corresponding parameter (numerical and alpha numerical)
// values
class ParameterDefinition {
private:
  static std::vector<Parameter> paramList;

public:
  ParameterDefinition(){}
  static bool readParameters(const std::string&);
  bool getParameter(const Alias& id, Parameter& p) const;
  std::string ParId2Str(ParId);
  ParId Str2ParId(const std::string&);
};

#endif
