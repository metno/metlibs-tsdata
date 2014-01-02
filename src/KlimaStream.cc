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
#include <puTools/miDate.h>
#include <set>

#include <curl/curl.h>

#include <iostream>

#include "KlimaStream.h"

using namespace std;

namespace pets {


static const string MONTHNORMALREPORT = "28";
static const string STATIONREPORT = "16";
static const string DATAFORMAT = "&ddel=dot&ct=text/plain&nod=line";
static const string DATAREPORT = "17";
static const int UNDEFINED_COL= -1;
static int FLAGLEVEL = 5;

void KlimaStream::setFlagLevel(int newFlagLevel)
{
  if (newFlagLevel < 0 || newFlagLevel > 7)
    return;
  FLAGLEVEL = newFlagLevel;
}

void KlimaStation::clear()
{
  name = "";
  amsl = 0;
  distance = 0;
  stationid = 0;
  wmo = 0;
}

KlimaParameter::KlimaParameter(const KlimaParameter& rhs) :
                transform(NULL)
{
  *this = rhs;
}

KlimaParameter & KlimaParameter::operator=(const KlimaParameter & rhs)
{
  if (this != &rhs) {

    parid = rhs.parid;
    klimaName = rhs.klimaName;
    type = rhs.type;

    if (transform) {
      delete transform;
      transform = NULL;
    }
    if (rhs.transform) {
      transform = new pets::math::DynamicFunction(rhs.transform->text(),
          rhs.transform->getFactor());
    }
  }
  return *this;
}

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

void KlimaStream::setBlacklist(std::set<std::string> bl)
{
  set<string>::iterator itr = bl.begin();
  blacklist.clear();
  for (; itr != bl.end(); itr++) {
    if (knownKlimaParameters.count(*itr))
      blacklist.insert(knownKlimaParameters[*itr]);
  }
}

std::set<std::string> KlimaStream::getBlacklist()
{
  set<string>::iterator itr = blacklist.begin();
  set<string> blacklistAsKlimaNames;
  for (; itr != blacklist.end(); itr++) {
    if (parameterDefinitions.count(*itr)) {
      blacklistAsKlimaNames.insert(parameterDefinitions[*itr].klimaName);
    }
  }
  return blacklistAsKlimaNames;
}

std::vector<std::string> KlimaStream::getAllParameters()
{
  return allParameters;
}

std::string KlimaStream::getObservationBlacklistAsString()
{
  ostringstream ost;
  set<string>::iterator itr = blacklist.begin();
  string delimiter = "";
  for (; itr != blacklist.end(); itr++) {
    ost << delimiter << *itr;
    delimiter = ":";
  }
  return ost.str();

}

void KlimaStream::setObservationBlacklistFromString(std::string blist)
{
  set<string> tokens;
  boost::split(tokens, blist, boost::algorithm::is_any_of(":"));
  setBlacklist(tokens);
}

void KlimaStream::initialize(std::string h,
    std::map<std::string, std::string> pars,
    std::map<std::string, std::string> norms, int max)
{
  host = h;
  maxDistance = max;
  currentStation.clear();

  map<string, string>::iterator par_itr = pars.begin();
  map<string, string>::iterator norm_itr = norms.begin();

  for (; par_itr != pars.end(); par_itr++) {
    setSingleParameterDefinition(par_itr->first, par_itr->second,
        pets::klima_observation);
  }

  for (; norm_itr != norms.end(); norm_itr++) {
    setSingleParameterDefinition(norm_itr->first, norm_itr->second,
        pets::klima_monthly_normal);
  }

  initialized = true;
  initialized = read(STATIONREPORT);
}

void KlimaStream::setSingleParameterDefinition(string key, string token,
    pets::KlimaDatatype type)
{
  if (token.empty())
    return;

  vector<string> tokens;
  boost::split(tokens, token, boost::algorithm::is_any_of(":"));

  KlimaParameter kpar;
  kpar.klimaName = tokens[0];
  kpar.parid.setFromString(key);
  kpar.type = type;
  boost::algorithm::trim(kpar.klimaName);
  string parameterkey = kpar.parid.alias;

  parameterDefinitions[parameterkey] = kpar;
  knownKlimaParameters[kpar.klimaName] = parameterkey;

  if (kpar.klimaName != "COMPUTE")
    allParameters.push_back(kpar.klimaName);

  if (tokens.size() > 1) {
    parameterDefinitions[parameterkey].transform =
        new pets::math::DynamicFunction(tokens[1]);
  }

}

bool KlimaStream::read(string report, string query, miutil::miTime from,
    miutil::miTime to)
{
  if (host.empty())
    return false;
  if (!initialized)
    return false;

  string url = host + "&re=" + report + query;

  vector<string> lines;

  // Decide if i have to read this or if this is in the cache
  // if we repaint and rebuild the same diagram again and again - we
  // pull the same klimadata out again and again - no point in that


  if(report==MONTHNORMALREPORT ) {
    if( url != cachedMonthlyQuery )  {
      cerr << "READING FROM: " << url << endl;
      cachedMonthly = getFromHttp(url);
    }
    lines = cachedMonthly;
    cachedMonthlyQuery = url;
  } else if (report==DATAREPORT) {
    if( url != cachedDataQuery )  {
        cerr << "READING FROM: " << url << endl;
        cachedData = getFromHttp(url);
      }
      lines = cachedData;
      cachedDataQuery = url;
  } else {
    lines = getFromHttp(url);
    cerr << "READING FROM: " << url << endl;
  }


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
  } else if (report == MONTHNORMALREPORT) {
    return setNormalFromResult(data, header, from, to);
  } else {
    return setDataFromResult(data, header);
  }

  return false;
}

bool KlimaStream::setStationsFromResult(vector<string>& data,
    vector<string>& header)
{
  int NAME = 0, STNR = 0, AMSL = 0, LAT = 0, LON = 0, WMO = 0;

  // Create index
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

vector<miutil::miTime>  KlimaStream::createTimeline( miutil::miTime from, miutil::miTime to)
{
  vector<miutil::miTime> timeline;
  while (from <= to ) {
    timeline.push_back(from);
    from.addHour(6);
  }
  return timeline;
}


bool KlimaStream::setNormalFromResult(vector<string>& data,
    vector<string>& header, miutil::miTime from, miutil::miTime to)
{

  from.setTime(from.date(), miutil::miClock("00:00:00"));

  // Create index
  int STNR = UNDEFINED_COL, MONTH = UNDEFINED_COL;
  for (unsigned int i = 0; i < header.size(); i++) {
    string key = boost::to_upper_copy(header[i]);
    if (key == "STNR")
      STNR = i;
    else if (key == "MONTH")
      MONTH = i;


    if (knownKlimaParameters.count(key)) {
      string petsName = knownKlimaParameters[key];
      KlimaData kd;
      klimaData.push_back(kd);
      klimaData.back().parameter = parameterDefinitions[petsName];
      klimaData.back().col = i;
    }
  }

  if (STNR == UNDEFINED_COL || MONTH == UNDEFINED_COL)
    return false;

  if (klimaData.empty())
    return false;

  vector<miutil::miTime> timeline = createTimeline(from, to);

  vector<string> token;

  map<int, map<int, double> > monthly_values;

  for (unsigned int i = 0; i < data.size(); i++) {
    boost::split(token, data[i], boost::algorithm::is_any_of(";"));
    if (token.size() < header.size()) {
      continue;
    }
    //    int stnr =  atoi(token[STNR].c_str());
    int month = atoi(token[MONTH].c_str());

    for (unsigned int k = 0; k < klimaData.size(); k++) {
      double value = 0;
      int col = klimaData[k].col;

      // this is a value from another query
      if(col == UNDEFINED_COL )
        continue;

      // drop empty lines
      if (token[col] == "-" || token[col] == "x")
        continue;

      value = atof(token[klimaData[k].col].c_str());

      if (klimaData[k].parameter.transform) {
        klimaData[k].parameter.transform->calc(value);
      }

      monthly_values[k][month] = value;

    }
  }

  // the return values have to be sorted by time!
  for (unsigned int k = 0; k < klimaData.size(); k++) {
    if ( !monthly_values.count(k))
      continue;

    for(int i=0; i<timeline.size();i++) {

      if(!monthly_values[k].count(timeline[i].month()))
        continue;

      double normal= monthly_values[k][timeline[i].month()];
      klimaData[k].data.push_back(normal);
      klimaData[k].times.push_back(timeline[i]);
    }

    // we need the columns for the next query - so we set them to nothing
    klimaData[k].col = UNDEFINED_COL;
  }

  return true;
}

bool KlimaStream::setDataFromResult(vector<string>& data,
    vector<string>& header)
{

  // Create index
  int STNR = UNDEFINED_COL, YEAR = UNDEFINED_COL, MONTH = UNDEFINED_COL, DAY = UNDEFINED_COL, TIME = UNDEFINED_COL, DD = UNDEFINED_COL, FF = UNDEFINED_COL;
  for (unsigned int i = 0; i < header.size(); i++) {
    string key = boost::to_upper_copy(header[i]);
    if (key == "STNR")
      STNR = i;
    else if (key == "YEAR")
      YEAR = i;
    else if (key == "MONTH")
      MONTH = i;
    else if (key == "DAY")
      DAY = i;
    else if (key == "TIME(UTC)") {
      TIME = i;
    }
    if (knownKlimaParameters.count(key)) {
      string petsName = knownKlimaParameters[key];
      KlimaData kd;
      klimaData.push_back(kd);
      klimaData.back().parameter = parameterDefinitions[petsName];
      klimaData.back().col = i;
      if (key == "DD")
        DD = i;
      else if (key == "FF")
        FF = i;
    }
  }

  if (STNR == UNDEFINED_COL || YEAR == UNDEFINED_COL || MONTH == UNDEFINED_COL || DAY == UNDEFINED_COL || TIME == UNDEFINED_COL)
    return false;

  if (klimaData.empty())
    return false;

  vector<string> token;

  for (unsigned int i = 0; i < data.size(); i++) {
    boost::split(token, data[i], boost::algorithm::is_any_of(";"));
    if (token.size() < header.size())
      continue;

    //    int stnr =  atoi(token[STNR].c_str());
    int year = atoi(token[YEAR].c_str());
    int month = atoi(token[MONTH].c_str());
    int day = atoi(token[DAY].c_str());
    int hour = atoi(token[TIME].c_str());
    int dayadd = 0;
    // in the climadatabase midnight is at 24 o'clock which is not iso standard - this is the workaround for that
    if (hour == 24) {
      hour = 0;
      dayadd = 1;
    }

    miutil::miTime valid(year, month, day, hour, 0, 0);
    if (dayadd)
      valid.addDay(dayadd);

    for (unsigned int k = 0; k < klimaData.size(); k++) {
      double value = 0;
      int col = klimaData[k].col;

      // this is a value from another query
      if(col == UNDEFINED_COL )
        continue;

      // drop empty lines
      if (token[col] == "-" || token[col] == "x")
        continue;

      // you need FF and DD to present vind
      if (col == DD)
        if (FF >= 0)
          if (token[FF] == "-" || token[FF] == "x")
            continue;

      if (col == FF)
        if (DD >= 0)
          if (token[DD] == "-" || token[DD] == "x")
            continue;

      value = atof(token[klimaData[k].col].c_str());

      if (klimaData[k].parameter.transform) {
        klimaData[k].parameter.transform->calc(value);
      }
      klimaData[k].data.push_back(value);
      klimaData[k].times.push_back(valid);
    }
  }
  for (unsigned int k = 0; k < klimaData.size(); k++) {
    // we need the columns for the next query - so we set them to nothing
    klimaData[k].col = UNDEFINED_COL;
  }

  return true;
}

pets::KlimaStation KlimaStream::getNearestKlimaStation(miCoordinates& pos)
{
  currentStation.clear();
  currentStation.distance = maxDistance;

  if (!initialized) {
    return currentStation;
  }
  if (stationlist.empty()) {
    read(STATIONREPORT);
    return currentStation;
  }
  list<KlimaStation>::iterator itr = stationlist.begin();
  for (; itr != stationlist.end(); itr++) {
    int dist = itr->coordinates.distance(pos);
    if (dist < maxDistance)
      if (dist < currentStation.distance) {
        currentStation = *itr;
        currentStation.distance = dist;
      }
  }

  return currentStation;
}

bool KlimaStream::readKlimaData(std::vector<ParId>& inpars,
    std::vector<ParId>& outpars, miutil::miTime fromTime, miutil::miTime toTime)
{
  // no place - no data - skipping
  if (!currentStation.stationid) {
    return false;
  }

  // really ? there is no time for data here - skipping
  if (fromTime >= toTime) {
    return false;
  }

  vector<string> klimaNames;
  vector<string> normalNames;

  map<string, KlimaParameter>::iterator pardef;
  vector<ParId> newinpars;
  // check the parameterlist - what to get and what not....
  for (unsigned int i = 0; i < inpars.size(); i++) {
    string alias = inpars[i].alias;

    if (blacklist.count(alias)) {
      continue;
    }

    if (parameterDefinitions.count(alias)) {
      pardef = parameterDefinitions.find(alias);
      if (pardef->second.klimaName == "COMPUTE")
        outpars.push_back(inpars[i]);
      else {

        if (pardef->second.type == pets::klima_monthly_normal) {
          normalNames.push_back(pardef->second.klimaName);
        } else {
          klimaNames.push_back(pardef->second.klimaName);
        }
        newinpars.push_back(inpars[i]);
      }
    }
  }
  inpars = newinpars;

  klimaData.clear();

  // get the data .....
  // no data known to pets and the klimadb - skip this
  if (klimaNames.empty() && normalNames.empty() ) {
    return false;
  }

  if(!klimaNames.empty() ) {
    string klimaquery = createDataQuery(klimaNames, fromTime, toTime,
        pets::klima_observation);
    read(DATAREPORT, klimaquery);
  }

  if(!normalNames.empty() ) {
    string normalquery = createDataQuery(normalNames, fromTime, toTime,
        pets::klima_monthly_normal);
    read(MONTHNORMALREPORT, normalquery,fromTime,toTime);
  }

  // from here its the pets world again ....

  for (unsigned int i = 0; i < klimaData.size(); i++) {
    WeatherParameter wp;

    ParId pid = klimaData[i].parameter.parid;
    pid.model = "OBS";
    pid.run = 0;
    pid.level = 0;

    //timeLine= dataList[didx].times;
    TimeLineIsRead = true;

    // TODO: check if there is an empty timeline and delete it if necessary
    int numOfTimes = klimaData[i].times.size();

    if (numOfTimes < 1)
      continue;

    wp.setDims(klimaData[i].times.size(), 1);
    int ipar = parameters.size();
    int tlindex;
    parameters.push_back(wp); // add it to the vector
    for (unsigned int j = 0; j < klimaData[i].times.size(); j++) {
      parameters[ipar].setData(j, 0, klimaData[i].data[j]);
    }

    if ((tlindex = timeLines.Exist(klimaData[i].times)) == UNDEFINED_COL) {
      tlindex = numTimeLines;
      timeLines.insert(klimaData[i].times, tlindex);
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

string KlimaStream::createDataQuery(vector<string> klimaNames,
    miutil::miTime fromTime, miutil::miTime toTime, pets::KlimaDatatype type)
{

  ostringstream query;

  query << DATAFORMAT;

  for (unsigned int i = 0; i < klimaNames.size(); i++)
    query << "&p=" << klimaNames[i];

  query << "&qa=" << FLAGLEVEL;
  query << "&s=" << currentStation.stationid;

  if (type == pets::klima_observation) {
    query << "&fd=" << fromTime.format("%d.%m.%Y");
    query << "&td=" << toTime.format("%d.%m.%Y");

  } else if (type == pets::klima_monthly_normal) {
    miutil::miDate fromDate=fromTime.date();
    miutil::miDate toDate=toTime.date();
    set<int> allmonth;

    while(fromDate < toDate) {
      allmonth.insert(fromDate.month() -1);
      fromDate.addDay(fromDate.daysInMonth());
    }
    allmonth.insert(fromDate.month() -1);

    set<int>::iterator currentmonth = allmonth.begin();
    for(; currentmonth !=allmonth.end();currentmonth++)
      query << "&m=" << *currentmonth;
  }

  return query.str();
}

// INHERITED FROM DATASTREAM  ---------------------------------------------

int KlimaStream::findStation(const std::string&)
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

bool KlimaStream::readData(const int posIndex, const ParId&,
    const miutil::miTime&, const miutil::miTime&, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getTimeLine(const int& index,
    std::vector<miutil::miTime>& tline, std::vector<int>& pline, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::getTimeLine(const int& index, vector<miutil::miTime>& tline,
    vector<int>& pline)
{
  if (TimeLineIsRead && timeLines.Timeline(index, tline)) {
    if (index < progLines.size())
      pline = progLines[index];
    return true;
  }
  return false;
}

bool KlimaStream::putTimeLine(const int& index,
    std::vector<miutil::miTime>& tline, std::vector<int>& pline, ErrorFlag*)
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

bool KlimaStream::getOnePar(int i, WeatherParameter& wp)
{
  if (i >= 0 && i < parameters.size()) {
    wp = parameters[i];
    return true;
  }
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

bool KlimaStream::getModelSeq(int, Model&, Run&, int&,
    std::vector<std::string>&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

int KlimaStream::putStation(const miPosition& s, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return 0;
}

bool KlimaStream::writeData(const int posIndex, const int modIndex, ErrorFlag*,
    bool complete_write, bool write_submodel)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

bool KlimaStream::close()
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in KlimaStream " << endl;
  return false;
}

void KlimaStream::getTextLines(const ParId p, std::vector<std::string>& tl)
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
