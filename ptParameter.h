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

#include <string.h>
#include <puTools/miString.h>

#include <parameter/parameter.h>

class Parameter {
  parameter* par;

public:
  Parameter() {
    par=0;
  }

  Parameter(const parameter& p) {
    par=new parameter;

    par->num=p.num;
    strcpy(par->name,p.name);
    strcpy(par->alias,p.alias);
    strcpy(par->unit,p.unit);
  //  strcpy(par->robsname,p.robsname);
    par->scale=p.scale;
    par->size=p.size;
    par->order=p.order;
    par->datatype=p.datatype;
    par->plottype=p.plottype;
  }

  Parameter(const Parameter& p) {
    par=new parameter;

    par->num=p.par->num;
    strcpy(par->name,p.par->name);
    strcpy(par->alias,p.par->alias);
    strcpy(par->unit,p.par->unit);
    strcpy(par->robsname,p.par->robsname);
    par->scale=p.par->scale;
    par->size=p.par->size;
    par->order=p.par->order;
    par->datatype=p.par->datatype;
    par->plottype=p.par->plottype;
  }

  Parameter& operator=(const Parameter& p) {
    if (this!=&p) {
      delete par;
      par=new parameter;

      par->num=p.par->num;
      strcpy(par->name,p.par->name);
      strcpy(par->alias,p.par->alias);
      strcpy(par->unit,p.par->unit);
      strcpy(par->robsname,p.par->robsname);
      par->scale=p.par->scale;
      par->size=p.par->size;
      par->order=p.par->order;
      par->datatype=p.par->datatype;
      par->plottype=p.par->plottype;
    }
    return *this;
  }

  Parameter& operator=(const parameter& p) {
    delete par;
    par=new parameter;

    par->num=p.num;
    strcpy(par->name,p.name);
    strcpy(par->alias,p.alias);
    strcpy(par->unit,p.unit);
    strcpy(par->robsname,p.robsname);
    par->scale=p.scale;
    par->size=p.size;
    par->order=p.order;
    par->datatype=p.datatype;
    par->plottype=p.plottype;

    return *this;
  }

  ~Parameter() {
    delete par;
  }

  miutil::miString alias() const {
    return par->alias;
  }
  miutil::miString name() const {
    return par->name;
  }
  miutil::miString unit() const {
    return par->unit;
  }
  miutil::miString robsname() const {
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

  void printName() const {
    std::cout << par->name << std::endl;
  }
};

#endif
