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


// ptHDFFile.cc

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <hdf.h>
#include "ptHDFFile.h"
#include "ptHDFUtils.h"
#include "HDFdefs.h"
#include <string.h>
#include <math.h>
#include <float.h>
#include <puCtools/puMath.h>

#include <algorithm>
#include <iostream>

#ifdef DEBUG
void out_of_store()
{
  cerr << "operator new failed: out of store\n";
  exit(1);
}
#endif

using namespace miutil;
using namespace std;

HDFFile::HDFFile(const std::string& fname)
  : DataStream(fname), hasposVG(false)
{
#ifdef DEBUG
  cout << "Inside HDFFile:constructor" << endl;
#endif
  timeLine.reserve(100);  // We don't expect more than 100 timepoints
  progLine.reserve(100);
  parameters.reserve(20);       // we don't expect more than 20 parameters
  //  textLines.reserve(0);
}

HDFFile::~HDFFile()
{
#ifdef DEBUG
  cout << "Inside HDFFiles destructor" << endl;
#endif
  close();
}

bool HDFFile::close()
{
  // close the HDF file
  if (IsOpen) {
    Vend(fid);
    Hclose(fid);

    IsOpen= false;
  }
  return true;
}

// return index of station 'statName' in posList, -1 if not found
int HDFFile::findStation(const std::string& statName)
{
#ifdef DEBUG
  cout << "HDFFile::findStation:" << statName << endl;
#endif
  int rn=-1, i;
  std::string sname= miutil::to_upper_latin1(statName);
  for (i=0; i<npos; ++i) {
    if (miutil::to_upper_latin1(posList[i].name) == sname) {
      rn = i;
      break;
    }
  }
  return rn;
}


bool HDFFile::getStations(vector<miPosition>& poslist){
#ifdef DEBUG
  cout << "HDFFile::getStations" << endl;
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


bool HDFFile::getStationSeq(int idx, miPosition& pos){
#ifdef DEBUG
  cout << "HDFFile::getStationSeq" << endl;
#endif
  if (idx >= 0 && idx < npos) {
    miCoordinates c(posList[idx].geopos[1],posList[idx].geopos[0]);
    int hoh=int(posList[idx].topo);
    pos.setPos(c,0,0,posList[idx].name,hoh,0,"");
    return true;
  } else {
    return false;
  }
}

int HDFFile::putStation(const miPosition& s,
			ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::putStation" << endl;
#endif
  HDFPos temp;
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


bool HDFFile::getModelSeq(int idx, Model& mod,
			  Run& run, int& id)
{
#ifdef DEBUG
  cout << "HDFFile::getModelSeq" << endl;
  cout << "nmod:"<<nmod<<" idx:"<<idx<<endl;
#endif
  if (idx >= 0 && idx < nmod) {
    mod  = modList[idx].modelid;
    run  = modList[idx].runid;
    id   = modList[idx].modelPn;
#ifdef DEBUG
    cout << "HDFFile::getModelSeq model,run,id:"<<mod<<" "<<run<<" "<<id<<endl;
#endif
    return true;
  } else {
    return false;
  }
}

bool HDFFile::getModelSeq(int idx, Model& mod,
			  Run& run, int& id,
			  vector<std::string>& vtl)
{
#ifdef DEBUG
  cout << "HDFFile::getModelSeq" << endl;
  cout << "nmod:"<<nmod<<" idx:"<<idx<<endl;
#endif
  if (idx >= 0 && idx < nmod) {
    mod  = modList[idx].modelid;
    run  = modList[idx].runid;
    id   = modList[idx].modelPn;
    vtl  = modList[idx].textlines;
#ifdef DEBUG
    cout << "HDFFile::getModelSeq model,run,id:"<<mod<<" "<<run<<" "<<id<<endl;
#endif
    return true;
  } else {
    return false;
  }
}

// return index of model 'modelName' and run modelRun
// in modList, -1 if not found
int HDFFile::findModel(const std::string& modelName,
		       const int& modelRun)
{
#ifdef DEBUG
  cout << "HDFFile::findModel" << endl;
#endif
  int rn=-1, i;
  for (i=0; i<nmod; ++i) {
    if ((modList[i].name==modelName) &&
	(modList[i].run[3]==modelRun || modelRun==R_UNDEF)) {
      rn = i;
      break;
    }
  }
  return rn;
}

bool HDFFile::getFullModeltime(int id, miTime& t)
{
  if ( id<0 || id>=modList.size() ) return false;

  t =  miTime(modList[id].run[0],modList[id].run[1],modList[id].run[2],
	      modList[id].run[3],modList[id].run[4],modList[id].run[5]);

  return true;
}


int HDFFile::findDataPar(const ParId& id)
{
#ifdef DEBUG
  cout << "HDFFile::findDataPar" << endl;
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

void HDFFile::clean()
{
#ifdef DEBUG
  cout << "HDFFile::clean" << endl;
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

bool HDFFile::openStream(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::openFileAndReadHeader" << endl;
#endif
  if (!_openFile(ef))
    return false;
  if (!_readPosList(ef))
    return false;
  if (!_readParList(ef))
    return false;
  if (!_readModList(ef))
    return false;
  InfoIsRead = true;
  *ef = OK;
  return true;
}

bool HDFFile::openStreamForWrite(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::openFileForWrite" << endl;
#endif
  if (!_createFile(ef))
    return false;
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
bool HDFFile::readData(const int posIndex,
		       const ParId& modid,
		       const miTime& start,
		       const miTime& stop,
		       ErrorFlag* ef)
{
  uint8* dataBuf;
  char   modelName[VSNAMELENMAX+1];
  int32  nrec;
  int32  nfields;
  vector<std::string> nameFields;
  vector<int>   orderFields;
  vector<int>   fieldSize;
  char  *fields;
  int32  vpos, vmod;
  int32  tag, ref, interlace, recsz, i, j, k, l;
  int    fnsize=0;

  int32 *tags, *refs;
  int npairs;
  std::string modname;
  int modrun, modidx= -1;
  bool foundmodel=false;
  const int HDFFAIL = -1;

#ifdef DEBUG
  cout << "HDFFile::readData" << endl;
#endif
#ifdef DEBUG
  set_new_handler(&out_of_store);
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
  cout << "Requested model name:" << modname << endl;
  cout << "Requested model run :" << modrun << endl;
#endif
  if (modname.length()) { // check if requested model exists on file
    modidx = findModel(modname, modrun);
    if (modidx==-1) return false;
    // set correct modelrun
    modrun=modList[modidx].run[3];
  }

  // Position vgroup ref's are stored in posList
  if ((vpos = Vattach(fid,posList[posIndex].ref,"r")) == HDFFAIL)
    return false;
  // Find correct model in position vgroup
  if (modname.length()) {
    npairs = Vntagrefs(vpos);
    if (npairs<=0) return false;
    tags = new int32[npairs];
    refs = new int32[npairs];

    if (Vgettagrefs(vpos,tags,refs,npairs) != HDFFAIL)
      for (i=0;i<npairs;i++)
	if (tags[i] == DFTAG_VH)
	  if ((vmod = VSattach(fid,refs[i],"r")) != HDFFAIL) {
	    VSgetname(vmod,modelName);
#ifdef DEBUG
	    cout << "Vdata name:" << modelName << endl;
#endif
	    if (strcmp(modelName,modname.c_str())==0) {
#ifdef DEBUG
	      cout << "It is a match!!" << endl;
#endif
	      foundmodel=true;
	      break;
	    }
	    VSdetach(vmod);
	  }

    delete[] tags;
    delete[] refs;
    if (!foundmodel) {
      if (vmod != HDFFAIL) VSdetach(vmod);
      Vdetach(vpos);
      return false;
    }

  } else {
    // model is undefined, get first model from file
    if (Vgettagref(vpos,0,&tag,&ref)==HDFFAIL) return false;
    if ((vmod = VSattach(fid,ref,"r"))==HDFFAIL) return false;
    if (VSgetname(vmod, modelName)==HDFFAIL) {
      VSdetach(vmod);
      Vdetach(vpos);
      return false;
    }
    // find model in modlist
    if ((modidx=findModel(modelName,R_UNDEF))==-1){
      VSdetach(vmod);
      Vdetach(vpos);
      return false;
    }
    // set correct modelrun
    modrun=modList[modidx].run[3];
  }

  // ok..we have a satisfactory model and run

  nfields = VFnfields(vmod);

  // the nameFields contains all parameter short names found in the vdata
  // the orderFields contains the corresponding order of these parameters
  // the fieldSize contains the data size in bytes
  nameFields.reserve(nfields);
  orderFields.reserve(nfields);
  parameters.reserve(nfields);
  for (j=0;j<nfields;j++) {
    nameFields.push_back(std::string(VFfieldname(vmod,j)));
    orderFields.push_back(VFfieldorder(vmod,j));
    fnsize += strlen(VFfieldname(vmod,j))+1;
    if (orderFields[j]>0)
      fieldSize.push_back(VSsizeof(vmod,VFfieldname(vmod,j))
			  /orderFields[j]);
    else
      fieldSize.push_back(0);
#ifdef DEBUG
    cout << "Fetching field characteristics:" << j << endl;
    cout << "Name:" << nameFields[j] << endl;
    cout << "Order:" << orderFields[j] << endl;
    cout << "Size:" << fieldSize[j] << endl;
#endif
  }

  *ef = DF_DATA_READING_ERROR;

  fields = new char[fnsize];
  if (VSinquire(vmod,&nrec,&interlace,
		fields,&recsz,modelName)==HDFFAIL){
    delete[] fields;
    VSdetach(vmod);
    Vdetach(vpos);
    return false;
  }
#ifdef DEBUG
  cout << "HDFFile:readData: fields:" << fields <<endl;
  cout << "..and nrec:"<<nrec<<" recsz:"<<recsz<<" interlace:"
       <<interlace<<endl;
#endif
  dataBuf = new uint8[nrec*recsz];
  if (VSsetfields(vmod,fields)==HDFFAIL || !dataBuf) {
    delete[] fields;
    VSdetach(vmod);
    Vdetach(vpos);
    return false;
  }
  // read vdata from file
  if (VSread(vmod,dataBuf,nrec,interlace)==HDFFAIL){
    cout << "HDFFile::readData. Error reading vdata from station:"<<
      posList[posIndex].name<<endl;
    delete[] fields;
    delete[] dataBuf;
    VSdetach(vmod);
    Vdetach(vpos);
    //exit(0);
    return false;
  }

  delete[] fields;

  VSdetach(vmod);
  Vdetach(vpos);

  // Organize the data in dataBuf. Separate the actual parameterdata
  // from the rest (Time, Prog, Level (and sensor))
  int inc,totinc=0;
  uint8 *tmp = dataBuf;
  float *fdata;
  int16 number16;
  int32 number32;
  vector<int> levelList;
  vector<float> uniqLevels;

  vector<int> ULCount; // Unique level count
  bool levelfound;
  WeatherParameter wp; // basis for all weatherparameters found
  ParId pid = ID_UNDEF;
  pid.model = modelName;
  pid.run   = modrun;

  for (i=0;i<nfields;++i){
    inc = fieldSize[i];
    fdata = new float[nrec*orderFields[i]]; //allocate for one field
    for (j=0;j<nrec;++j) {
      tmp = &dataBuf[j*recsz+totinc];
      for (k=0;k<orderFields[i];++k,tmp+=inc) {
 	if (inc == 2) {
 	  memcpy(&number16,tmp,inc);
 	  fdata[j+nrec*k] = (float)number16;
 	} else if (inc == 4) {
 	  memcpy(&number32,tmp,inc);
 	  fdata[j+nrec*k] = (float)number32;
 	} else fdata[j+nrec*k] = (float)undefValue;
      }
    }
    totinc += inc*orderFields[i]; //index to dataBuf start for next field

    // make the timeline
    if (nameFields[i] == FNTIME) {
      for (j=0;j<nrec;++j) {
	timeLine.push_back(miTime(int(fdata[j]),int(fdata[nrec+j]),
				  int(fdata[2*nrec+j]),int(fdata[3*nrec+j]),
				  int(fdata[4*nrec+j]),int(fdata[5*nrec+j])));
#ifdef DEBUG
	cout << "Adding to timeline:" << timeLine[j] << endl;
#endif
      }
      TimeLineIsRead = true;
    }

    // make the progline
    else if (nameFields[i] == FNPROG) {
      for (j=0;j<nrec;++j) {
	progLine.push_back(int(fdata[j]/3600));
#ifdef DEBUG
	cout << "Adding to progline:" << progLine[j] << endl;
#endif
      }
    }

    // make a list of levels
    else if (nameFields[i] == FNLEV) {
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

      // the weather parameters
    } else {
#ifdef DEBUG
      cout << "A new weatherparameter: " << nameFields[i] << endl;
#endif
      int ipar = parameters.size();
      int curlevel;
      vector<int> parTimes[MAXLEV]; // MAXLEV from HDFdefs.h
      vector<miTime> times; // timeline for each level
      vector<int> ptimes;   // progline for each level
      int tlindex; // new timeline index

      // check each timestep for data != undefValue
      for (j=0;j<nrec;++j) {
	curlevel = levelList[j]; // index to uniqLevels
	if (fdata[j]!=undefValue)
	  parTimes[curlevel].push_back(j);
      }
      // add a new weatherparameter for each unique level
      for (k=0;k<uniqLevels.size();k++){
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
	  pid.level = int(uniqLevels[k]);
	  _setData(ipar,nameFields[i],pid);
#ifdef DEBUG
	  cout << "the complete wp:"<<  parameters[ipar]<<endl;
#endif
	  ipar++;
	}
      }
    }
    delete[] fdata;
  } //nfields
  delete[] dataBuf;

  *ef = OK;
  DataIsRead = true;
  return true;
}


bool HDFFile::writeData(const int posIndex,
			const int modIndex,
			ErrorFlag* ef,
			bool complete_write,
			bool write_submodel)
{
  uint8 *buf, *tmp;
  int32  vpos;
  int32  modVD;
  std::string modname;
  char *thisPosFN;
  int i,j,k,is,kk,inc;
  int16 i16dum;
  int32 i32dum;

#ifdef DEBUG
  cout << "HDFFile::_writeData" << endl;
#endif

  if (posIndex < 0 || posIndex >= npos) {
    *ef = DF_RANGE_ERROR;
    return false;
  }

  *ef = DF_MODEL_NOT_FOUND;

  if (modIndex < 0 || modIndex >= nmod)
    return false;

  modname = modList[modIndex].name;

  if (!hasposVG){
    // make the main position vgroup
    posVG=Vattach(fid,-1,"w");
    Vsetname(posVG,VGPOSGROUP);
    hasposVG= true;
  }

  // make current positions vgroup
  vpos=Vattach(fid,-1,"w");
  Vsetname(vpos,posList[posIndex].name.c_str());
  Vinsert(posVG,vpos);

  // make vdata for current model
  modVD=VSattach(fid,-1,"w");
  VSsetname(modVD,modname.c_str());
  Vinsert(vpos,modVD);

  // Vdata field definitions
  VSfdefine(modVD,FNTIME,DFNT_INT16,6);
  VSfdefine(modVD,FNPROG,DFNT_INT32,1);
  VSfdefine(modVD,FNLEV, DFNT_INT16,1);
  if (write_submodel)
    VSfdefine(modVD,FNSMOD, DFNT_INT16,1);

  int fieldnamesize = parameters.size()*(ALIASZ+1)+
		      strlen(FNTIME)+1+strlen(FNPROG)+1+
		      strlen(FNLEV)+1;
  if (write_submodel)
    fieldnamesize+= strlen(FNSMOD)+1;

  thisPosFN = new char[fieldnamesize];
  strcpy(thisPosFN,FNTIME);
  strcat(thisPosFN,",");
  strcat(thisPosFN,FNPROG);
  strcat(thisPosFN,",");
  strcat(thisPosFN,FNLEV);
  if (write_submodel){
    strcat(thisPosFN,",");
    strcat(thisPosFN,FNSMOD);
  }

  vector<int> colidx;     // index to parList for each column
  vector<int> uniqLevels; // unique levels found in parameters
  vector<int> column;     // data-column number for parameter
  vector<Alias> aliaslist;// unique alias's in parameters
  vector<int> uniqSModels;// unique SubModels
  Alias alias;

  std::string atemp;
  int ltemp;
  int stemp;
  int numcomp=0; // total number of parameter components

  if (!write_submodel) uniqSModels.push_back(undefValue);

  for (i=0; i<parameters.size(); i++) {
    // make a list over unique levels
    ltemp = parameters[i].Id().level;
    for (k=0; (k<uniqLevels.size())&&(ltemp!=uniqLevels[k]); k++)
      ;
    if (k==uniqLevels.size()) uniqLevels.push_back(ltemp);

    if (write_submodel){
      // make a list over unique SubModels
      std::string sm= parameters[i].Id().submodel;
      if (sm != S_UNDEF)
	stemp = atoi(sm.c_str());
      else
	stemp = undefValue;
      for (is=0; (is<uniqSModels.size())&&(stemp!=uniqSModels[is]); is++)
	;
      if (is==uniqSModels.size()) uniqSModels.push_back(stemp);
    }

    // find parameter alias in parList
    alias = parameters[i].Id().alias;
    atemp = alias;
    for (j=0; (j<parList.size())&&(parList[j].alias!=atemp);j++)
      ;
    if (j==parList.size()){
      cerr << "ptHDFFile ERROR..parameter not found in parList:" << alias << endl;
      j=0;
    }
    // make a list of unique alias's
    for (kk=0; (kk<aliaslist.size()) && (aliaslist[kk] != alias); kk++)
      ;
    if (kk == aliaslist.size()){// new alias
      aliaslist.push_back(alias); // store it
      colidx.push_back(j);        // store corresponding index to parList
      numcomp+=parList[j].order;  // update number of datacomponents
      VSfdefine(modVD,parList[j].alias.c_str(),DFNT_INT16,parList[j].order);
      strcat(thisPosFN,",");
      strcat(thisPosFN,parList[j].alias.c_str());
    }
    column.push_back(kk);
  }

  VSsetfields(modVD,thisPosFN);
  delete[] thisPosFN;

  // find timeline
  TimeLine smallTL;
  vector<miTime> tl;
  int tlindex, nmt=0;
  vector<int> partimes;
  for (i=0; i<parameters.size(); i++) {
    partimes.push_back(0); // used in datawriting below
    timeLines.Timeline(parameters[i].TimeLineIndex(),tl);
    if ((tlindex=smallTL.Exist(tl))==-1){
      tlindex=nmt;
      smallTL.insert(tl,tlindex);
      nmt++;
    }
    parameters[i].setTimeLineIndex(tlindex);
  }
#ifdef DEBUG
  cout << "The small TimeLine:"<<smallTL<<endl;
#endif
  // Write data
  inc=0;
  int ntime = smallTL.size();
  int vdatasize = ntime*    // max number of timestep
    uniqLevels.size()*      // number of levels
    uniqSModels.size()*     // number of submodels
    (6*sizeof(int16)+       // time
     sizeof(int32)+         // prog time
     sizeof(int16)+         // level
     (write_submodel ? sizeof(int16) : 0)+ // SubModel
     numcomp*sizeof(int16));//data

  tmp=buf=new uint8[vdatasize];

  float fdata;
  int16 idata;
  int il, parc;
  Level curlev;
  SubModel cursubmodel;
  bool aliasfound;

  for (is=0; is<uniqSModels.size(); is++){
    cursubmodel= (uniqSModels[is] != undefValue
		  ? miutil::from_number(uniqSModels[is]) : S_UNDEF);

    for (il=0; il<uniqLevels.size(); il++){
      curlev = uniqLevels[il];
      for (i=0; i<ntime; i++) {
	i16dum=smallTL[i].year();
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	i16dum=smallTL[i].month();
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	i16dum=smallTL[i].day();
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	i16dum=smallTL[i].hour();
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	i16dum=smallTL[i].min();
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	i16dum=smallTL[i].sec();
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	i32dum = 0;
	memcpy(tmp,&(i32dum),inc=sizeof(int32));
	tmp+=inc;
	i16dum=uniqLevels[il]; // level..
	memcpy(tmp,&(i16dum),inc=sizeof(int16));
	tmp+=inc;
	if (write_submodel){
	  i16dum=uniqSModels[is]; // submodel..
	  memcpy(tmp,&(i16dum),inc=sizeof(int16));
	  tmp+=inc;
	}
	for (kk=0; kk<aliaslist.size(); kk++){
	  aliasfound = false;
	  parc = colidx[kk];
	  for (j=0; (j<parameters.size())&&(!aliasfound); j++) {
	    if (column[j] == kk) {
	      if ((parameters[j].Id().level == curlev) &&
		  (parameters[j].Id().submodel == cursubmodel ||
		   cursubmodel == S_UNDEF) &&
		  smallTL.flag(i,parameters[j].TimeLineIndex())){
		aliasfound = true;
		for (k=0; k<parList[parc].order; k++){
		  fdata = parameters[j].Data(partimes[j],k);
		  if (fdata == UNDEF) // check for undefvalue
		    idata= undefValue;
		  else
		    idata = static_cast<int16>
		      (puRint(fdata*(pow(10.0,-parList[parc].scale))));
		  memcpy(tmp,&idata,inc=sizeof(int16));
		  tmp+=inc;
		}
		partimes[j]++;
	      }
	    }
	  }
	  if (!aliasfound){
	    // if no alias found with correct level and timestep:
	    idata=undefValue;
	    for (k=0; k<parList[parc].order; k++){
	      memcpy(tmp,&idata,inc=sizeof(int16));
	      tmp+=inc;
	    }
	  }
	}
      }
    }
  }

  VSwrite(modVD,buf,ntime*uniqLevels.size()*uniqSModels.size(),FULL_INTERLACE);
  delete [] buf;

  // detach and close
  VSdetach(modVD);
  Vdetach(vpos);

  if (complete_write){
    Vdetach(posVG);
    if (!_writeParList(ef)) return false;
    if (!_prepPosRefs(ef))  return false;
    if (!_writePosList(ef)) return false;
    if (!_writeModList(ef)) return false;
  }

  *ef = OK;
  return true;
}



bool HDFFile::getTimeLine(const int& index,
			  vector<miTime>& tline,
			  vector<int>& pline,
			  ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::getTimeLine" << endl;
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

bool HDFFile::putTimeLine(const int& index,
			  vector<miTime>& tline,
			  vector<int>& pline,
			  ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::putTimeLine" << endl;
#endif
  timeLines.insert(tline,progLines.size());
  progLines.push_back(pline);
  *ef = OK;
  return true;
}

bool HDFFile::putTimeLine(TimeLine& tl,
			  vector<int>& pline,
			  ErrorFlag* ef)
{

#ifdef DEBUG
  cout << "HDFFile::putTimeLine" << endl;
#endif
  timeLines = tl;
  progLines.push_back(pline);
  *ef = OK;
  return true;
}

bool HDFFile::getOnePar(int index, WeatherParameter& wp, ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::getOnePar" << endl;
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

bool HDFFile::putOnePar(WeatherParameter& wp, ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::putOnePar" << endl;
#endif
  int i;
  Parameter par;
  HDFPar hpar;
  HDFMod hmod;
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
      cerr << "ptHDFFile::putOnePar  Warning, parameter:" << pid.alias
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

bool HDFFile::_openFile(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_openFile" << endl;
#endif
//   cerr << "***** Foer Hopen.."<<endl;
  if((fid = Hopen(Name.c_str(),DFACC_RDONLY,0))==-1) {
    *ef = DF_FILE_OPEN_ERROR;
    return false;
  }
//   cerr << "+++++ Foer Vstart.."<<endl;
  if (Vstart(fid)==-1) {
    *ef = DF_FILE_ACCESS_ERROR;
    return false;
  }
//   cerr << "----- Etter Vstart"<<endl;
  IsOpen = true;
  *ef = OK;
  return true;

}

bool HDFFile::_createFile(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_createFile" << endl;
#endif
  if((fid = Hopen(Name.c_str(),DFACC_CREATE,0))==-1) {
    *ef = DF_FILE_OPEN_ERROR;
    return false;
  }
  if (Vstart(fid)==-1) {
    *ef = DF_FILE_ACCESS_ERROR;
    return false;
  }
  IsOpen = true;
  *ef = OK;
  return true;
}

bool HDFFile::_readParList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_readParList" << endl;
#endif
  int32 vgparid, npairs, vdid, nrec, interlace, recsz, nfields, j;
  int32 *tags, *refs;
  char vdname[VSNAMELENMAX+1];
  char *fields;
  int  i, inc=0, fnsize=0;
  uint8 *buf=NULL, *tmp;
  int pnumSz,nameSz,aliasSz,unitSz; // field sizes
  int scaleSz,sizeSz,orderSz,datatSz,plottSz;
  //const int HDFFAIL = -1;

  // Open vgroup VGPARNAME and get its tags and reference numbers
  if((vgparid = findVG(fid,VGPARNAME))==-1) {
    *ef = DF_PARAMETERLIST_NOT_FOUND;
    return false;
  }
  vgparid = Vattach(fid,vgparid,"r");
  npairs = Vntagrefs(vgparid);
  tags = new int32[npairs];
  refs = new int32[npairs];
  Vgettagrefs(vgparid,tags,refs,npairs);

  for (i=0;i<npairs;i++){
    if (tags[i] == DFTAG_VH) {
      vdid = VSattach(fid,refs[i],"r");
      VSgetname(vdid,vdname);
      if (strcmp(vdname,VGPARNAME)==0) {
        nfields = VFnfields(vdid);
        for (j=0; j<nfields; j++)
          fnsize += strlen(VFfieldname(vdid,j))+1;
        fields = new char[fnsize];

	// find size of each field in bytes
	pnumSz  = VSsizeof(vdid,const_cast<char*>(FNNUM));
	nameSz  = VSsizeof(vdid,const_cast<char*>(FNNAME));
	aliasSz = VSsizeof(vdid,const_cast<char*>(FNALIAS));
	unitSz  = VSsizeof(vdid,const_cast<char*>(FNUNIT));
	scaleSz = VSsizeof(vdid,const_cast<char*>(FNSCALE));
	sizeSz  = VSsizeof(vdid,const_cast<char*>(FNSIZE));
	orderSz = VSsizeof(vdid,const_cast<char*>(FNORDER));
	datatSz = VSsizeof(vdid,const_cast<char*>(FNTYPE));
	plottSz = VSsizeof(vdid,const_cast<char*>(FNPLOT));
	if (pnumSz<0  || nameSz<0  || aliasSz<0 ||
	    unitSz<0  || scaleSz<0 || sizeSz<0  ||
	    orderSz<0 || datatSz<0 || plottSz<0) {
	  cout << "HDFFile::readParList missing parameter fields!"<<endl;
	  *ef = DF_DATA_READING_ERROR;
	  return false;
	}

        VSinquire(vdid,&nrec,&interlace,fields,&recsz,vdname);
        buf = new uint8[nrec*recsz];
        VSsetfields(vdid,fields);
        delete[] fields;
        VSread(vdid,buf,nrec,interlace);
      }
      VSdetach(vdid);
    }
  }
  Vdetach(vgparid);
  delete[] tags;
  delete[] refs;

  HDFPar tempPar;

  tmp = buf;
  if(tmp) {
    char *tempname = new char[nameSz+1];
    char *tempalias= new char[aliasSz+1];
    char *tempunit= new char[unitSz+1];
    npar=nrec;
    if (npar>MAXPAR)
      npar=MAXPAR;
    for (i=0; i<npar; i++) {
      memcpy(&(tempPar.num),     tmp,inc=pnumSz);  tmp+=inc;
      memcpy(tempname,           tmp,inc=nameSz);  tmp+=inc;
      memcpy(tempalias,          tmp,inc=aliasSz); tmp+=inc;
      memcpy(tempunit,           tmp,inc=unitSz);  tmp+=inc;
      memcpy(&(tempPar.scale),   tmp,inc=scaleSz); tmp+=inc;
      memcpy(&(tempPar.size),    tmp,inc=sizeSz);  tmp+=inc;
      memcpy(&(tempPar.order),   tmp,inc=orderSz); tmp+=inc;
      memcpy(&(tempPar.dataType),tmp,inc=datatSz); tmp+=inc;
      memcpy(&(tempPar.plotType),tmp,inc=plottSz); tmp+=inc;
      // '\0'-terminate strings and remove trailing blanks...
      tempname[nameSz] = '\0';
      rtrim(tempname);
      tempalias[aliasSz] = '\0';
      rtrim(tempalias);
      tempunit[unitSz] = '\0';
      rtrim(tempunit);
      tempPar.name=  tempname;
      tempPar.alias= tempalias;
      tempPar.unit=  tempunit;

      parList.push_back(tempPar);
#ifdef DEBUG
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
#endif
    }
    delete[] tempname;
    delete[] tempalias;
    delete[] tempunit;
  } else {
    *ef = DF_DATA_READING_ERROR;
    return false;
  }
  delete[] buf;
  *ef = OK;
  return true;
}


bool HDFFile::_writeParList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_writeParList" << endl;
#endif
  int   i,j,inc=0;
  char  *fnall;
  uint8 *buf,*tmp;
  int32 vgid,vdid;

  vgid=Vattach(fid,-1,"w");
  Vsetname(vgid,VGPARNAME);

  vdid=VSattach(fid,-1,"w");
  VSsetname(vdid,VGPARNAME);
  Vinsert(vgid,vdid);

  VSfdefine(vdid,FNNUM,  DFNT_UINT16,1);
  VSfdefine(vdid,FNNAME, DFNT_CHAR8 ,PARNSZ+1);
  VSfdefine(vdid,FNALIAS,DFNT_CHAR8 ,ALIASZ+1);
  VSfdefine(vdid,FNUNIT, DFNT_CHAR8 ,UNITSZ+1);
  VSfdefine(vdid,FNSCALE,DFNT_INT8  ,1);
  VSfdefine(vdid,FNSIZE, DFNT_INT32 ,1);
  VSfdefine(vdid,FNORDER,DFNT_INT32 ,1);
  VSfdefine(vdid,FNTYPE, DFNT_UINT16,1);
  VSfdefine(vdid,FNPLOT, DFNT_UINT16,1);

  fnall = new char[strlen(FNNUM)+2+strlen(FNNAME)+2+strlen(FNALIAS)+2
		  +strlen(FNUNIT)+2+strlen(FNSCALE)+2+strlen(FNSIZE)+2
		  +strlen(FNORDER)+2+strlen(FNTYPE)+2+strlen(FNPLOT)+2];

  strcat(strcpy(fnall,FNNUM),",");
  strcat(strcat(fnall,FNNAME),",");
  strcat(strcat(fnall,FNALIAS),",");
  strcat(strcat(fnall,FNUNIT),",");
  strcat(strcat(fnall,FNSCALE),",");
  strcat(strcat(fnall,FNSIZE),",");
  strcat(strcat(fnall,FNORDER),",");
  strcat(strcat(fnall,FNTYPE),",");
  strcat(fnall,FNPLOT);

  VSsetfields(vdid,fnall);

  delete[] (fnall);

  int recsz = sizeof(uint16)+
    (PARNSZ+1+ALIASZ+1+UNITSZ+1)*sizeof(char)+
    sizeof(int8)+2*sizeof(int32)+2*sizeof(uint16);

  tmp=buf=new uint8[npar*recsz];
  char ctmp[100];

  for (i=0; i<npar; i++) {
    memcpy(tmp,&(parList[i].num),     inc=sizeof(uint16)); tmp+=inc;
    strcpy(ctmp,parList[i].name.c_str());
    for (j=strlen(ctmp);j<=PARNSZ;j++) ctmp[j]='\0';
    memcpy(tmp,  ctmp,     inc=(PARNSZ+1)*sizeof(char)); tmp+=inc;
    strcpy(ctmp,parList[i].alias.c_str());
    for (j=strlen(ctmp);j<=ALIASZ;j++) ctmp[j]='\0';
    memcpy(tmp,  ctmp,    inc=(ALIASZ+1)*sizeof(char)); tmp+=inc;
    strcpy(ctmp,parList[i].unit.c_str());
    for (j=strlen(ctmp);j<=UNITSZ;j++) ctmp[j]='\0';
    memcpy(tmp,  ctmp,     inc=(UNITSZ+1)*sizeof(char)); tmp+=inc;
    memcpy(tmp,&(parList[i].scale),   inc=sizeof(int8));   tmp+=inc;
    memcpy(tmp,&(parList[i].size),    inc=sizeof(int32));  tmp+=inc;
    memcpy(tmp,&(parList[i].order),   inc=sizeof(int32));  tmp+=inc;
    memcpy(tmp,&(parList[i].dataType),inc=sizeof(uint16)); tmp+=inc;
    memcpy(tmp,&(parList[i].plotType),inc=sizeof(uint16)); tmp+=inc;
  }

  VSwrite(vdid,buf,npar,FULL_INTERLACE);

  delete[] buf;
  VSdetach(vdid);
  Vdetach(vgid);

  *ef = OK;
  return true;
}



//------------------------------------------------------
// _readPosList
//------------------------------------------------------
// Read list of positions from HDF file
//------------------------------------------------------
bool HDFFile::_readPosList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_readPosList" << endl;
#endif
  char vdname[VSNAMELENMAX+1];
  char *fields;
  int32 interlace, recsz, nrec;
  int32 vglopin, vdin, npairs, nfields, j;
  int32 *tags, *refs;
  uint8 *buf=NULL, *tmp;
  int   i, fnsize=0, inc=0;
  int   refSz,nameSz,posSz,topoSz; //size of fields in bytes

  // Open vgroup VGPOSNAME and get its tags and reference numbers
  if ((vglopin = findVG(fid,VGPOSNAME)) == -1) {
    *ef = DF_POSITIONLIST_NOT_FOUND;
    return false;
  }
  vglopin = Vattach(fid,vglopin,"r");
  npairs = Vntagrefs(vglopin);
  tags = new int32[npairs];
  refs = new int32[npairs];
  Vgettagrefs(vglopin,tags,refs,npairs);

  for (i=0;i<npairs;i++)
    if (tags[i] == DFTAG_VH) {
      vdin = VSattach(fid,refs[i],"r");
      VSgetname(vdin, vdname);
      if (strcmp(vdname,VGPOSNAME) == 0) {
        nfields = VFnfields(vdin); // number of fields in vdata

	// size of commaseparated list of fieldnames
        for (j=0;j<nfields;j++)
          fnsize += strlen(VFfieldname(vdin,j))+1;
        fields = new char[fnsize]; // allocate space for field-list

	// find size of each field in bytes
	refSz  = VSsizeof(vdin,const_cast<char*>(FNPOSREF));
	nameSz = VSsizeof(vdin,const_cast<char*>(FNPOSNAME));
	posSz  = VSsizeof(vdin,const_cast<char*>(FNGEOPOS));
	topoSz = VSsizeof(vdin,const_cast<char*>(FNTOPOGRAPHY));

	// retrieve number of records, size of each record etc.
        VSinquire(vdin,&nrec,&interlace,fields,&recsz,vdname);
        buf = new uint8[nrec*recsz]; // allocate buffer for complete vdata
        VSsetfields(vdin,fields);
        VSread(vdin,buf,nrec,interlace); // read vdata to buffer
        delete[] fields;
      }
      VSdetach(vdin);
    }
  delete[] refs;
  delete[] tags;
  Vdetach(vglopin);

  HDFPos tempPos;
  tmp = buf;
  if(tmp) {
    char *tempname = new char[nameSz+1];
    npos = nrec;
    if (npos>MAXPOS)
      npos = MAXPOS;
    for (i=0;i<npos;i++) {
      memcpy(&(tempPos.ref),  tmp,inc= refSz); tmp+=inc;
      memcpy(  tempname,      tmp,inc=nameSz); tmp+=inc;
      memcpy(  tempPos.geopos,tmp,inc= posSz); tmp+=inc;
      memcpy(&(tempPos.topo), tmp,inc=topoSz); tmp+=inc;
      tempname[nameSz] = '\0';
      rtrim(tempname);
      tempPos.name = tempname;
      posList.push_back(tempPos);
#ifdef DEBUG
      cout << "---- HDF position no:" << i << endl;
      cout << "Ref:" << posList[i].ref << endl;
      cout << "Name:" << posList[i].name << endl;
      cout << "geopos[0]:" << posList[i].geopos[0] << " geopos[1]:" <<
 	posList[i].geopos[1] << endl;
      cout << "topo:" << posList[i].topo << endl;
#endif
    }
    delete[] buf;
    delete[] tempname;
  }
  else {
    *ef = DF_DATA_READING_ERROR;
    return false;
  }
  *ef = OK;
  return true;
}


bool HDFFile::_prepPosRefs(ErrorFlag* ef)
{
  int i;
  int32 posVG, npairs;
  int32 *tags,*refs;

  if ((posVG=findVG(fid,VGPOSGROUP))!=-1) {
    posVG=Vattach(fid,posVG,"r");
    npairs=Vntagrefs(posVG);
    tags=new int32[npairs*sizeof(int32)];
    refs=new int32[npairs*sizeof(int32)];
    Vgettagrefs(posVG,tags,refs,npairs);
    for (i=0; i<npairs; i++){
      posList[i].ref=refs[i];
    }

    delete[] tags;
    delete[] refs;
    Vdetach(posVG);

    *ef = OK;
    return true;
  }
  *ef = DF_POSITIONLIST_NOT_FOUND;
  return false;
}


//------------------------------------------------------
// _writePosList
//------------------------------------------------------
// Write list of positions to HDF file
//------------------------------------------------------
bool HDFFile::_writePosList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_writePosList" << endl;
#endif
  int   i,j, inc=0;
  int   sz, posnsz;
  char  *fnall;
  uint8 *buf, *tmp;
  int32 vgid, vdid;

  vgid=Vattach(fid,-1,"w");
  Vsetname(vgid,VGPOSNAME);

  vdid=VSattach(fid,-1,"w");
  VSsetname(vdid,VGPOSNAME);
  Vinsert(vgid,vdid);

  posnsz=0;
  for (i=0; i<npos; i++)
    if ((sz=posList[i].name.length())>posnsz)
      posnsz=sz;
  posnsz++;

  VSfdefine(vdid,FNPOSREF,  DFNT_INT32,1);
  VSfdefine(vdid,FNPOSNAME, DFNT_CHAR8,posnsz);
  VSfdefine(vdid,FNGEOPOS,  DFNT_FLOAT32,2);
  VSfdefine(vdid,FNTOPOGRAPHY,DFNT_FLOAT32,1);

  fnall = new char[strlen(FNPOSREF)+2+strlen(FNPOSNAME)+2
		  +strlen(FNGEOPOS)+2+strlen(FNTOPOGRAPHY)+2];

  strcat(strcpy(fnall,FNPOSREF),",");
  strcat(strcat(fnall,FNPOSNAME),",");
  strcat(strcat(fnall,FNGEOPOS),",");
  strcat(fnall,FNTOPOGRAPHY);

  VSsetfields(vdid,fnall);
  delete[] fnall;

  int recsz=sizeof(int32)+posnsz*sizeof(char)+3*sizeof(float32);

  tmp=buf=new uint8[npos*recsz];
  char *ctmp = new char[posnsz];

  for (i=0; i<npos; i++) {
    memcpy(tmp,&(posList[i].ref),      inc=sizeof(int32)); tmp+=inc;
    strcpy(ctmp,posList[i].name.c_str());
    for (j=strlen(ctmp);j<posnsz;j++) ctmp[j]='\0';
    memcpy(tmp, ctmp, inc=posnsz*sizeof(char)); tmp+=inc;
    memcpy(tmp, posList[i].geopos,inc=2*sizeof(float32)); tmp+=inc;
    memcpy(tmp,&(posList[i].topo),     inc=sizeof(float32)); tmp+=inc;
  }

  VSwrite(vdid,buf,npos,FULL_INTERLACE);

  delete[] ctmp;
  delete[] buf;
  VSdetach(vdid);
  Vdetach(vgid);

  *ef = OK;
  return true;
}


//------------------------------------------------------
// _readModelList
//------------------------------------------------------
// Read list of models from HDF file
//------------------------------------------------------
bool HDFFile::_readModList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_readModList" << endl;
#endif
  char vdname[VSNAMELENMAX+1];
  char *fields;
  int32 interlace, recsz, nrec;
  int32 vglopin, vdin, npairs, nfields, j;
  int32 *tags, *refs;
  uint8 *buf=NULL, *tmp;
  int   i, fnsize=0, inc=0;
  int   idSz,nameSz,runSz; //size of fields in bytes
  int   txtSz; //size of fields in bytes

  // Open vgroup VGMODNAME and get its tags and reference numbers
  if ((vglopin = findVG(fid,VGMODNAME)) == -1) {
    *ef = DF_MODELLIST_NOT_FOUND;
    return false;
  }
  vglopin = Vattach(fid,vglopin,"r");
  npairs = Vntagrefs(vglopin);
  tags = new int32[npairs];
  refs = new int32[npairs];
  Vgettagrefs(vglopin,tags,refs,npairs);

  for (i=0;i<npairs;i++)
    if (tags[i] == DFTAG_VH) {
      vdin = VSattach(fid,refs[i],"r");
      VSgetname(vdin, vdname);
      if (strcmp(vdname,VGMODNAME) == 0) {
        nfields = VFnfields(vdin); // number of fields in vdata

	// size of commaseparated list of fieldnames
        for (j=0;j<nfields;j++)
          fnsize += strlen(VFfieldname(vdin,j))+1;
        fields = new char[fnsize]; // allocate space for field-list

	// find size of each field in bytes
	idSz   = VSsizeof(vdin,const_cast<char*>(FNMODID));
	nameSz = VSsizeof(vdin,const_cast<char*>(FNMODNAME));
	runSz  = VSsizeof(vdin,const_cast<char*>(FNMODRUN));
	txtSz  = VSsizeof(vdin,const_cast<char*>(FNMODTXT));

	// retrieve number of records, size of each record etc.
        VSinquire(vdin,&nrec,&interlace,fields,&recsz,vdname);
        buf = new uint8[nrec*recsz]; // allocate buffer for complete vdata
        VSsetfields(vdin,fields);
        VSread(vdin,buf,nrec,interlace); // read vdata to buffer
        delete[] fields;
      }
      VSdetach(vdin);
    }
  delete[] refs;
  delete[] tags;
  Vdetach(vglopin);

  HDFMod tempMod;
  tmp = buf;
  if(tmp) {
    char *tempname = new char[nameSz+1];
    char *temptext = new char[txtSz+1];
    nmod = nrec;
    if (nmod>MAXMOD)
      nmod = MAXMOD;
    for (i=0;i<nmod;i++) {
      memcpy(&(tempMod.modelPn),tmp,inc=  idSz); tmp+=inc;
      memcpy(  tempname,        tmp,inc=nameSz); tmp+=inc;
      memcpy(  tempMod.run,     tmp,inc= runSz); tmp+=inc;
      if (txtSz > 0){
	memcpy(  temptext,         tmp,inc= txtSz); tmp+=inc;
	temptext[txtSz] = '\0';
	std::string tmp;
	for (int j=0; j<txtSz; j++){
	  if (temptext[j]!='\0')
	    tmp+= temptext[j];
	  else if (tmp.length() > 0){
	    miutil::replace(tmp, "@","Ø");
	    miutil::replace(tmp, "$","Å");
	    miutil::replace(tmp, "#","Æ");
	    tempMod.textlines.push_back(tmp);
	    tmp= "";
	  }
	}
      }
      tempname[nameSz] = '\0';
      rtrim(tempname);
      tempMod.name = tempname;
      tempMod.modelid= tempMod.name;
      tempMod.runid= tempMod.run[3];

      modList.push_back(tempMod);
#ifdef DEBUG
      cout << "---- HDF model no:" << i << endl;
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
    delete[] buf;
    delete[] tempname;
    delete[] temptext;
  }
  else {
    *ef = DF_DATA_READING_ERROR;
    return false;
  }
  *ef = OK;
  return true;
}


void HDFFile::getTextLines(const ParId p,
			   vector<std::string>& tl)
{
  for (int i=0; i<modList.size(); i++)
    if (modList[i].modelid==p.model){
      tl= modList[i].textlines;
      break;
    }
}


//------------------------------------------------------
// _writeModelList
//------------------------------------------------------
// Write list of models to HDF file
//------------------------------------------------------
bool HDFFile::_writeModList(ErrorFlag* ef)
{
#ifdef DEBUG
  cout << "HDFFile::_writeModList" << endl;
#endif
  int   i, j, inc=0;
  int   sz, modnsz;
  int   modtxtsz, modtxtn;
  char  *fnall;
  char  *txtline;
  uint8 *buf, *tmp;
  int32 vgid, vdid;

  vgid=Vattach(fid,-1,"w");
  Vsetname(vgid,VGMODNAME);

  vdid=VSattach(fid,-1,"w");
  VSsetname(vdid,VGMODNAME);
  Vinsert(vgid,vdid);

  modnsz=0;
  for (i=0; i<nmod; i++)
    if ((sz=modList[i].name.length())>modnsz)
      modnsz=sz;
  modnsz++;

  modtxtsz=0;
  modtxtn=0;
  for (i=0; i<nmod; i++){
    if (modList[i].textlines.size() > modtxtn)
      modtxtn=modList[i].textlines.size();
    for (j=0; j<modList[i].textlines.size(); j++)
      if ((sz=modList[i].textlines[j].length())>modtxtsz)
	modtxtsz=sz;
  }
  modtxtsz++;
  txtline= new char[modtxtsz+1];

  VSfdefine(vdid,FNMODID,   DFNT_UINT16,1);
  VSfdefine(vdid,FNMODNAME, DFNT_CHAR8,modnsz);
  VSfdefine(vdid,FNMODRUN,  DFNT_UINT32,6);
  if (modtxtsz*modtxtn > 0)
    VSfdefine(vdid,FNMODTXT,  DFNT_CHAR8,modtxtsz*modtxtn);

  fnall = new char[strlen(FNMODID)+2+strlen(FNMODNAME)+2+strlen(FNMODRUN)+2
		   +strlen(FNMODTXT)+2];

  strcat(strcpy(fnall,FNMODID),",");
  strcat(strcat(fnall,FNMODNAME),",");
  if (modtxtsz*modtxtn > 0){
    strcat(strcat(fnall,FNMODRUN),",");
    strcat(fnall,FNMODTXT);
  } else {
    strcat(fnall,FNMODRUN);
  }

  VSsetfields(vdid,fnall);
  delete[] fnall;

  int recsz=sizeof(uint16)+modnsz*sizeof(char)+6*sizeof(uint32)+
    modtxtsz*modtxtn*sizeof(char);

  tmp=buf=new uint8[nmod*recsz];
  char *ctmp = new char[modnsz];

  for (i=0; i<nmod; i++) {
    memcpy(tmp,&(modList[i].modelPn),inc=sizeof(uint16)); tmp+=inc;
    strcpy(ctmp,modList[i].name.c_str());
    for (j=strlen(ctmp);j<modnsz;j++) ctmp[j]='\0';
    memcpy(tmp, ctmp, inc=modnsz); tmp+=inc;
    memcpy(tmp,&(modList[i].run[0]), inc=sizeof(uint32)); tmp+=inc;
    memcpy(tmp,&(modList[i].run[1]), inc=sizeof(uint32)); tmp+=inc;
    memcpy(tmp,&(modList[i].run[2]), inc=sizeof(uint32)); tmp+=inc;
    memcpy(tmp,&(modList[i].run[3]), inc=sizeof(uint32)); tmp+=inc;
    memcpy(tmp,&(modList[i].run[4]), inc=sizeof(uint32)); tmp+=inc;
    memcpy(tmp,&(modList[i].run[5]), inc=sizeof(uint32)); tmp+=inc;

    for (j=0; j<modList[i].textlines.size(); j++){
      strcpy(txtline, modList[i].textlines[j].c_str());
      for (int k=strlen(txtline); k<modtxtsz; k++)
	txtline[k]='\0';
      memcpy(tmp,  txtline, inc=modtxtsz);  tmp+=inc;
    }
  }

  VSwrite(vdid,buf,nmod,FULL_INTERLACE);

  delete[] ctmp;
  delete[] buf;
  delete[] txtline;
  VSdetach(vdid);
  Vdetach(vgid);

  *ef = OK;
  return true;
}



void HDFFile::_setData(int index, const std::string& shortName,
		       const ParId& pid)
{
#ifdef DEBUG
  cout << "HDFFile::_setData" << endl;
#endif
  HDFPar* onePar=0;

  // find the parameter in parList corresponding to shortName
  npar= parList.size();
  int i;
  for (i=0;i<npar;++i) {
    if (shortName == parList[i].alias) {
      onePar = &parList[i];
      break;
    }
  }
#ifdef DEBUG
  if (onePar)
    cout << "_setData: Found parameter with index:" << i << endl;
  else
    cout << "_setData: Parameter not found" << endl;
#endif
  if (!onePar) return;

  int np= parameters.size();
  if (index <0 || index >=np) return;
  // subtracting plottype by one because HDF plottype starts at 1
  parameters[index].setType((ptPrimitiveType)(onePar->plotType-1));
  parameters[index].setPolar(false); // default = skalar
  parameters[index].setId(pid);

  float scale = pow(10.0,onePar->scale);
  // loop through data: scale
  for (int i=0;i<parameters[index].size();++i)
    parameters[index].setData(i,parameters[index].Data(i)*scale);
}
