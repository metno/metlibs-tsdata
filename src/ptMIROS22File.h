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

const float MIROSundef=-999.99;   // parameter undef-value 

// defines one MIROS parameter
class MIROS22parameter {
public:
  std::string block;    // block-id, 2 alphabetic characters
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
  miutil::miTime t;    // timestep
  std::vector<MIROS22value> val; // list of parameter-values
};


class MIROS22location {
public:
  miPosition loc;    // pets position
  std::string name;     // pos.name on file
  std::string filepath; // path to datafiles
};

// MIROS DF022 definitions for MIROS22Server
// - define parameters to read
// - define filenames/locations
class MIROS22Definition {
public:
  std::string filename; // definitionfile
  ParameterDefinition pard;
  std::vector<MIROS22parameter> pars;
  std::vector<MIROS22location> locs;
  MIROS22Definition() {}
  MIROS22Definition(const std::string& fn);
  void set(const std::string& fn);
  bool scan();
  std::string replaceEnv(std::string);
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
  std::string filename;

public:
  MIROS22File(const std::string& fn);
  bool read(const std::string& location,
	    const miutil::miDate& date,
	    std::vector<MIROS22parset>& ps,
	    status& res);
};


// Handler for multiple MIROS DF022 files
class MIROS22Server : public DataStream {
public:
  MIROS22Server(const std::string&);
  ~MIROS22Server();

  int  findStation(const std::string& posName) override; // return index in posList
  int  findModel(const std::string& modelName,
		 const int& modelRun);       // return index in modList
  int  findDataPar(const ParId&) override;            // return index in parList
  void clean() override;
  bool openStream(ErrorFlag*) override;
  bool openStreamForWrite(ErrorFlag*) override;
  bool readData(const int posIndex, const ParId&,
                const miutil::miTime&,const miutil::miTime&,ErrorFlag*) override;
  bool getTimeLine(int index,
		   std::vector<miutil::miTime>& tline, std::vector<int>& pline,
                   ErrorFlag*) override;
  bool putTimeLine(int index,
		   std::vector<miutil::miTime>& tline, std::vector<int>& pline,
                   ErrorFlag*) override;
  bool putTimeLine(TimeLine& tl, std::vector<int>& pline,
                   ErrorFlag*) override;
  bool getOnePar(int,WeatherParameter&,ErrorFlag*) override;
  bool putOnePar(WeatherParameter&,ErrorFlag*) override;
  bool getStationSeq(int idx, std::string& name, // fetch name and position
		     float& lat, float& lng); // of station idx
  bool getStationSeq(int, miPosition&) override;
  bool getModelSeq(int idx, Model& mod,       // fetch model info
                   Run& run, int& id) override;
  bool getModelSeq(int idx, Model& mod,       // fetch model info
		   Run& run, int& id,
                   std::vector<std::string>& vs) override;
  int  putStation(const miPosition& s, //adds station to posList
                  ErrorFlag*) override;
  bool writeData(const int posIndex,      //write data to file
		 const int modIndex,
		 ErrorFlag*,
		 bool complete_write,
                 bool write_submodel) override;
  bool close() override; // close server
private:
  MIROS22Definition mirosdef;
  std::vector<std::string> modList;
};

#endif
