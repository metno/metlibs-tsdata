/*
  libtsData - Time Series Data
  
  $Id: ptGribStream.cc 576 2007-12-03 07:39:16Z audunc $

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


// ptGribStream.cc

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ptGribStream.h"

#include <puCtools/puMath.h>
#include <diField/diFieldManager.h>  //added

#include <string.h>
#include <math.h>
#include <float.h>
#include <fstream>
#include <algorithm>
#include <list>
#include <iostream>
//#define DEBUG

const std::string SectParameters=  "TSERIES_PARAMETERS";
const std::string SectModel=  "TSERIES_MODEL";
const std::string SectStationList= "STATION_LIST";
const std::string SectGribParMod= 	"PARAMETERS_MODELS";

//map<std::string,SetupSection> GribStream::sectionm;
//map<std::string,std::string>     GribStream::substitutions;

using namespace std;
using miutil::SetupParser;

bool GribStream::_parseGrib()
{
#ifdef DEBUG
  cerr << "GribStream::_parseGrib:-----starting ---setup filename : " << Name << endl;
#endif
  vector<std::string> list,tokens,stokens;
  std::string key,value, str;
  size_t n,p;

   sp->parse(Name);

  //parsing <STATION_LIST>
  if (!sp->getSection(SectStationList,list))
    return false;
  for (n=0; n< list.size(); n++) {
      str = list[n];
      if ((p = str.find ("#")) == 0) {
        continue;
      }
      else {
        vector<std::string> vs= miutil::split(list[n], 2, "=", true);
        if (vs.size()==2) {
           key=miutil::to_lower(vs[0]); // always converting keyword to lowercase !
           value=vs[1];
           // structures of type: A=B 
           if (key == "file" ) stFileName = value;
        }
     }
  }

  list.clear();
  //parsing <PARAMETERS_MODELS>

  if (!sp->getSection(SectGribParMod,list))
    return false;
  for (n=0; n< list.size(); n++) {
      str = list[n];
      if ((p = str.find ("#")) == 0) {
        continue;
      }
      else {
        vector<std::string> vs= miutil::split(list[n], 2, "=", true);
        if (vs.size()==2) {
         key=miutil::to_lower(vs[0]); // always converting keyword to lowercase !
         value=vs[1];
         // structures of type: A=B 
         if (key == "file")
             parmodFileName = value;
       }
     }
  }
  list.clear();
#ifdef DEBUG
  cerr << "GribStream::_parseGrib:   stationfile:  " << stFileName << endl;
  cerr << "GribStream::_parseGrib:   commongribfile:  " << parmodFileName << endl;
  cerr << "GribStream::_parseGrib:   n=  " << n << endl;
#endif
  sp->parse(parmodFileName);
  

 
  return true; 

}

GribStream::GribStream(const std::string& fname)
  : DataStream(fname), hasposVG(false)
{ 
#ifdef DEBUG
  cout << "GribStream::GribStream Inside GribStream:constructor" << endl;
#endif
  timeLine.reserve(100);  // We don't expect more than 100 timepoints
  progLine.reserve(100);
  parameters.reserve(20);       // we don't expect more than 20 parameters
  //  textLines.reserve(0); 
  //sectionm.clear(); 

  // Starting FieldManager to handle gribfiles
  fieldm=     new FieldManager;
  // Starting SetupParser to parse .grib file and other related setup files 
  sp=     new SetupParser;
  sp->clearSect(); 
  //parse(.grib setupfile);
  _parseGrib();

  //Parse field sections
  vector<std::string> fieldSubSect = fieldm->subsections();
  fieldSubSect.push_back(fieldm->section());
  int nsect = fieldSubSect.size();
  vector<std::string> errors;
  for( int i=0; i<nsect; i++){
    vector<std::string> lines;
#ifdef DEBUG
        cout<<"*****************Section "<<fieldSubSect[i]<<" in FieldManager."<<endl;
#endif
    if (sp->getSection(fieldSubSect[i],lines)) {
#ifdef DEBUG
        cout<<"*****************section "<<fieldSubSect[i]<<" in setupfile."<<endl;
#endif
        if (!fieldm->parseSetup(lines,fieldSubSect[i],errors /* ,false*/))
           cerr << "*****************FieldManager-> parseSetup misslyckas" << endl;
    }
  }

  
 
}

GribStream::~GribStream()
{
#ifdef DEBUG
  cout << "Inside GribStreams destructor" << endl;
#endif
  delete fieldm;
  delete sp;
  close();
}


bool GribStream::close()
{
  // setupfile is already closed after have been parsed
  if (IsOpen) IsOpen= false;
  
  return true;
}

// return index of station 'statName' in posList, -1 if not found
int GribStream::findStation(const std::string& statName)
{
#ifdef DEBUG
  cout << "GribStream::findStation:" << statName << endl;
#endif
  int rn=-1, i;
  std::string sname= miutil::to_upper(statName);
  for (i=0; i<npos; ++i) {
    if (miutil::to_upper(posList[i].name) == sname) {
      rn = i;
      break;
    }
  }
  return rn;
}


bool GribStream::getStations(vector<miPosition>& poslist){
#ifdef DEBUG
  cout << "GribStream::getStations" << endl;
#endif
  poslist.clear();
  miPosition pos;
  miCoordinates c;
  for (int i=0; i<npos; i++) {
    c= miCoordinates(posList[i].geopos[1],posList[i].geopos[0]);
    pos.setPos(c,0,0,posList[i].name,0,0,"");
    poslist.push_back(pos);
  }
  return true;
}


bool GribStream::getStationSeq(int idx, miPosition& pos){
#ifdef DEBUG
  cout << "GribStream::getStationSeq---------starting-------" << endl;
//  cout << "          ::getStationSeq idx= " << idx << "   npos=" << npos << endl;

#endif
  if (idx >= 0 && idx < npos) {
    miCoordinates c(posList[idx].geopos[1],posList[idx].geopos[0]);
    pos.setPos(c,0,0,posList[idx].name,0,0,"");
/*#ifdef DEBUG
  cout << "          ::getStationSeq: idx =" <<idx << endl;
  cout << "          ::getStationSeq: posList[idx].name= " <<posList[idx].name<< endl;
#endif*/
    return true;
  } else {
#ifdef DEBUG
  cout << "          ::getStationSeq ---- returning false-------" << endl;
#endif
    return false;
  }
}

int GribStream::putStation(const miPosition& s,
			ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::putStation" << endl;
#endif
  GribPos temp;
  temp.ref = 0;
  temp.name = s.Name();
  temp.topo = 0; // obs..
  temp.geopos[0] = s.lat();
  temp.geopos[1] = s.lon();

  posList.push_back(temp);
  npos++;
  *ef = OK;
  return posList.size()-1;
}


bool GribStream::getModelSeq(int idx, Model& mod,
			  Run& run, int& id)
{
   nmod=1;  //Måste redas ut vad är för något
#ifdef DEBUG
  cout << "GribStream::getModelSeq1" << endl;
  cout << "nmod:"<<nmod<<" idx:"<<idx<<endl;
#endif
  if (idx >= 0 && idx < nmod) {
    mod  = modList[idx].modelid;
    run  = modList[idx].runid;
    id   = modList[idx].modelPn;
#ifdef DEBUG
    cout << "GribStream::getModelSeq1 model,run,id,idx :"<<mod<<" "<<run<<" "<<id << "  "<<idx<<endl;
#endif
    return true;
  } else {
    return false;
  }
}

bool GribStream::getModelSeq(int idx, Model& mod,
			  Run& run, int& id,
			  vector<std::string>& vtl)
{
   nmod=1;  //Måste redas ut vad är för något
#ifdef DEBUG
  cout << "GribStream::getModelSeq2" << endl;
  cout << "nmod:"<<nmod<<" idx:"<<idx<<endl;
#endif
  if (idx >= 0 && idx < nmod) {
    mod  = modList[idx].modelid;
    run  = modList[idx].runid;
    id   = modList[idx].modelPn;
    vtl  = modList[idx].textlines;
#ifdef DEBUG
    cout << "GribStream::getModelSeq2 model,run,id, idx :"<<mod<<" "<<run<<" "<<id <<"  " <<idx <<endl;
#endif
    return true;
  } else {
    return false;
  }
}

// return index of model 'modelName' and run modelRun 
// in modList, -1 if not found
int GribStream::findModel(const std::string& modelName,
		       const int& modelRun)
{
#ifdef DEBUG
  cout << "GribStream::findModel" << endl;
  cout << "GribStream::findModel modelName"<< modelName << endl;
  cout << "GribStream::findModel modelRun" << modelRun << endl;
#endif
  int rn=-1, i;
  for (i=0; i<nmod; ++i) {
    if ((modList[i].name==modelName) &&
	(modList[i].run[3]==modelRun || modelRun==R_UNDEF)) {
      rn = i;
      break;
    }
  }
#ifdef DEBUG
  cout << "GribStream::findModel returning" << rn << endl;
#endif
  return rn;
}

bool GribStream::getFullModeltime(int id, miutil::miTime& t)
{
  if ( id<0 || id>=modList.size() ) return false;

  t = miutil::miTime(modList[id].run[0],modList[id].run[1],modList[id].run[2],
                     modList[id].run[3],modList[id].run[4],modList[id].run[5]);

  return true;
}


int GribStream::findDataPar(const ParId& id)
{
#ifdef DEBUG
  cout << "GribStream::findDataPar" << endl;
#endif
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
      
void GribStream::clean()
{
#ifdef DEBUG
  cout << "GribStream::clean" << endl;
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

bool GribStream::openStream(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::openStream -----starting-----" << endl;
#endif
  if (!_openFile(ef)){
    cerr << "Error opening file." <<  endl;
    return false;
}
  if (!_readPosList(ef)){
    cerr << "Error reading position list." << endl;
    return false;
}
  if (!_readParList(ef)){
    cerr << "Error reading parameter list." << endl;
    return false;
}
  if (!_readModList(ef)){
    cerr << "Error reading model list." << endl;
    return false;
}
  InfoIsRead = true;
  *ef = OK;
#ifdef DEBUG
  cout << "GribStream::openStream -----finishing-----" << endl;
#endif
  return true;
}

// This function not used in this class
bool GribStream::openStreamForWrite(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::openFileForWrite" << endl;
#endif
/*  if (!_createFile(ef))
    return false;
*/
  *ef = OK;
  return true;
}

//---------------------------------------------------------------
// Reads data from file for position "posIndex" and model and 
// modelrun as specified in the ParId "modid"
// If modid.model is undefined the first model found for "posIndex"
// is taken.
// The same goes for modid.run.
//---------------------------------------------------------------
// Problems: If multiple runs exists for a given model, the first
// run will be chosen..
//---------------------------------------------------------------
bool GribStream::readData(const int posIndex, 
		       const ParId& modid,
		       const miutil::miTime& start,
		       const miutil::miTime& stop,
		       ErrorFlag* ef)
{
  int32  nrec;
  int32  nfields;
  vector<std::string> nameFields;
  vector<int>   orderFields;
  vector<int>   fieldSize;
  size_t i, j, k, l;

  std::string modname,modName;
  int modrun, modidx= -1;

#ifdef DEBUG
  cout << "GribStream::readData posIndex:   " << posIndex<< endl;
  cout << "GribStream::readData ParId& modid:      " << modid<< endl;
  cout << "GribStream::readData start:      " << start<< endl;
  cout << "GribStream::readData stop:       " << stop<< endl;
#endif

  if (posIndex < 0 || posIndex >= npos) {
    *ef = DF_RANGE_ERROR;
    return false;
  }

  *ef = DF_MODEL_NOT_FOUND;

  // find name of requested model
  modname = (modid.model!=M_UNDEF ? modid.model : "");
  modrun  = modid.run;
#ifdef DEBUG
  cout << "Requested model name: " << modname << endl;
  cout << "Requested model run : " << modrun << endl;
#endif
  if (modname.length()) { // check if requested model exists on file
    modidx = findModel(modname, modrun);
    if (modidx==-1) return false;
    // set correct modelrun
    modrun=modList[modidx].run[3];
    modName=modList[modidx].alias;
#ifdef DEBUG
  cout << "     ::readData modrun: " << modrun<< endl;
#endif

  }
  //Getting lat and long for pos with posIndex.
  float lat =0, lon=0;
  vector<std::string> params;
  for (size_t posnum= 0; posnum< posList.size(); posnum++){
      if (posnum == posIndex){
       lat = posList[posIndex].geopos[0];
       lon = posList[posIndex].geopos[1];
      }
  }
#ifdef DEBUG
  cout << "Requested lon:  " << lon << endl;
  cout << "Requested lat:  " << lat << endl;
  cout << "Requested gribModelName:  " << modName << endl;
#endif
  params.clear();
  //Getting list of parameters for gribfile
  for (size_t pl= 0; pl < parList.size(); pl++) {
      params.push_back(parList[pl].name);
  }

  //clearing the data map 
  data.clear();


  if (!fieldm->makeTseries(modName, validTime, lat, lon, params,data)) { 
     *ef = DF_DATA_READING_ERROR;
     cerr << "************************FALSE*****makeTseries for model " << modName << endl;
     return false;
  }
/*
#ifdef DEBUG
  cout << "Who map data_iterator starting " <<  endl;

  std::map<std::string,std::vector<float> >::iterator p;
  for (p=data.begin(); p!=data.end(); ++p){
    cout << "Who (key=first): " << p->first<< endl;
    for (int j= 0; j < p->second.size(); j++) {
        cout << "Value (j):   " <<p->second[j]<< endl;
    }
  }
  for(int i=0;i<validTime.size();i++) {
    for(int j=0;j<params.size();j++) {
      cerr << "data[" << params[j] << "][" << i << "]: " << data[params[j]][i] << endl;
    }
  }
  vector<float> arival = data["tk2m"];
  for ( int ari=0; ari<arival.size(); ari++){
     cout << "Value ( " << ari << " ):   " << arival[ari]<< endl;
  }
  cout << "Who map data_iterator finishing " <<  endl;


#endif*/

/////////////////////////////////////////////////////////////////////////////
  // ok..we have a satisfactory model and run

  nfields = parList.size();
  nrec = validTime.size();
  // the nameFields contains all parameter short names found in the vdata
  // the orderFields contains the corresponding order of these parameters
  // the fieldSize contains the data size in bytes
  nameFields.reserve(nfields);
  orderFields.reserve(nfields);
  parameters.reserve(nfields);
  //Getting list of parameters for gribfile   
  for (int p= 0; p < nfields; p++) {
      nameFields.push_back(parList[p].alias);
      orderFields.push_back(parList[p].order);
      fieldSize.push_back(parList[p].size);
  

#ifdef DEBUG
    cout << "Fetching field characteristics:" << p << endl;
    cout << "Name:" << nameFields[p] << endl;
    cout << "Order:" << orderFields[p] << endl;
    cout << "Size:" << fieldSize[p] << endl;
#endif
  }

  *ef = DF_DATA_READING_ERROR;

#ifdef DEBUG
//  cout << "GribStream:readData: fields:" << fields <<endl;
  cout << "..and nrec:"<<nrec<<" recsz:"<<recsz<<" interlace:"
       <<interlace<<endl;
#endif

  // Organize the data in dataBuf. Separate the actual parameterdata
  // from the rest (Time, Prog, Level (and sensor))
  vector<int> levelList;
  vector<float> uniqLevels;

  vector<int> ULCount; // Unique level count
  WeatherParameter wp; // basis for all weatherparameters found
  ParId pid = ID_UNDEF;
  pid.model = modname;
  pid.run   = modrun;
//******************************
    // make the timeline
    for (j=0;j<validTime.size();j++) {
	timeLine.push_back(validTime[j]);
#ifdef DEBUG
	cout << "Adding to timeline:" << timeLine[j] << endl;
#endif
      
      TimeLineIsRead = true;
    }

    // make the progline and levellist
    for (int jk=0;jk<forecastHour.size();jk++) {
	progLine.push_back(forecastHour[jk]);
	levelList.push_back(0);
#ifdef DEBUG
	cout << "Adding to progline:" << progLine[jk] << endl;
#endif
      
    }
// just one level is supported for gribfiles
/*    // make a list of levels
    for (j=0;j<nrec;++j) {
	// keep a list of unique levels
 	levelfound = false;
 	for (k=0;k<uniqLevels.size() && !levelfound; k++)
 	  levelfound=(uniqLevels[k]==fdata[j]);
	if (!levelfound){
	  uniqLevels.push_back(fdata[j]);
	  k=uniqLevels.size()-1;
	  ULCount.push_back(0);//List of occurences
#ifdef DEBUG
	  cout <<"NEW level:"<<fdata[j]<<endl;
#endif
	} else k--;
	levelList.push_back(k);
	ULCount[k]++;
#ifdef DEBUG
 	cout << "Number of timesteps for level:"<<k
 	     <<" is now:"<<ULCount[k]<<endl;
#endif
    }
#ifdef DEBUG
      cout << "Num levels found:"<<uniqLevels.size()<<endl;
#endif

*/ float temp = 0.0;
   for (i=0;i<nfields;++i){
      // the weather parameters
#ifdef DEBUG
      cout << "A new weatherparameter:-----starting----  " << nameFields[i] << endl;
#endif
   // getting an array of values for each parameters from gribfiles
   // and make approriate calculations
      vector<float> fdata = data[parList[i].name];
      vector<float> fdata_nrb = data["se_accumprecip"];
  for ( int ari=0; ari < fdata.size(); ari++){
    if (parList[i].name == "tk2m"){
       temp= (fdata[ari] -273.13)*10.0 ;
       fdata[ari]= temp;
       //cout << "fdata[" << ari <<" ] "<<fdata[ari]<<  endl;
    }
    else if (parList[i].name == "mslp_pa"){
       temp= fdata[ari] /10.0 ;
       fdata[ari]= temp;
       //cout << "fdata[" << ari <<" ] "<<fdata[ari]<<  endl;
    }
    else if ((parList[i].name == "u10m") ||(parList[i].name == "v10m") ){
       temp= fdata[ari] *100.0 ;
       fdata[ari]= temp;
       //cout << "fdata[" << ari <<" ] "<<fdata[ari]<<  endl;
    }
    else if ((parList[i].name == "cloud.low")|| (parList[i].name == "cloud.medium") ||(parList[i].name == "cloud.high")|| (parList[i].name == "cloud.fog")){
       temp= fdata[ari] * 100.0 ;
       fdata[ari]= temp;
       //cout << "fdata[" << ari <<" ] "<<fdata[ari]<<  endl;
    }
    else if (parList[i].name == "se_accumprecip" ){
       if (ari >0) {
         temp= (fdata_nrb[ari] - fdata_nrb[ari-1])*100.0 ;
         fdata[ari]= temp; 
       }
       else {
          temp= fdata[ari] *100.0 ;
          fdata[ari]= temp;
       }
       //cout << "fdata[" << ari <<" ] "<<fdata[ari]<<  endl;
    }
   
    else {continue;}
  }
        
      int ipar = parameters.size();
      int curlevel;
      vector<int> parTimes[MAXLEV]; // MAXLEV from HDFdefs.h
      vector<miutil::miTime> times; // timeline for each level
      vector<int> ptimes;   // progline for each level
      int tlindex; // new timeline index

      // check each timestep for data != undefValue (data is always defined)
      for (j=0;j<nrec;++j) {
	curlevel = levelList[j]; // index to uniqLevels
	parTimes[curlevel].push_back(j);
#ifdef DEBUG
 	cout << "Timestep for data" << j <<" : "<<j<<  endl;
 	cout << "Timestep for curlevel" << curlevel<< endl;
#endif

      }
      // add a new weatherparameter for each unique level=1
      for (k=0;k<1;k++){
	if (parTimes[k].size()){
	  wp.setDims(parTimes[k].size(),orderFields[i]);
	  parameters.push_back(wp); // add it to the vector
	  // insert the actual data
	  for (j=0;j<parTimes[k].size();++j) {
	    for (l=0;l<orderFields[i];++l) // insert values..
	      parameters[ipar].setData(j,l,fdata[parTimes[k][j]+l*nrec]);
	    times.push_back(timeLine[parTimes[k][j]]);
	    ptimes.push_back(progLine[parTimes[k][j]]);
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
	  progLines.push_back(ptimes);
	  times.erase(times.begin(),times.end());
	  ptimes.erase(ptimes.begin(),ptimes.end());

	  // complete the update in _setData
	  // (pid is now fully defined)
	  pid.alias = nameFields[i];
	  pid.level = 0;
	  _setData(ipar,nameFields[i],pid);
#ifdef DEBUG
	  cout << "the complete wp:"<<  parameters[ipar]<<endl;
#endif
	  ipar++;
	}
        fdata.erase(fdata.begin(),fdata.end()); 
        fdata_nrb.erase(fdata_nrb.begin(),fdata_nrb.end()); 
      }
    
   
  } //nfields
  

  *ef = OK;
  DataIsRead = true;
  return true;
}


bool GribStream::writeData(const int posIndex,
			const int modIndex,
			ErrorFlag* ef,
			bool complete_write,
			bool write_submodel)
{
/*
  if (complete_write){
    Vdetach(posVG);
    if (!_writeParList(ef)) return false;
    if (!_prepPosRefs(ef))  return false;
    if (!_writePosList(ef)) return false;
    if (!_writeModList(ef)) return false;
  }
*/

  *ef = OK;
  return true;
}



bool GribStream::getTimeLine(const int& index,
			  vector<miutil::miTime>& tline,
			  vector<int>& pline,
			  ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::getTimeLine" << endl;
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

bool GribStream::putTimeLine(const int& index,
			  vector<miutil::miTime>& tline,
			  vector<int>& pline,
			  ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::putTimeLine" << endl;
#endif
  timeLines.insert(tline,progLines.size());
  progLines.push_back(pline);
  *ef = OK;
  return true;
}

bool GribStream::putTimeLine(TimeLine& tl,
			  vector<int>& pline,
			  ErrorFlag* ef)
{

#ifdef DEBUG
  cout << "GribStream::putTimeLine" << endl;
#endif
  timeLines = tl;
  progLines.push_back(pline);
  *ef = OK;
  return true;
}

bool GribStream::getOnePar(int index, WeatherParameter& wp, ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::getOnePar1" << endl;
#endif
  int np= parameters.size();
  if (index < 0 || index >= np) {
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

bool GribStream::putOnePar(WeatherParameter& wp, ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::putOnePar2" << endl;
#endif
  size_t i;
  Parameter par;
  GribPar hpar;
  GribMod hmod;
  parameters.push_back(wp);

  ParId pid = wp.Id();

  // check if parameter exist in parList
  for (i=0; (i<parList.size())&&(parList[i].alias!=pid.alias);i++)
    ;
  if (i==parList.size()){ //parameter not found
    if (pdef.getParameter(pid.alias,par)) {
      hpar.num   = uint16(par.num());
      hpar.name  = par.name();
      hpar.alias = par.alias();
      hpar.unit  = par.unit();
      hpar.scale = int8(par.scale());
      hpar.size  = int32(par.size());
      hpar.order = int32(par.order());
      hpar.dataType = uint16(par.datatype());
      hpar.plotType = uint16(par.plottype());
      parList.push_back(hpar);
      npar++;
    } else {
      cerr << "ptGribStream::putOnePar  Warning, parameter:" << pid.alias
	   << " not found in parameters.def" << endl;
      hpar.num   = 100000+npar;
      hpar.name  = pid.alias;
      hpar.alias = pid.alias;
      hpar.unit  = "";
      hpar.scale = int8(-1);
      hpar.size  = int32(2);
      hpar.order = int32(1);
      hpar.dataType = uint16(0);
      hpar.plotType = uint16(1);
      parList.push_back(hpar);
      npar++;
    }
  }
  // check if model exist in modList
  for (i=0; (i<modList.size())&&(modList[i].name!=pid.model);i++);
  if (i==modList.size()){ //model not found
    hmod.modelPn = 0;
    hmod.name = pid.model;
    hmod.run[0]  = 0;
    hmod.run[1]  = 0;
    hmod.run[2]  = 0;
    hmod.run[3]  = (pid.run != R_UNDEF ? pid.run : 0);
    hmod.run[4]  = 0;
    hmod.run[5]  = 0;
    modList.push_back(hmod);
    nmod++;
  }

  *ef = OK;
  return true;
}

bool GribStream::_openFile(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::_openFile" << endl;
#endif
  IsOpen = true;
  *ef = OK;
  return true;

}


bool GribStream::_readPosList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::_readPosList --starting-----" << endl;
  cout << "          ::_readPosList filename =    " << stFileName<< endl;
#endif
  ifstream file(stFileName.c_str());
  if (!file){
    *ef = DF_FILE_OPEN_ERROR;
    return false;
  }

  //------------------------------------------------------
  // Read list of positions from stationlist file
  //------------------------------------------------------

  GribPos tempPos;
  std::string stLine;
  vector<std::string>stationVector;
  posList.clear();
  size_t i = 0;
  while (getline(file, stLine, '\n')){
    stationVector = miutil::split(stLine, ",",true);
    if(stationVector.size() == 6) {
  	tempPos.ref = miutil::to_int(stationVector[0], -1);
  	tempPos.name = stationVector[1];
  	tempPos.geopos[0] = miutil::to_double(stationVector[2]);
  	tempPos.geopos[1] = miutil::to_double(stationVector[3]);
  	tempPos.topo = miutil::to_double(stationVector[4]);
  	tempPos.topo2 = miutil::to_double(stationVector[5]);
        posList.push_back(tempPos);
    }
 }
  npos = posList.size();

#ifdef DEBUG
      cout << "         ::_readPosList npos= " << npos << endl;
#endif

  for (i = 0; i< posList.size(); i++) {  
      cout << "---- StationList for Grib position no:" << i << endl;
/*#ifdef DEBUG
      cout << "Ref:" << posList[i].ref << endl;
      cout << "Name:" << posList[i].name << endl;
      cout << "geopos[0]:" << posList[i].geopos[0] << " geopos[1]:" << 
 	posList[i].geopos[1] << endl; 
      cout << "topo:" << posList[i].topo << endl;
      cout << "topo2:" << posList[i].topo2 << endl;
#endif*/
 }
  file.close(); 
  *ef = OK;
  return true;

}

// reads parameters from setupfile for grib files
bool GribStream::_readParList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::_readParList" << endl;
#endif
 /* std::string Name="/data/proj5/diana/NYVIS_DEV/Ariunaa.Bertelsen/DIANA/local/hirlam_11.param"; 
  ifstream file(Name.c_str());
  if (!file){
    *ef = DF_FILE_OPEN_ERROR;
    return false;
  }*/

  //------------------------------------------------------
  // Read list of positions from section
  //------------------------------------------------------

  GribPar tempPar;
  std::string stLine;
  vector<std::string> parameterVector, list;
  parList.clear();
  size_t n, p;
  sp->parse(Name);
  list.clear();
#ifdef DEBUG
      cout << "Före getSection: list.size() =     "<< list.size()<<endl;
#endif

  if (!sp->getSection(SectParameters,list)) {
    //cout << "********** error getSection **********" << endl;
    *ef = DF_DATA_READING_ERROR;
    return false;
  }
 // n=list.size(); 
#ifdef DEBUG
      cout << "After getSection: list.size() =     "<< list.size()<<endl;
#endif
  for (n=0; n< list.size(); n++) {
      stLine = list[n];
      cout << "***stLine =     "<< stLine << endl;

      if (((p = stLine.find ("#")) == 0) || (stLine.size()==0)) {
        continue;
      }
      else {
        parameterVector = miutil::split(stLine, ",",true);
        if(parameterVector.size() == 9) {
           tempPar.num = miutil::to_int(parameterVector[0], -1);
           tempPar.name = parameterVector[1];
           tempPar.alias = parameterVector[2];
           tempPar.unit = parameterVector[3];
           tempPar.scale = miutil::to_int(parameterVector[4], -1);
           tempPar.size = miutil::to_int(parameterVector[5], -1);
           tempPar.order = miutil::to_int(parameterVector[6], -1);
           tempPar.dataType = miutil::to_int(parameterVector[7], -1);
           tempPar.plotType = miutil::to_int(parameterVector[8], -1);
           parList.push_back(tempPar);
       }
        
     }   
  }


#ifdef DEBUG
      cout << "parList.size() =     "<<parList.size()<<endl;
 
  for (i = 0; i< parList.size(); i++) {
      cout << "---- Parameter lista for Gribfiler, Parameter no:" << i << endl;
      cout <<"---- Parameter ["<<i<<"] ->" <<endl;
      cout <<"Num :"<<parList[i].num<<endl;
      cout <<"Name:"<<parList[i].name<<endl;
      cout <<"Alias:"<<parList[i].alias<<endl;
      cout <<"Unit:"<<parList[i].unit<<endl;
      cout <<"Scale:"<<parList[i].scale<<endl;
      cout <<"Size:"<<parList[i].size<<endl;
      cout <<"Order:"<<parList[i].order<<endl;
      cout <<"DataType:"<<parList[i].dataType<<endl;
      cout <<"PlotType:"<<parList[i].plotType<<endl;
 }
#endif
  *ef = OK;
  return true;
}





//------------------------------------------------------
// _readModelList
//------------------------------------------------------
// Read list of models from setupfile for gribfiles
//------------------------------------------------------
bool GribStream::_readModList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "GribStream::_readModList --starting---" << endl;
#endif
  std::string Line, modName, key,value;
  vector<std::string> modelVector, modlist;
  //parList.clear();
  size_t n, p;
  GribMod tempMod;

  if (!sp->getSection(SectModel,modlist)) {
    *ef = DF_DATA_READING_ERROR;
    return false;
  }
#ifdef DEBUG
  cout << "_readModList getSection() passed : "<<endl;
#endif

  //Parsing TSERIES_MODEL section and retrieving values.
  for (n=0; n< modlist.size(); n++) {
      Line = modlist[n];
      if (((p = Line.find ("#")) == 0) || (Line.size()==0)) {
        continue;
      }
      else {
        modelVector = miutil::split(Line, 2, "=", true);
        if(modelVector.size()==2)  {
           key=miutil::to_lower(modelVector[0]); // always converting keyword to lowercase !
           value=modelVector[1];
           // structures of type: A=B 
           if (key == "id" ) tempMod.modelPn = miutil::to_int(value);
           else if (key=="name")  tempMod.name=value;   
           else if (key=="alias") modName=tempMod.alias=value;   
           else  
               continue;
            
        }
        
     }   
  }

//.................................
//    std::string modName ="E11";
//    vector<miTime> validTime;
//    vector<int> forecastHour;
    if (!fieldm->invTseries(modName, validTime, forecastHour)) {
       *ef = DF_DATA_READING_ERROR;
       return false;
    }

#ifdef DEBUG
  for(int i=0;i<forecastHour.size();i++) {
    cerr << "********forecastHour: " << forecastHour[i] << endl;
  }
  cerr << "***********forecastHour.size(): " << forecastHour.size() <<endl;
  for(int i=0;i<validTime.size();i++) {
    cerr << "*********validTime: " << validTime[i] << endl;
  }
  cerr << "validTime.size(): " << validTime.size() <<endl;
#endif




//................................
//   GribMod tempMod;
//   tempMod.modelPn = 55;
//   tempMod.name = "HIRLAM.11km";
   tempMod.modelid= tempMod.name;
   tempMod.run[0]= validTime[0].year(); 
   tempMod.run[1]= validTime[0].month(); 
   tempMod.run[2]= validTime[0].day(); 
   tempMod.run[3]= validTime[0].hour(); 
   tempMod.run[4]= validTime[0].min(); 
   tempMod.run[5]= validTime[0].sec(); 
   tempMod.runid= tempMod.run[3];

   modList.push_back(tempMod);

   for (size_t i =0; i< modList.size(); i++){
#ifdef DEBUG
      cout << "---- Grib model no:" << i << endl;
      cout << "Id:" << modList[i].modelPn << endl;
      cout << "Name:" << modList[i].name << endl;
      cout << "Model:" << modList[i].modelid << endl;
      cout << "runid:" << modList[i].runid <<endl;
      cout << "run[0]:" << modList[i].run[0] <<
	" run[1]:" << modList[i].run[1] << 
	" run[2]:" << modList[i].run[2] << 
	" run[3]:" << modList[i].run[3] << 
	" run[4]:" << modList[i].run[4] << 
	" run[5]:" << modList[i].run[5] << endl;
#endif
   }
  /*else {
    *ef = DF_DATA_READING_ERROR;
    return false;
  }*/
  *ef = OK;
  return true;
}


void GribStream::getTextLines(const ParId p,
			   vector<std::string>& tl)
{
  for (size_t i=0; i<modList.size(); i++)
    if (modList[i].modelid==p.model){
      tl= modList[i].textlines;
      break;
    }
}





void GribStream::_setData(int index, const std::string& shortName, 
		       const ParId& pid)
{
#ifdef DEBUG
  cout << "GribStream::_setData" << endl;
  cout << "GribStream::_setData index: " << index<< endl;
  cout << "GribStream::_setData shortName: " << shortName << endl;
  cout << "GribStream::_setData ParId: " << pid << endl;

#endif
  GribPar* onePar=0;

  // find the parameter in parList corresponding to shortName
  npar= parList.size();
  for (int i=0;i<npar;++i)
    if (shortName == parList[i].alias) {
      onePar = &parList[i];
      break;
    }
/*    if (onePar)

#ifdef DEBUG
    cout << "_setData: Found parameter with index:" << i << endl;
#endif
    if (!onePar) {
#ifdef DEBUG
    cout << "_setData: Parameter not found" << endl;
#endif
    return;
    }
*/
  

  int np= parameters.size();
  if (index <0 || index >=np) return;
  // subtracting plottype by one because HDF plottype starts at 1
  parameters[index].setType((ptPrimitiveType)(onePar->plotType-1));
  // TODO uncomment!
  //parameters[index].setPolar((onePar->dataType == polar));
  parameters[index].setId(pid);

  float scale = pow(10.0,onePar->scale);
  // loop through data: scale
  for (int i=0;i<parameters[index].size();++i)
    parameters[index].setData(i,parameters[index].Data(i)*scale);
}
