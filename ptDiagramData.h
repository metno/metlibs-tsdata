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

#ifndef diagramdata_h
#define diagramdata_h

#include "ptDataStream.h"
#include "ptWeatherParameter.h"
#include "ptTimeLine.h"
#include "ptParameterDefinition.h"
#include "ptError.h"
#include "ptSHCinfo.h"
#include "WdbStream.h"

#include <puDatatypes/miPosition.h>
#include <puTools/miString.h>
#include <puMet/symbolMaker.h>
#include <puMet/WindCalc.h>

#include <set>
#include <map>
#include <vector>
#include <iostream>

#define MERGE_RAW   0
#define MERGE_ADAPT 1

using namespace std;

//typedef vector<miString> StringVector;

struct time_interval {
  miTime from;
  miTime until;
  int step;
};

struct Range {
  int first, last;

  friend ostream& operator<<(ostream& out, const Range& rhs)
  {
    return out << rhs.first << '\t' << rhs.last;
  }
};

struct datasetparam {
  ParId dest; // destination
  ParId temp; // temporary
  ParId subj; // subjective input
  ParId prim; // primary model input
  ParId secu; // secondary model input
  ParId tert; // tertiary model input
  ParId defprim; // default primary model input
  ParId defsecu; // default secondary model input
  ParId deftert; // default tertiary model input
  bool active;
};

struct parameter_info {
  miString alias;
  float step;
  float min;
  float max;
  float def;
  bool wrap;
  bool interpok;
  bool spreadok;
  parameter_info() :
    step(1), min(-10000000), max(100000000), def(0), wrap(false),
        interpok(true), spreadok(false)
  {
  }
  parameter_info(miString a, float st, float mi, float ma, float de, bool wr,
      bool in, bool sp) :
    alias(a), step(st), min(mi), max(ma), def(de), wrap(wr), interpok(in),
        spreadok(sp)
  {
  }
  parameter_info(miString a, float de, bool in, bool sp) :
    alias(a), step(1), min(-10000000), max(10000000), def(de), wrap(false),
        interpok(in), spreadok(sp)
  {
  }

};

class ptDiagramData {
private:
  miPosition station;
  TimeLine timeLine;
  vector<ProgLine> progLines;
  int nfetches;
  vector<Range> fetchRange;
  vector<WeatherParameter> parList;
  map<miString, vector<miString> > textLines;

  WeatherParameter emptypar;
  symbolMaker wsymbols;
  bool new_symbolmaker;
  SHCinfo shcinfo;
  WindCalc windCalc;

  map<miString, parameter_info> parInfo;

  void cleanDataStructure_();
  void makeWeatherSymbols_(ParId);
  void makeWeatherSymbols_new_(ParId p, bool update);
  float calcHec_(float hst, float hs);
  float calcCMC_(float hst, float hs, float tm01, float tm02);
  bool makeSHC_(const int diridx, WeatherParameter& wp, const int level);
  void copyMembers(const ptDiagramData&);
  bool parameterInfo(const ParId&, float&, bool&, bool&);
  bool parameterInfo(const ParId&, parameter_info& info);
  void makedefaultParInfo();

public:
  ptDiagramData();
  ptDiagramData(symbolMaker& wsym);
  ptDiagramData(const ptDiagramData&);

  ptDiagramData& operator=(const ptDiagramData&);

  WeatherParameter& operator[](const int i)
  {
    if (i >= 0 && i < (int) parList.size())
      return parList[i];
    else
      return emptypar;
  }
  const WeatherParameter& operator[](const int i) const
  {
    if (i >= 0 && i < (int) parList.size())
      return parList[i];
    else
      return emptypar;
  }

  friend ostream& operator<<(ostream&, /*const*/ptDiagramData&);

  void Erase();
  int size() const
  {
    return parList.size();
  }

  const miPosition& getStation() const
  {
    return station;
  }
  void setStation(const miPosition& stat)
  {
    station = stat;
  }
  vector<miString> getAllTextLines();
  vector<miString> getTextLines(const miString modelname);

  void setSHCinfo(const SHCinfo& shc);
  void setWindCalc(const WindCalc& wc);
  bool findParameter(const ParId&, int& index, ErrorFlag*);
  bool copyParameter(int index, WeatherParameter& wp, ErrorFlag*);
  bool copyParameter(const ParId&, WeatherParameter& wp, ErrorFlag*);
  int addParameter(const WeatherParameter&);
  void deleteParameter(const ParId&);
  void deleteNegParameter(const ParId&);
  void deleteParameter(const int);
  // Clear the dirty flags of all weather parameters
  void clearDirty();
  // precipitation state
  float precip_state(float& tt, float& rr);
  // update one wp
  void UpdateOneParameter(const ParId&);
  // calculate one wp from existing ones
  void makeOneParameter(const ParId&);
  // calculate wp's from existing ones
  void makeParameters(const vector<ParId>&, bool doupdate);
  // make a wp from scratch, return index
  int makeOneParameter(const ParId&, const int, const int);
  // remove unused timepoints from the timeline
  void cleanUpTimeline();
  // add data by extrapolation
  void addData(const int&, const miTime&, const int, vector<miTime>&);
  // merge two datasets
  void mergeData(const int&, const int&, const miTime&, vector<miTime>&,
      const int method = 0);
  // interpolate wp's data to remove gaps of default values
  void interpData(const int, vector<bool>&);
  // replace wp's data for matching timepoints
  void replaceData(const int, const int, const vector<miTime> , vector<bool>&);
  // make datasets with given timepoints; add data from models
  void makeDatasets(const vector<ParId>& tempmod, const vector<ParId>& destmod,
      const vector<ParId>& subjmod, const vector<ParId>& mainmod, const vector<
          ParId>& xtramod, const vector<time_interval>&);
  // make dataset with given timepoints; add data from models
  void makeDatasets(const datasetparam&, vector<miTime>&);
  // make a vector from two components
  bool makeVector(const ParId& comp1, const ParId& comp2, const ParId& result,
      bool polar);
  // make a polar vector from two cartesian components.
  // zangle is startangle and rotsign is sign of rotation (+1=clockwise)
  bool makePolarVector(const ParId& comp1, const ParId& comp2,
      const ParId& result, const float& zangle, const int& rotsign);
  // make a cartesian vector from two polar components.
  // zangle is startangle and rotsign is sign of rotation (+1=clockwise)
  bool makeCartesianVector(const ParId& comp1, const ParId& comp2,
      const ParId& result, const float& zangle, const int& rotsign);
  // split a vector into two components
  bool splitVector(const ParId& vect, const ParId& comp1, const ParId& comp2);
  // fetch parameters specified by inpars
  bool fetchDataFromFile(DataStream*, const miPosition&, const ParId&,
      const Model&, const miTime&, const miTime&, const vector<ParId>& inpars,
      int* first, int* last, vector<ParId>& outPars, bool append, ErrorFlag*);
  // fetch all parameters
  bool fetchDataFromFile(DataStream*, const miPosition&, const ParId&,
      const Model&, const miTime&, const miTime&, int* first, int* last,
      vector<ParId>& outPars, bool append, ErrorFlag*);


 bool fetchDataFromWDB(pets::WdbStream*,float lat, float lon,
     miString model, miTime run,vector<ParId>& inpars, vector<ParId>& outpars, unsigned long& readtime,miString stationname);




  //bool fetchTextLinesFromFile(DataStream*, int* nlines);
  bool writeAllToFile(DataStream*, const miString&, ErrorFlag*);
  bool writeWeatherparametersToFile(DataStream*, const miPosition&,
      const vector<int>& wpindexes, bool append, ErrorFlag*,
      bool complete_write = true, bool write_submodel = false);
  Range getRange(int i)
  {
    return fetchRange[i];
  }
  void getTimeLine(vector<miTime>& timePoints);
  void getTimeLine(const int idx, vector<miTime>& timePoints);
  void getTimeLine(const miTime& start, const miTime& stop,
      vector<int>& indexes, vector<miTime>& timePoints, int skip = 1);
  int addTimeLine(vector<miTime>&); // return index of the new timeLine
  void deleteTimeLine(int);
  bool getProgLine(int index, vector<int>& prog, ErrorFlag*);
  bool addTimePoint(const miTime& tp, int tlIndex, ErrorFlag*);

  bool tobeplotted(int i, int j)
  {
    return timeLine.flag(j, parList[i].TimeLineIndex());
  }
  void useAlternateSymbolmaker(const bool use);
  set<Model> allModels();
};

#endif
