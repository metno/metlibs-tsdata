/*
 * FimexStream.cc
 *
 *  Created on: Aug 2, 2013
 *      Author: juergens
 */

/*
 $Id$

 Copyright (C) 2006 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: diana@met.no

 This file is part of generated by met.no

 This is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Tseries; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "FimexStream.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <puTools/miString.h>

#include <fimex/CDMReaderUtils.h>
#include <fimex/Utils.h>


using namespace std;
using namespace miutil;

namespace pets
{
FimexPoslist FimexStream::commonposlist;
int          FimexStream::commonposlistVersion=0;



void FimexStream::setCommonPoslist(const FimexPoslist& cposlist)
{
  commonposlist = cposlist;
  commonposlistVersion++;
}

void FimexStream::setCommonPoslistFromStringlist(std::vector<std::string> newposlist)
{
  cerr << "Creating new fimexposlist" << endl;
  FimexPoslist newFimexposlist;

  for(unsigned int i=0;i<newposlist.size();i++)
    newFimexposlist.addEntry(newposlist[i]);

  setCommonPoslist(newFimexposlist);

}



void FimexStream::setPositions()
{
 poslist = commonposlist;
 poslistVersion = commonposlistVersion;
}



FimexStream::FimexStream(const std::string& fname,
    const std::string& modname,
    const std::string& ftype,
    const std::vector<pets::FimexParameter>& fimexparameters)
:  filename(fname) , modelname(modname), filetype(ftype), fimexpar(fimexparameters) , progtime(0)
{
  timeLineIsRead=false;
  poslist = commonposlist;
  poslistVersion = commonposlistVersion;

}


FimexStream::~FimexStream()
{

}

void FimexStream::clean()
{

}


bool FimexStream::createPoslistInterpolator()
{
  try {
    if(!reader)
      openStream();

    interpol.reset( new MetNoFimex::CDMInterpolator (reader));
    interpol->changeProjection(MIFI_INTERPOL_BILINEAR,poslist.getLon(), poslist.getLat() );

  } catch (exception& e) {
    cerr << e.what() << endl;
    return false;
  }

  return true;
}





void FimexStream::createTimeLine()
{
  basetimeline.clear();

  try {
    if(!interpol)
      createPoslistInterpolator();

    MetNoFimex::DataPtr timeData = interpol->getScaledDataInUnit("time","seconds since 1970-01-01 00:00:00 +00:00");
    boost::shared_array<unsigned long long> uTimes = timeData->asUInt64();

    for(size_t u = 0; u < timeData->size(); ++u)
      basetimeline.push_back( miutil::miTime(uTimes[u]) );


  } catch (exception& e) {
    cerr << "Exception catched in createTimeline" << e.what() << endl;
  }

  timeLineIsRead=true;
}



void FimexStream::openStream()
{
  try {
    reader = MetNoFimex::CDMFileReaderFactory::create(filetype,filename);
  } catch (exception& e) {
    throw FimexstreamException("Could not open fimexstream");
  }
}


boost::posix_time::ptime FimexStream::getReferencetime()
{

  if(!reader)
    openStream();

  return MetNoFimex::getUniqueForecastReferenceTime(reader);

}


bool FimexStream::readData(std::string placename,float lat, float lon, vector<ParId>& inpar,vector<ParId>& outpar)
{
  try {

    clean();

    if(!interpol) {
      createPoslistInterpolator();
    }

    if(!timeLineIsRead) {
      createTimeLine();
    }

    if(placename.empty()) {
      return false;
    }

    activePosition = poslist.getPos(placename,lat,lon);

    // position not found
    if(activePosition < 0 ) {

      // aha the common poslist has changed - try to reload the cache
      if(poslistVersion != commonposlistVersion) {
        cerr << "Poslist has changed - filling cache" << endl;
        poslist = commonposlist;
        poslistVersion = commonposlistVersion;
        createPoslistInterpolator();
        cache.clear();
        activePosition = poslist.getPos(placename,lat,lon);
      }
    }


    if(cache.empty() ) {
      addToCache(0,poslist.getNumPos(),inpar,true);
    } else {
      // check if there are parameters that were not interpolated earlier
      vector<ParId> extrapar;
      cache[0].getExtrapars(inpar,extrapar);


      // try to interpolate the missing parameters
      addToCache(0,poslist.getNumPos(),extrapar,false);

    }



    if(activePosition < 0 ) {
      cerr << placename << " not found " << endl;
      return false;

    }

    cache[activePosition].getOutpars(inpar,outpar);

  } catch ( exception& e) {
    cerr << "exception caught  in Fimex read : " << e.what();
    return false;
  }

  return true;
}



void FimexStream::addToCache(int posstart, int poslen,vector<ParId>& inpar, bool createPoslist)
{

  FimexPetsCache tmp;
  if(createPoslist)
    for(unsigned int i=0;i<poslen;i++)
      cache.push_back(tmp);
  // check the parameterlist - what to get and what not....

  vector<FimexParameter> activeParameters;

  cerr << "Filling cache" << endl;
  boost::posix_time::ptime start  = boost::posix_time::microsec_clock::universal_time();
  for(unsigned int i=0;i<inpar.size();i++) {

    for( unsigned int j=0;j<fimexpar.size();j++) {
      if ( fimexpar[j].parid == inpar[i] ) {
        try {
          readFromFimexSlice(fimexpar[j]);
        } catch ( exception& e) {
          cerr << e.what() << endl;
        }

        break;


      }
    }
  }
  boost::posix_time::ptime last   = boost::posix_time::microsec_clock::universal_time();
  cerr << "Cache filled in: "  << (last-start).total_milliseconds() << " ms   " << endl;
}



bool FimexStream::readFromFimexSlice(FimexParameter par)
{
  cerr << "Interpolating Parameter: " << par.parametername;
  boost::posix_time::ptime start  = boost::posix_time::microsec_clock::universal_time();
  MetNoFimex::SliceBuilder slice(interpol->getCDM(),par.parametername);

  unsigned int numPos=poslist.getNumPos();

  slice.setStartAndSize("x", 0, poslist.getNumPos());

  for(unsigned int i=0;i<cache.size();i++)
    cache[i].clear_tmp();

  for(unsigned int i=0; i<par.dimensions.size();i++) {
    slice.setStartAndSize(par.dimensions[i].name ,par.dimensions[i].start ,par.dimensions[i].size );
  }


  MetNoFimex::DataPtr sliceddata  = interpol->getScaledDataSliceInUnit( par.parametername, par.unit, slice);


  if(sliceddata.get()) {


    boost::shared_array<float> valuesInSlice = sliceddata->asFloat();

    unsigned int numTim= sliceddata->size() / numPos;

    for(unsigned int tim=0;tim < numTim; tim++)
      for(unsigned int pos=0;pos<numPos;pos++) {

        if(!MetNoFimex::mifi_isnan(valuesInSlice[tim*numPos + pos ]) ) {
          cache[pos].tmp_times.push_back(basetimeline[tim]);
          cache[pos].tmp_values.push_back(valuesInSlice[ tim*numPos + pos ]);
        }
      }


    ParId pid = par.parid;
    pid.model = modelname;
    pid.run   = 0;
    pid.level = 0;

    for(unsigned int i=0;i<cache.size();i++)
      cache[i].process(pid);



    boost::posix_time::ptime last   = boost::posix_time::microsec_clock::universal_time();
    cerr << " ... done. Used : "  << (last-start).total_milliseconds() << " ms   " << endl;

    return true;
  }
  cerr << " ... empty  "<< endl;
  return false;
}




bool FimexStream::getOnePar(int i, WeatherParameter& wp)
{
  if(i>=0 && i<(int)cache[activePosition].parameters.size()) {
    wp=cache[activePosition].parameters[i];
    return true;
  }
  return false;
}

bool FimexStream::getTimeLine(int index,vector<miTime>& tline, vector<int>& pline)
{
  pline=progline;

  return cache[activePosition].timeLines.Timeline(index,tline);
}

int FimexStream::numParameters()
{
  return cache[activePosition].parameters.size();
}



} /* namespace pets */
