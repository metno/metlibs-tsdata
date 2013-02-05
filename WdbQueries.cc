
/*
 * WdbQueries.cc
 *
 *  Created on: Feb 26, 2010
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
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <set>

#include "WdbQueries.h"

using namespace std;

namespace pets  {
namespace QUERY   {

static string quote= "\'";

string RAW()
{
  return " (RAW)";
}


static string quoted(string token)
{
  if(token=="NULL") return token;

  return quote+token+quote;
}

string CONNECT(string host, string user)
{
  ostringstream query;
  query << "dbname=wdb user=" << user << " host=" << host;
  return query.str();
};

string BROWSE(std::string browse)
{
  ostringstream query;
  query << "SELECT * FROM wci.browse(" <<  browse << ")";
  return query.str();
};

string BEGIN(std::string user)
{
  ostringstream query;
  query <<  "SELECT wci.begin(" << quoted(user) << ")";
  return query.str();
}

string READ(string token)
{
  ostringstream query;
  query << "SELECT * FROM wci.read(" << token << ")";
  return query.str();
}


string REFERENCETIMES( std::string providerName)
{
  ostringstream query;
  query << "ARRAY["  << quoted(providerName) << "],NULL, NULL,NULL, NULL,NULL, NULL,NULL::wci.browsereferencetime";

  return BROWSE(query.str() );
}


string PARAMETERS(std::string providerName,miutil::miTime referencetime)
{
  ostringstream query;
  query << "ARRAY["  << quoted(providerName)
        << "],NULL," << quoted(referencetime.isoTime())
        << ",NULL, NULL, NULL, NULL, NULL::wci.browsevalueparameter";

  return BROWSE( query.str());
}


string TIMESERIES( string model,
    miutil::miTime run,
    string parameter,
    float  lat,
    float  lon,
    string height)
{

  bool isRaw = bool(boost::find_first(parameter,RAW()));

  if(isRaw)
    boost::erase_all(parameter,RAW());

  ostringstream query;
  query << "ARRAY["
      << quoted(model)         << "],"
      << quote                 <<  "bilinear POINT(" << lon << " " << lat  << ")" << quote  << ","
      << quote << run << quote << ",NULL, ARRAY["    << quoted(parameter)  << "],"
      << quoted(height)        << ",ARRAY["
      << (isRaw ? 0 : -1 )
      << "], NULL::wci.returnfloat";

  return READ(query.str());
}
std::string CACHEQUERY(std::string model, std::string run, std::vector<std::string> parameters, std::string height)
{
  set<string> par;

  bool hasRaw=false;
  ostringstream query;
  query << " SELECT  wci.cachequery(ARRAY["  << quoted(model) << "],NULL,"<< quoted(run) << ",NULL,";

  if(parameters.empty())
    query << "NULL";
  else {
    query << "ARRAY[";
    bool first=true;
    for(size_t i=0 ; i < parameters.size();i++) {

      if(boost::find_first(parameters[i],RAW())){
        hasRaw=true;
        boost::erase_all(parameters[i],RAW());
        if(par.count(parameters[i]))
          continue;
        par.insert(parameters[i]);
      }

      query  << ( !first ? "," : "" )  << quoted(parameters[i]);
      first=false;

    }
    query << "]";
  }
  query<< "," << quoted(height) << "," <<  ( hasRaw ? "NULL" : "ARRAY[-1]" )  << ")";

  return query.str();
}





string LEVELS(std::string providerName, miutil::miTime referencetime)
{
  ostringstream query;
  query << "ARRAY["
        << quoted(providerName)
        << "],NULL," << quoted(referencetime.isoTime())
        << ",NULL, NULL, NULL, NULL, NULL::wci.browselevelparameter";

   return BROWSE(query.str());
}


std::string GRIDNAME(std::string providerName)
{
  ostringstream query;
  query << "ARRAY[" << quoted(providerName) << "],NULL, NULL,NULL, NULL,NULL, NULL, NULL::wci.browseplace";
  return BROWSE(query.str());
}


std::string GEOMETRY(std::string gridName)
{

  ostringstream query;
  query << "SELECT astext(placegeometry) FROM wci.getPlaceDefinition(" << quoted(gridName) <<")";
  return query.str();

}

std::string PROJECTION(std::string gridName) {
  ostringstream query;
  query << "select projdefinition,startx,starty from wci.getplaceregulargrid(" << quoted(gridName) <<")";
  return query.str();
}

} // << QUERY
} // << pets
