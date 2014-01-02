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

// ptDataStream.h

#ifndef DataStream_h
#define DataStream_h

#include "ptError.h"
#include "ptParameterDefinition.h"
#include "ptWeatherParameter.h"
#include "ptTimeLine.h"

#include <puDatatypes/miPosition.h>
#include <puTools/miTime.h>

#include <vector>

class DataStream {
public:
  DataStream(const std::string& fname) :
    numTimeLines(0), IsOpen(false), InfoIsRead(false), IsCleaned(true),
        DataIsRead(false), TimeLineIsRead(false), npar(0), npos(0), nmod(0)
  {
    Name = fname;
  }

  virtual ~DataStream()
  {
  }

  const std::string& name()
  {
    return Name;
  }
  bool isOpen()
  {
    return IsOpen;
  }
  bool infoIsRead()
  {
    return InfoIsRead;
  }
  bool dataIsRead()
  {
    return DataIsRead;
  }
  bool timeLineIsRead()
  {
    return TimeLineIsRead;
  }
  int numParameters()
  {
    return npar;
  }
  int numPositions()
  {
    return npos;
  }
  int numModels()
  {
    return nmod;
  }

  virtual int findStation(const std::string&)=0; // find station index
  virtual int findDataPar(const ParId&)=0;
  virtual void clean()=0;
  virtual void cleanParData()
  {
    parameters.clear();
    DataIsRead = false;
  }
  virtual bool openStream(ErrorFlag*)=0;
  virtual bool openStreamForWrite(ErrorFlag*)=0;
  virtual bool readData(const int posIndex, const ParId&, const miutil::miTime&,
      const miutil::miTime&, ErrorFlag*)=0;
  virtual bool getTimeLine(const int& index, std::vector<miutil::miTime>& tline,
      std::vector<int>& pline, ErrorFlag*)=0;
  virtual bool putTimeLine(const int& /*index*/, std::vector<miutil::miTime>& /*tline*/,
      std::vector<int>& /*pline*/, ErrorFlag*)
  {
    return false;
  }
  virtual bool putTimeLine(TimeLine& /*tl*/, std::vector<int>& /*pline*/, ErrorFlag*)
  {
    return false;
  }
  virtual bool getOnePar(int, WeatherParameter&, ErrorFlag*)=0;
  virtual bool putOnePar(WeatherParameter&, ErrorFlag*)
  {
    return false;
  }
  virtual bool getStations(std::vector<miPosition>&)
  {
    return false;
  }
  virtual bool getStationSeq(int, miPosition&)=0;
  virtual bool getModelSeq(int, Model&, // fetch model info
      Run&, int&)=0;
  virtual bool getModelSeq(int, Model&, // fetch model info
      Run&, int&, std::vector<std::string>&)=0;
  virtual int putStation(const miPosition& /*s*/, //adds station to posList
      ErrorFlag*)
  {
    return 0;
  }
  virtual bool writeData(const int /*posIndex*/, //write data to file
      const int /*modIndex*/, ErrorFlag*, bool /*complete_write*/, bool /*write_submodel*/)
  {
    return false;
  }
  virtual bool close() = 0;
  virtual void getTextLines(const ParId /*p*/, std::vector<std::string>& tl)
  {
    tl = textLines;
  }
protected:
  std::vector<miutil::miTime> timeLine; // temp. storing variables
  std::vector<int> progLine; // "
  std::vector<ProgLine> progLines; // List of proglines
  TimeLine timeLines; // Full timeline
  int numTimeLines;
  std::vector<WeatherParameter> parameters;
  std::vector<std::string> textLines;

  ParameterDefinition pdef; // parameter and model id mappings
  std::string Name;
  bool IsOpen;
  bool InfoIsRead;
  bool IsCleaned;
  bool DataIsRead;
  bool TimeLineIsRead;
  int npar;
  int npos;
  int nmod;
};

#endif
