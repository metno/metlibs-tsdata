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

#include "ptAsciiStream.h"
#include "puCtools/puMath.h"

#include <cmath>
#include <iostream>
#include <fstream>

#include <string.h>
#include <float.h>

using namespace miutil;
using namespace std;

AsciiStream::AsciiStream(const std::string& fname)
  : DataStream(fname)
{ 
#ifdef DEBUG
  cout << "Inside AsciiStream:constructor" << endl;
#endif
}

AsciiStream::~AsciiStream()
{
#ifdef DEBUG
  cout << "Inside AsciiStreams destructor" << endl;
#endif
  close();
}

bool AsciiStream::close()
{
  // close the AsciiStream
  if (IsOpen) {
    IsOpen= false;
  }
  return true;
}

// return index of station 'statName' in posList, -1 if not found
int AsciiStream::findStation(const std::string& statName)
{
#ifdef DEBUG
  cout << "AsciiStream::findStation:" << statName << endl;
#endif
  int rn=-1, i;
  std::string sname = miutil::to_upper(statName);
  for (i=0; i<npos; ++i) {
    if (miutil::to_upper(posList[i].Name()) == sname) {
      rn = i;
      break;
    }
  }
  return rn;
}


bool AsciiStream::getStations(vector<miPosition>& poslist){
#ifdef DEBUG
  cout << "AsciiStream::getStations" << endl;
#endif
  poslist= posList;
  return true;
}


bool AsciiStream::getStationSeq(int idx, miPosition& pos){
#ifdef DEBUG
  cout << "AsciiStream::getStationSeq" << endl;
#endif
  if (idx >= 0 && idx < npos) {
    pos= posList[idx];
    return true;
  } else {
    return false;
  }
}

int AsciiStream::putStation(const miPosition& s,
			    ErrorFlag* ef)
{
  posList.push_back(s);
  npos++;
  *ef = OK;
  return posList.size()-1;
}


bool AsciiStream::getModelSeq(int idx, Model& mod,
			      Run& run, int& id)
{
  if (idx >= 0 && idx < nmod) {
    mod  = modList[idx].name;
    run  = (modList[idx].run.undef() ? R_UNDEF : modList[idx].run.hour());
    id   = 0;
    return true;
  } else {
    return false;
  }
}

// return index of model 'modelName' and run modelRun 
// in modList, -1 if not found
int AsciiStream::findModel(const std::string& modelName,
			   const int& modelRun)
{
  int rn=-1, i;
  for (i=0; i<nmod; ++i) {
    if ((modList[i].name==modelName) &&
	(modelRun==R_UNDEF || modList[i].run.undef() ||
	 modList[i].run.hour()==modelRun)) {
      rn = i;
      break;
    }
  }
  return rn;
}


int AsciiStream::findDataPar(const ParId& id)
{
#ifdef DEBUG
  cout << "AsciiStream::findDataPar" << endl;
#endif
  int rn = -1;
  for (int i=0; i<npar; ++i) {
    if (id == parameters[i].Id()) {
      rn = i;
      break;
    }
  }
  return rn;
}
      
void AsciiStream::clean()
{
#ifdef DEBUG
  cout << "AsciiStream::clean" << endl;
#endif
  parameters.clear();
  npar= 0;
  timeLine.clear();
  timeLines.clear();
  numTimeLines=0;
  progLine.clear();
  progLines.clear();
  DataIsRead = false;
  TimeLineIsRead = false;
  IsCleaned = true;
}

bool AsciiStream::openStream(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::openFileAndReadHeader" << endl;
#endif
  if (!_openFile(ef))
    return false;
  InfoIsRead = true;
  *ef = OK;
  return true;
}

bool AsciiStream::openStreamForWrite(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::openFileForWrite" << endl;
#endif
  return false;
}

//---------------------------------------------------------------
// Reads data from file for position "posIndex" and model and 
// modelrun as specified in the ParId "modid"
// If modid.model is undefined the first model found for "posIndex"
// is taken.
// The same goes for modid.run.
//---------------------------------------------------------------
bool AsciiStream::readData(const int posIndex, 
			   const ParId& modid,
			   const miTime& start,
			   const miTime& stop,
			   ErrorFlag* ef)
{
  std::string modname;
  int modrun, modidx= -1;

#ifdef DEBUG
  cout << "AsciiStream::readData" << endl;
#endif

  if (posIndex < 0 || posIndex >= npos) {
    *ef = DF_RANGE_ERROR;
    return false;
  }

  *ef = DF_MODEL_NOT_FOUND;

  // find name of requested model
  modname = modid.model;
  modrun  = modid.run;
#ifdef DEBUG
  cout << "Requested model name:" << modname << endl;
  cout << "Requested model run :" << modrun << endl;
#endif
  if (modname != M_UNDEF) { // check if requested model exists on file
    modidx = findModel(modname, modrun);
    if (modidx==-1) return false;
    // set correct modelrun
    modrun=modList[modidx].run.hour();
  }

  // find correct data for position and model
  int n= dataList.size();
  int didx;
  for (didx=0; didx<n; didx++){
    if (dataList[didx].pos.Name()==posList[posIndex].Name()){
      if (modidx < 0) break;
      if (dataList[didx].model.name==modList[modidx].name)
	break;
    }
  }

  if (didx == n){
    return false;
  }

  *ef = DF_DATA_READING_ERROR;

  WeatherParameter wp; // basis for all weatherparameters found
  ParId pid = ID_UNDEF;
  pid.model = modname;
  pid.run   = modrun;

  //timeLine= dataList[didx].times;
  TimeLineIsRead = true;

  set<int>::iterator leveli, submi;
  set<miTime>::iterator timei;

  int np= dataList[didx].params.size();
  int nt= dataList[didx].pardata.size();

  for (submi=dataList[didx].submodels.begin();
       submi != dataList[didx].submodels.end();
       submi++){
    //cerr << "Reading for SubModel:" << *submi << endl;
    
    for (leveli=dataList[didx].levels.begin();
	 leveli != dataList[didx].levels.end();
	 leveli++){
      //cerr << "Reading for Level:" << *leveli << endl;
      
      // loop over parameters
      // add a new weatherparameter for each unique level and submodel
      for (int ip=0; ip<np; ip++){
	//cerr << "Reading for parameter:" << dataList[didx].params[ip] << endl;
	// find data for current param, level and submodel
	vector<float>  data;
	vector<miTime> times;
	for (int it=0; it<nt; it++){
	  if (dataList[didx].pardata[it].level == *leveli &&
	      dataList[didx].pardata[it].submodel == *submi &&
	      dataList[didx].pardata[it].data[ip] != UNDEF){
	    data.push_back(dataList[didx].pardata[it].data[ip]);
	    times.push_back(dataList[didx].pardata[it].time);
	  }
	}
	if (data.size() == 0) continue;

	wp.setDims(times.size(),1);
	int ipar= parameters.size();
	int tlindex;
	parameters.push_back(wp); // add it to the vector
	
	// insert the actual data
	for (size_t j=0; j<times.size(); j++) {
	  parameters[ipar].setData(j,0,data[j]);
	}
	// check if new timeline already exist, add it
	if ((tlindex=timeLines.Exist(times))==-1){
	  tlindex=numTimeLines;
	  timeLines.insert(times,tlindex);
	  numTimeLines++;
	}
	parameters[ipar].setTimeLineIndex(tlindex);
#ifdef DEBUG
	cout << "The full timeline so far:\n" << timeLines << endl;
#endif
	// add the progLine
	//progLines.push_back(ptimes);

	// (pid is now fully defined)
	pid.alias = dataList[didx].params[ip];
	pid.level = int(*leveli);
	pid.submodel= miutil::from_number(*submi);
	parameters[ipar].setId(pid);
#ifdef DEBUG
	cout << "the complete wp:"<<  parameters[ipar]<<endl;
#endif
      }
    }    
  }

  npar= parameters.size();
  *ef = OK;
  DataIsRead = true;
  return true;
}


bool AsciiStream::writeData(const int posIndex,
			    const int modIndex,
			    ErrorFlag* ef,
			    bool complete_write,
			    bool write_submodel)
{
  return false;
}



bool AsciiStream::getTimeLine(const int& index,
			      vector<miTime>& tline,
			      vector<int>& pline,
			      ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::getTimeLine" << endl;
#endif
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

bool AsciiStream::putTimeLine(const int& index,
			      vector<miTime>& tline,
			      vector<int>& pline,
			      ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::putTimeLine" << endl;
#endif
  timeLines.insert(tline,progLines.size());
  progLines.push_back(pline);
  *ef = OK;
  return true;
}

bool AsciiStream::putTimeLine(TimeLine& tl,
			      vector<int>& pline,
			      ErrorFlag* ef)
{

#ifdef DEBUG
  cout << "AsciiStream::putTimeLine" << endl;
#endif
  timeLines = tl;
  progLines.push_back(pline);
  *ef = OK;
  return true;
}

bool AsciiStream::getOnePar(int index, WeatherParameter& wp, ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::getOnePar" << endl;
#endif
  npar= parameters.size();
  if (index < 0 || index >= npar) {
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

bool AsciiStream::putOnePar(WeatherParameter& wp, ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::putOnePar" << endl;
#endif
  return false;
}

bool AsciiStream::_openFile(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "AsciiStream::_openFile" << endl;
#endif
  ifstream file(Name.c_str());
  if (!file){
    *ef = DF_FILE_OPEN_ERROR;
    return false;
  }

  dataList.clear();
  posList.clear();
  modList.clear();

  std::string buf;
  vector<std::string> vt1, vt2;
  bool header_found= false;
  bool data_saved= true;
  bool use_submodel= false;
  AsciiData adata;

  while (getline(file,buf)){
    int p;
    if ((p=buf.find("#"))!=string::npos)
      buf.erase(p);

    miutil::remove(buf, '\n');
    miutil::trim(buf);

    if (buf.length() == 0) continue;

    if (miutil::contains(miutil::to_upper(buf), "MODEL=")){
      if (!data_saved) dataList.push_back(adata);
      data_saved= true;
      adata.params.clear();

      AsciiMod  amod;
      vt1= miutil::split(buf, "=");
      vt2= miutil::split(vt1[1], ",");
      amod.name= vt2[0];
      if (vt2.size()>1) {
	if (miTime::isValid(vt2[1])) amod.run.setTime(vt2[1]);
      }
      //if (amod.run.undef()) amod.run= miTime::nowTime();
      modList.push_back(amod);
      adata.model= amod;
      
      header_found= false;
    } else if (miutil::contains(miutil::to_upper(buf), "POSITION=")){
      if (!data_saved) dataList.push_back(adata);
      data_saved= true;
      adata.params.clear();

      vt1= miutil::split(buf, "=");
      vt2= miutil::split(vt1[1], ",");
      std::string name= vt2[0];
      miCoordinates c;
      if (vt2.size()>2){
        float loni= miutil::to_double(vt2[1]);
	float lati= miutil::to_double(vt2[2]);
	c= miCoordinates(loni,lati);
      }
      miPosition pos(c,0,0,name);
      posList.push_back(pos);
      adata.pos= pos;
      header_found= false;

    } else if (header_found){
      vt1= miutil::split(buf, " ");
      if (vt1.size() != 2+1+(use_submodel ? 1 : 0)+adata.params.size()){
	cerr << "AsciiStream error: number of entries in dataline does not match header:"
	     << buf << endl;
	return false;
      }
      miTime t;
      std::string times= vt1[0]+" "+vt1[1];
      if (miTime::isValid(times)) 
	t.setTime(times);
      else {
	cerr << "AsciiStream error: invalid isotime in dataline:"
	     << buf << endl;
	return false;
      }
      int level= miutil::to_int(vt1[2]);
      int submodel= 0;
      int ds= 3;
      if (use_submodel) {
        submodel= miutil::to_int(vt1[3]);
	ds= 4;
      }
      AsciiLine line;
      line.time= t;
      line.prog= 0;
      line.level= level;
      line.submodel= submodel;
      for (size_t j=ds; j<vt1.size(); j++){
	float f= UNDEF;
	if (vt1[j] != "-")
	  f= atof(vt1[j].c_str());
	line.data.push_back(f);
      }
      adata.pardata.push_back(line);

      adata.levels.insert(level);
      adata.submodels.insert(submodel);
      adata.times.insert(t);
      
    } else {
      vt1= miutil::split(buf, " ");
      if (vt1.size() < 2 || 
	  (miutil::to_upper(vt1[0])!="TIME" || miutil::to_upper(vt1[1])!="LEVEL")){
	cerr << "AsciiStream error: table-header should start with"
	     << " \"TIME LEVEL\" " << endl;
	return false;
      }
      int ds= 2;
      if (vt1.size()>2 && miutil::to_upper(vt1[2])=="SUBMODEL"){
	ds= 3;
	use_submodel= true;
      }
      for (size_t j=ds; j<vt1.size(); j++){
	adata.params.push_back(vt1[j]);
      }
      adata.levels.clear();
      adata.submodels.clear();
      adata.times.clear();
      adata.pardata.clear();
      header_found= true;
      data_saved= false;
    }
  }
  if (!data_saved) dataList.push_back(adata);

  npos= posList.size();
  nmod= modList.size();
  npar= 0;

  //     *ef = DF_FILE_ACCESS_ERROR;

  IsOpen = true;
  *ef = OK;
  return true;
}
