/*
 MoraStream.cc

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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>
#include <puTools/miDate.h>
#include <puTools/miStringFunctions.h>
#include <set>

#include <curl/curl.h>

#include <iostream>

#include <QDomDocument>
#include <QRegExp>

#include "SMHIMoraStream.h"

using namespace std;

namespace pets {

static const int UNDEFINED_COL= -1;
static int FLAGLEVEL = 5;

void MoraStation::clear()
{
  name.clear(); // All station types
  type.clear(); // Valid types, Wmo, Clim, Nat, Icao
  // Empty coordinates
  miCoordinates tmp;
  coordinates=tmp;
  amsl.clear(); //stationheigth
  number.clear(); // Wmo, Nat, Clim
  blk.clear(); // Wmo
  subtype.clear(); // Nat, Wmo
  code.clear(); // Icao
  distance = 0; // backward kompatibility
}

MoraParameter::MoraParameter(const MoraParameter& rhs) :
                transform(NULL)
{
  *this = rhs;
}

MoraParameter & MoraParameter::operator=(const MoraParameter & rhs)
{
  if (this != &rhs) {

    parid = rhs.parid;
    moraName = rhs.moraName;
    parameterId = rhs.parameterId;
    statisticsFormulaId = rhs.statisticsFormulaId;
    samplingTime = rhs.samplingTime;
    unit = rhs.unit;
    levelParameterId = rhs.levelParameterId;
    levelFrom = rhs.levelFrom;
    levelTo = rhs.levelTo;
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

// Present in klima stream
extern size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

// Curl or Qt ?
string MoraStream::getFromHttp(string url)
{
 
  CURL *curl = NULL;
  CURLcode res;
  string data;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }
  return data;
}

std::string pets::MoraStation::description()
{
  ostringstream ost;
  ost << distance << "km | hoh=" << amsl;
  return ost.str();
}
std::string pets::MoraStation::toString()
{
  ostringstream ost;
  ost << name << "," << type << "\n";
  ost << coordinates << "\n";
  ost << amsl << "," << number << "," << blk << "," << subtype << "," << code << "," << distance << "\n";
  return ost.str();
}

std::vector<std::string> MoraStream::getAllParameters()
{
  return allParameters;
}

void MoraStream::initialize(std::string h,
    std::string stationreport,
    std::string datareport,
    std::string monthlynormalreport,
    std::map<std::string, std::string> pars,
    std::map<std::string, std::string> norms, int max)
{
  host = h;
  STATIONREPORT = stationreport;
  DATAREPORT = datareport;
  MONTHNORMALREPORT = monthlynormalreport;
  maxDistance = max;
  currentStation.clear();

  map<string, string>::iterator par_itr = pars.begin();
  map<string, string>::iterator norm_itr = norms.begin();

  for (; par_itr != pars.end(); par_itr++) {
    setSingleParameterDefinition(par_itr->first, par_itr->second,
        pets::Mora_observation);
  }

  for (; norm_itr != norms.end(); norm_itr++) {
    setSingleParameterDefinition(norm_itr->first, norm_itr->second,
        pets::Mora_monthly_normal);
  }

  initialized = true;
  // report and query are the same
  initialized = read(STATIONREPORT,STATIONREPORT);
  IsOpen = initialized;
}
// # PETSPAR   = namn | parameterId | statisticsFormulaId | samplingTime | unit : transform (-273.15, *.01)
void MoraStream::setSingleParameterDefinition(string key, string token,
    pets::MoraDatatype type)
{
  if (token.empty())
    return;

  vector<string> tokens;
  vector<string> subTokens;
  boost::split(tokens, token, boost::algorithm::is_any_of(":"));

  MoraParameter kpar;
  string subToken = tokens[0];
  boost::split(subTokens, subToken, boost::algorithm::is_any_of("|"));
  if (subTokens.size() > 0) {
    kpar.moraName = subTokens[0];
    boost::algorithm::trim(kpar.moraName);
  }
  if (subTokens.size() > 1) {
    kpar.parameterId = subTokens[1];
    boost::algorithm::trim(kpar.parameterId);
  }
  if (subTokens.size() > 2) {
    kpar.statisticsFormulaId = subTokens[2];
    boost::algorithm::trim(kpar.statisticsFormulaId);
  }
  if (subTokens.size() > 3) {
    kpar.samplingTime = subTokens[3];
    boost::algorithm::trim(kpar.samplingTime);
  }
  if (subTokens.size() > 4) {
    kpar.unit = subTokens[4];
    boost::algorithm::trim(kpar.unit);
  }
  if (subTokens.size() > 5) {
    kpar.levelParameterId = subTokens[5];
    boost::algorithm::trim(kpar.levelParameterId);
  }
  if (subTokens.size() > 6) {
    kpar.levelFrom = subTokens[6];
    boost::algorithm::trim(kpar.levelFrom);
  }
  if (subTokens.size() > 7) {
    kpar.levelTo = subTokens[7];
    boost::algorithm::trim(kpar.levelTo);
  }
  kpar.parid.setFromString(key);
  kpar.type = type;
  boost::algorithm::trim(kpar.moraName);
  string parameterkey = kpar.parid.alias;

  parameterDefinitions[parameterkey] = kpar;
  knownMoraParameters[kpar.moraName] = parameterkey;

  if (kpar.moraName != "COMPUTE")
    allParameters.push_back(kpar.moraName);

  if (tokens.size() > 1) {
    parameterDefinitions[parameterkey].transform =
        new pets::math::DynamicFunction(tokens[1]);
  }

}

bool MoraStream::read(string report, string query, miutil::miTime from,
    miutil::miTime to)
{
  if (host.empty())
    return false;
  if (!initialized)
    return false;

  string url = host + query;
  // the result in xml
  string lines;

  // Decide if i have to read this or if this is in the cache
  // if we repaint and rebuild the same diagram again and again - we
  // pull the same Moradata out again and again - no point in that


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
  } else if (report==STATIONREPORT) {
    if( url != cachedStationQuery )  {
        cerr << "READING FROM: " << url << endl;
        cachedStations = getFromHttp(url);
      }
      lines = cachedStations;
      cachedStationQuery = url;
  } else {
    cerr << "READING FROM: " << url << endl;
    lines = getFromHttp(url);
  }

  if (lines.empty())
    return false;

  if (report == STATIONREPORT) {
    return setStationsFromResult(lines);
  } else if (report == MONTHNORMALREPORT) {
    return setNormalFromResult(lines, from, to);
  } else {
    return setDataFromResult(lines);
  }

  return false;
}

bool MoraStream::setStationsFromResult(string& data_)
{
  // Parse the xml....
  QByteArray data = data_.c_str();
  QDomDocument document;
  QString errorMsg;
  int errorLine, errorColumn;
  std::vector<std::string> result;

  if(document.setContent ( data, true, &errorMsg, &errorLine, &errorColumn) ) {

    QDomNodeList nodes;
    nodes = document.elementsByTagName("StationList");
    for(int i = 0; i < nodes.count(); i++)
    {
      QDomNode element = nodes.at(i);
      if(element.isElement())
      {
        std::string name="";
        float latitude=-99999,longitude=-99999;
        std::string height="-99999";

        QDomNode entry = element.toElement().firstChild();

        while(!entry.isNull()) {
            if (QString("StationPlace") == entry.toElement().tagName()) {
              // SMHI format
              longitude = entry.toElement().attributeNode("lon").value().toFloat();
              latitude = entry.toElement().attributeNode("lat").value().toFloat();
              height = entry.toElement().attributeNode("hgt").value().toStdString();
              // First level of child nodes, Wmo, Clim, Ikv 
              QDomNode child_element = entry.firstChild();
              if(child_element.isElement())
              {
                pets::MoraStation s;
                s.type = child_element.toElement().tagName().toStdString();
                miCoordinates coordinates(longitude,latitude);
                s.coordinates = coordinates;
                s.amsl = height;
                s.distance = 0;
                // Next level of child nodes, Name, Num osv
                QDomNodeList element_nodes = child_element.childNodes();
                for(int i = 0; i < element_nodes.count(); i++)
                {
                  QDomNode the_element = element_nodes.at(i);
                  if(the_element.isElement())
                  {
                    QString tag_name = the_element.toElement().tagName();
                    if (tag_name == QString("Name")) {
                      QString tmp_name = the_element.toElement().text();
                      // "." not allowed in names, replace it
                      if (tmp_name.count(QString("."))) {
                         tmp_name.replace(QString("."),QString(" "));
                      }
                      s.name = tmp_name.toStdString();
                      boost::algorithm::trim(s.name);
                      std::string uppername = miutil::to_upper_latin1(s.name);
                      s.name = uppername;
                    }
                    if (tag_name == QString("Num")) {
                      s.number = the_element.toElement().text().toStdString();
                    }
                    if (tag_name == QString("Blk")) {
                      s.blk = the_element.toElement().text().toStdString();
                    }
                    if (tag_name == QString("Typ")) {
                      s.subtype = the_element.toElement().text().toStdString();
                    }
                    if (tag_name == QString("Code")) {
                      s.code = the_element.toElement().text().toStdString();
                    }
                  }
                }
                // No need to have garbage in list
                if (!s.name.empty() && (s.type != "Ikv"))
                  stationlist.push_back(s);
              }          
            } // End StationPlace
          entry = entry.nextSibling();
        }       
      }
    }
  }
  
  npos = stationlist.size();

  return npos != 0;
}

vector<miutil::miTime>  MoraStream::createTimeline( miutil::miTime from, miutil::miTime to)
{
  vector<miutil::miTime> timeline;
  while (from <= to ) {
    timeline.push_back(from);
    from.addHour(1);
  }
  return timeline;
}


bool MoraStream::setNormalFromResult(string & data_,
    miutil::miTime from, miutil::miTime to)
{
  // TBD: Not implemented yet
  
/*
  from.setTime(from.date(), miutil::miClock("00:00:00"));

  // Create index
  int STNR = UNDEFINED_COL, MONTH = UNDEFINED_COL;
  for (unsigned int i = 0; i < header.size(); i++) {
    string key = boost::to_upper_copy(header[i]);
    if (key == "STNR")
      STNR = i;
    else if (key == "MONTH")
      MONTH = i;


    if (knownMoraParameters.count(key)) {
      string petsName = knownMoraParameters[key];
      MoraData kd;
      moraData.push_back(kd);
      moraData.back().parameter = parameterDefinitions[petsName];
      moraData.back().col = i;
    }
  }

  if (STNR == UNDEFINED_COL || MONTH == UNDEFINED_COL)
    return false;

  if (moraData.empty())
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

    for (unsigned int k = 0; k < moraData.size(); k++) {
      double value = 0;
      int col = moraData[k].col;

      // this is a value from another query
      if(col == UNDEFINED_COL )
        continue;

      // drop empty lines
      if (token[col] == "-" || token[col] == "x")
        continue;

      value = atof(token[moraData[k].col].c_str());

      if (moraData[k].parameter.transform) {
        moraData[k].parameter.transform->calc(value);
      }

      monthly_values[k][month] = value;

    }
  }

  // the return values have to be sorted by time!
  for (unsigned int k = 0; k < moraData.size(); k++) {
    if ( !monthly_values.count(k))
      continue;

    for(int i=0; i<timeline.size();i++) {

      if(!monthly_values[k].count(timeline[i].month()))
        continue;

      double normal= monthly_values[k][timeline[i].month()];
      moraData[k].data.push_back(normal);
      moraData[k].times.push_back(timeline[i]);
    }

    // we need the columns for the next query - so we set them to nothing
    moraData[k].col = UNDEFINED_COL;
  }

  return true;
  */
  // dummy implementation
  return false;
}

bool MoraStream::setDataFromResult(string& data_)
{
  // Parse the xml....
  QByteArray data = data_.c_str();
  QDomDocument document;
  QString errorMsg;
  int errorLine, errorColumn;

  if(document.setContent ( data, true, &errorMsg, &errorLine, &errorColumn) ) {

    QDomNodeList nodes;
    nodes = document.elementsByTagName("Values");
    for(int i = 0; i < nodes.count(); i++)
    {
      QDomNode element = nodes.at(i);
      if(element.isElement())
      {
        QDomNode entry = element.toElement().firstChild();
        int noOfValues = -1;
        string parameterId;
        string statisticsFormulaId;
        while(!entry.isNull()) {
            if (QString("Query") == entry.toElement().tagName()) {
              // Dont parse this for now
            } else if (QString("Header") == entry.toElement().tagName()) {
              // Get number of values and match with mora parameterDefinitions
              // If noofvalues == 0, continue
              QDomNodeList entry_nodes = entry.childNodes();
              for(int i = 0; i < entry_nodes.count(); i++)
              {
                QDomNode child_element = entry_nodes.at(i);
                if(child_element.isElement())
                {
                  if (QString("NumberOfValues") == child_element.toElement().tagName()) {
                    noOfValues = child_element.toElement().text().toInt();
                  }
                  if (QString("Parameter") == child_element.toElement().tagName()) {
                    QDomNode child_entry = child_element.toElement().firstChild();
                    while(!child_entry.isNull()) {
                      if (QString("ParameterId") == child_entry.toElement().tagName()) {
                        parameterId = child_entry.toElement().text().toStdString();
                      }
                      child_entry = child_entry.nextSibling();
                    }
                  }
                  if (QString("StatisticsType") == child_element.toElement().tagName()) {
                    QDomNode child_entry = child_element.toElement().firstChild();
                    while(!child_entry.isNull()) {
                      if (QString("StatisticsFormulaTypeId") == child_entry.toElement().tagName()) {
                        statisticsFormulaId = child_entry.toElement().text().toStdString();
                      }
                      child_entry = child_entry.nextSibling();
                    }
                  }
                  
                }
              }
            } else if (QString("ValueList") == entry.toElement().tagName()) {
              // get the correct MoraParameter;
              // instance attribute
              MoraData md;
              for (unsigned int i = 0; i < moraPars.size(); i++) {
                if ((moraPars[i].parameterId == parameterId) && (moraPars[i].statisticsFormulaId == statisticsFormulaId)) {
                  md.parameter = moraPars[i];
                  md.col = i;
                  break;
                }
              }
              QDomNodeList entry_nodes = entry.childNodes();
              for(int i = 0; i < entry_nodes.count(); i++)
              {
 
                QDomNode child_element = entry_nodes.at(i);
                if(child_element.isElement())
                {
                  if (QString("Value") == child_element.toElement().tagName()) {
                    // get time and value and add it to MoraData
                    // 2016-02-21T21:00:00.000Z, timeformat ?
                    string time_string = child_element.toElement().attributeNode("tick").value().toStdString();
                    // remove the millisecond part...
                    vector<string> tokens;
                    boost::split(tokens, time_string, boost::algorithm::is_any_of("."));
                    miutil::miTime time(tokens[0]);
                    string offset = child_element.toElement().attributeNode("offset").value().toStdString();
                    if (offset != "PT0S") {
                      // offset PTXXM
                      vector<string> offset_tokens;
                      string delims = "PTM";
                      boost::split(offset_tokens, offset, boost::algorithm::is_any_of(delims));
                      
                      if (offset_tokens.size() == 4) {
                        if (!offset_tokens[2].empty()) {
                          int offset_min = miutil::to_int(offset_tokens[2]);
                          time.addMin(-offset_min);
                        }
                      }
                    }
                    // The pres attribute is sortconverted
                    float value = child_element.toElement().attributeNode("pres").value().toFloat();
                    if (md.parameter.transform) {
                      double tmp_value = value;
                      md.parameter.transform->calc(tmp_value);
                      value = tmp_value;
                    }
                    md.data.push_back(value);
                    md.times.push_back(time);
                  }
                }
              }
              moraData.push_back(md);
            } // End StationPlace
          entry = entry.nextSibling();
        } // End Query,Header,ValueList      
      }
    } // End Values
  }
  
  return true;
}

pets::MoraStation MoraStream::getNearestMoraStation(miCoordinates& pos)
{
  currentStation.clear();
  currentStation.distance = maxDistance;

  if (!initialized) {
    return currentStation;
  }
  if (stationlist.empty()) {
    // report and query are the same
    read(STATIONREPORT, STATIONREPORT);
    return currentStation;
  }
  vector<MoraStation>::iterator itr = stationlist.begin();
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

bool MoraStream::readMoraData(std::vector<ParId>& inpars,
    std::vector<ParId>& outpars, miutil::miTime fromTime, miutil::miTime toTime)
{
  // no place - no data - skipping
  if (currentStation.name.empty()) {
    return false;
  }

  // really ? there is no time for data here - skipping
  if (fromTime >= toTime) {
    return false;
  }
  // instance attributes
  moraPars.clear();
  normalPars.clear();
  moraData.clear();

  map<string, MoraParameter>::iterator pardef;
  vector<ParId> newinpars;
  // check the parameterlist - what to get and what not....
  for (unsigned int i = 0; i < inpars.size(); i++) {
    string alias = inpars[i].alias;

    if (parameterDefinitions.count(alias)) {
      pardef = parameterDefinitions.find(alias);
      if (pardef->second.moraName == "COMPUTE")
        outpars.push_back(inpars[i]);
      else {

        if (pardef->second.type == pets::Mora_monthly_normal) {
          normalPars.push_back(pardef->second);
        } else {
          moraPars.push_back(pardef->second);
        }
        newinpars.push_back(inpars[i]);
      }
    }
  }
  inpars = newinpars;
 
  // get the data .....
  // no data known to pets and the Moradb - skip this
  if (moraPars.empty() && normalPars.empty() ) {
    return false;
  }

  if(!moraPars.empty() ) {
    string moraquery = createDataQuery(moraPars, fromTime, toTime,
        pets::Mora_observation);
    read(DATAREPORT,moraquery);
  }

  if(!normalPars.empty() ) {
    string normalquery = createDataQuery(normalPars, fromTime, toTime,
        pets::Mora_monthly_normal);
    read(MONTHNORMALREPORT,normalquery);
  }

  // from here its the pets world again ....

  for (unsigned int i = 0; i < moraData.size(); i++) {
    WeatherParameter wp;
    ParId pid = moraData[i].parameter.parid;
    pid.model = "OBS";
    pid.run = 0;
    pid.level = 0;

    //timeLine= dataList[didx].times;
    TimeLineIsRead = true;

    // TODO: check if there is an empty timeline and delete it if necessary
    int numOfTimes = moraData[i].times.size();

    if (numOfTimes < 1)
      continue;

    wp.setDims(moraData[i].times.size(), 1);
    int ipar = parameters.size();
    int tlindex;
    parameters.push_back(wp); // add it to the vector
    for (unsigned int j = 0; j < moraData[i].times.size(); j++) {
      parameters[ipar].setData(j, 0, moraData[i].data[j]);
    }

    if ((tlindex = timeLines.Exist(moraData[i].times)) == UNDEFINED_COL) {
      tlindex = numTimeLines;
      timeLines.insert(moraData[i].times, tlindex);
      numTimeLines++;
    }

    parameters[ipar].setTimeLineIndex(tlindex);
    parameters[ipar].setId(pid);
    parameters[ipar].calcAllProperties();
    cerr << parameters[ipar] << endl;

  }

  npar = parameters.size();
  DataIsRead = true;

  return true;
}

string MoraStream::createDataQuery(vector<MoraParameter> & moraPars,
    miutil::miTime fromTime, miutil::miTime toTime, pets::MoraDatatype type)
{

  MoraStation station = currentStation;
  string query;

  if (type == pets::Mora_observation) {
    // /value/stationsvalues/v1/stationType/%/stationId/%/parameterId/%/statisticsFormulaId/%/samplingTime/%/from/%/to/%/levelParameterId/%/levelFrom/%/levelTo/%.xml
    
    // Add parameters to url
    string parameterids;
    string statisticsFormulaids;
    string samplingTimes;
    string levelParameterids;
    string levelFroms;
    string levelTos;
    for (unsigned int i = 0; i < moraPars.size(); i++) {
      if (i == 0) {
        parameterids = moraPars[i].parameterId;
        statisticsFormulaids = moraPars[i].statisticsFormulaId;
        samplingTimes = moraPars[i].samplingTime;
        levelParameterids = moraPars[i].levelParameterId;
        levelFroms = moraPars[i].levelFrom;
        levelTos = moraPars[i].levelTo;
      } else {
        parameterids += "," + moraPars[i].parameterId;
        statisticsFormulaids += "," + moraPars[i].statisticsFormulaId;
        samplingTimes += "," + moraPars[i].samplingTime;
        levelParameterids += "," + moraPars[i].levelParameterId;
        levelFroms += "," + moraPars[i].levelFrom;
        levelTos += "," + moraPars[i].levelTo;
      }
    }
    // Add station metadata to url
    /* stationType/clim/stationId/71420/
         stationType/icao/stationId/ESTL/
         stationType/nat/stationId/11991/9/
         tationType/wmo/stationId/407/0/2/
    */
    string stationType = miutil::to_lower(station.type); // Must be lower case
    string stationId;
    if (station.type == "Clim") {
      stationId = station.number;
    } else if (station.type == "Icao") {
      stationId = station.code;
    } else if (station.type == "Nat") {
      stationId = station.number + "/" + station.subtype;
    } else if (station.type == "Wmo") {
      stationId = station.number + "/" + station.subtype + "/" + station.blk;
    } else {
      cerr << "Unknown station.type " << station.type << endl;
      return query;
    }
    // /value/stationsvalues/v1/stationType/%/stationId/%/parameterId/%/statisticsFormulaId/%/samplingTime/%/from/%/to/%/levelParameterId/%/levelFrom/%/levelTo/%.xml
    vector<string> tokens; // size should be 8!
    boost::split(tokens, DATAREPORT, boost::algorithm::is_any_of("%"));
    if (tokens.size() != 11) {
      cerr << "Invalid DATAREPORT: " << DATAREPORT << endl;
      return query;
    }
    query = tokens[0] + stationType + tokens[1] + stationId + tokens[2]
      + parameterids + tokens[3] + statisticsFormulaids + tokens[4] 
      + samplingTimes + tokens[5] + fromTime.isoTime("T") + tokens[6] + toTime.isoTime("T") + tokens[7]
      + levelParameterids + tokens[8] + levelFroms + tokens[9] + levelTos + tokens[10];
    // remove spaces
    miutil::replace(query, " ", "");
      
  } else if (type == pets::Mora_monthly_normal) {
    // Only Lufttemperatur and Climate is supported yet
    // //value/normalvalue/v1/climatenr/%/parameterId/%/statisticsFormulaId/%.xml
    // Add parameters to url
    string parameterids;
    string statisticsFormulaids;
    for (unsigned int i = 0; i < moraPars.size(); i++) {
      if (i == 0) {
        parameterids = moraPars[i].parameterId;
        statisticsFormulaids = moraPars[i].statisticsFormulaId;
      } else {
        parameterids += "," + moraPars[i].parameterId;
        statisticsFormulaids += "," + moraPars[i].statisticsFormulaId;
      }
    }
    string stationId;
    if (station.type == "Clim") {
      stationId = station.number;
    }  else {
      cerr << "Unknown or not supported station.type " << station.type << endl;
      return query;
    }
    vector<string> tokens; // size should be 4!
    boost::split(tokens, MONTHNORMALREPORT, boost::algorithm::is_any_of("%"));
    if (tokens.size() != 4) {
      cerr << "Invalid MONTHNORMALREPORT: " << MONTHNORMALREPORT << endl;
      return query;
    }
    query = tokens[0] + stationId + tokens[1] + parameterids + tokens[2]
      + statisticsFormulaids + tokens[3];
    // remove spces
    miutil::replace(query, " ", "");
    
  }
  return query;
}

// INHERITED FROM DATASTREAM  ---------------------------------------------

int MoraStream::findStation(const std::string& name)
{
  if (!initialized) {
    return -1;
  }
  if (stationlist.empty()) {
    // report and query are the same
    read(STATIONREPORT, STATIONREPORT);
  }
  for (int i = 0; i < stationlist.size(); i++) {
    if (stationlist[i].name == name) {
      return i;
    }
  }
  return -1;
} // find station index

int MoraStream::findDataPar(const ParId& id)
{
  for (int i = 0; i < parameters.size(); i++) {
    if (parameters[i].Id().alias == id.alias) {
      return i;
    }
  }
  return -1;
}

void MoraStream::cleanParData()
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
}

bool MoraStream::openStream(ErrorFlag*)
{
  return initialized;
}

bool MoraStream::openStreamForWrite(ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

bool MoraStream::readData(const int posIndex, const ParId& id,
    const miutil::miTime& fromTime, const miutil::miTime& toTime, ErrorFlag*)
{  
  miutil::miTime _toTime = miutil::miTime::nowTime();
  _toTime.setTime(_toTime.year(),_toTime.month(),_toTime.day(),_toTime.hour(), _toTime.min(), 0);
  miutil::miTime _fromTime;
  _fromTime = _toTime;
  // The default timespan
  _fromTime.addHour(-300);
  std::vector<ParId> inpars;
  std::vector<ParId> outpars;
  // Get all parameters known to morastream.
  std::map<std::string,MoraParameter>::iterator iter = parameterDefinitions.begin();
  for(; iter != parameterDefinitions.end(); iter++)
  {
    inpars.push_back(iter->second.parid);
  }
  // Get the current station
  currentStation = stationlist[posIndex];
  return readMoraData(inpars, outpars, _fromTime, _toTime);
}

bool MoraStream::getTimeLine(const int& index,
    std::vector<miutil::miTime>& tline, std::vector<int>& pline, ErrorFlag*)
{
  
  return getTimeLine(index,tline,pline);
}

bool MoraStream::getTimeLine(const int& index, vector<miutil::miTime>& tline,
    vector<int>& pline)
{
  if (TimeLineIsRead && timeLines.Timeline(index, tline)) {
    if (index < progLines.size())
      pline = progLines[index];
    return true;
  }
  return false;
}

bool MoraStream::putTimeLine(const int& index,
    std::vector<miutil::miTime>& tline, std::vector<int>& pline, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

bool MoraStream::putTimeLine(TimeLine& tl, std::vector<int>& pline, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

bool MoraStream::getOnePar(int i, WeatherParameter& wp, ErrorFlag*)
{
  return getOnePar(i,wp);
}

bool MoraStream::getOnePar(int i, WeatherParameter& wp)
{
  if (i >= 0 && i < parameters.size()) {
    wp = parameters[i];
    return true;
  }
  return false;
}

bool MoraStream::putOnePar(WeatherParameter&, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

bool MoraStream::getStations(std::vector<miPosition>&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

bool MoraStream::getStationSeq(int index, miPosition& st)
{
  if (index < 0 || index > stationlist.size() - 1)
    return false;
  MoraStation mst = stationlist[index];
  miPosition _st;
  _st.setCoor(mst.coordinates);
  _st.setName(mst.name);
  _st.setIcao(mst.code);
  _st.setHoH(miutil::to_int(mst.amsl));
  st = _st;
  return true;
}

bool MoraStream::getModelSeq(int, Model&, Run&, int&)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}
/*
getModelSeq(numm,
            datastreams[idx].modelList[numm], datastreams[idx].runList[numm],
            datastreams[idx].idList[numm], dt)*/
bool MoraStream::getModelSeq(int numn, Model& model, Run& run, int& id,
    std::vector<std::string>& s)
{
  std::vector<std::string> _s;
  if (numn == 0)
  {
    model="OBS";
    run=0;
    id=0;
    s = _s;
    return true;
  }
  return false;
}

int MoraStream::putStation(const miPosition& s, ErrorFlag*)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return 0;
}

bool MoraStream::writeData(const int posIndex, const int modIndex, ErrorFlag*,
    bool complete_write, bool write_submodel)
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

bool MoraStream::close()
{
  cerr << "Unimplemented " << __FUNCTION__ << " called in MoraStream " << endl;
  return false;
}

void MoraStream::getTextLines(const ParId p, std::vector<std::string>& tl)
{
  tl = textLines;
}

void MoraStream::clean()
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
  moraPars.clear();
  normalPars.clear();
  IsCleaned = true;
}

} // namespace pets end
