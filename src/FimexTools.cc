/*
 * FimexTools.cc
 *
 *  Created on: Aug 8, 2013
 *      Author: juergens
 */

#include "FimexTools.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <fstream>

using namespace std;

namespace pets {

void FimexPoslist::addFile(std::string filename)
{
  ifstream in(filename.c_str());
  if(!in)
    throw FimexstreamException( filename+"not found");

  while (in) {
    string line;
    getline(in,line);
    int c = line.find_first_of("#",0);
    if(0 <= c &&  c < line.length() ) {
      int k=line.length() -  c;
      line.erase(c,k);
    }
    boost::algorithm::trim(line);
    if(!line.empty())
      addEntry(line);
  }
}
void FimexPoslist::addEntry(std::string entry)
{
  vector<string> words;
  boost::split( words, entry, boost::is_any_of("|") );
  if(words.size() < 2 ) return;
  string data  = words[1];
  string posname = words[0];


  int n = posname.rfind(".");

  if( n>=0 && n < posname.size() )
    posname.erase(0,n+1);

  boost::trim(posname);

  // avoid duplicates
  if(pos.count(posname))
    return;

  vector<string> coordinates;

  boost::split( coordinates, data, boost::is_any_of(":") );

  if(coordinates.size() < 2 ) return;

  double latitude = atof(coordinates[0].c_str());
  double longitude= atof(coordinates[1].c_str());

  addEntry(posname,latitude,longitude);

}

void FimexPoslist::addEntry(std::string posname,float latitude, float longitude)
{
  // avoid duplicates
  if(pos.count(posname))
    return;

  unsigned int lastPos=lat.size();
  lat.push_back(latitude);
  lon.push_back(longitude);
  pos[posname] = lastPos;

}










void FimexPoslist::clear()
{
  lat.clear();
  lon.clear();
  pos.clear();
}


int FimexPoslist::getPos(std::string posname, float nlat, float nlon)
{
  if(pos.count(posname))
    return pos[posname];

  for(unsigned int i=0; i< lat.size(); i++) {
    if( fabs(lat[i] - nlat) < 0.00001)
      if( fabs(lon[i] - nlon ) < 0.00001)
        return i;
  }

  return -1;
}


void FimexDimension::setFromString(std::string token)
{
  boost::trim(token);
  vector<string> tokens;
  boost::split( tokens, token, boost::is_any_of(" ") );

  if(tokens.size() < 3 )
    throw FimexstreamException(string("unable to set fimexDimension from ")+token);

  name  = tokens[0];
  start = atoi(tokens[1].c_str());
  size  = atoi(tokens[2].c_str());
}



void FimexParameter::setFromString(std::string token)
{
  vector<string> tokenlist;
  boost::split( tokenlist, token, boost::is_any_of("=") );
  if(tokenlist.size() < 2 )
    throw FimexstreamException(string("unable to set fimexParameter from ")+token);

  petsname = boost::trim_copy(tokenlist[0]);
  parid.setFromString(petsname);

  string         parameterdef=tokenlist[1];
  vector<string> parameterdeflist;


  boost::split( parameterdeflist, parameterdef, boost::is_any_of("|") );

  if(parameterdeflist.size() < 3 )
    throw FimexstreamException(string("unable to set fimexParameterDefinitions from ")+parameterdef);

  streamtype    = boost::trim_copy(parameterdeflist[0]);
  parametername = boost::trim_copy(parameterdeflist[1]);
  unit          = boost::trim_copy(parameterdeflist[2]);
  dimensions.clear();

  if(parameterdeflist.size() > 3) {
    for(unsigned int i=3;i<parameterdeflist.size();i++) {
      FimexDimension dim;
      try {
        dim.setFromString(parameterdeflist[i]);
        dimensions.push_back(dim);
      } catch( exception& e){
        cerr << e.what() << endl;
      }
    }
  }
}

// ParId Assignement is asymetric!!!!! A==B is not B==A !!
// this is why we have getExtrapars and getOutpars !!!
void FimexPetsCache::getOutpars(std::vector<ParId>& inpar, std::vector<ParId>& outpar)
{
  for(unsigned int i=0;i<inpar.size();i++) {
    bool foundpid=false;
    for(unsigned int j=0; j < parameters.size() ; j++) {
      if( parameters[j].Id() == inpar[i]) {
        foundpid=true;
        break;
      }
    }

    if(!foundpid)
      outpar.push_back(inpar[i]);
  }
}

//  ParId Assignement is asymetric!!!!! A==B is not B==A !!
// this is why we have getExtrapars and getOutpars !!!
void FimexPetsCache::getExtrapars(std::vector<ParId>& inpar, std::vector<ParId>& extrapar)
{
  for(unsigned int i=0;i<inpar.size();i++) {
    bool foundpid=false;
    for(unsigned int j=0; j < parameters.size() ; j++) {
      if( inpar[i] == parameters[j].Id()) {
        foundpid=true;
        break;
      }
    }

    if(!foundpid)
      extrapar.push_back(inpar[i]);
  }
}








void FimexPetsCache::process(ParId pid)
{
  // add Data dimensions
  WeatherParameter wp;

  unsigned int numTimes   = tmp_times.size();

  if(!numTimes) // the timeline is empty - nothing to to
    return;

  unsigned int numPardims = tmp_values.size() / numTimes; // equals 1 except for ensembles

  wp.setDims(numTimes,numPardims);
  parameters.push_back(wp);

  for (unsigned int tim=0; tim < numTimes ; tim++) {
    for (unsigned int pardim=0; pardim < numPardims ; pardim++) {


      unsigned int index = pardim * numTimes + tim;

      parameters.back().setData(tim,pardim,tmp_values[ index ]);
    }
  }

  int tlindex = timeLines.addTimeline(tmp_times);
  parameters.back().setTimeLineIndex(tlindex);
  parameters.back().setId(pid);
  parameters.back().calcAllProperties();

}


void FimexPetsCache::clear_tmp()
{
  tmp_values.clear();
  tmp_times.clear();

}



} /* namespace pets */
