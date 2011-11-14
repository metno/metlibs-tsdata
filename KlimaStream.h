/*
 KlimaStream.h

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

#ifndef KLIMASTREAM_H
#define KLIMASTREAM_H

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <puDatatypes/miCoordinates.h>
#include "ptDataStream.h"
#include "DynamicFunction.h"

namespace pets {

struct KlimaStation {
  std::string name;
  miCoordinates coordinates;
  int amsl;
  int distance;
  int stationid;
  int wmo;
  KlimaStation() :
    stationid(0)
  {
  }
  ;
  std::string description();
  void clear();
};


struct KlimaParameter {
  ParId parid;
  std::string klimaName;
  pets::math::DynamicFunction * transform;


  KlimaParameter() :
    transform(NULL)
  {
  }

  KlimaParameter(const KlimaParameter&);

  ~KlimaParameter()
  {
    if (transform)
      delete transform;
  }
  KlimaParameter & operator= (const KlimaParameter & rhs);
};

struct KlimaData {
  KlimaParameter parameter;
  std::vector<float> data;
  std::vector<miutil::miTime> times;
  int col; // column in datafile

};


class KlimaStream: public DataStream {
private:

  KlimaStation currentStation;
  bool initialized;
  std::list<KlimaStation> stationlist;
  bool setDataFromResult(std::vector<std::string> & data, std::vector<std::string>& header);
  bool setStationsFromResult(std::vector<std::string>& data, std::vector<std::string>& header);
  int maxDistance;
  std::string host;
  std::vector<std::string> getFromHttp(std::string url);
  std::map<std::string,KlimaParameter> parameterDefinitions;
  std::map<std::string,std::string> knownKlimaParameters;
  std::string createDataQuery(std::vector<std::string> klimaNames, miutil::miTime fromTime,
      miutil::miTime toTime);

  std::vector<KlimaData> klimaData;

public:
  KlimaStream(std::string h, std::map<std::string, std::string> pars, int maxd = 50) :
    DataStream("KLIMA"), initialized(false)
  {
    initialize(h, pars, maxd);
  }
  ~KlimaStream()
  {
  }
  void initialize(std::string h, std::map<std::string, std::string> pars, int maxd = 50);
  bool read(std::string report_, std::string query = "");
  // see documentation for flaglevel
  // at http://metklim.met.no/klima/userservices/urlinterface/brukerdok#observasjoner
  // default=5
  void setFlagLevel(int newFlagLevel);
  bool isInitialized() const
  {
    return initialized;
  }

  pets::KlimaStation getNearestKlimaStation(miCoordinates& pos);

  /// Inherited from DataStream ---------------------------

  int findStation(const miutil::miString&); // find station index
  int findDataPar(const ParId&);
  void clean();
  void cleanParData();
  bool openStream(ErrorFlag*);
  bool openStreamForWrite(ErrorFlag*);
  bool readData(const int posIndex, const ParId&, const miutil::miTime&, const miutil::miTime&, ErrorFlag*);

  bool readKlimaData(std::vector<ParId>& inpars, std::vector<ParId>& outpars,
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
  bool getModelSeq(int, Model&, Run&, int&, std::vector<miutil::miString>&);
  int putStation(const miPosition& s, ErrorFlag*);
  bool
  writeData(const int posIndex, const int modIndex, ErrorFlag*, bool complete_write, bool write_submodel);
  bool close();
  void getTextLines(const ParId p, std::vector<miutil::miString>& tl);

};
}
;

#endif /* KURL_H_ */
