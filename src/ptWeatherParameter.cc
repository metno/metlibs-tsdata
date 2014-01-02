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


/***************************************************************************
 * SYSTEM:         PETS - Presentation and Editing of Time Series
 * MODULE:
 * FILE:           ptWeatherParameter.cc
 * VERSION:        0.1 alpha, 1.5
 * RELEASE DATE:
 * RELEASED BY:    met.no/FoU '97
 * SPEC. REF:
 * PLATFORM:
 * BUILD:
 * DESCRIPTION:    Definition of WeatherParameter member functions
 * CLASS(ES):      WeatherParameter
 * ROUTINES:
 * AUTHOR:
 * UPDATES:
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "ptWeatherParameter.h"

using namespace std;

// constructor
WeatherParameter::WeatherParameter()
  : polar(false)
  , type(DUM_PRIMITIVE)
  , ndim(0)
  , npoints(0)
  , timeLineIndex(0)
  , dirty(false)
  , temp_dirty(false)
  , locked(false)
  , starti(0), stopi(0)
{
}

// destructor
WeatherParameter::~WeatherParameter()
{
}

/****************************************************************************
 * ROUTINE:          WeatherParameter::_copyDataMembers()
 * PURPOSE:          copy all data members except pointer values from rhs to
 *                   this
 * ALGORITHM:
 * ARGUMENTS:        const WeatherParameter& rhs
 *
 * RETURN VALUES:
 *
 * USES/DEPENDS ON:
 *
 * UPDATES:          Written 05.06.97
 ***************************************************************************/
void WeatherParameter::_copyDataMembers(const WeatherParameter& rhs)
{
//   cout << "Inside WeatherParameter._copyDataMembers" << endl;
  data = rhs.data;
  cmin= rhs.cmin;
  cmax= rhs.cmax;
  cdelta= rhs.cdelta;
  polar = rhs.polar;
  id = rhs.id;
  type = rhs.type;
  ndim = rhs.ndim;
  npoints = rhs.npoints;
  timeLineIndex = rhs.timeLineIndex;
  dirty= rhs.dirty;
  temp_dirty= rhs.temp_dirty;
  modified= rhs.modified;
  locked= rhs.locked;
  starti= rhs.starti;
  stopi= rhs.stopi;
}

/****************************************************************************
 * ROUTINE:          ptWeatherParameter copy constructor
 * PURPOSE:
 * ALGORITHM:
 * ARGUMENTS:
 *
 * RETURN VALUES:
 *
 * USES/DEPENDS ON:
 *
 * UPDATES:
 ***************************************************************************/
WeatherParameter::WeatherParameter(const WeatherParameter& rhs)
{
//   cout << "Inside WeatherParameter.copy-constructor" << endl;
  _copyDataMembers(rhs);
}

/****************************************************************************
 * ROUTINE:          assignment operator
 * PURPOSE:
 * ALGORITHM:
 * ARGUMENTS:
 *
 * RETURN VALUES:
 *
 * USES/DEPENDS ON:
 *
 * UPDATES:
 ***************************************************************************/
WeatherParameter& WeatherParameter::operator=(const WeatherParameter& rhs)
{
//   cout << "Inside WeatherParameter.equal-operator" << endl;
  if (this == &rhs)
    return *this;

  // elementwise copy
  _copyDataMembers(rhs);

  return *this;
}


/****************************************************************************
 * ROUTINE:          equal operator
 * PURPOSE:
 * ALGORITHM:
 * ARGUMENTS:
 *
 * RETURN VALUES:
 *
 * USES/DEPENDS ON:
 *
 * UPDATES:
 ***************************************************************************/
bool WeatherParameter::operator==(const WeatherParameter& rhs)
{
  return id == rhs.id;
}


/****************************************************************************
 * ROUTINE:          ostream operator
 * PURPOSE:
 * ALGORITHM:
 * ARGUMENTS:
 *
 * RETURN VALUES:
 *
 * USES/DEPENDS ON:
 *
 * UPDATES:
 ***************************************************************************/
ostream& operator<<(ostream& out, const WeatherParameter& wp)
{
  out << "Parameter ID: " << wp.id << " type: " << wp.type << endl
      << "timeLineIndex: " << wp.timeLineIndex << " ndim: " << wp.ndim
      << " npoints: " << wp.npoints << endl;

  for (int i=0; i<wp.ndim; i++)
    out << i << ".comp> min: " << wp.cmin[i] << " max: " << wp.cmax[i]
	<< " delta: " << wp.cdelta[i] << endl;
  out << "Datavalues:" << endl;
  for (size_t i=0;i<wp.data.size();i++)
    out << wp.data[i] << '\t';
  return out;
}

void WeatherParameter::setTimeInterval(const int start, const int stop)
{
  if ( starti != start || stopi != stop){
    starti = start;
    stopi  = stop;
    calcAllProperties();
  }
}


void WeatherParameter::calcCompProperties(const int icomp)
{
  if (icomp<0 || icomp>=ndim) return;
  float dv;
  cmin[icomp] = FLT_MAX;
  cmax[icomp] = -FLT_MAX;
  int i1 = 0;
  int i2 = npoints-1;
  if (starti != stopi){
    if (starti >= 0 && starti < npoints) i1 = starti;
    if (stopi >= 0 && stopi < npoints) i2 = stopi;
    // if no datapoints, keep at least one!
    if (starti == stopi && stopi < npoints-1)
      stopi++;
  }
  for (int j=i1; j<=i2; j++) {
    dv = data[j+icomp*npoints];
    if (dv < cmin[icomp]) cmin[icomp] = dv;
    if (dv > cmax[icomp]) cmax[icomp] = dv;
  }
  cdelta[icomp] = cmax[icomp] - cmin[icomp];
}

void WeatherParameter::calcAllProperties()
{
  for (int i=0; i<ndim; i++)
    calcCompProperties(i);
}

const float& WeatherParameter::Cmin(const int c) const {
  return ((c>=0 && c<ndim)?cmin[c]:UNDEF);
}

const float& WeatherParameter::Cmax(const int c) const {
  return ((c>=0 && c<ndim)?cmax[c]:UNDEF);
}

const float& WeatherParameter::Cdelta(const int c) const {
  return ((c>=0 && c<ndim)?cdelta[c]:UNDEF);
}

void WeatherParameter::setDims(const int nt, const int nd)
{
  if (nd>MAXDIM) ndim=MAXDIM; else ndim=nd;
  npoints=nt;
  data.clear();
  cmin.clear();
  cmax.clear();
  cdelta.clear();
  modified.clear();

  for (int i=0; i<ndim; i++){
    cmin.push_back(FLT_MAX);
    cmax.push_back(-FLT_MAX);
    cdelta.push_back(FLT_MAX);
    for (int j=0; j<npoints; j++){
      data.push_back(UNDEF);
      modified.push_back(false);
    }
  }
}

void WeatherParameter::setData(const int i, const float val)
{
  if (i>=0 && i<data.size()){
    float oldd= data[i];
    if (fabs(oldd-val)>0.000001){
      data[i] = val;
      setDirty(true);
      modified[i]= true;
    }
  }
}

void WeatherParameter::setData(const int i, const int comp,
			       const float val)
{
  if (i>=0 && i<npoints && comp>=0 && comp<ndim){
    float oldd= data[comp*npoints + i];
    if (fabs(oldd-val)>0.000001){
      data[comp*npoints + i] = val;
      setDirty(true);
      modified[comp*npoints + i] = true;
    }
  }
}

void WeatherParameter::clearTempDirty()
{
  temp_dirty=false;
  std::fill(modified.begin(), modified.end(), false);
}

//==========================================================

// void CompositeParameter::_copyDataMembers(const CompositeParameter& rhs)
// {
//   cout << "Inside CompositeParameter._copyDataMembers" << endl;
//   WeatherParameter::_copyDataMembers(rhs);
//   wp = rhs.wp;
//   wpexist = rhs.wpexist;
// }

// CompositeParameter::CompositeParameter(const CompositeParameter& rhs)
// {
//   cout << "Inside CompositeParameter.copy-constructor" << endl;
//   _copyDataMembers(rhs);
// }

// CompositeParameter& CompositeParameter::operator=(const CompositeParameter& rhs)
// {
//   cout << "Inside CompositeParameter.equal-operator" << endl;
//   if (this == &rhs)
//     return *this;

//   // elementwise copy
//   _copyDataMembers(rhs);

//   return *this;
// }


// void CompositeParameter::calcCompProperties(const int icomp)
// {
//   if (icomp>=ndim || !wpexist) return;

//   float dv;
//   cmin[icomp] = FLT_MAX;
//   cmax[icomp] = -FLT_MAX;
//   for (int j=0; j<npoints; j++) {
//     dv = datafunc(j,icomp);
//     if (dv < cmin[icomp]) cmin[icomp] = dv;
//     if (dv > cmax[icomp]) cmax[icomp] = dv;
//   }
//   cdelta[icomp] = cmax[icomp] - cmin[icomp];
// }


// void CompositeParameter::setWPvector(const vector<WeatherParameter*>& w)
// {
//   wp = w;
//   if (wp.size()) {
//     wpexist = true;
//     setPolar(wp[0]->Polar());
//     setId(wp[0]->Id());
//     setType(wp[0]->Type());
//     setNdim(wp[0]->Ndim());
//     setNpoints(wp[0]->Npoints());
// //     setDims(wp[0]->Npoints(),wp[0]->Ndim());
//     setTimeLineIndex(wp[0]->TimeLineIndex());
//   }
// }


// const vector<float>& CompositeParameter::copyDataVector() const
// {
//   vector<float> tempvec;
//   if (wpexist)
//     for (int i=0; i<ndim; i++)
//       for (int j=0; j<npoints; j++)
// 	tempvec.push_back(datafunc(j,i));

//   return tempvec;
// }


