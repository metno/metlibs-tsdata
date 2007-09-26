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


#ifndef _ptMIROS22File_h
#define _ptMIROS22File_h

#include "ptDataStream.h"
#include <puTools/miDate.h>
#include <puTools/miClock.h>

using namespace std; 

const float MIROSundef=-999.99;   // parameter undef-value 

// defines one MIROS parameter
class MIROS22parameter {
public:
  miString block;    // block-id, 2 alphabetic characters
  char sensor;       // sensor-id, 1 alphanumeric character
  int parnum;        // number in block
  ParId parid;       // Pets parameter-ident
  MIROS22parameter()
    : sensor(' '),parnum(0){}
};

// defines one MIROS parameter-value
class MIROS22value {
public:
  MIROS22parameter par; // parameter definition
  bool changed;         // parameter found and value changed
  float value;          // measurement value
  MIROS22value()
    : changed(false),value(MIROSundef){}
};

// defines a set of parameters for one timestep
class MIROS22parset {
public:
  miTime t;    // timestep
  vector<MIROS22value> val; // list of parameter-values
};


class MIROS22location {
public:
  miPosition loc;    // pets position
  miString name;     // pos.name on file
  miString filepath; // path to datafiles
};

// MIROS DF022 definitions for MIROS22Server
// - define parameters to read
// - define filenames/locations
class MIROS22Definition {
public:
  miString filename; // definitionfile
  ParameterDefinition pard;
  vector<MIROS22parameter> pars;
  vector<MIROS22location> locs;
  MIROS22Definition() {}
  MIROS22Definition(const miString& fn);
  void set(const miString& fn);
  bool scan();
};

// Single MIROS DF022 fileclass
class MIROS22File {
public:
  enum status {
    read_ok,
    missing_par,
    wrong_time,
    wrong_place,
    bad_file,
    cant_open
  };
private:
  miString filename;

public:
  MIROS22File(const miString& fn);
  bool read(const miString& location,
	    const miDate& date,
	    vector<MIROS22parset>& ps,
	    status& res);
};


// Handler for multiple MIROS DF022 files
class MIROS22Server : public DataStream {
public:
  MIROS22Server(const miString&);
  ~MIROS22Server();

  int  findStation(const miString& posName); // return index in posList
  int  findModel(const miString& modelName,
		 const int& modelRun);       // return index in modList
  int  findDataPar(const ParId&);            // return index in parList
  void clean();
  bool openStream(ErrorFlag*);
  bool openStreamForWrite(ErrorFlag*);
  bool readData(const int posIndex, const ParId&,
		const miTime&,const miTime&,ErrorFlag*);
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
  bool getStationSeq(int idx, miString& name, // fetch name and position
		     float& lat, float& lng); // of station idx
  bool getStationSeq(int, miPosition&);
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id);
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id,
		   vector<miString>& vs);
  int  putStation(const miPosition& s, //adds station to posList
		  ErrorFlag*);
  bool writeData(const int posIndex,      //write data to file
		 const int modIndex,
		 ErrorFlag*,
		 bool complete_write,
		 bool write_submodel);
  bool close(); // close server
private:
  MIROS22Definition mirosdef;
  vector<miString> modList;
};

#endif
