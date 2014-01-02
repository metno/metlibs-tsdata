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


#ifndef weatherparameter_h
#define weatherparameter_h

#include "ptPrimitiveType.h"
#include "ptParameterDefinition.h"

#include <math.h>
#include <float.h>

#include <vector>

const float UNDEF = FLT_MAX;
const int MAXDIM = 100;

class WeatherParameter
{
protected:
  std::vector<float> data;   // the data-points
  std::vector<float> cmin;   // data minimum-value
  std::vector<float> cmax;   // data maximum-value
  std::vector<float> cdelta; // data cmax-cmin
  bool polar;           // if ndim > 1, true if in polar coordinates
  ParId id;             // parameter id
  ptPrimitiveType type; // suggested plotElement type
  int ndim;             // number of vector dimensions
  int npoints;          // number of distinct data points, if ndim == 1,
                        // npoints == data.size()
  int timeLineIndex;    // index to correct timeLine

  bool dirty;           // dirty flags for editing etc.
  bool temp_dirty;
  std::vector<bool> modified;// timepoints modified
  bool locked;          // WP "locked" for data-modification
  int starti;           // start index
  int stopi;            // stop index

  // used by assignment operator and copy constructor
  void _copyDataMembers(const WeatherParameter& rhs);
  void setNpoints(const int n) { npoints=n; }

public:
  WeatherParameter(); // constructor
  virtual ~WeatherParameter();// destructor
  WeatherParameter(const WeatherParameter& rhs);// copy constructor

  // retrieve values of WP variables
  const bool& Polar() const { return polar; }
  const ParId& Id() const { return id; }
  const ptPrimitiveType& Type() const { return type; }
  const int& Ndim() const { return ndim; }
  const int& Npoints() const { return npoints; }
  const int& TimeLineIndex() const { return timeLineIndex; }
  const float& Cmin(const int c) const ;
  const float& Cmax(const int c) const ;
  const float& Cdelta(const int c) const ;
  const bool& isDirty() const {return dirty; }
  const bool& isTempDirty() const {return temp_dirty; }
  const bool& isLocked() const {return locked; }

  // assignment of WP variables
  void setPolar(const bool p){ polar=p; }
  void setId(const ParId i) { id = i; }
  void setType(const ptPrimitiveType t) { type=t; }
  void setDims(const int nt, const int nd);
  void setTimeLineIndex(const int ti) { timeLineIndex=ti; }
  void setDirty(const bool b) {dirty=b; temp_dirty=b; }
  void clearTempDirty();
  void setLocked(const bool b) {locked=b; }

  // total size of data-vector
  int size() const { return npoints*ndim;}
  // retrieve a copy of the data-vector
  virtual const std::vector<float>& copyDataVector() const
    { return data; }
  // calc. data's min, max and delta for one component
  virtual void calcCompProperties(const int icomp);
  // ...for all components
  void calcAllProperties();
  // change a data-value, index i
  void setData(const int i, const float val);
  // change a data-value, time-step i, component comp
  void setData(const int i, const int comp, const float val);
  // retrieve data-value, index i
  virtual const float& Data(const int i) const
  {
    if (i>=0 && i<(int)data.size())
      return data[i];
    else return UNDEF;
  }
  // retrieve data-value, time-step i, component comp
  virtual const float& Data(const int i, const int comp) const
  {
    if (i>=0 && i<npoints && comp>=0 && comp<ndim)
      return data[npoints*comp + i];
    else return UNDEF;
  }
  // check if one value has been modified
  bool isModified(const int i, const int comp) const
  {
    if (i>=0 && i<npoints && comp>=0 && comp<ndim)
      return modified[npoints*comp + i];
    else return false;
  }
  bool isModified(const int i) const
  {
    if (i>=0 && i<(int)modified.size())
      return modified[i];
    else return false;
  }
  void setTimeInterval(const int start, const int stop);
  // assignment operator
  WeatherParameter& operator=(const WeatherParameter& rhs);
  // equalness operator
  bool operator==(const WeatherParameter& rhs);
  // ostream operator
  friend std::ostream& operator<<(std::ostream& out, const WeatherParameter& wp);
};


//
//
// class CompositeParameter : public WeatherParameter {
// protected:
//   vector<WeatherParameter*> wp;
//   bool wpexist;

//   void _copyDataMembers(const CompositeParameter& rhs);
//   virtual const float& datafunc(const int i, const int comp) const
//     {
//       if (wpexist && i>=0 && i<npoints && comp<ndim && comp>=0)
// 	return wp[0]->Data(i,comp);
//       else return UNDEF;
//     }

// public:
//   CompositeParameter() : WeatherParameter(),
//     wpexist(false)
//     {}
//   ~CompositeParameter(){}
//   CompositeParameter(const CompositeParameter& rhs);
//   CompositeParameter& operator=(const CompositeParameter& rhs);

//   virtual void setWPvector(const vector<WeatherParameter*>& w);
//   // retrieve data-value, index i
//   const float& Data(const int i) const
//     {
//       int comp= i/npoints;
//       int idx= i - (comp*npoints);
//       cout << "Data(" << i << ") - Idx:"
// 	   << idx << " Comp:" << comp << endl;
//       return datafunc(idx,comp);
//     }
//   const float& Data(const int i, const int comp) const
//     {return datafunc(i,comp); }
//   // calc. data's min, max and delta for one component
//   void calcCompProperties(const int icomp);
//   // retrieve a copy of the data-vector
//   const vector<float>& copyDataVector() const;
// };


// class vectorParameter : public CompositeParameter {
// protected:
//   virtual const float& datafunc(const int i, const int comp) const
//     {
//       if (wpexist && comp<2 && comp>=0){
// 	cout << "VP: Data point:" << wp[comp]->Data(i,0) << endl;
// 	return wp[comp]->Data(i,0);
//       } else return UNDEF;
//     }

// public:
//   virtual void setWPvector(const vector<WeatherParameter*>& w)
//     {
//       wp = w;
//       if (wp.size()==2) {
// 	cout << "VP: Heisan - riktig storrelse" << endl;
// 	wpexist = true;
// 	setNdim(2);
// 	setNpoints(wp[0]->Npoints());
// 	setTimeLineIndex(wp[0]->TimeLineIndex());
// 	cout << "VP: ndim,npoints,timeline:"<<ndim<<" "
// 	     << npoints << " " << timeLineIndex << endl;
//       }
//     }
// };

#endif
