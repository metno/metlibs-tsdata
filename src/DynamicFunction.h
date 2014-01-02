
#ifndef DYNAMICFUNCTION_H
#define DYNAMICFUNCTION_H

/*
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

#include <string>


namespace pets {
namespace math {


class DynamicFunction {
protected:
  double           factor;
  DynamicFunction *next;
  std::string      txt;

public:
  DynamicFunction(std::string nf, double f=0);
  virtual void calc(double& res);
  ~DynamicFunction() { delete next;}
  std::string text() const { return txt;}
  double      getFactor() const { return factor;}
};


class Add : public DynamicFunction {
public:
  Add(std::string nf, double f=0) : DynamicFunction(nf,f) {}
  void calc(double& res); 
};

class Divide : public DynamicFunction {
public:
  Divide(std::string nf, double f=0) : DynamicFunction(nf,f) {}
  void calc(double& res); 
};

class Multiply : public DynamicFunction {
public:
  Multiply(std::string nf, double f=0) : DynamicFunction(nf,f) {}
  void calc(double& res); 
};

class Substract : public DynamicFunction {
public:
  Substract(std::string nf, double f=0) : DynamicFunction(nf,f) {}
  void calc(double& res); 
};    

}};





#endif
