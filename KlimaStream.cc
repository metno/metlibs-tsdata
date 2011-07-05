/*
 KlimaStream.cc

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

//"http://klapp.oslo.dnmi.no/metnopub/production/metno?ct=text/plain&del=semicolon"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>

#include <curl/curl.h>

#include "KlimaStream.h"

using namespace std;


namespace pets {

static const string STATIONREPORT = "16";
static const string DATAFORMAT="&ddel=dot&ct=text/plain&nod=line";
static const string DATAREPORT="17";

void KlimaStation::clear()
{
  name = "";
  amsl = 0;
  distance = 0;
  stationid = 0;
  wmo = 0;
}
;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  string tmp = (char *) buffer;
  (*(string*) userp) += tmp.substr(0, nmemb);
  return (size_t) (size * nmemb);
}

vector<string> KlimaStream::getFromHttp(string url)
{
  CURL *curl = NULL;
  CURLcode res;
  string data;
  vector<string> result;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }

  boost::split(result, data, boost::algorithm::is_any_of("\n"));
  return result;
}

std::string pets::KlimaStation::description()
{
  ostringstream ost;
  ost << name << " " << distance << "km | hoh=" << amsl;
  return ost.str();
}

void KlimaStream::initialize(std::string h, std::map<std::string, std::string> pars, int max)
{
  host = h;
  maxDistance = max;
  currentStation.clear();

  map<string, string>::iterator itr = pars.begin();
  for (; itr != pars.end(); itr++) {

    string key = itr->first;
    string token = itr->second;

    if (token.empty())
      continue;

    vector<string> tokens;

    boost::split(tokens, token, boost::algorithm::is_any_of(":"));

    KlimaParameter kpar;
    kpar.klimaName = tokens[0];
    kpar.parid.setFromString(key);
    boost::algorithm::trim(kpar.klimaName);
    string parameterkey = kpar.parid.alias;

    parameterDefinitions[parameterkey] = kpar;

    if (tokens.size() > 1) {
      parameterDefinitions[parameterkey].transform = new pets::math::DynamicFunction(tokens[1]);
    }
  }

  initialized = true;
  initialized = read(STATIONREPORT);
}

bool KlimaStream::read(string report, string query)
{
  if (host.empty())
    return false;
  if (!initialized)
    return false;

  string url = host + "&re=" + report + query;

  cerr << "READING FROM: " << url <<  endl;

  vector<string> lines = getFromHttp(url);
  vector<string> data;
  vector<string> header;


  if (lines.empty())
    return false;

  for (unsigned int i = 0; i < lines.size(); i++) {
    boost::trim(lines[i]);
    if (lines[i].empty())
      continue;
    if (header.empty()) {
      boost::split(header, lines[i], boost::algorithm::is_any_of(";"));
      continue;
    }

    data.push_back(lines[i]);
  }

  if (report == STATIONREPORT) {
    return setStationsFromResult(data, header);
  } else
    return setDataFromResult(data, header);

  return false;
}

bool KlimaStream::setStationsFromResult(vector<string>& data, vector<string>& header)
{
  int NAME = 0, STNR = 0, AMSL = 0, LAT = 0, LON = 0, WMO = 0;

  for (unsigned int i = 0; i < header.size(); i++) {

    if (header[i] == "ST_NAME")
      NAME = i;
    else if (header[i] == "STNR")
      STNR = i;
    else if (header[i] == "AMSL")
      AMSL = i;
    else if (header[i] == "LAT_DEC")
      LAT = i;
    else if (header[i] == "LON_DEC")
      LON = i;
    else if (header[i] == "WMO_NO")
      WMO = i;

  }

  if (!(NAME * STNR * AMSL * LAT * LON * WMO)) {
    cerr << "Wrong header in " << __FUNCTION__ << endl;
    return false;
  }

  vector<string> token;

  for (unsigned int i = 0; i < data.size(); i++) {
    pets::KlimaStation s;
    boost::split(token, data[i], boost::algorithm::is_any_of(";"));
    if (token.size() < header.size())
      continue;
    s.name = token[NAME].c_str();
    s.coordinates.setLat(atof(token[LAT].c_str()));
    s.coordinates.setLon(atof(token[LON].c_str()));
    s.amsl = atoi(token[AMSL].c_str());
    s.distance = 0;
    s.stationid = atoi(token[STNR].c_str());
    s.wmo = atoi(token[WMO].c_str());

    stationlist.push_back(s);
  }

  npos = stationlist.size();

  return npos != 0;
}

bool KlimaStream::setDataFromResult(vector<string>& data, vector<string>& header)
{
  cerr << __FUNCTION__ << endl;


  data.clear();

//  Stnr;Year;Month;Day;Time(NMT);TA

  int STNR,YEAR,MONTH,DAY,TIME;

  for(unsigned int i=0;i<header.size();i++) {
    cerr << header[i] << endl;
      if (header[i] == "STNR")
        STNR = i;
      else if (header[i] == "YEAR")
        YEAR = i;
      else if (header[i] == "MONTH")
        MONTH = i;
      else if (header[i] == "DAY")
        DAY = i;
      else if (header[i] == "TIME(NMT)")
        TIME = i;
      else
        cerr << "parameter" << endl;
    }
/*
    vector<string> token;

    for (unsigned int i = 0; i < data.size(); i++) {
      pets::KlimaStation s;
      boost::split(token, data[i], boost::algorithm::is_any_of(";"));
      if (token.size() < header.size())
        continue;
      s.name = token[NAME].c_str();
      s.coordinates.setLat(atof(token[LAT].c_str()));
      s.coordinates.setLon(atof(token[LON].c_str()));
      s.amsl = atoi(token[AMSL].c_str());
      s.distance = 0;
      s.stationid = atoi(token[STNR].c_str());
      s.wmo = atoi(token[WMO].c_str());

      stationlist.push_back(s);
    }

*/
  return true;
}

pets::KlimaStation KlimaStream::getNearestKlimaStation(miCoordinates& pos)
{
  currentStation.clear();
  currentStation.distance = maxDistance;

  if (!initialized)
    return currentStation;

  if (stationlist.empty()) {
    read(STATIONREPORT);
    return currentStation;
  }

  list<KlimaStation>::iterator itr = stationlist.begin();
  for (; itr != stationlist.end(); itr++) {
    int dist = itr->coordinates.distance(pos);
    if (dist < currentStation.distance) {
      currentStation = *itr;
      currentStation.distance = dist;
    }
  }
  return currentStation;
}

bool KlimaStream::readKlimaData(int stationid, std::vector<ParId>& inpars, std::vector<ParId>& outpars,
    miutil::miTime fromTime, miutil::miTime toTime)
{

  KlimaData kld;
  map<string, KlimaData> klimaData;

  vector<string> klimNames;

  map<string, KlimaParameter>::iterator pardef;

  // check the parameterlist - what to get and what not....
  for (unsigned int i = 0; i < inpars.size(); i++) {
    string alias = inpars[i].alias;
    //   cerr << __FUNCTION__ << " alias(inpar) = " << alias << endl;
    if (parameterDefinitions.count(alias)) {
      pardef = parameterDefinitions.find(alias);

      klimNames.push_back(pardef->second.klimaName);
      klimaData[alias] = kld;
      klimaData[alias].parameter = pardef->second;
    } else {
      // we dont know what this is in Klima - skipping
      //  outpars.push_back(inpars[i]);
    }
  }

  // get the data .....

  // create query ...................

  string query = createDataQuery(klimNames, stationid, fromTime, toTime);

  read(DATAREPORT,query);


  /// execute search............

  // from here its the pets world again ....


  return true;

  map<string, KlimaData>::iterator kdata = klimaData.begin();

  for (; kdata != klimaData.end(); kdata++) {
    WeatherParameter wp;

    ParId pid = kdata->second.parameter.parid;
    pid.model = "OBS";
    pid.run = 0;
    pid.level = 0;

    //timeLine= dataList[didx].times;
    TimeLineIsRead = true;

    wp.setDims(kdata->second.times.size(), 1);
    int ipar = parameters.size();
    int tlindex;
    parameters.push_back(wp); // add it to the vector
    for (unsigned int j = 0; j < kdata->second.times.size(); j++) {
      parameters[ipar].setData(j, 0, kdata->second.data[j]);
    }

    if ((tlindex = timeLines.Exist(kdata->second.times)) == -1) {
      tlindex = numTimeLines;
      timeLines.insert(kdata->second.times, tlindex);
      numTimeLines++;
    }

    parameters[ipar].setTimeLineIndex(tlindex);
    parameters[ipar].setId(pid);
    parameters[ipar].calcAllProperties();

  }

  npar = parameters.size();
  DataIsRead = true;

  return true;
}

string KlimaStream::createDataQuery(vector<string> klimaNames, int stationid, miutil::miTime fromTime,
    miutil::miTime toTime)
{

  ostringstream query;

  query << DATAFORMAT;


  for (unsigned int i = 0; i < klimaNames.size(); i++)
    query << "&p=" << klimaNames[i];

  query << "&s=" << currentStation.stationid;
  query << "&fd=" << fromTime.format("%d.%m.%Y") << "%" << fromTime.hour();
  query << "&td=" << toTime.format("%d.%m.%Y") << "%" << toTime.hour();


  return query.str();
}

// INHERITED FROM DATASTREAM  ---------------------------------------------


int KlimaStream::findStation(const miutil::miString&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return 0;
} // find station index


int KlimaStream::findDataPar(const ParId&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return 0;
}

void KlimaStream::cleanParData()
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
}

bool KlimaStream::openStream(ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::openStreamForWrite(ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::readData(const int posIndex, const ParId&, const miutil::miTime&, const miutil::miTime&,
    ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getTimeLine(const int& index, std::vector<miutil::miTime>& tline, std::vector<int>& pline,
    ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::putTimeLine(const int& index, std::vector<miutil::miTime>& tline, std::vector<int>& pline,
    ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::putTimeLine(TimeLine& tl, std::vector<int>& pline, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getOnePar(int, WeatherParameter&, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::putOnePar(WeatherParameter&, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getStations(std::vector<miPosition>&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getStationSeq(int, miPosition&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getModelSeq(int, Model&, Run&, int&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getModelSeq(int, Model&, Run&, int&, std::vector<miutil::miString>&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

int KlimaStream::putStation(const miPosition& s, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return 0;
}

bool KlimaStream::writeData(const int posIndex, const int modIndex, ErrorFlag*, bool complete_write,
    bool write_submodel)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::close()
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

void KlimaStream::getTextLines(const ParId p, std::vector<miutil::miString>& tl)
{
  tl = textLines;
}

void KlimaStream::clean()
{
  parameters.clear();
  npar = 0;
  timeLine.clear();
  timeLines.clear();
  numTimeLines = 0;
  progLine.clear();
  progLines.clear();
  DataIsRead = false;
  TimeLineIsRead = false;
  IsCleaned = true;
}

} // namespace pets end