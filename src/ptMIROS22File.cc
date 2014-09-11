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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>
#include <iostream>
#include <set>

#include <glob.h>

#include "ptMIROS22File.h"

typedef std::vector<std::string> string_v;

using namespace miutil;
using namespace std;

// --------------- MIROS22File -------------------------------

const std::string MIROSstart= "!!!!";
const std::string MIROSid= "022";
const std::string MIROSend= "$$$$$$$";

MIROS22File::MIROS22File(const std::string& fn)
  : filename(fn)
{
}

bool MIROS22File::read(const std::string& location,
		       const miDate& date,
		       vector<MIROS22parset>& ps,
		       status& res)
{
  //cerr << "MIROS22File::read: " << filename << " location:" << location
  //     << " date:" << date << endl;
  bool clockfound= false;
  bool anypars= false;
  bool allpars=false;
  res= cant_open;
  
  ifstream f(filename.c_str());
  if (!f) return false;

  res= bad_file;
  std::string buf;
  getline(f, buf);
  if (not miutil::contains(buf, MIROSstart)) return false;
  getline(f, buf);
  if (not miutil::contains(buf, MIROSid)) return false;

  res= wrong_place;
  getline(f, buf);
  if (not miutil::contains(buf, location)) return false;
  
  res= wrong_time;
  getline(f, buf);
  vector<std::string> vs= miutil::split(buf, "-");
  if (vs.size()!=3){
    res= bad_file;
    return false;
  }
  miDate d(atoi(vs[2].c_str()),atoi(vs[1].c_str()),atoi(vs[0].c_str()));
  if (d.undef() || d!=date) return false;

  // cerr << "File:" << filename << " is ok, and has location:" << location << " and date:" << d << endl;
  
  res= read_ok;
  int nt= ps.size();
  if (nt==0) return true;
  
  bool foundclock;
  for (int i=0; i<nt; i++){
    //cerr << "-- Scanning for clock:" << ps[i].t << endl;
    int np= ps[i].val.size();
    if (np==0) continue;

    foundclock= false;
    while (getline(f, buf)){
      if (miutil::contains(buf, ":")){
 	//cerr << "Found clock-string:" << buf << endl;
        vs= miutil::split(buf, ":");
	if (vs.size()!=2){
	  res= bad_file;
	  return false;
	}
	miClock c(atoi(vs[0].c_str()),atoi(vs[1].c_str()),0);
	// trunc to 10-minute values
        int min10= c.min()/10*10;
        c = miClock(c.hour(), min10, 0);

	if (c.undef()){
	  res= bad_file;
	  return false;
	}

        if (ps[i].t.undef()){ // time not requested, setting file time
          ps[i].t = miTime(d,c);
	  foundclock= true;
          clockfound= true;
	  break;
        } else { // match file time-value against requested time
	  if (c==ps[i].t.clock()){
	    foundclock= true;
	    clockfound= true;
	    break;
	  }
        }
      }
    }
    if (foundclock){
      //cerr << "Found correct clock:" << ps[i].t << endl;
      bool readok= getline(f, buf);
      while (readok && not miutil::contains(buf, MIROSend)){
 	//cerr << "--Block header:" << buf << endl;
	if (buf.length()<8){
	  res= bad_file;
	  return false;
	}
	std::string block= buf.substr(1,2);
	char sensor= buf[3];
	int numl= atoi(buf.substr(5,3).c_str());
 	//cerr << "Block:" << block << " Sensor:" << sensor
 	//     << " numlines:" << numl << endl;
	numl--;
	vector<float> values;
	for (int k=0; k<numl; k++){
	  getline(f,buf);
	  values.push_back(atof(buf.c_str()));
	}
	// extract parameter values
	for (int k=0; k<np; k++){
	  if (ps[i].val[k].par.block==block &&
	      ps[i].val[k].par.sensor==sensor){
 	    //cerr << "++ Requested parameter:" << block << " and " << sensor << endl;
	    if (ps[i].val[k].par.parnum-1<numl){
	      anypars= true;
	      ps[i].val[k].changed= true;
	      ps[i].val[k].value= values[ps[i].val[k].par.parnum-1];
	    }
	  }
	}
	readok= getline(f,buf);
      }
    }
  }
  if (!clockfound){
    res= wrong_time;
    return false;
  }
  if (!anypars){
    res= missing_par;
    return false;
  } 
  // check for missing parameters
  allpars=true;
  for (int i=0; i<nt; i++){
    int np= ps[i].val.size();
    if (np==0) continue;
    for (int k=0; k<np; k++){
      allpars= (allpars && ps[i].val[k].changed);
//       if (!ps[i].val[k].changed)
// 	cerr << "Parameter not found:" << ps[i].val[k].par.block
// 	     << " " << ps[i].val[k].par.sensor
// 	     << " " << ps[i].val[k].par.parnum << endl;
    }
  }
  if (allpars) res= read_ok;
  else res= missing_par;
  return true;
}


// --------------- MIROS22Definition -------------------------------

MIROS22Definition::MIROS22Definition(const std::string& fn)
{
  set(fn);
}

void MIROS22Definition::set(const std::string& fn)
{
  filename= fn;
}

bool MIROS22Definition::scan()
{
  if (filename.empty()) return false;
  
  ifstream f(filename.c_str());
  if (!f) return false;

  enum bstatus {
    no_block,
    parameter_block,
    location_block
  };
  bstatus s= no_block;
  std::string buf;

  pars.clear();
  locs.clear();
  MIROS22parameter par;
  MIROS22location loc;
  vector<std::string> vs1, vs2;

  while (getline(f, buf)){
      miutil::trim(buf);
    if (buf.empty()) continue;
    if (buf[0]=='#') continue;

    if (miutil::contains(buf, "PARAMETERS")){
//       cerr << "----- Found parameter block" << endl;
      s= parameter_block;
    } else if (miutil::contains(buf, "LOCATIONS")){
//       cerr << "----- Found locations block" << endl;
      s= location_block;
    } else if (s == parameter_block) {
      vs1= miutil::split(buf, "=");
      if (vs1.size()!=2){
	cerr << "MIROS22Definition Warning: Bad parameter definition:" << buf << endl;
	continue;
      }
      // parse ParId
      ParId p= pard.Str2ParId(vs1[0]);
      // parse MIROS def
      vs2= miutil::split(vs1[1], ",");
      if (vs2.size()<3){
	cerr << "MIROS22Definition Warning: Bad MIROS definition:" << buf << endl;
	continue;
      }
      for (int i=0; i<3; i++)
	if (vs2[i].length()==0){
	  cerr << "MIROS22Definition Warning: Bad MIROS definition:" << buf << endl;
	  continue;
	}
      int n= pars.size();
      pars.push_back(par);
      pars[n].block= vs2[0];
      pars[n].sensor= vs2[1][0];
      pars[n].parnum= atoi(vs2[2].c_str());
      pars[n].parid= p;
//       cerr << "Found parameter:" << pars[n].parid << " --> "
// 	   << pars[n].block << "," << pars[n].sensor << ","
// 	   << pars[n].parnum << endl;
      
    } else if (s == location_block) {
      int n= locs.size();
      if (miutil::contains(buf, "Location=")){
	vs1= miutil::split(buf, "=");
	if (vs1.size()<2){
	  cerr << "MIROS22Definition Warning: Bad Location definition:" << buf << endl;
	  continue;
	}
	vs2= miutil::split(vs1[1], ";");
	if (vs2.size()<2){
	  cerr << "MIROS22Definition Warning: Bad Location definition:" << buf << endl;
	  continue;
	}
	locs.push_back(loc);
	locs[n].loc.setName(vs2[0]);
	locs[n].name= vs2[1];
// 	cerr << "Found location:" << locs[n].name << " --> " << locs[n].loc.Name() << endl;
      } else if (miutil::contains(buf, "Position=")){
	if (n==0){
	  cerr << "MIROS22Definition Warning: Define Location before Position:" << buf << endl;
	  continue;
	}
	vs1= miutil::split(buf, "=");
	if (vs1.size()<2){
	  cerr << "MIROS22Definition Warning: Bad Position definition:" << buf << endl;
	  continue;
	}
	vs2= miutil::split(vs1[1], ",");
	if (vs2.size()<2){
	  cerr << "MIROS22Definition Warning: Bad Position definition:" << buf << endl;
	  continue;
	}
	float lat= atof(vs2[0].c_str());
	float lon= atof(vs2[1].c_str());
	locs[n-1].loc.setPos(miCoordinates(lon,lat),
			     0,0,locs[n-1].loc.Name());
// 	cerr << "Location:" << locs[n-1].loc.Name() << " has position:" 
//             << locs[n-1].loc.Coordinates() << endl;
      } else if (miutil::contains(buf, "Filepath=")){
	if (n==0){
	  cerr << "MIROS22Definition Warning: Define Location before Filepath:" << buf << endl;
	  continue;
	}
	vs1= miutil::split(buf, "=");
	if (vs1.size()<2){
	  cerr << "MIROS22Definition Warning: Bad Filepath definition:" << buf << endl;
	  continue;
	}
	locs[n-1].filepath= vs1[1];
// 	cerr << "Location:" << locs[n-1].loc.Name() << " has filepath:"
// 	     << locs[n-1].filepath << endl;
      }
    }
  }

  return true;
}


// --------------- MIROS22Server -------------------------------

MIROS22Server::MIROS22Server(const std::string& deffile)
  : DataStream(deffile)
{
  mirosdef.set(deffile);
}

MIROS22Server::~MIROS22Server()
{
}

bool MIROS22Server::close()
{
  IsOpen= false;
  return true;
}

int  MIROS22Server::findStation(const std::string& posName) // return index in posList
{
  std::string sname= miutil::to_upper(posName);
  for (int i=0; i<npos; i++){
    if (miutil::to_upper(mirosdef.locs[i].loc.Name()) == sname)
      return i;
  }
  return -1;
}

int  MIROS22Server::findModel(const std::string& modelName,
			      const int& modelRun)// return index in modList
{
  for (int i=0; i<nmod; i++){
    if (miutil::to_lower(modList[i]) == miutil::to_lower(modelName))
      return i;
  }
  
  return -1;
}

int MIROS22Server::findDataPar(const ParId& id) // return index in parList
{
  int rn = -1;
  int np= parameters.size();
  for (int i=0; i<np; ++i) {
    if (id == parameters[i].Id()) {
      rn = i;
      break;
    }
  }
  return rn;
}

void MIROS22Server::clean()
{
  parameters.clear();
  timeLine.clear();
  timeLines.clear();
  numTimeLines=0;
  progLine.clear();
  progLines.clear();
  DataIsRead = false;
  TimeLineIsRead = false;
  IsCleaned = true;
}

bool MIROS22Server::openStream(ErrorFlag* ef)
{
  IsOpen= false;
  if (!mirosdef.scan()) return false;
  modList.clear();
  set<Model> models;
  int n= mirosdef.pars.size();
  for (int i=0; i<n; i++)
    models.insert(mirosdef.pars[i].parid.model);
  set<Model>::iterator p= models.begin();
  for (; p!=models.end(); p++)
    modList.push_back(*p);

  nmod= modList.size();
  npos= mirosdef.locs.size();
  npar= 0;

  IsOpen=true;
  InfoIsRead = true;
  *ef = OK;
  return true;
}

bool MIROS22Server::openStreamForWrite(ErrorFlag*)
{
  return false;
}

bool MIROS22Server::readData(const int posIndex,
			     const ParId& modid,
			     const miTime& start,
			     const miTime& stop,
			     ErrorFlag* ef)
{
//  cerr << "Requesting MIROS read for position:" << posIndex << endl;
  if (posIndex < 0 || posIndex >= npos) {
    *ef = DF_RANGE_ERROR;
    return false;
  }

  *ef = DF_MODEL_NOT_FOUND;

  // find name of requested model
  std::string modname;
  int modrun, modidx= -1;
  modname = modid.model;
  modrun  = modid.run;
  if (modname.length()) { // check if requested model exists on file
    modidx = findModel(modname, modrun);
  }
  if (modidx==-1) return false;
  
  miTime t1= start;
  miTime t2= stop;
  if (t1.undef()){
    t1= miTime::nowTime();
    t1.addHour(-12);
  }
  if (t2.undef()){
    t2= miTime::nowTime();
  }
  int min10= t1.min()/10*10;
  t1.setTime(t1.year(),t1.month(),t1.day(),
	     t1.hour(), min10, 0);
  min10= t2.min()/10*10;
  t2.setTime(t2.year(),t2.month(),t2.day(),
	     t2.hour(), min10, 0);
  
  *ef = DF_DATA_READING_ERROR;
  cerr << "Reading d22-data for position:" << mirosdef.locs[posIndex].loc.Name()
       << " and model:" << modname << " from:" << t1 << " until:"
       << t2 << endl;
  
  vector<MIROS22parset> totvps;
  std::string filepath= mirosdef.locs[posIndex].filepath;

  int n= mirosdef.pars.size();

  /* New approach, fetch all files in a given time period */
  for (miDate date=t1.date(); date<=t2.date(); date.addDay(1)){
    glob_t globBuf;
    int glob_flags=0;

    std::string year= miutil::from_number(date.year());
    std::string month= miutil::from_number(date.month());
    if (month.length()==1) month= "0"+month;
    std::string day= miutil::from_number(date.day());
    if (day.length()==1) day= "0"+day;

    std::string pattern = filepath + year + month + day + "?*" + std::string(".d22");

    bool error = (glob(pattern.c_str(), glob_flags, 0, &globBuf) != 0);

    string_v matches;
    if (not error){
      matches = string_v(globBuf.gl_pathv, globBuf.gl_pathv + globBuf.gl_pathc);
    }

    globfree(&globBuf);

    for (int count = 0; count < matches.size(); count++){
      MIROS22value v;
      MIROS22parset ps;
      vector<MIROS22parset> vps;
      vps.push_back(ps);
      for (int i=0; i<mirosdef.pars.size(); i++){
        vps[0].val.push_back(v);
        vps[0].val[i].par= mirosdef.pars[i];
      }

      std::string filename = matches[count];
      //cerr << "Reading file: " << filename << endl;
      MIROS22File file(filename);
      MIROS22File::status stat;
      if (!file.read(mirosdef.locs[posIndex].name,date,vps,stat)){
        cerr << "Mirosfile:" << filename <<  " returned bad status:" << stat << endl;
      }
      // push on total vector
      totvps.push_back(vps[0]);
    }
  }

/*
  for (miTime t=t1; t<=t2; t.addMin(10)){
    //cerr << "Reading for time:" << t << endl;
    vector<MIROS22parset> vps;
    vps.push_back(ps);
    vps[0].t= t;
    for (int i=0; i<n; i++){
      vps[0].val.push_back(v);
      vps[0].val[i].par= mirosdef.pars[i];
    }
    // read data
    std::string year= miutil::from_number(t.year());
    std::string month= miutil::from_number(t.month());
    if (month.length()==1) month= "0"+month;
    std::string day= miutil::from_number(t.day());
    if (day.length()==1) day= "0"+day;
    std::string hour= miutil::from_number(t.hour());
    if (hour.length()==1) hour= "0"+hour;
    std::string minute= miutil::from_number(t.min());
    if (minute.length()==1) minute= "0"+minute;
    
    std::string filename= filepath 
      + year + month + day + hour + minute + std::string(".d22");

    MIROS22File file(filename);
    MIROS22File::status stat;
    if (!file.read(mirosdef.locs[posIndex].name,t.date(),vps,stat)){
       cerr << "Mirosfile:" << filename <<  " returned bad status:" << stat << endl;
    }
//     if (stat==MIROS22File::read_ok){
//       cerr << "------------Success" << endl;
//       *ef = OK;
//     } else if (stat==MIROS22File::missing_par) {
//       cerr << "------------Some parameters not found" << endl;
//       *ef= DD_SOME_PARAMETERS_NOT_FOUND;
//     }
    // push on total vector
    totvps.push_back(vps[0]);
  }
*/
  WeatherParameter wp;
  int tlidx;
  vector<miTime> times;
  vector<float> fdata;
  int numt= totvps.size(); // number of times requested

  for (int i=0; i<n; i++){
    times.clear();
    fdata.clear();
    for (int j=0; j<numt; j++){
      float value= totvps[j].val[i].value;
      //float scale=(j==3*numt/4 ? 50.0 : 1.0);
      if (value>(MIROSundef+99)){
	fdata.push_back(value);
	times.push_back(totvps[j].t);
      }
    }
    int nt= times.size();
    if (nt > 0) {
      // update timeline
      if ((tlidx=timeLines.Exist(times))==-1){
	tlidx=numTimeLines;
	timeLines.insert(times,tlidx);
	numTimeLines++;
	TimeLineIsRead = true;
      }
    
      wp.setDims(nt,1);
      for (int j=0; j<nt; j++){
	wp.setData(j,0,fdata[j]);
      }
      wp.setTimeLineIndex(tlidx);
      wp.setType(LINE);
      wp.setPolar(false);
      ParId id = totvps[0].val[i].par.parid;
      id.run = modrun;
      wp.setId(id);
      wp.calcAllProperties();
      // put wp on the list
      parameters.push_back(wp);
    }
  }
  npar= parameters.size();
  DataIsRead = true;
  return true;
}

bool MIROS22Server::getTimeLine(const int& index,
				vector<miTime>& tline,
				vector<int>& pline,
				ErrorFlag* ef)
{
  if (TimeLineIsRead && timeLines.Timeline(index,tline)) {
    if (index<progLines.size())
      pline = progLines[index];
    *ef = OK;
    return true;
  } 
  else {
    *ef = DF_TIMELINE_NOT_READ;
    return false;
  }
}

bool MIROS22Server::putTimeLine(const int& index,
				vector<miTime>& tline, vector<int>& pline,
				ErrorFlag* ef)
{
  return false;
}

bool MIROS22Server::putTimeLine(TimeLine& tl, vector<int>& pline,
				ErrorFlag* ef)
{
  return false;
}

bool MIROS22Server::getOnePar(int index,
			      WeatherParameter& wp,
			      ErrorFlag* ef)
{
  int n= parameters.size();
  if (index < 0 || index >= n) {
    *ef = DF_RANGE_ERROR;
    return false;
  }
  if (!DataIsRead) {
    *ef = DF_DATA_NOT_READ;
    return false;
  }
  
  wp = parameters[index];
  *ef = OK;
  return true;
}

bool MIROS22Server::putOnePar(WeatherParameter&,ErrorFlag*)
{
  return false;
}

// fetch name and position of station idx
bool MIROS22Server::getStationSeq(int idx,
				  std::string& name,
				  float& lat,
				  float& lng)
{
  if (idx >= 0 && idx < npos) {
    name= mirosdef.locs[idx].loc.Name();
    lat= mirosdef.locs[idx].loc.lat();
    lng= mirosdef.locs[idx].loc.lon();
    return true;
  } else return false;
}

bool MIROS22Server::getStationSeq(int idx,
				  miPosition& p)
{
  if (idx >= 0 && idx < npos) {
    p= mirosdef.locs[idx].loc;
    return true;
  } else return false;
}

bool MIROS22Server::getModelSeq(int idx, Model& mod,       // fetch model info
				Run& run, int& id)
{
  if (idx >= 0 && idx < nmod) {
    mod= modList[idx];
    run = 0;
    id= 33;
    return true;
  }
  return false;
}

bool MIROS22Server::getModelSeq(int idx, Model& mod,       // fetch model info
				Run& run, int& id,
				vector<std::string>& vs)
{
  if (idx >= 0 && idx < nmod) {
    mod= modList[idx];
    run = 0;
    id= 33;
    return true;
  }
  return false;
}

int  MIROS22Server::putStation(const miPosition& s, //adds station to posList
			       ErrorFlag*)
{
  return -1;
}

bool MIROS22Server::writeData(const int posIndex,      //write data to file
			      const int modIndex,
			      ErrorFlag*,
			      bool complete_write,
			      bool write_submodel)
{
  return false;
}
