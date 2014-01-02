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


#ifndef _ptHDFFile_h
#define _ptHDFFile_h

#include "ptDataStream.h"

#include <puCtools/porttypes.h>  

//#include <parameter/parameter.h> 

class HDFFile : public DataStream {
public:
  HDFFile(const std::string&);
  ~HDFFile();

  int  findStation(const std::string& posName); // return index in posList
  int  findModel(const std::string& modelName,
		 const int& modelRun);       // return index in modList
  int  findDataPar(const ParId&);            // return index in parList
  void clean();
  bool openStream(ErrorFlag*);
  bool openStreamForWrite(ErrorFlag*);
  bool readData(const int posIndex, const ParId&,
		const miutil::miTime&, const miutil::miTime&,
		ErrorFlag*);
  bool getTimeLine(const int& index,
		   std::vector<miutil::miTime>& tline, std::vector<int>& pline,
		   ErrorFlag*);
  bool putTimeLine(const int& index,
		   std::vector<miutil::miTime>& tline, std::vector<int>& pline,
		   ErrorFlag*);
  bool putTimeLine(TimeLine& tl, std::vector<int>& pline,
		   ErrorFlag*);
  bool getOnePar(int,WeatherParameter&,ErrorFlag*);
  bool putOnePar(WeatherParameter&,ErrorFlag*);
  bool getStations(std::vector<miPosition>&);
  bool getStationSeq(int, miPosition&);
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id);
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id,
		   std::vector<std::string>& vtl);
  int  putStation(const miPosition& s, //adds station to posList
		  ErrorFlag*);
  bool writeData(const int posIndex,      //write data to file
		 const int modIndex,
		 ErrorFlag*,
		 bool complete_write,
		 bool write_submodel);
  bool close(); // close file
  void getTextLines(const ParId p, std::vector<std::string>& tl);
private:
  int32 fid;            // HDF file handler
  int32 posVG;          // the main position vgroup
  bool hasposVG;

  struct HDFPar {
    uint16 num;
    std::string name;
    std::string alias;
    std::string unit;
    int8   scale;
    int32  size;     // size in bytes
    int32  order;    // scalar, vector
    uint16 dataType; // cartesian, polar
    uint16 plotType; // line, histogram etc.
  };
  struct HDFPos {
    int32    ref;       // Reference number in HDF file
    std::string name;      // Modelname
    float32  geopos[2]; // Longitude/latitude (before: int16)
    float32  topo;      // Topography
  };
  struct HDFMod {
    uint16   modelPn; // production number
    std::string name;    // model name
    uint32   run[6];  // year,month,day,hour,minute,second
    Model    modelid; // model id
    Run      runid;   // run id
    std::vector<std::string> textlines;
  };

  std::vector<HDFPar> parList;
  std::vector<HDFPos> posList;
  std::vector<HDFMod> modList;

  bool _openFile(ErrorFlag*);
  bool _readParList(ErrorFlag*);
  bool _readPosList(ErrorFlag*);
  bool _readModList(ErrorFlag*);
  void _setData(int,const std::string&, const ParId&);

  bool _createFile(ErrorFlag*);
  bool _writeParList(ErrorFlag*);
  bool _prepPosRefs(ErrorFlag*);
  bool _writePosList(ErrorFlag*);
  bool _writeModList(ErrorFlag*);
public:
  bool getFullModeltime(int id, miutil::miTime& t);
};

#endif
