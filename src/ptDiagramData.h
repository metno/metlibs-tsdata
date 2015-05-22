// -*- c++ -*-
/*
 libtsData - Time Series Data

 Copyright (C) 2013 met.no

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
#include "KlimaStream.h"
#include "FimexStream.h"
#include <puDatatypes/miPosition.h>
#include <puMet/symbolMaker.h>
#include <puMet/WindCalc.h>

#include <set>
#include <map>
#include <vector>
#include <iosfwd>

#define MERGE_RAW   0
#define MERGE_ADAPT 1

//typedef std::vector<miString> StringVector;

struct time_interval {
  miutil::miTime from;
  miutil::miTime until;
  int step;
};

struct Range {
  int first, last;

  friend std::ostream& operator<<(std::ostream& out, const Range& rhs)
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
  std::string alias;
  float step;
  float min;
  float max;
  float def;
  bool wrap;
  bool interpok;
  bool spreadok;
  parameter_info() :
    step(1), min(-10000000), max(100000000), def(0), wrap(false), interpok(true), spreadok(false)
  {
  }
  parameter_info(const std::string& a, float st, float mi, float ma, float de, bool wr, bool in, bool sp) :
    alias(a), step(st), min(mi), max(ma), def(de), wrap(wr), interpok(in), spreadok(sp)
  {
  }
  parameter_info(const std::string& a, float de, bool in, bool sp) :
    alias(a), step(1), min(-10000000), max(10000000), def(de), wrap(false), interpok(in), spreadok(sp)
  {
  }

};

class ptDiagramData {
private:
  miPosition station;
  TimeLine timeLine;
  std::vector<ProgLine> progLines;
  int nfetches;
  std::vector<Range> fetchRange;
  std::vector<WeatherParameter> parList;
  std::map<std::string, std::vector<std::string> > textLines;

  WeatherParameter emptypar;
  symbolMaker wsymbols;
  bool new_symbolmaker;
  SHCinfo shcinfo;
  WindCalc windCalc;

  std::map<std::string, parameter_info> parInfo;

  void cleanDataStructure_();
  void makeWeatherSymbols_(ParId);
  void makeYrWeatherSymbols(ParId);
  void makeWeatherSymbols_new_(ParId p, bool update);
  float calcMedianTM01(float hst);
  float calcMedianTM02(float hst);
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

  friend std::ostream& operator<<(std::ostream&, /*const*/ptDiagramData&);

  void Erase();
  int size() const
  {
    return parList.size();
  }

  int timeLineLengthInHours()  { return timeLine.lengthInHours(); }

  miutil::miTime timelineEnd()   { return timeLine.endTime(); }
  miutil::miTime timelineBegin() { return timeLine.startTime();}

  const miPosition& getStation() const
  {
    return station;
  }
  void setStation(const miPosition& stat)
  {
    station = stat;
  }
  std::vector<std::string> getAllTextLines();
  std::vector<std::string> getTextLines(const std::string& modelname);

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
  void makeParameters(const std::vector<ParId>&, bool doupdate);
  // make a wp from scratch, return index
  int makeOneParameter(const ParId&, const int, const int);
  // remove unused timepoints from the timeline
  void cleanUpTimeline();
  // add data by extrapolation
  void addData(const int&, const miutil::miTime&, const int, std::vector<miutil::miTime>&);
  // merge two datasets
  void mergeData(const int&, const int&, const miutil::miTime&, std::vector<miutil::miTime>&,
      const int method = 0);
  // interpolate wp's data to remove gaps of default values
  void interpData(const int, std::vector<bool>&);
  // replace wp's data for matching timepoints
  void replaceData(const int, const int, const std::vector<miutil::miTime>, std::vector<bool>&);
  // make datasets with given timepoints; add data from models
  void makeDatasets(const std::vector<ParId>& tempmod, const std::vector<ParId>& destmod, const std::vector<
      ParId>& subjmod, const std::vector<ParId>& mainmod, const std::vector<ParId>& xtramod,
      const std::vector<time_interval>&);
  // make dataset with given timepoints; add data from models
  void makeDatasets(const datasetparam&, std::vector<miutil::miTime>&);
  // make a vector from two components
  bool makeVector(const ParId& comp1, const ParId& comp2, const ParId& result, bool polar);
  // make a polar vector from two cartesian components.
  // zangle is startangle and rotsign is sign of rotation (+1=clockwise)
  bool makePolarVector(const ParId& comp1, const ParId& comp2, const ParId& result, const float& zangle,
      const int& rotsign);
  // make a cartesian vector from two polar components.
  // zangle is startangle and rotsign is sign of rotation (+1=clockwise)
  bool makeCartesianVector(const ParId& comp1, const ParId& comp2, const ParId& result, const float& zangle,
      const int& rotsign);
  // split a vector into two components
  bool splitVector(const ParId& vect, const ParId& comp1, const ParId& comp2);
  // fetch parameters specified by inpars
  bool fetchDataFromFile(DataStream*, const miPosition&, const ParId&, const Model&, const miutil::miTime&,
      const miutil::miTime&, const std::vector<ParId>& inpars, int* first, int* last,
      std::vector<ParId>& outPars, bool append, ErrorFlag*);
  // fetch all parameters
  bool fetchDataFromFile(DataStream*, const miPosition&, const ParId&, const Model&, const miutil::miTime&,
      const miutil::miTime&, int* first, int* last, std::vector<ParId>& outPars, bool append, ErrorFlag*);

  bool fetchDataFromWDB(pets::WdbStream*, float lat, float lon, const std::string& model, miutil::miTime run,
      std::vector<ParId>& inpars, std::vector<ParId>& outpars, unsigned long& readtime,
      const std::string& stationname);

  bool fetchDataFromKlimaDB(pets::KlimaStream* klima, std::vector<ParId>& inpars,
      std::vector<ParId>& outpars, miutil::miTime fromTime, miutil::miTime toTime);
  bool fetchDataFromFimex(pets::FimexStream* fimex, double lat, double lon, miutil::miString fimexname,
      std::vector<ParId>& inpars, std::vector<ParId>& outpars);

  //bool fetchTextLinesFromFile(DataStream*, int* nlines);
  bool writeAllToFile(DataStream*, const std::string&, ErrorFlag*);
  bool writeWeatherparametersToFile(DataStream*, const miPosition&, const std::vector<int>& wpindexes,
      bool append, ErrorFlag*, bool complete_write = true, bool write_submodel = false);
  Range getRange(int i)
  {
    return fetchRange[i];
  }
  void getTimeLine(std::vector<miutil::miTime>& timePoints);
  void getTimeLine(const int idx, std::vector<miutil::miTime>& timePoints);
  void getTimeLine(const miutil::miTime& start, const miutil::miTime& stop, std::vector<int>& indexes,
      std::vector<miutil::miTime>& timePoints, int skip = 1);
  int addTimeLine(const std::vector<miutil::miTime>&); // return index of the new timeLine
  void deleteTimeLine(int);
  bool getProgLine(int index, std::vector<int>& prog, ErrorFlag*);
  bool addTimePoint(const miutil::miTime& tp, int tlIndex, ErrorFlag*);

  bool tobeplotted(int i, int j)
  {
    return timeLine.flag(j, parList[i].TimeLineIndex());
  }
  void useAlternateSymbolmaker(const bool use);
  std::set<Model> allModels();
};

#endif
