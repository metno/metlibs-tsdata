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


#ifndef _ptAsciiStream_h
#define _ptAsciiStream_h


/* Created by met.no/FoU/PU 
   at Fri Aug  9 07:08:18 2002 */

#include "ptDataStream.h"
#include <set>

class AsciiStream : public DataStream {
private:
  struct AsciiLine{
    miutil::miTime time;
    int prog;
    int level;
    int submodel;
    std::vector<float> data;
  };
  struct AsciiMod{
    std::string name;
    miutil::miTime run;
  };
  struct AsciiData{
    AsciiMod model;           // current model
    miPosition pos;           // current position
    std::set<int> levels;          // unique levels
    std::set<int> submodels;       // unique submodels
    std::set<miutil::miTime> times;        // unique times
    std::vector<std::string> params;  // all param-names
    std::vector<AsciiLine> pardata;// one time/data line
  };

  std::vector<AsciiData> dataList;
  std::vector<miPosition> posList;
  std::vector<AsciiMod> modList;

  bool _openFile(ErrorFlag*);
//   bool _readParList(ErrorFlag*);
//   bool _readPosList(ErrorFlag*);
//   bool _readModList(ErrorFlag*);
//   void _setData(int,const miString&, const ParId&);

//   bool _createFile(ErrorFlag*);
//   bool _writeParList(ErrorFlag*);
//   bool _prepPosRefs(ErrorFlag*);
//   bool _writePosList(ErrorFlag*);
//   bool _writeModList(ErrorFlag*);
  

public:
  AsciiStream(const std::string&);
  ~AsciiStream();

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
		   std::vector<std::string>& vtl)
  {return getModelSeq(idx,mod,run,id);}
  int  putStation(const miPosition& s, //adds station to posList
		  ErrorFlag*);
  bool writeData(const int posIndex,      //write data to file
		 const int modIndex,
		 ErrorFlag*,
		 bool complete_write,
		 bool write_submodel);
  bool close(); // close file

};

#endif
