/*
 MoraStream.h

 Copyright (C) 2007 met.no

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

#ifndef MoraSTREAM_H
#define MoraSTREAM_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <puDatatypes/miCoordinates.h>
#include "ptDataStream.h"
#include "DynamicFunction.h"

namespace pets {

enum MoraDatatype { Mora_observation, Mora_monthly_normal };

// The old one
/*
struct MoraStation {
  std::string name;
  miCoordinates coordinates;
  int amsl;
  int distance;
  int stationid;
  int wmo;
  MoraStation() :
    stationid(0)
  {
  }
  ;
  std::string description();
  void clear();
};
*/
/* The new */
struct MoraStation {
  std::string name; // All station types
  std::string type; // Valid types, Wmo, Clim, Nat, Icao
  miCoordinates coordinates;
  std::string amsl; //stationheigth
  std::string number; // Wmo, Nat, Clim
  std::string blk; // Wmo
  std::string subtype; // Nat, Wmo
  std::string code; // Icao
  int distance; // backward kompatibility
  MoraStation()
  {
  }
  ;
  std::string description();
  std::string toString();
  void clear();
};

//PETSPAR   = namn | parameterId | statisticsFormulaId | samplingTime | unit | levelParameterId | levelFrom | levelTo : transform (-273.15, *.01)
struct MoraParameter {
  ParId parid;
  std::string moraName;
  std::string parameterId;
  std::string statisticsFormulaId;
  std::string samplingTime;
  std::string unit;
  std::string levelParameterId;
  std::string levelFrom;
  std::string levelTo;
  
  pets::math::DynamicFunction * transform;
  MoraDatatype type;

  MoraParameter() :
    transform(NULL)
  {
  }

  MoraParameter(const MoraParameter&);

  ~MoraParameter()
  {
    if (transform)
      delete transform;
  }
  MoraParameter & operator= (const MoraParameter & rhs);
};

struct MoraData {
  MoraParameter parameter;
  std::vector<float> data;
  std::vector<miutil::miTime> times;
  int col; // column in datafile

};


class MoraStream: public DataStream {
private:

  MoraStation currentStation;
  bool initialized;
  std::vector<MoraStation> stationlist;
  bool setDataFromResult(std::string & data_);
  bool setStationsFromResult(std::string & data_);
  bool setNormalFromResult(std::string& data_, miutil::miTime from, miutil::miTime to);
  int maxDistance;
  std::string host;
  std::string MONTHNORMALREPORT;
  std::string STATIONREPORT;
  std::string DATAREPORT;
  std::string getFromHttp(std::string url);
  std::map<std::string,MoraParameter> parameterDefinitions;
  std::vector<MoraParameter> moraPars;
  std::vector<MoraParameter> normalPars;
  std::map<std::string,std::string> knownMoraParameters;
  std::string createDataQuery(std::vector<MoraParameter> & moraPars, miutil::miTime fromTime,
      miutil::miTime toTime, pets::MoraDatatype type);

  std::vector<miutil::miTime>  createTimeline( miutil::miTime from, miutil::miTime to);

  std::vector<MoraData> moraData;
  std::vector<std::string> allParameters;

  // result from the last queries
  std::string cachedMonthly;
  std::string cachedData;
  std::string cachedStations;
  // we asked this once before - pull it from cache instead
  std::string cachedMonthlyQuery;
  std::string cachedDataQuery;
  std::string cachedStationQuery;



public:
  MoraStream(std::string h, std::string stationreport, std::string datareport, std::string monthlynormalreport,
    std::map<std::string, std::string> pars, std::map<std::string, std::string> norms, int maxd = 50) :
    DataStream("MORA"), initialized(false)
  {
    initialize(h, stationreport, datareport, monthlynormalreport, pars, norms, maxd);
  }
  ~MoraStream()
  {
  }
  void initialize(std::string h, std::string stationreport, std::string datareport, std::string monthlynormalreport,
    std::map<std::string, std::string> pars, std::map<std::string, std::string> norms, int maxd = 50);

  void setSingleParameterDefinition(std::string key, std::string token, pets::MoraDatatype type);

  bool read(std::string report_, std::string query = "", miutil::miTime from = miutil::miTime::nowTime(), miutil::miTime to =miutil::miTime::nowTime() );
 
  bool isInitialized() const
  {
    return initialized;
  }

  pets::MoraStation getNearestMoraStation(miCoordinates& pos);

  std::vector<std::string> getAllParameters();

  /// Inherited from DataStream ---------------------------

  int findStation(const std::string&); // find station index
  int findDataPar(const ParId&);
  void clean();
  void cleanParData();
  bool openStream(ErrorFlag*);
  bool openStreamForWrite(ErrorFlag*);
  bool readData(const int posIndex, const ParId&, const miutil::miTime&, const miutil::miTime&, ErrorFlag*);

  bool readMoraData(std::vector<ParId>& inpars, std::vector<ParId>& outpars,
      miutil::miTime fromTime, miutil::miTime toTime);
  bool getTimeLine(const int& index, std::vector<miutil::miTime>& tline, std::vector<int>& pline, ErrorFlag*);
  bool getTimeLine(const int& index, std::vector<miutil::miTime>& tline, std::vector<int>& pline);
  bool putTimeLine(const int& index, std::vector<miutil::miTime>& tline, std::vector<int>& pline, ErrorFlag*);
  bool putTimeLine(TimeLine& tl, std::vector<int>& pline, ErrorFlag*);
  bool getOnePar(int, WeatherParameter&, ErrorFlag*);
  bool getOnePar(int, WeatherParameter&);
  bool putOnePar(WeatherParameter&, ErrorFlag*);
  bool getStations(std::vector<miPosition>&);
  bool getStationSeq(int, miPosition&);
  bool getModelSeq(int, Model&, Run&, int&);
  bool getModelSeq(int, Model&, Run&, int&, std::vector<std::string>&);
  int putStation(const miPosition& s, ErrorFlag*);
  bool
  writeData(const int posIndex, const int modIndex, ErrorFlag*, bool complete_write, bool write_submodel);
  bool close();
  void getTextLines(const ParId p, std::vector<std::string>& tl);

};
}
;

#endif /* KURL_H_ */
