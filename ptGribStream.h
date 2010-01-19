/*
  libtsData - Time Series Data
  
  $Id: ptHDFFile.h 16 2007-09-26 06:55:49Z audunc $

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


#ifndef _ptGribStream_h
#define _ptGribStream_h

#include "ptDataStream.h"
#include <set>
#include <map>
#include <vector>
#include <puCtools/porttypes.h>  
#include <puTools/miSetupParser.h>  

#include <parameter/parameter.h> 

using namespace std; 
using miutil::SetupParser; 
class FieldManager;
class SetupParser;

/**
   \brief one section in setupfile

   list of strings with references to original linenumbers and filesources
*/
struct SetupSection {
  vector<miString> strlist;
  vector<int> linenum;
  vector<int> filenum;
};


class GribStream : public DataStream {
public:
  GribStream(const miString&);
  ~GribStream();

  int  findStation(const miString& posName); // return index in posList
  int  findModel(const miString& modelName,
		 const int& modelRun);       // return index in modList
  int  findDataPar(const ParId&);            // return index in parList
  void clean();
  bool openStream(ErrorFlag*);
  bool openStreamForWrite(ErrorFlag*);
  bool readData(const int posIndex, const ParId&,
		const miTime&, const miTime&,
		ErrorFlag*);
  bool getTimeLine(const int& index,
		   vector<miTime>& tline, vector<int>& pline,
		   ErrorFlag*);
  bool putTimeLine(const int& index,
		   vector<miTime>& tline, vector<int>& pline,
		   ErrorFlag*);
  bool putTimeLine(TimeLine& tl, vector<int>& pline,
		   ErrorFlag*);
  bool getOnePar(int,WeatherParameter&,ErrorFlag*);
  bool putOnePar(WeatherParameter&,ErrorFlag*);
  bool getStations(vector<miPosition>&);
  bool getStationSeq(int, miPosition&);
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id);
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id,
		   vector<miString>& vtl);
  int  putStation(const miPosition& s, //adds station to posList
		  ErrorFlag*);
  bool writeData(const int posIndex,      //write data to file
		 const int modIndex,
		 ErrorFlag*,
		 bool complete_write,
		 bool write_submodel);
  bool close(); // close file
  void getTextLines(const ParId p, vector<miString>& tl);
private:

  FieldManager  *fieldm; // FieldManager handles gribfiles
  SetupParser  *sp; // SetupParser handles all related .grib and other setup files
  miString stFileName;   // Stationlist file name
  miString parmodFileName;  // SMHI's setupfile i Diana for grib parameters and models  
  vector<miTime> validTime; // times from gribfiles 
  vector<int> forecastHour; // forecast hours from gribfiles
  std::map<miString,std::vector<float> > data; //map with a key as  from 

  /// Setuptext hashed by Section name
  //static map<miString, SetupSection> sectionm;
  //static map<miString, miString> substitutions;
  //static map<miString, miString> user_variables;

//  int32 posVG;          // the main position vgroup
  bool hasposVG;

  struct GribPar {
    int num;
    miString name;     // name that matches name in dianas GRIB_PARAMETERS
    miString alias;    // name used in tseries     
    miString unit;
    int      scale;
    int      size;     // size in bytes
    int      order;    // scalar, vector
    int      dataType; // cartesian, polar
    int      plotType; // line, histogram etc.
  };
  struct GribPos {
    int    ref;       // Reference number in Grib file
    miString name;      // Modelname
    float  geopos[2]; // Longitude/latitude (before: int16)
    float  topo;      // Topography
    float  topo2;      // Topography
  };
  struct GribMod {
    int   modelPn; // production number
    miString name;    // model name
    miString alias;  //name that matches model name in dianas setupfile
    int   run[6];  // year,month,day,hour,minute,second
    Model    modelid; // model id
    Run      runid;   // run id
    vector<miString> textlines;
  };

  vector<GribPar> parList;
  vector<GribPos> posList;
  vector<GribMod> modList;

  bool _openFile(ErrorFlag*);
  bool _readParList(ErrorFlag*);
  bool _readPosList(ErrorFlag*);
  bool _readModList(ErrorFlag*);
  void _setData(int,const miString&, const ParId&);

//  bool _createFile(ErrorFlag*);
//  bool _writeParLis/t(ErrorFlag*);
//  bool _prepPosRefs(ErrorFlag*);
//  bool _writePosList(ErrorFlag*);
//  bool _writeModLis/t(ErrorFlag*);
  bool _parseGrib();
//  bool _parseFile(const miString filename );
//  bool _checkSubstitutions(miString& t);
//  bool _getSection(const miString& sectname, vector<miString>& setuplines);
public:
  bool getFullModeltime(int id, miTime& t);
};

#endif
