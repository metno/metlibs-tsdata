/*
  libtsData - Time Series Data

  Copyright (C) 2006-2016 met.no

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

#include "ptSHCinfo.h"

#include <puTools/miStringFunctions.h>

#include <fstream>

#define MILOGGER_CATEGORY "metlibs.tsdata.SHCinfo"
#include <miLogger/miLogging.h>

using namespace miutil;
using namespace std;

int SHClevel::dirIndex(const float& angle)
{
  int n= dirs.size();
  float a= angle;
  if (a<0) a+= 360.0;
  else if (a>360) a-= 360.0;

  for (int idx=0; idx<n; idx++){
    // check special for angels crossing zero
    if (dirs[idx].low > dirs[idx].high){
      if ((a > dirs[idx].low && a <= 0) ||
          (a>=0 && a<= dirs[idx].high))
        return idx;
    } else {
      if (a > dirs[idx].low && a <= dirs[idx].high)
        return idx;
    }
  }

  return -1;
}

int SHCinfo::levelIndex(const int& level)
{
  if (levels.size()>0){
    for (int i=0; i<(int)levels.size(); i++)
      if (levels[i].level==level)
        return i;
  }
  return -1;
}

int SHCinfo::lowLevel()
{
  if (levels.size()){
    return levels[0].level;
  } else return 0;
}

int SHCinfo::highLevel()
{
  if (levels.size()){
    return levels[levels.size()-1].level;
  } else return 0;
}

bool SHCcollection::readList(const std::string& filename)
{
  METLIBS_LOG_SCOPE("READ SHC-information from:" << filename);
  // first clean up
  total.levels.clear();
  list.clear();

  ifstream f(filename);
  if (!f) {
    METLIBS_LOG_ERROR("could not open file '" << filename << "'");
    return false;
  }

  enum read_mode {
    read_common,
    read_loc,
    read_val
  };
  read_mode mode= read_common;
  std::string buf;
  std::string totalname;
  int numloc= -1;
  int numlev=-1, numdir=-1, ilevel=-1;
  SHCdir dir;
  SHClevel level;
  SHCinfo info;
  vector<float> fdir;

  while (getline(f, buf)){
    miutil::trim(buf);
    if (buf.length()==0) continue;
    if (buf[0]=='#') continue;

    const vector<std::string> vs= miutil::split(buf, "=");

    if (miutil::contains(buf, "[LOCATION]")){
      if (numlev<0 || numdir<0){
        METLIBS_LOG_ERROR("directions and levels before locations");
        return false;
      }
      list.push_back(info);
      numloc++;
      ilevel=-1;
      mode= read_loc;
      for (int i=0; i<numlev; i++){
        list[numloc].levels.push_back(level);
        list[numloc].levels[i].level= i+1;
        for (int j=0; j<numdir; j++){
          list[numloc].levels[i].dirs.push_back(dir);
          // put directions into levels
          list[numloc].levels[i].dirs[j].low= fdir[j];
          list[numloc].levels[i].dirs[j].high= fdir[j+1];
        }
      }

    } else if (mode==read_loc) {
      if (miutil::contains(buf, "name=")){
        list[numloc].name = vs[1];
      } else if (miutil::contains(buf, "[values]")){
        mode= read_val;
        if (numdir<0 || numlev<0){
          METLIBS_LOG_ERROR("directions and levels must be defined before values");
          return false;
        }
      }

    } else if (miutil::contains(buf, "name=")) {
      totalname= vs[1];

    } else if (miutil::contains(buf, "numlevels=")) {
      numlev= atoi(vs[1].c_str());

    } else if (miutil::contains(buf, "numdirections=")) {
      numdir= atoi(vs[1].c_str());

    } else if (miutil::contains(buf, "directions=")){
      if (numdir<0) {
        METLIBS_LOG_ERROR("numdirections before directions");
        return false;
      }
      const vector<std::string> vvs = miutil::split(vs[1], ",");
      if ((int)vvs.size()!=numdir+1) {
        METLIBS_LOG_ERROR("numdirections doesn't match");
        return false;
      }
      for (int i=0; i<numdir+1; i++)
        fdir.push_back(atof(vvs[i].c_str()));

    } else if (mode==read_val){
      ilevel++;
      if(ilevel >= numlev){
        METLIBS_LOG_ERROR("Too many levels in [values] block");
        return false;
      }
      const vector<std::string> vals = miutil::split(buf, ",");
      int nd= vals.size();
      if (nd!=numdir){
        METLIBS_LOG_ERROR("values' number of directions doesn't match");
        return false;
      }
      for (int i=0; i<numdir; i++){
        list[numloc].levels[ilevel].dirs[i].value= atof(vals[i].c_str());
      }
    }
  }

  // prepare total SHC info
  total.name= totalname;
  for (int i=0; i<numlev; i++){
    total.levels.push_back(level);
    total.levels[i].level= i+1;
    for (int j=0; j<numdir; j++){
      total.levels[i].dirs.push_back(dir);
      // put directions into levels
      total.levels[i].dirs[j].low= fdir[j];
      total.levels[i].dirs[j].high= fdir[j+1];
      total.levels[i].dirs[j].value= 10000;
    }
  }
  // find strongest criteria
  for (int i=0; i<(int)list.size(); i++){
    for (int j=0; j<(int)list[i].levels.size(); j++){
      for (int k=0; k<(int)list[i].levels[j].dirs.size(); k++){
        if (list[i].levels[j].dirs[k].value<total.levels[j].dirs[k].value)
          total.levels[j].dirs[k].value= list[i].levels[j].dirs[k].value;
      }
    }
  }

  return true;
}
