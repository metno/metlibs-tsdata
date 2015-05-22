/*
 libtsData - Time Series Data

 Copyright (C) 2013 met.no

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "ptDiagramData.h"

#include <puMet/ptStatistics.h>
#include <puMet/vision.h>
#include <puMet/windProfile.h>
#include <puMet/cloudGrp.h>
#include <puMet/iceAccretion.h>

//#include <weather_symbol/Factory.h>
//#include <weather_symbol/WeatherData.h>

#include <cmath>

#include <iostream>

using namespace std;

ptDiagramData::ptDiagramData() :
                                            nfetches(0), new_symbolmaker(false)
{
}

ptDiagramData::ptDiagramData(symbolMaker& wsym) :
                                                  nfetches(0), wsymbols(wsym), new_symbolmaker(false)
{
  makedefaultParInfo();
}

ptDiagramData::ptDiagramData(const ptDiagramData& rhs)
{
#ifdef DEBUG
  cerr << "ptDiagramData::ptDiagramData()" << endl;
#endif
  copyMembers(rhs);
}

ptDiagramData& ptDiagramData::operator=(const ptDiagramData& rhs)
{
#ifdef DEBUG
  cerr << "ptDiagramData::operator=" << endl;
#endif
  if (this == &rhs)
    return *this;

  copyMembers(rhs);
  return *this;
}

void ptDiagramData::copyMembers(const ptDiagramData& rhs)
{
#ifdef DEBUG
  cerr << "ptDiagramData::copyMembers" << endl;
#endif
  station = rhs.station;
  timeLine = rhs.timeLine;// sjekk = operator
  progLines = rhs.progLines;
  nfetches = rhs.nfetches;
  fetchRange = rhs.fetchRange;
  parList = rhs.parList; // sjekk = operator
  textLines = rhs.textLines;

  new_symbolmaker = rhs.new_symbolmaker;
  shcinfo = rhs.shcinfo;
  parInfo = rhs.parInfo;
  windCalc = rhs.windCalc;
}

void ptDiagramData::Erase()
{
  cleanDataStructure_();
}

void ptDiagramData::cleanDataStructure_()
{
#ifdef DEBUG
  cerr << "ptDiagramData::cleanDataStructure_" << endl;
#endif
  fetchRange.clear();
  timeLine.clear();
  progLines.clear();
  parList.clear();
  textLines.clear();
  nfetches = 0;
}

ostream& operator<<(ostream& out, /*const*/ptDiagramData& dd)
{
  unsigned int i, j;

  out << "Printing of ptDiagramData:\n" << "Station : " << dd.station << '\n';
  out << "The timeLine elements :\n";
  out << dd.timeLine;
  out << "\nThe progLine elements : ";
  for (i = 0; i < dd.progLines.size(); ++i) {
    out << "\nNo. " << i << ' ';
    for (j = 0; j < dd.progLines[i].size(); ++j)
      out << dd.progLines[i][j] << '\t';
  }
  out << "\nnfetches : " << dd.nfetches << '\n';
  for (i = 0; i < dd.fetchRange.size(); i++)
    out << i << ": " << dd.fetchRange[i] << '\n';

  if (dd.parList.size()) {
    out << "\nThe WeatherParameters :\n";
    for (i = 0; i < dd.parList.size(); i++)
      out << i << ": " << dd.parList[i] << '\n';
  }
  if (dd.textLines.size()) {
    out << "Printing of textlines:\n";
    map<std::string, vector<std::string> >::iterator itr;
    for (itr = dd.textLines.begin(); itr != dd.textLines.end(); itr++)
      for (j = 0; j < itr->second.size(); j++)
        out << "model " << itr->first << " line " << j << ": "
        << (itr->second)[j] << '\n';
  }
  out << "Puh! Finished printing of ptDiagramData\n";

  return out;
}

// Search in parameterlist for parameter matching id.
// index contains index to found parameter, or -1.
bool ptDiagramData::findParameter(const ParId& id, int& index, ErrorFlag* ef)
{
  index = -1;
  for (unsigned int i = 0; i < parList.size(); i++)
    if (id == parList[i].Id())
      index = i;
  *ef = index != -1 ? OK : DD_PARAMETER_NOT_FOUND;
  return *ef == OK ? true : false;
}

bool ptDiagramData::copyParameter(int index, WeatherParameter& wp,
    ErrorFlag* ef)
{
  if (index < 0 || index >= parList.size())
    *ef = DD_RANGE_ERROR;
  else {
    wp = parList[index];
    wp.setLocked(false);
    wp.clearTempDirty();
    *ef = OK;
  }
  return *ef == OK ? true : false;
}

bool ptDiagramData::copyParameter(const ParId& id, WeatherParameter& wp,
    ErrorFlag* ef)
{
  int index;

  if (findParameter(id, index, ef)) {
    wp = parList[index];
    wp.setLocked(false);
    wp.clearTempDirty();
    return true;
  } else
    return false;
}

// returns index of added parameter
int ptDiagramData::addParameter(const WeatherParameter& wp)
{
  parList.push_back(wp);
  return parList.size() - 1;
}

// remove weatherparameters matching ParId
void ptDiagramData::deleteParameter(const ParId& id)
{
  for (unsigned int i = 0; i < parList.size(); i++)
    if (id == parList[i].Id()) {
      vector<WeatherParameter>::iterator p = parList.begin() + i;
      parList.erase(p);
      i--;
    }
}

// remove weatherparameters not matching ParId
void ptDiagramData::deleteNegParameter(const ParId& id)
{
  for (unsigned int i = 0; i < parList.size(); i++)
    if (id != parList[i].Id()) {
      vector<WeatherParameter>::iterator p = parList.begin() + i;
      parList.erase(p);
      i--;
    }
}

// remove a weatherparameter; given index
void ptDiagramData::deleteParameter(const int index)
{
  if (index >= 0 && index < parList.size()) {
    vector<WeatherParameter>::iterator p = parList.begin() + index;
    parList.erase(p);
  }
}

// Clear the dirty flags of all weather parameters
void ptDiagramData::clearDirty()
{
  for (unsigned int i = 0; i < parList.size(); i++)
    parList[i].clearTempDirty();
}

void ptDiagramData::useAlternateSymbolmaker(const bool use)
{
  cerr << "useAlternateSymbolmaker called" << endl;
  new_symbolmaker = use;
}

float ptDiagramData::precip_state(float& tt, float& rr)
{
  float ps;
  if (tt < 0.0 && rr > 2)
    ps = 5;
  else if (tt < 0.0 && rr > 0.1)
    ps = 4;
  else if (rr >= 8)
    ps = 3;
  else if (rr >= 2)
    ps = 2;
  else if (rr >= 0.1)
    ps = 1;
  else
    ps = 0;

  return ps;
}

// update weatherparameter
void ptDiagramData::UpdateOneParameter(const ParId& inpid)
{

  const float ktms = float(1847.0 / 3600.0);
  int j, n;
  ErrorFlag error;
  ParId id1, id2, id3, id4;
  int wpx, wpx1, wpx2, wpx3, wpx4;
  float d, f1, f2, f3;
  int level;
  Uprofile profs;
  miSymbol tmpSymbol;
  vision sight;
  cloudGrp lucy;
  double reflevel = 10;
  bool locked;
  vector<float> fdata;

  id1 = id2 = id3 = inpid;

  //    cerr << "Update one parameter:" << inpid << endl;

  // Hmax
  if (inpid.alias == "HSX") {
    id1.alias = "HST";
    id2.alias = "HSXF";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      //       locked= parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        // 	if (!locked || parList[wpx1].isModified(j)
        // 	    || parList[wpx2].isModified(j))
        parList[wpx].setData(j, parList[wpx1].Data(j) * parList[wpx2].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // Hextreme
  } else if (inpid.alias == "HSEX") {
    id1.alias = "HST";
    id2.alias = "HEXF";
    UpdateOneParameter(id2); // update Hextreme factor
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      locked = parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++)
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(
            j))
          parList[wpx].setData(j, parList[wpx1].Data(j) * parList[wpx2].Data(j));
      parList[wpx].calcAllProperties();
    }

    // HMIX
  } else if (inpid.alias == "HMIX") {
    id1.alias = "HMIN";
    id2.alias = "HMAX";

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        parList[wpx].setData(j, 0, parList[wpx1].Data(j));
        parList[wpx].setData(j, 1, parList[wpx2].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // HST
  } else if (inpid.alias == "HST") {
    id1.alias = "HSD";
    id2.alias = "HSP";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      locked = parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(
            j)) {
          f1 = parList[wpx1].Data(j);
          f2 = parList[wpx2].Data(j);
          f3 = f1 * f1 + f2 * f2;
          parList[wpx].setData(j, sqrt(f3));
        }
      }
      parList[wpx].calcAllProperties();
    }

    // HSP
  } else if (inpid.alias == "HSP") {
    id1.alias = "HST";
    id2.alias = "HSD";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      locked = parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(
            j)) {
          f1 = parList[wpx1].Data(j);
          f2 = parList[wpx2].Data(j);
          f3 = f1 * f1 - f2 * f2;
          if (f3 < 0.0) {
            // HST less than HSD is clearly wrong..
            parList[wpx2].setData(j, f1);
            f3 = 0.0;
          }
          parList[wpx].setData(j, sqrt(f3));
        }
      }
      parList[wpx].calcAllProperties();
    }

    // TSD
  } else if (inpid.alias == "TSD") {
    id1.alias = "HSD";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      locked = parList[wpx].isLocked();
      if (parList[wpx].Npoints() == parList[wpx1].Npoints()) {
        for (j = 0; j < parList[wpx].Npoints(); j++) {
          if (!locked || parList[wpx1].isModified(j))
            if (parList[wpx1].Data(j) < 0.05)
              parList[wpx].setData(j, 0.0);
        }
        parList[wpx].calcAllProperties();
      }
    }

    // cartesian Hs and Hmax vector
  } else if (inpid.alias == "HSTX") {
    id1.alias = "HST";
    id2.alias = "HSX";
    UpdateOneParameter(id2); // update Hmax
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        parList[wpx].setData(j, 0, parList[wpx1].Data(j));
        parList[wpx].setData(j, 1, parList[wpx2].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // polar HSD and DDPD
  } else if (inpid.alias == "HSDD") {
    id1.alias = "HSD";
    id2.alias = "DDPD";
    UpdateOneParameter(id1); // update HSD
    UpdateOneParameter(id2); // update DDPD
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        parList[wpx].setData(j, 0, parList[wpx1].Data(j));
        parList[wpx].setData(j, 1, parList[wpx2].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // cartesian Hs and Hextreme vector
  } else if (inpid.alias == "HSTE") {
    id1.alias = "HST";
    id2.alias = "HSEX";
    UpdateOneParameter(id2); // update Hextreme
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        parList[wpx].setData(j, 0, parList[wpx1].Data(j));
        parList[wpx].setData(j, 1, parList[wpx2].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // HEC
  } else if (inpid.alias == "HEC") {
    id1.alias = "HST";
    id2.alias = "HS";

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {

      // check for identical timelines
      int n = parList[wpx].Npoints();
      if (n != parList[wpx1].Npoints() || n != parList[wpx2].Npoints()) {
        return;
      }

      locked = parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(
            j)) {
          float hec = calcHec_(parList[wpx1].Data(j), parList[wpx2].Data(j));
          parList[wpx].setData(j, hec);
        }
      }
      parList[wpx].calcAllProperties();
    }

    // TM01
  } else if (inpid.alias == "TM01") {
    id1.alias = "HST";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {

      // check for identical timelines
      int n = parList[wpx].Npoints();
      if (n != parList[wpx1].Npoints()) {
        return;
      }
      //cerr << "Updating " << inpid << endl;

      locked = parList[wpx].isLocked();
      for (j = 0; j < n; j++) {
        if (!locked || parList[wpx1].isModified(j)) {
          float tm01 = calcMedianTM01(parList[wpx1].Data(j));
          parList[wpx].setData(j, tm01);
        }
      }
      parList[wpx].calcAllProperties();
    }

    // EMC
  } else if (inpid.alias == "EMC") {
    id1.alias = "HST";
    id2.alias = "HS";
    id3.alias = "TM01";
    id4.alias = "TST";

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error) && findParameter(id3, wpx3, &error)
        && findParameter(id4, wpx4, &error)) {
      locked = parList[wpx].isLocked();

      // check for identical timelines
      int n = parList[wpx].TimeLineIndex();
      if (n != parList[wpx1].TimeLineIndex() || n
          != parList[wpx2].TimeLineIndex() || n
          != parList[wpx3].TimeLineIndex() || n
          != parList[wpx4].TimeLineIndex()) {
        return;
      }


      //cerr << "--- updateParameter:" << inpid << " TM01:" << wpx3 << " TM02:" << wpx4 << endl;
      //if (wpx3 >= 0) cerr << " size of emc:" << parList[wpx].Npoints() << " size of TM01:" << parList[wpx3].Npoints() << endl;
      //if (wpx4 >= 0) cerr << " size of emc:" << parList[wpx].Npoints() << " size of TM02:" << parList[wpx4].Npoints() << endl;


      for (j = 0; j < parList[wpx].Npoints(); j++) {
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(j) ||
            parList[wpx3].isModified(j) || parList[wpx4].isModified(j)) {
          float hst = parList[wpx1].Data(j);
          float hs = parList[wpx2].Data(j);
          float tm01 = parList[wpx3].Data(j);
          float tm02 = parList[wpx4].Data(j);
          float emc = calcCMC_(hst, hs, tm01, tm02);
          parList[wpx].setData(j, emc);
        }
      }
      parList[wpx].calcAllProperties();
    }

    // SHC
  } else if (inpid.alias == "SHC") {
    id1.alias = "DDPE"; // peak wave direction
    id1.level = L_UNDEF;
    int level = inpid.level;
    ParId pid = inpid;
    if (findParameter(id1, wpx1, &error)) {
      int l, l1 = level, l2 = level;
      // if level==L_UNDEF, update for all defined levels
      if (level == L_UNDEF) {
        l1 = shcinfo.lowLevel();
        l2 = shcinfo.highLevel();
      }
      //cerr << "About to make SHC for levels:" << l1 << " to " << l2 << endl;
      for (l = l1; l <= l2; l++) {
        pid.level = l;
        if (findParameter(pid, wpx, &error)) {
          if (makeSHC_(wpx1, parList[wpx], l)) {
            parList[wpx].calcAllProperties();
          }
        }
      }
    }

    // SHL
  } else if (inpid.alias == "SHL") {
    id1.alias = "HEC";
    id2.alias = "SHC";

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++){
        parList[wpx].setData(j, 0.0);
      }

      int l1 = shcinfo.lowLevel();
      int l2 = shcinfo.highLevel();
      for (int k = l1; k <= l2; k++) {
        id2.level = k;
        if (findParameter(id2, wpx2, &error)) {
          for (j = 0; j < parList[wpx].Npoints(); j++) {
            // 	    if (!locked || parList[wpx1].isModified(j)
            // 		|| parList[wpx2].isModified(j)){
            if (parList[wpx1].Data(j) > parList[wpx2].Data(j))
              parList[wpx].setData(j, k);
            // 	  }
          }
        }
      }
      parList[wpx].calcAllProperties();
    }

    // polar wind-vector
  } else if (inpid.alias == "WVFD") {
    if ((inpid.level != 0) && (inpid.level != L_UNDEF)) {
      level = inpid.level;
      //       cerr << "Updating wind at level:"<<level<<endl;
      id1.level = 0;

      if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
        locked = parList[wpx].isLocked();
        double speed10;
        double Ri = 0;
        double dlevel = static_cast<double> (level);
        float speed;

        for (j = 0; j < parList[wpx].Npoints(); j++) {
          if (!locked || parList[wpx1].isModified(j)) {
            speed10 = static_cast<double> (parList[wpx1].Data(j, 0) * ktms);
            profs.editRi(Ri);
            speed = profs.compute(reflevel, speed10, dlevel);
            parList[wpx].setData(j, 0, speed / ktms);
            parList[wpx].setData(j, 1, parList[wpx1].Data(j, 1));
          }
        }
        parList[wpx].calcAllProperties();
      }
    } else {
      id1.alias = "WVMD";

      if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
        for (j = 0; j < parList[wpx].Npoints(); j++) {
          d = parList[wpx1].Data(j, 0);
          parList[wpx].setData(j, 0, d / ktms);
          d = parList[wpx1].Data(j, 1);
          parList[wpx].setData(j, 1, d);
        }
        parList[wpx].calcAllProperties();
      }
    }

    // polar wind-vector in m/s
  } else if (inpid.alias == "WVMD") {
    id1.alias = "FF";
    id2.alias = "DD";

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        d = parList[wpx1].Data(j, 0);
        parList[wpx].setData(j, 0, d);
        d = parList[wpx2].Data(j, 0);
        parList[wpx].setData(j, 1, d);
      }
      parList[wpx].calcAllProperties();
    }

  } else if (inpid.alias == "FF" && inpid.level != L_UNDEF && inpid.level != 0) {
    level = inpid.level;
    //cerr << "tsData:Updating FF at level:" << level << endl;
    id1.level = 0;
    id2.alias = "STAQ";
    id2.level = 0;
    if (level == 0)
      level = 10;

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        float speed10 = parList[wpx1].Data(j, 0);
        float st = parList[wpx2].Data(j, 0);
        float speed = windCalc.meanWind(speed10, static_cast<int> (st), level);
        parList[wpx].setData(j, 0, speed);
      }
      parList[wpx].calcAllProperties();
    }

    //     if (findParameter(inpid,wpx,&error) &&
    // 	findParameter(id1,wpx1,&error)){
    //       locked= parList[wpx].isLocked();
    //       double speed10;
    //       double Ri = 0;
    //       double dlevel= static_cast<double>(level);
    //       float speed;

    //       for (j=0; j<parList[wpx].Npoints(); j++){
    // 	speed10 = static_cast<double>(parList[wpx1].Data(j,0));
    // 	profs.editRi(Ri);
    // 	speed = profs.compute(reflevel, speed10, dlevel);
    // 	parList[wpx].setData(j,0,speed);
    //       }
    //       parList[wpx].calcAllProperties();
    //     }


    // Wind speed min and max (m/s)
  } else if (inpid.alias == "FFMIX") {
    id1.alias = "FFMI";
    id2.alias = "FFMA";

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        parList[wpx].setData(j, 0, parList[wpx1].Data(j));
        parList[wpx].setData(j, 1, parList[wpx2].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // Gust
  } else if (inpid.alias == "GU") {
    id1.alias = "FF";
    id1.level = 0;
    id2.alias = "STAQ";
    id2.level = 0;
    level = inpid.level;
    //cerr << "tsData: Updating gust at level:" << level << endl;
    if (level == 0)
      level = 10;

    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        float speed10 = parList[wpx1].Data(j, 0);
        float st = parList[wpx2].Data(j, 0);
        float gust = windCalc.gust(speed10, static_cast<int> (st), level);
        parList[wpx].setData(j, 0, gust);
      }
      parList[wpx].calcAllProperties();
    }

    //     // Gust
    //   } else if (inpid.alias == "GU"){
    //     id1.alias = "FF";
    //     level= inpid.level;
    //     cerr << "tsData: Updating gust at level:"<<level<<endl;
    //     if (level==0) level = 10;
    //     float gust;

    //     if (findParameter(inpid,wpx,&error) &&
    // 	findParameter(id1,wpx1,&error)){
    //       locked= parList[wpx].isLocked();
    //       for (j=0; j<parList[wpx].Npoints(); j++){
    // 	d = parList[wpx1].Data(j,0);
    // 	gust  = profs.gust(level, d);
    // 	parList[wpx].setData(j,0,gust);
    //       }
    //       parList[wpx].calcAllProperties();
    //     }

    // discrete visibility
  } else if (inpid.alias == "VVQ") {
    id1.alias = "CWW";
    id2.alias = "XWW";
    int visi;
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      locked = parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(
            j)) {
          f1 = parList[wpx1].Data(j);
          f2 = parList[wpx2].Data(j);
          tmpSymbol = wsymbols.getSymbol(static_cast<int> (f1));
          visi = sight.findVision(tmpSymbol.vis(), static_cast<int> (f2));
          parList[wpx].setData(j, 0, visi);
        }
      }
      parList[wpx].calcAllProperties();
    }

    // visibility
  } else if (inpid.alias == "VIS") {
    id1.alias = "CWW";
    //id2.alias = "XWW";
    //findParameter(id2, wpx2, &error);
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        //f2 = ( wpx2 != -1 ? parList[wpx2].Data(j) : 0.0 );
        int ws = static_cast<int> (f1);
        //int xws = static_cast<int> (f2);
        float visi = 15.0;
        /*
        if (xws == 2) // Disig
          visi = 3.0;
        else if (xws == 1) // T�kebanker
          visi = 2.0;
        else if (xws == -1) // T�ke
          visi = 1.0;
        else
         */
        if (ws == 1)
          visi = 15.0; // Sun
        else if (ws == 2)
          visi = 15.0; // Lightcloudy
        else if (ws == 3)
          visi = 15.0; // Partlycloudy
        else if (ws == 4)
          visi = 15.0; // Cloudy
        else if (ws == 5)
          visi = 7.0; // Rainshowers
        else if (ws == 6)
          visi = 7.0; // Rainshowers w/ thunder
        else if (ws == 7)
          visi = 2.5; // Sleetshowers
        else if (ws == 8)
          visi = 1.0; // Snowshowers
        else if (ws == 9)
          visi = 6.0; // Lightrain
        else if (ws == 10)
          visi = 4.0; // Rain
        else if (ws == 11)
          visi = 4.0; // rain w/ thunder
        else if (ws == 12)
          visi = 3.0; // Sleet
        else if (ws == 13)
          visi = 2.0; // Snow
        else if (ws == 14)
          visi = 2.0; // Snow w/ thunder
        else if (ws == 15)
          visi = 0.8; // Fog
        parList[wpx].setData(j, 0, visi);
      }
      parList[wpx].calcAllProperties();
    }

    // cloudbase
  } else if (inpid.alias == "CB") {
    id1.alias = "CWW";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        int ws = static_cast<int> (f1);
        float cb = 3000;
        if (ws == 1)
          cb = 5000; // 10// Sun
        else if (ws == 2)
          cb = 3500; // 10// Lightcloudy
        else if (ws == 3)
          cb = 3500; // 10// Partlycloudy
        else if (ws == 4)
          cb = 3000; // 9 // Cloudy
        else if (ws == 5)
          cb = 1500; // 8 // Rainshowers
        else if (ws == 6)
          cb = 1500; // 8 // Rainshowers w/ thunder
        else if (ws == 7)
          cb = 800; // 6 // Sleetshowers
        else if (ws == 8)
          cb = 500; // 4 // Snowshowers
        else if (ws == 9)
          cb = 1500; // 7 // Lightrain
        else if (ws == 10)
          cb = 1000; // 7 // Rain
        else if (ws == 11)
          cb = 1000; // 7 // rain w/ thunder
        else if (ws == 12)
          cb = 1000; // 5 // Sleet
        else if (ws == 13)
          cb = 800; // 3 // Snow
        else if (ws == 14)
          cb = 800; // 3 // Snow w/ thunder
        else if (ws == 15)
          cb = 300; // 1 // Fog
        //tmpSymbol = wsymbols.getSymbol(static_cast<int>(f1));
        //visi= sight.findVision(tmpSymbol.vis(), static_cast<int>(f2));
        parList[wpx].setData(j, 0, cb);
      }
      parList[wpx].calcAllProperties();
    }

    // discrete cloudcover
  } else if (inpid.alias == "CCQ") {
    id1.alias = "WW";
    id2.alias = "XWW";
    int clgrp;
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      locked = parList[wpx].isLocked();
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        if (!locked || parList[wpx1].isModified(j) || parList[wpx2].isModified(
            j)) {
          f1 = parList[wpx1].Data(j);
          f2 = parList[wpx2].Data(j);
          symbolMaker::Symboltype tmpSymbol = wsymbols.getSymboltype(static_cast<int> (f1));
          clgrp = lucy.findCl(tmpSymbol, static_cast<int> (f2));
          parList[wpx].setData(j, 0, clgrp);
        }
      }
      parList[wpx].calcAllProperties();
    }

    // cloudcover % from cloudcover 1/8
  } else if (inpid.alias == "CC") {
    id1.alias = "CC8";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, f1 * 100.0 / 8.0);
      }
      parList[wpx].calcAllProperties();
    } else {
      // from CL,CM and CH
      id1.alias = "CL";
      id2.alias = "CM";
      id3.alias = "CH";
      if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
          && findParameter(id2, wpx2, &error) && findParameter(id3, wpx3,
              &error)) {
        for (j = 0; j < parList[wpx].Npoints(); j++) {
          f1 = parList[wpx1].Data(j);
          f2 = parList[wpx2].Data(j);
          f3 = parList[wpx3].Data(j);
          float cc = f1;
          if (f2 > cc)
            cc = f2;
          if (f3 > cc)
            cc = f3;
          parList[wpx].setData(j, 0, cc);
        }
        parList[wpx].calcAllProperties();
      }
    }

    // low cloudcover % from low cloudcover 1/8
  } else if (inpid.alias == "CL") {
    id1.alias = "CL8";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, f1 * 100.0 / 8.0);
      }
      parList[wpx].calcAllProperties();
    }

    // medium cloudcover % from medium cloudcover 1/8
  } else if (inpid.alias == "CM") {
    id1.alias = "CM8";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, f1 * 100.0 / 8.0);
      }
      parList[wpx].calcAllProperties();
    }

    // high cloudcover % from high cloudcover 1/8
  } else if (inpid.alias == "CH") {
    id1.alias = "CH8";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, f1 * 100.0 / 8.0);
      }
      parList[wpx].calcAllProperties();
    }

    // cloudcover 1/8 from cloudcover %
  } else if (inpid.alias == "CC8") {
    id1.alias = "CC";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      parList[wpx].calcAllProperties();
    }

    // low cloudcover 1/8 from low cloudcover %
  } else if (inpid.alias == "CL8") {
    id1.alias = "CL";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      parList[wpx].calcAllProperties();
    }

    // medium cloudcover 1/8 from medium cloudcover %
  } else if (inpid.alias == "CM8") {
    id1.alias = "CM";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      parList[wpx].calcAllProperties();
    }

    // high cloudcover 1/8 from high cloudcover %
  } else if (inpid.alias == "CH8") {
    id1.alias = "CH";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      for (j = 0; j < parList[wpx].Npoints(); j++) {
        f1 = parList[wpx1].Data(j);
        parList[wpx].setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      parList[wpx].calcAllProperties();
    }

    // Water state
  } else if (inpid.alias == "AGR") {
    id1.alias = "TT";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      fdata.clear();
      n = parList[wpx1].Npoints();
      int n2 = parList[wpx].Npoints();
      if (n == n2) {
        for (j = 0; j < n; j++)
          fdata.push_back(parList[wpx1].Data(j));
        vector<float> sdata = wsymbols.water_state(fdata);

        for (j = 0; j < n; j++)
          parList[wpx].setData(j, 0, sdata[j]);
      }
      parList[wpx].calcAllProperties();
    }

    // Precipitation state state
  } else if (inpid.alias == "PS") {
    id1.alias = "TT";
    id2.alias = "RR";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error)) {
      n = parList[wpx].Npoints();
      int n2 = parList[wpx1].Npoints();
      int n3 = parList[wpx2].Npoints();
      if (n == n2 && n == n3) {
        for (j = 0; j < n; j++) {
          float tt = parList[wpx1].Data(j);
          float rr = parList[wpx2].Data(j);
          parList[wpx].setData(j, 0, precip_state(tt, rr));
        }
      }
      parList[wpx].calcAllProperties();
    }

    // Ensure Dewpoint temp sanity
  } else if (inpid.alias == "TD") {
    id1.alias = "TT";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)) {
      n = parList[wpx1].Npoints();
      int n2 = parList[wpx].Npoints();
      if (n == n2) {
        for (j = 0; j < n; j++)
          if (parList[wpx1].Data(j) < parList[wpx].Data(j))
            parList[wpx].setData(j, 0, parList[wpx1].Data(j));
      }
      parList[wpx].calcAllProperties();
    }

    // Weather symbol
  } else if (inpid.alias == "WW") {
    if (new_symbolmaker)
      makeWeatherSymbols_new_(inpid, true);

    // Weather symbol
  } else if (inpid.alias == "CWW") {
    if (new_symbolmaker)
      makeWeatherSymbols_new_(inpid, true);

    // Ice Accretion
  } else if (inpid.alias == "ICEA") {
    id1.alias = "TT";
    id2.alias = "SST";
    id3.alias = "FF";
    if (findParameter(inpid, wpx, &error) && findParameter(id1, wpx1, &error)
        && findParameter(id2, wpx2, &error) && findParameter(id3, wpx3, &error)) {
      n = parList[wpx].Npoints();
      int n1 = parList[wpx1].Npoints();
      int n2 = parList[wpx2].Npoints();
      int n3 = parList[wpx3].Npoints();
      if (n == n1 && n == n2 && n == n3) {
        for (j = 0; j < n; j++) {
          double t = parList[wpx1].Data(j);
          double sst = parList[wpx2].Data(j);
          double ff = parList[wpx3].Data(j);
          float icea = iceAccretion(t, sst, ff);
          parList[wpx].setData(j, icea);
        }
      }
      parList[wpx].calcAllProperties();
    }

  } else {
    //     cerr << "updateOneParameter: Unknown parameter:"<<inpid<<endl;
  }
}

// make weatherparameter from existing wp's
void ptDiagramData::makeOneParameter(const ParId& inpid)
{

  int j, idx, idx2;
  ErrorFlag error;
  ParId id1, id2, id3, id4, newpid;
  WeatherParameter wp, wp2, wp3, wp4;
  vector<float> fdata;
  bool wpok = false;
  int level;
  double reflevel = 10;
  Uprofile profs;
  symbolMaker::Symboltype tmpSymbol;
  vision sight;
  cloudGrp lucy;
  ptStatistics statist;
  float ktms = float(1847.0 / 3600.0);
  float f1, f2;

  id1 = id2 = id3 = newpid = inpid;

  //cerr << "Make one parameter:" << inpid << endl;

  // polar wind-vector
  if (inpid.alias == "WVFD") {
    if ((inpid.level == L_UNDEF) || (inpid.level == 0)) {

      //       cerr << "creating WVFD from FF and DD" << endl;
      id1.alias = "FF";
      id2.alias = "DD";
      if (!makeVector(id1, id2, inpid, true)) {
        // 	cerr << " creating WVFD from UU and VV" << endl;
        id1.alias = "UU";
        id2.alias = "VV";
        makePolarVector(id1, id2, inpid, M_PI,-1); //OBS: zangle er ikke i bruk!
      }

    } else {
      level = inpid.level;
      //       cerr << "Asking for wind at level:"<<level<<endl;
      id1.level = 0;
      if (!findParameter(id1, idx, &error))
        makeOneParameter(id1);

      if (copyParameter(id1, wp, &error)) {
        double speed10;
        double Ri = 0;
        double dlevel = static_cast<double> (level);
        float speed;
        for (j = 0; j < wp.Npoints(); j++) {
          speed10 = static_cast<double> (wp.Data(j, 0) * ktms);
          //  	  profs.setRi(Ri, reflevel, speed10);
          profs.editRi(Ri);
          speed = profs.compute(reflevel, speed10, dlevel);
          wp.setData(j, 0, speed / ktms);
        }
        if (newpid.run == R_UNDEF)
          newpid.run = wp.Id().run;
        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }

    // polar wind-vector in m/s
  } else if (inpid.alias == "WVMD") {
    if ((inpid.level == L_UNDEF) || (inpid.level == 0)) {

      //       cerr << "creating WVMD from FF and DD" << endl;
      id1.alias = "FF";
      id2.alias = "DD";
      if (!makeVector(id1, id2, inpid, true)) {
        // 	cerr << " creating WVMD from UU and VV" << endl;
        id1.alias = "UU";
        id2.alias = "VV";
        makePolarVector(id1, id2, inpid, M_PI,-1); //OBS: zangle er ikke i bruk!
      }

    } else {
      level = inpid.level;
      //       cerr << "Asking for wind at level:"<<level<<endl;
      id1.level = 0;
      if (!findParameter(id1, idx, &error))
        makeOneParameter(id1);

      if (copyParameter(id1, wp, &error)) {
        double speed10;
        double Ri = 0;
        double dlevel = static_cast<double> (level);
        float speed;
        for (j = 0; j < wp.Npoints(); j++) {
          speed10 = static_cast<double> (wp.Data(j, 0));
          //  	  profs.setRi(Ri, reflevel, speed10);
          profs.editRi(Ri);
          speed = profs.compute(reflevel, speed10, dlevel);
          wp.setData(j, 0, speed);
        }
        if (newpid.run == R_UNDEF)
          newpid.run = wp.Id().run;
        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }

    // FF in different level
  } else if (inpid.alias == "FF" && inpid.level != L_UNDEF && inpid.level != 0) {
    level = inpid.level;
    //cerr << "Asking for FF at level:" << level << endl;
    id1.level = 0;
    id2.alias = "STAQ";
    id2.level = 0;

    //     if (!findParameter(id2,wpx2,&error)){
    //     }

    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        float speed10 = wp.Data(j, 0);
        float st = wp2.Data(j, 0);
        float speed = windCalc.meanWind(speed10, static_cast<int> (st), level);
        wp.setData(j, 0, speed);
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // Gust
  } else if (inpid.alias == "GU") {
    level = inpid.level;
    //cerr << "Asking for gust at level:" << level << endl;
    if (level == 0)
      level = 10;
    id1.alias = "FF";
    id1.level = 0;
    id2.alias = "STAQ";
    id2.level = 0;
    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        float speed10 = wp.Data(j, 0);
        float st = wp2.Data(j, 0);
        float gust = windCalc.gust(speed10, static_cast<int> (st), level);
        wp.setData(j, 0, gust);
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // cartesian FFmin and FFmax vector
  } else if (inpid.alias == "FFMIX") {
    id1.alias = "FFMI";
    id2.alias = "FFMA";

    makeVector(id1, id2, inpid, false);

    // cartesian current vector
  } else if (inpid.alias == "CUV") {
    id1.alias = "CU";
    id2.alias = "CV";
    if (findParameter(id1, idx, &error) && findParameter(id2, idx2, &error)) {

      makeVector(id1, id2, inpid, false);

    } else {
      id1.alias = "CFF";
      id2.alias = "CDD";
      if (findParameter(id1, idx, &error) && findParameter(id2, idx2, &error)) {

        makeCartesianVector(id1, id2, inpid, M_PI_2,-1);
      }
    }

    // Hmax factor
  } else if (inpid.alias == "HSXF") {
    id1.alias = "HST";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++)
        wp.setData(j, 1.65);

      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // Hextreme factor
  } else if (inpid.alias == "HEXF") {
    id1.alias = "HST";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++)
        if (wp.Data(j) > 6)
          wp.setData(j, 2.0);
        else
          wp.setData(j, 0.0);

      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // Hmax
  } else if (inpid.alias == "HSX") {
    id1.alias = "HST";
    id2.alias = "HSXF";
    if (copyParameter(id1, wp, &error)) {
      if (!findParameter(id2, idx, &error))
        makeOneParameter(id2);

      if (copyParameter(id2, wp2, &error)) {
        for (j = 0; j < wp.Npoints(); j++)
          wp.setData(j, wp.Data(j) * wp2.Data(j));

        if (newpid.run == R_UNDEF)
          newpid.run = wp.Id().run;
        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }

    // Hextreme
  } else if (inpid.alias == "HSEX") {
    id1.alias = "HST";
    id2.alias = "HEXF";
    if (copyParameter(id1, wp, &error)) {
      if (!findParameter(id2, idx, &error))
        makeOneParameter(id2);

      if (copyParameter(id2, wp2, &error)) {
        for (j = 0; j < wp.Npoints(); j++)
          wp.setData(j, wp.Data(j) * wp2.Data(j));

        if (newpid.run == R_UNDEF)
          newpid.run = wp.Id().run;
        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }

    /*
    // HEC
  } else if (inpid.alias == "HEC") {
    id1.alias = "HST";
    id2.alias = "HS";
    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
      cerr << "--- makeParameter:" << inpid << endl;
      // check for identical timelines
      if (wp.TimeLineIndex() != wp2.TimeLineIndex()) {
        return;
      }

      for (j = 0; j < wp.Npoints(); j++) {
        float hec = calcHec_(wp.Data(j), wp2.Data(j));
        wp.setData(j, hec);
      }

      wp.calcAllProperties();
      wp.setId(inpid);
      addParameter(wp);
    }
     */

    /*
    // EMC
  } else if (inpid.alias == "EMC") {
    id1.alias = "HST";
    id2.alias = "HS";
    id3.alias = "TM01";
    id4.alias = "TST";

    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {

      bool btm01 = copyParameter(id3, wp3, &error);
      bool btm02 = copyParameter(id4, wp4, &error);

      int n = wp.Npoints();
      if (n != wp2.Npoints() || (btm01 && n != wp3.Npoints()) || (btm02 && n
          != wp4.Npoints())) {
        return;
      }

      cerr << "--- makeParameter:" << inpid << " TM01:" << btm01 << " TM02:" << btm02 << endl;
      cerr << " size of emc:" << wp.Npoints() << " size of HS:" << wp2.Npoints() << endl;
      if (btm01) cerr << " size of emc:" << wp.Npoints() << " size of TM01:" << wp3.Npoints() << endl;
      if (btm02) cerr << " size of emc:" << wp.Npoints() << " size of TM02:" << wp4.Npoints() << endl;

      for (j = 0; j < wp.Npoints(); j++) {
        float hst = wp.Data(j);
        float hs = wp2.Data(j);
        float tm01 = (btm01 ? wp3.Data(j) : -1);
        float tm02 = (btm02 ? wp4.Data(j) : -1);
        float emc = calcCMC_(hst, hs, tm01, tm02);
        wp.setData(j, emc);
      }

      wp.calcAllProperties();
      wp.setId(inpid);
      addParameter(wp);
    }
     */

    // TM01
  } else if (inpid.alias == "TM01") {
    //cerr << "Making " << inpid << endl;
    id1.alias = "HST";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++){
        wp.setData(j, calcMedianTM01(wp.Data(j)));
      }
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // RR
  } else if (inpid.alias == "RR"  ) {
    //cerr << "Asked for RR" << endl;
    ParId pid = inpid;
    id1 = id2 = id3 = id4 = inpid;
    bool fixed = true;
    id1.alias = "RR1"; // total convective
    id2.alias = "RR2"; // total stratiform

    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
      //cerr << "Found RR1 + RR2" << endl;
      for (j = 0; j < wp.Npoints(); j++) {
        float rr = wp.Data(j, 0) + wp2.Data(j, 0);
        wp.setData(j, 0, rr);
      }

    } else {
      id1.alias = "SNPR"; // total snow
      id2.alias = "RAPR"; // total rain
      if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
        //cerr << "Found SNPR + RAPR" << endl;
        for (j = 0; j < wp.Npoints(); j++) {
          float rr = wp.Data(j, 0) + wp2.Data(j, 0);
          wp.setData(j, 0, rr);
        }

      } else {
        id1.alias = "COSN"; // convective snow
        id2.alias = "CORA"; // convective rain
        id3.alias = "STSN"; // stratiform snow
        id4.alias = "STRA"; // stratiform rain
        if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)
            && copyParameter(id3, wp3, &error) && copyParameter(id4, wp4,
                &error)) {
          //cerr << "Found COSN + CORA + STSN + STRA" << endl;
          for (j = 0; j < wp.Npoints(); j++) {
            float rr = wp.Data(j, 0) + wp2.Data(j, 0) + wp3.Data(j, 0)
                                                                + wp4.Data(j, 0);
            wp.setData(j, 0, rr);
          }

        } else if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2,
            &error)) {
          //cerr << "Found COSN + CORA" << endl;
          for (j = 0; j < wp.Npoints(); j++) {
            float rr = wp.Data(j, 0) + wp2.Data(j, 0);
            wp.setData(j, 0, rr);
          }
        } else if (copyParameter(id3, wp, &error) && copyParameter(id4, wp2,
            &error)) {
          //cerr << "Found STSN + STRA" << endl;
          for (j = 0; j < wp.Npoints(); j++) {
            float rr = wp.Data(j, 0) + wp2.Data(j, 0);
            wp.setData(j, 0, rr);
          }
        } else {

          id1.alias = "RRAC"; // make RR from accumulated precipitation
          if (copyParameter(id1, wp, &error) ) {
            int last= wp.Npoints();
            if (last>0) {
              for (j = last; j > 0 ; j--) {
                float rr = wp.Data(j, 0) - wp.Data(j-1, 0);
                wp.setData(j,0,rr);
              }
              fixed=true;
            } else
              fixed=false;
          } else{
            fixed = false; // failed
          }
        }
      }
      if (fixed) {
        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }

    // EPRR (from RRAC for fimex interpolation )
  } else if (inpid.alias == "EPRR"  ) {
    ParId pid = inpid;
    bool fixed = true;

    pid.alias = "RRAC"; // make RR from accumulated precipitation
    if (copyParameter(pid, wp, &error) ) {
      int numTimes= wp.Npoints();
      int numPardims = wp.Ndim();

      std::vector<float> tmp_data =  wp.copyDataVector();

      if(numPardims && numTimes) {


        for (unsigned int tim=0; tim < numTimes ; tim++) {
          for (unsigned int pardim=0; pardim < numPardims ; pardim++) {
            unsigned int index = pardim * numTimes  + tim;

            float rrac = tmp_data[index];
            if (!tim) {
              wp.setData(tim,pardim,rrac);
            } else {
              float rr = rrac - tmp_data[index-1];
              wp.setData(tim,pardim,rr);
            }
          }
        }

        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }
  }else if (inpid.alias == "SHC") {
    //cerr << "Asked for creation of  :" << inpid << endl;
    id1.alias = "DDPE"; // peak wave direction
    id1.level = L_UNDEF;
    level = inpid.level;
    ParId pid = inpid;
    if (findParameter(id1, idx, &error)) {
      if (copyParameter(id1, wp, &error)) {
        int l, l1 = level, l2 = level;
        // if level==L_UNDEF, make for all defined levels
        if (level == L_UNDEF) {
          l1 = shcinfo.lowLevel();
          l2 = shcinfo.highLevel();
        }
        //cerr << "Making for levels:" << l1 << " to " << l2 << endl;
        for (l = l1; l <= l2; l++) {
          if (makeSHC_(idx, wp, l)) {
            wp.calcAllProperties();
            pid.level = l;
            wp.setId(pid);
            addParameter(wp);
          }
        }
      }
    }






    // SHL
  } else if (inpid.alias == "SHL") {
    id1.alias = "HEC";
    id2.alias = "SHC";

    if (!findParameter(id1, idx, &error))
      makeOneParameter(id1);

    if (copyParameter(id1, wp, &error)) {
      copyParameter(id1, wp3, &error);
      for (j = 0; j < wp.Npoints(); j++)
        wp.setData(j, 0.0);

      int l1 = shcinfo.lowLevel();
      int l2 = shcinfo.highLevel();
      for (int k = l1; k <= l2; k++) {
        id2.level = k;
        if (!findParameter(id2, idx, &error))
          makeOneParameter(id2);

        if (copyParameter(id2, wp2, &error)) {
          for (j = 0; j < wp.Npoints(); j++) {
            if (wp3.Data(j) > wp2.Data(j))
              wp.setData(j, k);
          }
        }
      }
      wp.calcAllProperties();
      wp.setId(inpid);
      addParameter(wp);
    }

    // cartesian Hs and Hmax vector
  } else if (inpid.alias == "HSTX") {
    id1.alias = "HST";
    id2.alias = "HSX";
    if (!findParameter(id2, idx, &error))
      makeOneParameter(id2);

    makeVector(id1, id2, inpid, false);

    // cartesian Hs and Hextreme vector
  } else if (inpid.alias == "HSTE") {
    id1.alias = "HST";
    id2.alias = "HSEX";
    if (!findParameter(id2, idx, &error))
      makeOneParameter(id2);

    makeVector(id1, id2, inpid, false);

    // cartesian Hmin and Hmax vector
  } else if (inpid.alias == "HMIX") {
    id1.alias = "HMIN";
    id2.alias = "HMAX";

    makeVector(id1, id2, inpid, false);

    // polar HSD and DDPD vector
  } else if (inpid.alias == "HSDD") {
    id1.alias = "HSD";
    id2.alias = "DDPD";

    makeVector(id1, id2, inpid, false);

    // polar current vector
  } else if (inpid.alias == "CFD") {
    id1.alias = "CU";
    id2.alias = "CV";
    if (findParameter(id1, idx, &error) && findParameter(id2, idx2, &error)) {

      makePolarVector(id1, id2, inpid, M_PI,-1);

    } else {
      id1.alias = "CFF";
      id2.alias = "CDD";
      if (findParameter(id1, idx, &error) && findParameter(id2, idx2, &error)) {

        makeVector(id1, id2, inpid, true);
      }
    }

    // stability
  } else if (inpid.alias == "STA") {
    id1.alias = "FF";
    if (!findParameter(id1, idx, &error))
      makeOneParameter(id1);

    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++)
        wp.setData(j, 0);
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // discrete stability
  } else if (inpid.alias == "STAQ") {
    id1.alias = "STA";
    if (!findParameter(id1, idx, &error))
      makeOneParameter(id1);

    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++)
        wp.setData(j, 0);
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // visibility
  } else if (inpid.alias == "VVC") {
    id1.alias = "CL";
    id2.alias = "CC";
    if (!(wpok = copyParameter(id1, wp, &error)))
      wpok = copyParameter(id2, wp, &error);
    if (wpok) {
      for (j = 0; j < wp.Npoints(); j++)
        wp.setData(j, (10000 - wp.Data(j) * 100) / 1000.0);
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // TT EPS mean
  } else if (inpid.alias == "EPTM") {
    id1.alias = "EPTT";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      wp2.setDims(wp.Npoints(), 1);
      int i, n;
      fdata = statist.getMean(wp.copyDataVector(), wp.Ndim());
      n = fdata.size();
      if (n) {
        for (i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // TT EPS median
  } else if (inpid.alias == "EPTE") {
    id1.alias = "EPTT";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      fdata = statist.getMedian(wp.copyDataVector(), wp.Ndim());
      int n = fdata.size();
      if (n) {
        wp2.setDims(n, 1);
        for (int i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // TT EPS deciler (1..9)
  } else if (inpid.alias == "EPTD") {
    id1.alias = "EPTT";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 1);
      vector<float> dezils(9);
      for (i = 0; i < 9; i++)
        dezils[i] = i + 1;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 9 * m) {
        for (i = 0; i < 9; i++) {
          for (j = 0; j < m; j++)
            wp2.setData(j, 0, fdata[i * m + j]);
          wp2.calcAllProperties();
          outpid.level = static_cast<int> (dezils[i] * 10);
          wp2.setId(outpid);
          addParameter(wp2);
        }
      }
    }

    // TT EPS deciler (1..9) (total)
  } else if (inpid.alias == "EPTDT") {
    id1.alias = "EPTT";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 5);
      vector<float> dezils(5);
      dezils[0] = 0;
      dezils[1] = 1;
      dezils[2] = 5;
      dezils[3] = 9;
      dezils[4] = 10;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 5 * m) {
        for (i = 0; i < 5; i++)
          for (j = 0; j < m; j++)
            wp2.setData(j, i, fdata[i * m + j]);
        wp2.calcAllProperties();
        wp2.setId(outpid);
        addParameter(wp2);
      }
    }

    // RR EPS mean
  } else if (inpid.alias == "EPRM") {
    id1.alias = "EPRR";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      wp2.setDims(wp.Npoints(), 1);
      int i, n;
      fdata = statist.getMean(wp.copyDataVector(), wp.Ndim());
      n = fdata.size();
      if (n) {
        for (i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // RR EPS median
  } else if (inpid.alias == "EPRE") {
    id1.alias = "EPRR";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      fdata = statist.getMedian(wp.copyDataVector(), wp.Ndim());
      int n = fdata.size();
      if (n) {
        wp2.setDims(n, 1);
        for (int i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // RR EPS deciler (1..9)
  } else if (inpid.alias == "EPRD") {
    id1.alias = "EPRR";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 1);
      vector<float> dezils(9);
      for (i = 0; i < 9; i++)
        dezils[i] = i + 1;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 9 * m) {
        for (i = 0; i < 9; i++) {
          for (j = 0; j < m; j++)
            wp2.setData(j, 0, fdata[i * m + j]);
          wp2.calcAllProperties();
          outpid.level = static_cast<int> (dezils[i] * 10);
          wp2.setId(outpid);
          addParameter(wp2);
        }
      }
    }

    // RR EPS deciler (0,1,5,9,10)
  } else if (inpid.alias == "EPRDT") {
    id1.alias = "EPRR";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 5);
      vector<float> dezils(5);
      dezils[0] = 0;
      dezils[1] = 1;
      dezils[2] = 5;
      dezils[3] = 9;
      dezils[4] = 10;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 5 * m) {
        for (i = 0; i < 5; i++)
          for (j = 0; j < m; j++)
            wp2.setData(j, i, fdata[i * m + j]);
        wp2.calcAllProperties();
        wp2.setId(outpid);
        addParameter(wp2);
      }
    }

    // Z EPS mean
  } else if (inpid.alias == "EPZM") {
    id1.alias = "EPZ";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      wp2.setDims(wp.Npoints(), 1);
      int i, n;
      fdata = statist.getMean(wp.copyDataVector(), wp.Ndim());
      n = fdata.size();
      if (n) {
        for (i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // Z EPS median
  } else if (inpid.alias == "EPZE") {
    id1.alias = "EPZ";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      fdata = statist.getMedian(wp.copyDataVector(), wp.Ndim());
      int n = fdata.size();
      if (n) {
        wp2.setDims(n, 1);
        for (int i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // Z EPS deciler (1..9)
  } else if (inpid.alias == "EPZD") {
    id1.alias = "EPZ";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 1);
      vector<float> dezils(9);
      for (i = 0; i < 9; i++)
        dezils[i] = i + 1;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 9 * m) {
        for (i = 0; i < 9; i++) {
          for (j = 0; j < m; j++)
            wp2.setData(j, 0, fdata[i * m + j]);
          wp2.calcAllProperties();
          outpid.level = static_cast<int> (dezils[i] * 10);
          wp2.setId(outpid);
          addParameter(wp2);
        }
      }
    }

    // Z EPS deciler (0,1,5,9,10)
  } else if (inpid.alias == "EPZDT") {
    id1.alias = "EPZ";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 5);
      vector<float> dezils(5);
      dezils[0] = 0;
      dezils[1] = 1;
      dezils[2] = 5;
      dezils[3] = 9;
      dezils[4] = 10;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 5 * m) {
        for (i = 0; i < 5; i++)
          for (j = 0; j < m; j++)
            wp2.setData(j, i, fdata[i * m + j]);
        wp2.calcAllProperties();
        wp2.setId(outpid);
        addParameter(wp2);
      }
    }

    // FF EPS mean
  } else if (inpid.alias == "EPFM") {
    id1.alias = "EPFF";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      wp2.setDims(wp.Npoints(), 1);
      int i, n;
      fdata = statist.getMean(wp.copyDataVector(), wp.Ndim());
      n = fdata.size();
      if (n) {
        for (i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // FF EPS median
  } else if (inpid.alias == "EPFE") {
    id1.alias = "EPFF";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      fdata = statist.getMedian(wp.copyDataVector(), wp.Ndim());
      int n = fdata.size();
      if (n) {
        wp2.setDims(n, 1);
        for (int i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // FF EPS deciler (1..9)
  } else if (inpid.alias == "EPFD") {
    id1.alias = "EPFF";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 1);
      vector<float> dezils(9);
      for (i = 0; i < 9; i++)
        dezils[i] = i + 1;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 9 * m) {
        for (i = 0; i < 9; i++) {
          for (j = 0; j < m; j++)
            wp2.setData(j, 0, fdata[i * m + j]);
          wp2.calcAllProperties();
          outpid.level = static_cast<int> (dezils[i] * 10);
          wp2.setId(outpid);
          addParameter(wp2);
        }
      }
    }

    // FF EPS deciler (0,1,5,9,10)
  } else if (inpid.alias == "EPFDT") {
    id1.alias = "EPFF";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 5);
      vector<float> dezils(5);
      dezils[0] = 0;
      dezils[1] = 1;
      dezils[2] = 5;
      dezils[3] = 9;
      dezils[4] = 10;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 5 * m) {
        for (i = 0; i < 5; i++)
          for (j = 0; j < m; j++)
            wp2.setData(j, i, fdata[i * m + j]);
        wp2.calcAllProperties();
        wp2.setId(outpid);
        addParameter(wp2);
      }
    }

    // DD EPS mean
  } else if (inpid.alias == "EPDM") {
    id1.alias = "EPD";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      wp2.setDims(wp.Npoints(), 1);
      int i, n;
      fdata = statist.getMean(wp.copyDataVector(), wp.Ndim());
      n = fdata.size();
      if (n) {
        for (i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // DD EPS median
  } else if (inpid.alias == "EPDE") {
    id1.alias = "EPD";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      wp2 = wp;
      fdata = statist.getMedian(wp.copyDataVector(), wp.Ndim());
      int n = fdata.size();
      if (n) {
        wp2.setDims(n, 1);
        for (int i = 0; i < n; i++)
          wp2.setData(i, 0, fdata[i]);
        wp2.calcAllProperties();
        wp2.setId(inpid);
        addParameter(wp2);
      }
    }

    // DD EPS deciler (1..9)
  } else if (inpid.alias == "EPDD") {
    id1.alias = "EPD";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 1);
      vector<float> dezils(9);
      for (i = 0; i < 9; i++)
        dezils[i] = i + 1;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 9 * m) {
        for (i = 0; i < 9; i++) {
          for (j = 0; j < m; j++)
            wp2.setData(j, 0, fdata[i * m + j]);
          wp2.calcAllProperties();
          outpid.level = static_cast<int> (dezils[i] * 10);
          wp2.setId(outpid);
          addParameter(wp2);
        }
      }
    }

    // DD EPS deciler (0,1,5,9,10)
  } else if (inpid.alias == "EPDDT") {
    id1.alias = "EPD";
    wpok = copyParameter(id1, wp, &error);
    if (wpok) {
      int n, m, i;
      ParId outpid = inpid;
      m = wp.Npoints();
      wp2 = wp;
      wp2.setDims(m, 5);
      vector<float> dezils(5);
      dezils[0] = 0;
      dezils[1] = 1;
      dezils[2] = 5;
      dezils[3] = 9;
      dezils[4] = 10;
      fdata = statist.getXdecil(wp.copyDataVector(), wp.Ndim(), dezils);
      n = fdata.size();
      if (n == 5 * m) {
        for (i = 0; i < 5; i++)
          for (j = 0; j < m; j++)
            wp2.setData(j, i, fdata[i * m + j]);
        wp2.calcAllProperties();
        wp2.setId(outpid);
        addParameter(wp2);
      }
    }

    // FX 5,25,50,75,95 decils
  } else if (inpid.alias == "AFX") {
    id1.alias = "FX";
    int m=0, i, j;
    for (i = 1; i < 6; i++) {
      id1.submodel = miutil::from_number(i);

      wpok = copyParameter(id1, wp, &error);
      if (wpok) {
        if (i == 1) {
          m = wp.Npoints();
          wp2 = wp;
          wp2.setDims(m, 5);
        }
        for (j = 0; j < m; j++)
          wp2.setData(j, i - 1, wp.Data(j));
      }
    }
    ParId outpid = inpid;
    outpid.submodel = S_UNDEF;
    wp2.calcAllProperties();
    wp2.setId(outpid);
    addParameter(wp2);

    // FX 5,25,50,75,95 decils
  } else if (inpid.alias == "FXQT-6h") {
    std::string name = "fx10-6h_Q";
    int m=0, i, j;
    const std::string q[5] = { "5", "25", "50", "75", "95" };
    for (i = 0; i < 5; i++) {
      id1.alias = name + q[i];

      wpok = copyParameter(id1, wp, &error);
      if (wpok) {
        if (i == 0) {
          m = wp.Npoints();
          wp2 = wp;
          wp2.setDims(m, 5);
        }
        for (j = 0; j < m; j++)
          wp2.setData(j, i, wp.Data(j));
      }
    }
    ParId outpid = inpid;
    outpid.submodel = S_UNDEF;
    wp2.calcAllProperties();
    wp2.setId(outpid);
    addParameter(wp2);

    // RR 5,25,50,75,95 decils
  } else if (inpid.alias == "RRQT") {
    std::string name = "rr24_Q";
    int m=0, i, j;
    const std::string q[5] = { "5", "25", "50", "75", "95" };
    for (i = 0; i < 5; i++) {
      id1.alias = name + q[i];

      wpok = copyParameter(id1, wp, &error);
      if (wpok) {
        if (i == 0) {
          m = wp.Npoints();
          wp2 = wp;
          wp2.setDims(m, 5);
        }
        for (j = 0; j < m; j++)
          wp2.setData(j, i, wp.Data(j));
      }
    }
    ParId outpid = inpid;
    outpid.submodel = S_UNDEF;
    wp2.calcAllProperties();
    wp2.setId(outpid);
    addParameter(wp2);

    // t2m 5,25,50,75,95 decils
  } else if (inpid.alias == "t2mQT") {
    std::string name = "t2m_Q";
    int m=0, i, j;
    const std::string q[5] = { "5", "25", "50", "75", "95" };
    for (i = 0; i < 5; i++) {
      id1.alias = name + q[i];

      wpok = copyParameter(id1, wp, &error);
      if (wpok) {
        if (i == 0) {
          m = wp.Npoints();
          wp2 = wp;
          wp2.setDims(m, 5);
        }
        for (j = 0; j < m; j++)
          wp2.setData(j, i, wp.Data(j));
      }
    }
    ParId outpid = inpid;
    outpid.submodel = S_UNDEF;
    wp2.calcAllProperties();
    wp2.setId(outpid);
    addParameter(wp2);

    // confidence
  } else if (inpid.alias == "CONQ") {
    id1.alias = "TT";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++)
        wp.setData(j, 0.0);
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // discrete visibility
  } else if (inpid.alias == "VVQ") {
    id1.alias = "CWW";
    id2.alias = "XWW";
    if (!findParameter(id1, idx, &error))
      makeOneParameter(id1);
    if (!findParameter(id2, idx2, &error))
      makeOneParameter(id2);

    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
      int visi;
      for (j = 0; j < wp.Npoints(); j++) {
        f1 = wp.Data(j);
        f2 = wp2.Data(j);
        tmpSymbol = wsymbols.getSymboltype(static_cast<int> (f1));
        visi = sight.findVision(wsymbols.visibility(tmpSymbol), static_cast<int> (f2));
        wp.setData(j, visi);
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // discrete cloudcover
  } else if (inpid.alias == "CCQ") {
    id1.alias = "WW";
    id2.alias = "XWW";
    if (!findParameter(id1, idx, &error))
      makeOneParameter(id1);
    if (!findParameter(id2, idx2, &error))
      makeOneParameter(id2);

    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)) {
      int clgrp;
      for (j = 0; j < wp.Npoints(); j++) {
        f1 = wp.Data(j);
        f2 = wp2.Data(j);
        tmpSymbol = wsymbols.getSymboltype(static_cast<int> (f1));
        clgrp = lucy.findCl(tmpSymbol, static_cast<int> (f2));
        wp.setData(j, clgrp);
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // cloudcover 1/8 from cloudcover %
  } else if (inpid.alias == "CC8") {
    id1.alias = "CC";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        f1 = wp.Data(j);
        wp.setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // low cloudcover 1/8 from low cloudcover %
  } else if (inpid.alias == "CL8") {
    id1.alias = "CL";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        f1 = wp.Data(j);
        wp.setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // medium cloudcover 1/8 from medium cloudcover %
  } else if (inpid.alias == "CM8") {
    id1.alias = "CM";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        f1 = wp.Data(j);
        wp.setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // high cloudcover 1/8 from high cloudcover %
  } else if (inpid.alias == "CH8") {
    id1.alias = "CH";
    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        f1 = wp.Data(j);
        wp.setData(j, 0, int(f1 * 8.0 / 100.0));
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // additional weather
  } else if (inpid.alias == "XWW") {
    id1.alias = "WW";
    if (!findParameter(id1, idx, &error))
      makeOneParameter(id1);

    if (copyParameter(id1, wp, &error)) {
      for (j = 0; j < wp.Npoints(); j++) {
        wp.setData(j, 0);
      }
      if (newpid.run == R_UNDEF)
        newpid.run = wp.Id().run;
      wp.calcAllProperties();
      wp.setId(newpid);
      addParameter(wp);
    }

    // Water state
  } else if (inpid.alias == "AGR") {
    id1.alias = "TT";
    if (findParameter(id1, idx, &error)) {
      if (copyParameter(id1, wp, &error)) {
        if (newpid.run == R_UNDEF)
          newpid.run = wp.Id().run;
        fdata.clear();
        int n = wp.Npoints();
        for (j = 0; j < n; j++)
          fdata.push_back(wp.Data(j));
        vector<float> sdata = wsymbols.water_state(fdata);
        for (j = 0; j < n; j++)
          wp.setData(j, 0, sdata[j]);

        wp.calcAllProperties();
        wp.setId(newpid);
        addParameter(wp);
      }
    }

    // Precipitation state
  } else if (inpid.alias == "PS") {
    id1.alias = "TT";
    id2.alias = "RR";
    if (findParameter(id1, idx, &error) && findParameter(id2, idx2, &error)) {
      if (copyParameter(id1, wp, &error)) {
        int n1 = wp.Npoints();
        int n2 = parList[idx2].Npoints();
        if (n1 == n2) {
          for (j = 0; j < n1; j++) {
            float tt = parList[idx].Data(j);
            float rr = parList[idx2].Data(j);
            wp.setData(j, 0, precip_state(tt, rr));
          }
          wp.calcAllProperties();
          wp.setId(newpid);
          addParameter(wp);
        }
      }
    }

    // Weather symbol
  } else if (inpid.alias == "WW") {
    if (new_symbolmaker) {
      cerr << "USING NEW SYMBOLMAKER" << endl;

      makeWeatherSymbols_new_(inpid, false);
    } else {
      cerr << "USING OLD SYMBOLMAKER" << endl;

      makeWeatherSymbols_(inpid);
    }
    // Mean Weather symbol
  } else if (inpid.alias == "CWW") {
    if (new_symbolmaker)
      makeWeatherSymbols_new_(inpid, false);
    else
      makeWeatherSymbols_(inpid);

    // Ice Accretion
  } else if (inpid.alias == "ICEA") {
    id1.alias = "TT";
    id2.alias = "SST";
    id3.alias = "FF";
    if (copyParameter(id1, wp, &error) && copyParameter(id2, wp2, &error)
        && copyParameter(id3, wp3, &error)) {

      for (j = 0; j < wp.Npoints(); j++) {
        double t = wp.Data(j);
        double sst = wp2.Data(j);
        double ff = wp3.Data(j);
        float icea = iceAccretion(t, sst, ff);
        wp.setData(j, icea);
      }

      wp.calcAllProperties();
      wp.setId(inpid);
      addParameter(wp);
    }

  } else {
    //     cerr << "makeOneParameter: Unknown parameter:"<<inpid<<endl;
  }
}

// make named weatherparameters from existing wp's
void ptDiagramData::makeParameters(const vector<ParId>& pars, bool doupdate)
{
  int  idx;
  ErrorFlag error;

  // Lets see if can make the missing params
  for (unsigned int i = 0; i < pars.size(); i++) {
    if (!findParameter(pars[i], idx, &error))
      makeOneParameter(pars[i]);
    else if (doupdate)
      UpdateOneParameter(pars[i]);
  }
}

// find all timeLine-indices in use, and ship it to timeLine for cleanup
void ptDiagramData::cleanUpTimeline()
{
  vector<int> usedTimeIdx;
  unsigned int i,timeidx;
  bool used;

  for (i = 0; i < parList.size(); i++) {
    timeidx = parList[i].TimeLineIndex();
    used = (find(usedTimeIdx.begin(), usedTimeIdx.end(), timeidx)
        != usedTimeIdx.end());
    if (!used)
      usedTimeIdx.push_back(timeidx);
  }
  timeLine.cleanup(usedTimeIdx);
}

// add data by extrapolation
void ptDiagramData::addData(const int& id1, const miutil::miTime& until,
    const int step, vector<miutil::miTime>& timeline)
{
  WeatherParameter cwp;

  int x1;
  int i, j, k;
  miutil::miTime curr;
  vector<miutil::miTime> newtimes;

  x1 = timeline.size() - 1;
  if (timeline[x1] < until) {
    // make a copy of wp1
    cwp = parList[id1];
    // find new timepoints
    for (curr = until; curr > timeline[x1]; curr.addHour(-step)) {
      newtimes.push_back(curr);
    }
    // change wp1's dimensions
    parList[id1].setDims(cwp.Npoints() + newtimes.size(), cwp.Ndim());
    // copy old data to wp1
    for (j = 0; j < cwp.Npoints(); j++)
      for (k = 0; k < cwp.Ndim(); k++)
        parList[id1].setData(j, k, cwp.Data(j, k));

    // copy new data to wp1, and update timeline
    i = cwp.Npoints();
    for (j = newtimes.size() - 1; j >= 0; j--) {
      timeline.push_back(newtimes[j]);
      for (k = 0; k < cwp.Ndim(); k++)
        parList[id1].setData(i, k, cwp.Data(cwp.Npoints() - 1, k));
      i++;
    }
  }
}

// merge two datasets
void ptDiagramData::mergeData(const int& id1, const int& id2,
    const miutil::miTime& until, vector<miutil::miTime>& timeline, const int method)
{
  WeatherParameter cwp;
  vector<miutil::miTime> tl1, tl2;
  int tlid2 = parList[id2].TimeLineIndex();
  int merge_met = method;

  // get the timeline
  timeLine.Timeline(tlid2, tl2);

  int x1, x2, x3, x4;
  int i, j, k;

  x1 = timeline.size() - 1;
  x2 = tl2.size() - 1;
  if (timeline[x1] < until && tl2[x2] > timeline[x1]) {
    // make a copy of wp1
    cwp = parList[id1];
    // find first timepoint in wp2 > wp1's endpoint
    for (x3 = 0; (x3 <= x2) && (tl2[x3] <= timeline[x1]); x3++)
      ;
    if (x3 > x2)
      x3 = x2;
    // find last timepoint in wp2 <= until
    for (x4 = x3; (x4 <= x2) && (tl2[x4] <= until); x4++)
      ;
    x4--;
    // change wp1's dimensions
    parList[id1].setDims(cwp.Npoints() + x4 - x3 + 1, cwp.Ndim());
    if (merge_met == MERGE_ADAPT) {
      // check if adapt method is legal on current alias
      merge_met = MERGE_RAW;
    }
    if (merge_met == MERGE_RAW) {
      // copy old data to wp1
      for (j = 0; j < cwp.Npoints(); j++)
        for (k = 0; k < cwp.Ndim(); k++)
          parList[id1].setData(j, k, cwp.Data(j, k));

      // copy new data to wp1, and update timeline
      i = cwp.Npoints();
      for (j = x3; j <= x4; j++) {
        timeline.push_back(tl2[j]);
        for (k = 0; k < cwp.Ndim(); k++)
          parList[id1].setData(i, k, parList[id2].Data(j, k));
        i++;
      }
    }
  }
}

// make default list of parameter info
void ptDiagramData::makedefaultParInfo()
{
  parInfo["TD"] = parameter_info("TD", 0, true, false);
  parInfo["RR1"] = parameter_info("RR1", 0, true, true);
  parInfo["RR2"] = parameter_info("RR2", 0, true, true);
  parInfo["RR"] = parameter_info("RR", 0, true, true);
  parInfo["CC"] = parameter_info("CC", 0, true, false);
  parInfo["CC8"] = parameter_info("CC8", 0, true, false);
  parInfo["TT"] = parameter_info("TT", 0, true, false);
  parInfo["RH"] = parameter_info("RH", 0, true, false);

  parInfo["UU"] = parameter_info("UU", 0, false, false);
  parInfo["VV"] = parameter_info("VV", 0, false, false);
  parInfo["WV"] = parameter_info("WV", 0, false, false);
  parInfo["FG"] = parameter_info("FG", 0, true, false);
  parInfo["CL"] = parameter_info("CL", 0, true, false);
  parInfo["CM"] = parameter_info("CM", 0, true, false);
  parInfo["CH"] = parameter_info("CH", 0, true, false);
  parInfo["VVC"] = parameter_info("VVC", 10, true, false);

  parInfo["VVQ"] = parameter_info("VVQ", 9, false, false);
  parInfo["WW"] = parameter_info("WW", 0, false, false);
  parInfo["MSLP"] = parameter_info("MSLP", 1000, true, false);
  parInfo["HST"] = parameter_info("HST", 0.1, 0, 100, 0, false, true, false); //
  parInfo["HSX"] = parameter_info("HSX", 0, true, false);
  parInfo["HSTX"] = parameter_info("HSTX", 0, true, false);
  parInfo["HSTE"] = parameter_info("HSTE", 0, true, false);
  parInfo["HSEX"] = parameter_info("HSEX", 0, true, false);

  parInfo["HSXF"] = parameter_info("HSXF", 1.65, true, false);
  parInfo["HEXF"] = parameter_info("HEXF", 1, true, false);
  parInfo["TPT"] = parameter_info("TPT", 1, true, false);
  parInfo["DDPT"] = parameter_info("DDPT", 1, 0, 360, 360, true, false, false); //
  parInfo["TST"] = parameter_info("TST", 1, true, false);
  parInfo["TM01"] = parameter_info("TM01", 1, true, false);
  parInfo["HSP"] = parameter_info("HSP", 0, true, false);
  parInfo["TSP"] = parameter_info("TSP", 1, true, false);
  parInfo["DDPP"] = parameter_info("DDPP", 1, 0, 360, 360, true, false, false); //

  parInfo["HSD"] = parameter_info("HSD", 0, true, false);
  parInfo["TSD"] = parameter_info("TSD", 1, true, false);
  parInfo["DDPD"] = parameter_info("DDPD", 1, 0, 360, 360, true, false, false); //
  parInfo["FF"] = parameter_info("FF", 1, 0, 50, 0, false, true, false); //
  parInfo["DD"] = parameter_info("DD", 1, 0, 360, 0, true, false, false); //
  parInfo["GU"] = parameter_info("GU", 0, true, false);
  parInfo["WVFD"] = parameter_info("WVFD", 0, false, false);
  parInfo["WVMD"] = parameter_info("WVMD", 0, false, false);

  parInfo["HS"] = parameter_info("HS", 0.01, -1, 6, 0, false, true, false); //
  parInfo["CU"] = parameter_info("CU", 0, false, false);
  parInfo["CV"] = parameter_info("CV", 0, false, false);
  parInfo["CUV"] = parameter_info("CUV", 0, false, false);
  parInfo["CFD"] = parameter_info("CFD", 0, false, false);
  parInfo["CFF"] = parameter_info("CFF", 1, 0, 50, 0, false, true, false);
  parInfo["CDD"] = parameter_info("CDD", 1, 0, 360, 360, true, false, false);

  parInfo["STA"] = parameter_info("STA", 0, true, false);
  parInfo["STAQ"] = parameter_info("STAQ", 0, false, false);
  parInfo["CON"] = parameter_info("CON", 100, false, false);

  parInfo["CONQ"] = parameter_info("CONQ", 0, false, false);
  parInfo["XWW"] = parameter_info("XWW", 0, false, false);
  parInfo["HEC"] = parameter_info("HEC", 0.01, -15, 50, 0, false, true, false);//
  parInfo["EMC"] = parameter_info("EMC", 0.01, -15, 50, 0, false, true, false);//
  parInfo["SHC"] = parameter_info("SHC", 0, false, false);
  parInfo["CWW"] = parameter_info("CWW", 0, false, false);
  parInfo["AGR"] = parameter_info("AGR", 0, false, false);
  parInfo["LII"] = parameter_info("LII", 0, false, false);
  parInfo["SHL"] = parameter_info("SHL", 0, false, false);

  parInfo["DDPE"] = parameter_info("DDPE", 0, false, false);
  parInfo["FOI"] = parameter_info("FOI", 0, false, false);
  parInfo["FFMS"] = parameter_info("FFMS", 0, true, false);
  parInfo["CL8"] = parameter_info("CL8", 0, true, false);
  parInfo["CM8"] = parameter_info("CM8", 0, true, false);
  parInfo["CH8"] = parameter_info("CH8", 0, true, false);
  parInfo["PS"] = parameter_info("PS", 0, false, false);
  parInfo["SST"] = parameter_info("SST", 0, true, false);

  parInfo["FFMI"] = parameter_info("FFMI", 0, true, false);
  parInfo["FFMA"] = parameter_info("FFMA", 0, true, false);
  parInfo["ICEA"] = parameter_info("ICEA", 0, true, false);
  parInfo["VIS"] = parameter_info("VIS", 10, true, false);
  parInfo["CB"] = parameter_info("CB", 5000, true, false);
  parInfo["ICE"] = parameter_info("ICE", 0, true, false);
}

// return info about a specific parameter
bool ptDiagramData::parameterInfo(const ParId& pid, float& def, bool& interpok,
    bool& spreadok)
// input: one parameter id
// out: def=      default value for parameter
//      interpok= ok to interpolate values
//      spreadok= if !interpok, spread datavalues
{
  if (pid.alias == A_UNDEF)
    return false;
  std::string alias = pid.alias;

  if (parInfo.count(alias) > 0) {
    parameter_info pai = parInfo[alias];
    def = pai.def;
    interpok = pai.interpok;
    spreadok = pai.spreadok;
    return true;
  }
  cerr << "ptDiagramData::parameterInfo Id not found:" << pid << endl;
  return false;
}

// return info about a specific parameter
bool ptDiagramData::parameterInfo(const ParId& pid, parameter_info& pai)
// input: one parameter id
// out: parameter_info
{
  if (pid.alias == A_UNDEF)
    return false;
  std::string alias = pid.alias;

  if (parInfo.count(alias) > 0) {
    pai = parInfo[alias];
    return true;
  }
  cerr << "ptDiagramData::parameterInfo Id not found:" << pid << endl;
  return false;
}

// make a wp from scratch, return index
int ptDiagramData::makeOneParameter(const ParId& pid, const int tlindex,
    const int ntimep)
{
  // 21.03.2014: Audun
  // TEST: removing dependency on libparameter (and the old parameters.def file)
  // by forcing Dimension of parameter = 1

  //  ParameterDefinition parDef;
  WeatherParameter wp;
  int index = -1;
  int i, j;
  std::string alias = pid.alias;
  //  Parameter pp;
  parameter_info pai;

  //  if (parDef.getParameter(pid.alias, pp)) {
  parameterInfo(pid, pai);
  wp.setPolar(false); // default is scalar
  wp.setTimeLineIndex(tlindex);
  // set wp's dimensions
  //    wp.setDims(ntimep, pp.order());
  wp.setDims(ntimep, 1);
  wp.setId(pid);
  // fill with default data
  //    for (i = 0; i < ntimep; i++)
  //      for (j = 0; j < pp.order(); j++)
  //        wp.setData(i, j, pai.def);
  for (i = 0; i < ntimep; i++)
    wp.setData(i, 0, pai.def);
  wp.calcAllProperties();
  index = addParameter(wp);
  //  }
  return index;
}

// interpolate wp's data to remove gaps of default values
void ptDiagramData::interpData(const int idx, vector<bool>& locked)
{
  // 1. find undefined timepoints
  // 2. find neighboring defined timepoints
  // 3. if two neighbors: linear interpolation
  //    only one neighbor: raw copy
  if (idx == -1)
    return; // illegal index
  unsigned int j, l, m;
  parameter_info pai;
  //   float dum;
  //   bool interp= false, spread= false;
  ParId pid = parList[idx].Id();
  unsigned int np = parList[idx].Npoints();
  unsigned int nd = parList[idx].Ndim();
  if (np != locked.size())
    return; // locked-array and data doesn't match

  parameterInfo(parList[idx].Id(), pai);
  bool anyp = false;
  for (j = 0; j < np; j++)
    anyp = anyp || locked[j];
  if (!anyp)
    return; // data has default values ONLY; nothing to do

  int left, right;
  float leftd, rightd, dat, slope, meanv;
  for (j = 0; j < np; j++) {
    if (locked[j])
      continue;
    // left neighbor
    left = j - 1;
    // find right neighbor
    for (right = j + 1; right < np; right++)
      if (locked[right])
        break;

    // loop through all dimensions
    for (l = 0; l < nd; l++) {
      if (left > -1 && right < np) {
        leftd = parList[idx].Data(left, l);
        rightd = parList[idx].Data(right, l);
      } else if (left > -1) {
        leftd = parList[idx].Data(left, l);
        rightd = leftd;
      } else if (right < np) {
        rightd = parList[idx].Data(right, l);
        leftd = rightd;
      } else {
        cerr << "ptDiagramData::interpData internal error" << endl;
        continue;
      }
      // calculate slope of interpolating curve
      slope = (rightd - leftd) / (right - left);
      // calculate spread-value
      meanv = (rightd / (right - left));
      // fill data-points
      for (m = left + 1; m < right; m++) {
        if (pai.interpok) {
          // interpolation ok
          dat = leftd + slope * (m - left);
        } else {
          if (pai.spreadok) {
            // even spread of data
            dat = meanv;
          } else {
            // persistence
            dat = leftd;
          }
        }
        parList[idx].setData(m, l, dat);
      }
      // correct right value if spread of data
      if (right < np && !pai.interpok && pai.spreadok)
        parList[idx].setData(right, l, meanv);
    }
    // adjust main counter
    j = right;
  }
}

// replace wp's data for matching timepoints
void ptDiagramData::replaceData(const int oldidx, const int newidx,
    const vector<miutil::miTime> inptline, vector<bool>& locked)
{
  unsigned  int j, k, l;
  vector<miutil::miTime> curtline;
  parameter_info pai;

  if (newidx != -1 && oldidx != -1 && parList[oldidx].Ndim()
      == parList[newidx].Ndim() && inptline.size() == locked.size()) {
    parameterInfo(parList[oldidx].Id(), pai);
    j = parList[newidx].TimeLineIndex();
    timeLine.Timeline(j, curtline);
    for (j = 0; j < inptline.size(); j++) {
      if (locked[j])
        continue;
      for (k = 0; k < curtline.size(); k++) {
        if (inptline[j] == curtline[k]) {
          for (l = 0; l < parList[oldidx].Ndim(); l++) {
            float data = parList[newidx].Data(k, l);
            if (data < pai.min)
              data = pai.min;
            if (data > pai.max)
              data = pai.max;
            parList[oldidx].setData(j, l, data);
          }
          locked[j] = true;
          break;
        }
      }
    }
  }
}

// make datasets with given timepoints; add data from models
void ptDiagramData::makeDatasets(const vector<ParId>& tempmod, // work buffer
    const vector<ParId>& destmod, // destination
    const vector<ParId>& subjmod, // subjective values
    const vector<ParId>& mainmod, // main model values
    const vector<ParId>& xtramod, // sec. model values
    const vector<time_interval>& intervals)
{
  ErrorFlag error;
  WeatherParameter inpwp, curwp;
  unsigned int i, j;
  miutil::miTime ct;
  vector<miutil::miTime> inptline, curtline;
  vector<bool> locked;
  int tlindex; // index to new timeline
  int tempindex, subjindex, mainindex, xtraindex; // indices to wp's
  ParId tempid, destid, subjid, mainid, xtraid;

  if (tempmod.size() != destmod.size() || tempmod.size() != subjmod.size()
      || tempmod.size() != mainmod.size() || tempmod.size() != xtramod.size()) {
    cerr << "++++++++++++++++OBS! ptDiagramData::makeDatasets wrong sizes"
        << endl;
    cerr << "tempmod:" << tempmod.size() << endl;
    cerr << "destmod:" << destmod.size() << endl;
    cerr << "subjmod:" << subjmod.size() << endl;
    cerr << "mainmod:" << mainmod.size() << endl;
    cerr << "xtramod:" << xtramod.size() << endl;
    return;
  }

  if (!intervals.size()) {
    cerr << "ptDiagramData::makeDatasets error: intervals.size()==0" << endl;
    return;
  }
  // check intervals
  for (i = 0; i < intervals.size(); i++)
    if (intervals[i].from < intervals[i].until && intervals[i].step == 0) {
      cerr << "Illegal stepsize in time-interval: 0" << endl;
      return;
    }
  // make new timeline
  for (i = 0; i < intervals.size(); i++) {
    for (ct = intervals[i].from; ct <= intervals[i].until; (intervals[i].step
        > 0 ? ct.addHour(intervals[i].step) : ct.addMin(-intervals[i].step))) {
      inptline.push_back(ct);
      locked.push_back(false);
    }
  }
  // sort and check timeline for ambiguities
  sort(inptline.begin(), inptline.end());
  vector<miutil::miTime>::iterator p;
  for (i = 1; i < inptline.size(); i++) {
    if (inptline[i] == inptline[i - 1]) {
      p = inptline.begin() + i;
      inptline.erase(p);
      locked.erase(locked.begin());
      i--;
    }
  }

  // check if new timeline already exist, add it
  if ((tlindex = timeLine.Exist(inptline)) == -1)
    tlindex = addTimeLine(inptline);

  for (i = 0; i < tempmod.size(); i++) {
    tempid = tempmod[i];
    //     cerr << "ptDiagramData::makeDatasets New parameter " << tempid << endl;

    for (j = 0; j < locked.size(); j++)
      locked[j] = false;

    // make default parameter
    tempindex = makeOneParameter(tempid, tlindex, inptline.size());
    //     cerr << " has index: " << tempindex << endl;

    if (tempindex == -1) {
      cerr << "ptDiagramData::makeDatasets error: Could not make wp:" << tempid
          << endl;
      // This goes wrong after we removed libParameter. In preliminary tests
      // there was no negative effect on this - Libparameter has to be avoided
      // due to its ancient lex parser
      continue;
    }

    // find reference and model wp's
    destid = tempid;
    destid.model = destmod[i].model;
    destid.run = destmod[i].run;
    destid.submodel = destmod[i].submodel;
    //     cerr << "   destid:" << destid << endl;

    subjindex = -1;
    subjid = tempid;
    subjid.model = subjmod[i].model;
    subjid.run = subjmod[i].run;
    subjid.submodel = subjmod[i].submodel;
    if (subjid.model != M_UNDEF)
      findParameter(subjid, subjindex, &error);
    //     cerr << "   subjid:" << subjid << endl;
    //     cerr << "       index:" << subjindex << endl;

    mainindex = -1;
    mainid = tempid;
    mainid.model = mainmod[i].model;
    mainid.run = mainmod[i].run;
    mainid.submodel = mainmod[i].submodel;
    if (mainid.model != M_UNDEF)
      findParameter(mainid, mainindex, &error);
    //     cerr << "   mainid:" << mainid << endl;
    //     cerr << "       index:" << mainindex << endl;

    xtraindex = -1;
    xtraid = tempid;
    xtraid.model = xtramod[i].model;
    xtraid.submodel = xtramod[i].submodel;
    xtraid.run = xtramod[i].run;
    if (xtraid.model != M_UNDEF)
      findParameter(xtraid, xtraindex, &error);
    //     cerr << "   xtraid:" << xtraid << endl;
    //     cerr << "       index:" << xtraindex << endl;

    // insert destindex-data
    replaceData(tempindex, subjindex, inptline, locked);
    // insert mainindex-data
    replaceData(tempindex, mainindex, inptline, locked);
    // insert xtraindex-data
    replaceData(tempindex, xtraindex, inptline, locked);
    // fill remaining holes
    interpData(tempindex, locked);

    // remove destination wp
    deleteParameter(subjindex);
    // find new tempindex
    if (findParameter(tempid, tempindex, &error)) {
      // set input id to dest id
      parList[tempindex].setId(destid);

      // check if any real values
      bool anyp = false;
      for (j = 0; j < locked.size(); j++)
        if (locked[j]) {
          anyp = true;
          break;
        }

      if (!anyp) {
        cerr << destid << " does not contain any real data, do an update.."
            << endl;
        UpdateOneParameter(destid);
      }

      parList[tempindex].calcAllProperties();
      // lock this WP for unnecessary modifications
      parList[tempindex].setLocked(true);
      // .. and clear modification flags
      parList[tempindex].clearTempDirty();
    } else {
      cerr << "PtDiagramData::makeDatasets error: Tempid disappeared:"
          << tempid << endl;
    }
  }
}

// make datasets with given timepoints; add data from models
void ptDiagramData::makeDatasets(const datasetparam& dsp,
    vector<miutil::miTime>& inptline)
{
  ErrorFlag error;
  WeatherParameter inpwp, curwp;
  unsigned int i, j;
  miutil::miTime ct;
  vector<miutil::miTime> curtline;
  vector<bool> locked;
  int tlindex; // index to new timeline
  int tempindex, subjindex, mainindex, xtraindex, xtra2index; // indices to wp's
  ParId tempid, destid, subjid, mainid, xtraid, xtra2id;

  if (!inptline.size()) {
    cerr << "ptDiagramData::makeDatasets error: inputtimes.size()==0" << endl;
    return;
  }

  // sort and check timeline for ambiguities
  sort(inptline.begin(), inptline.end());
  vector<miutil::miTime>::iterator p;
  for (i = 1; i < inptline.size(); i++) {
    if (inptline[i] == inptline[i - 1]) {
      p = inptline.begin() + i;
      inptline.erase(p);
      i--;
    }
  }

  locked.insert(locked.end(), inptline.size(), false);

  // check if new timeline already exist, add it
  if ((tlindex = timeLine.Exist(inptline)) == -1)
    tlindex = addTimeLine(inptline);

  //   for (i=0; i<tempmod.size(); i++){
  tempid = dsp.temp;
  //   cerr << "ptDiagramData::makeDatasets New parameter " << tempid << endl;

  // make default parameter
  tempindex = makeOneParameter(tempid, tlindex, inptline.size());

  if (tempindex == -1) {
    cerr << "ptDiagramData::makeDatasets error: Could not make wp:" << tempid
        << endl;
    return;
    // This goes wrong after we removed libParameter. In preliminary tests
    // there was no negative effect on this - Libparameter has to be avoided
    // due to its ancient lex parser
  }

  // find reference and model wp's
  destid = dsp.dest;
  //   cerr << "   destid:" << destid << endl;

  subjindex = -1;
  subjid = dsp.subj;
  if (subjid.model != M_UNDEF)
    findParameter(subjid, subjindex, &error);
  //   cerr << "   subjid:" << subjid << endl;
  //   cerr << "       index:" << subjindex << endl;

  mainindex = -1;
  mainid = dsp.prim;
  if (mainid.model != M_UNDEF)
    findParameter(mainid, mainindex, &error);
  //   cerr << "   mainid:" << mainid << endl;
  //   cerr << "       index:" << mainindex << endl;

  xtraindex = -1;
  xtraid = dsp.secu;
  if (xtraid.model != M_UNDEF)
    findParameter(xtraid, xtraindex, &error);
  //   cerr << "   xtraid:" << xtraid << endl;
  //   cerr << "       index:" << xtraindex << endl;

  xtra2index = -1;
  xtra2id = dsp.tert;
  if (xtra2id.model != M_UNDEF)
    findParameter(xtra2id, xtra2index, &error);
  //   cerr << "   xtra2id:" << xtra2id << endl;
  //   cerr << "       index:" << xtra2index << endl;

  // insert destindex-data
  replaceData(tempindex, subjindex, inptline, locked);
  // insert mainindex-data
  replaceData(tempindex, mainindex, inptline, locked);
  // insert xtraindex-data
  replaceData(tempindex, xtraindex, inptline, locked);
  // insert xtra2index-data
  replaceData(tempindex, xtra2index, inptline, locked);
  // fill remaining holes
  interpData(tempindex, locked);

  // remove destination wp
  deleteParameter(subjindex);
  // find new tempindex
  if (findParameter(tempid, tempindex, &error)) {
    // set input id to dest id
    parList[tempindex].setId(destid);

    // check if any real values
    bool anyp = false;
    for (j = 0; j < locked.size(); j++)
      if (locked[j]) {
        anyp = true;
        break;
      }

    if (!anyp) {
      cerr << destid << " does not contain any real data, doing an update.."
          << endl;
      UpdateOneParameter(destid);
    }

    parList[tempindex].calcAllProperties();
    // lock this WP for unnecessary modifications
    parList[tempindex].setLocked(true);
    // .. and clear modification flags
    parList[tempindex].clearTempDirty();
  } else {
    cerr << "PtDiagramData::makeDatasets error: Tempid disappeared:" << tempid
        << endl;
  }
}

void ptDiagramData::setSHCinfo(const SHCinfo& shc)
{
  shcinfo = shc;
}

void ptDiagramData::setWindCalc(const WindCalc& wc)
{
  windCalc = wc;
}

bool ptDiagramData::makeSHC_(const int diridx, WeatherParameter& wp,
    const int level)
{
  //cerr << "Making SHC for level:" << level << endl;
  if (diridx < 0 || diridx >= parList.size())
    return false;

  int levelidx = shcinfo.levelIndex(level);
  if (levelidx < 0)
    return false;

  //cerr << "Levelindex=" << levelidx << endl;

  for (int j = 0; j < wp.Npoints(); j++) {
    float dir = parList[diridx].Data(j);
    int didx = shcinfo.levels[levelidx].dirIndex(dir);
    if (didx < 0)
      return false;
    float value = shcinfo.levels[levelidx].dirs[didx].value;
    //     cerr << "Value for time:" << j << " and dir:" << dir
    // 	 << " dirindex:" << didx << " is " << value << endl;
    wp.setData(j, value);
  }

  return true;
}

float ptDiagramData::calcMedianTM01(float hst)
{
  // calculate median of TM01 period from wave-height
  float a1 = 0.4211;
  float a2 = 1.1133;
  float a3 = 0.2477;
  float a8 = 0.0;
  float a9 = 1.0;
  float tm01 = expf(a1 + a2 * powf(hst, a3)) + a8 * powf(hst, a9);
  //cerr << "  .. calculated tm01:" << tm01 << endl;
  return tm01;
}

float ptDiagramData::calcMedianTM02(float hst)
{
  // calculate median of TM02 period from wave-height
  float a1 = 0.9;
  float a2 = -0.093;
  float a3 = 0.8947;
  float a8 = 2.0781;
  float a9 = 0.6331;
  float tm02 = expf(a1 + a2 * powf(hst, a3)) + a8 * powf(hst, a9);
  //cerr << "  .. calculated tm02:" << tm02 << endl;
  return tm02;
}

// calculate Height Ekofisk Crest from:
// hst: Total Wave Height
// hs:  sealevel (stormsurge+tide)
float ptDiagramData::calcHec_(float hst, float hs)
{
  // H(stormsurge+tide) + DeltaH
  float hlat = hs;// + deltah;

  // Height Ekofisk Crest
  return -0.0197 * hst * hst + 1.115 * hst + hlat / 1.5;
}

// calculate Ekofisk (Characteristic) Maximum Crest from:
// hst: Total Wave Height
// hs:  sealevel (stormsurge+tide)
// tm01
// tm02
float ptDiagramData::calcCMC_(float hst, float hs, float tm01, float tm02)
{
  const float d = 75.0;
  const float t = 3600.0;
  const float pi = 4 * atanf(1.0);
  const float g = 9.81;

  //cerr << ".";
  //const float deltah = 0.67;
  //hs += deltah;

  //if (tm01 < 0 || tm02 < 0) {
  //cerr << "incoming tm01:" <<  tm01 << " tm02:" << tm02;
  // calculate median of tm01 (tm)
  //tm01 = calcMedianTM01(hst);
  // calculate median of tm02 (tz)
  //tm02 = calcMedianTM02(hst);
  //cerr << "  .. calculated tm01:" << tm01 << " tm02:" << tm02 << endl;
  //} else {
  //  //cerr << " tm01 and tm02 FOUND! tm01:" <<  tm01 << " tm02:" << tm02 << endl;
  //}
  // calculate steepness (s1)
  float s1 = 2 * pi * hst / g / (tm01*tm01);
  // calculate deep water wave number (k1)
  float k1 = 4 * pi*pi / g / (tm01*tm01);
  // calculate Ursell number (urs)
  float urs = hst / (k1 * k1) / powf(d, 3);
  // calculate Weibull parameters
  float afc = 0.3536 + 0.2568 * s1 + 0.08 * urs;
  float bfc = 2.0 - 1.7912 * s1 - 0.5302 * urs + 0.284 * urs * urs;
  // calculate number of waves in one hour
  float xn = t / tm02;
  // calculate characteristic largest crest
  float crx = hs + afc * hst * powf(logf(xn), (1. / bfc));
  return crx;
}

void ptDiagramData::makeWeatherSymbols_(ParId p)
{
  unsigned int i;
  int j;
  int tlidx;
  const unsigned int numparams = 7;
  ErrorFlag error;
  TimeLine times;
  vector<miutil::miTime> termin, tline;
  vector<ParId> id(numparams);
  vector<int> num(numparams, 0);
  vector<miSymbol> symbols;

  map<miutil::miTime, float> modelIn;
  vector<paramet> modelVec;
  paramet modelUse;


  float glat;
  WeatherParameter wp;

  glat = station.lat();

  for (i = 0; i < numparams; i++)
    id[i] = p;
  id[0].alias = "FG";
  id[1].alias = "CL";
  id[2].alias = "CM";
  id[3].alias = "CH";
  id[4].alias = "CC";
  id[5].alias = "RR";
  id[6].alias = "TT";
  num[0] = 3901;
  num[1] = 3902;
  num[2] = 3903;
  num[3] = 3904;
  num[4] = 25;
  num[5] = 17;
  num[6] = 31;

  for (i = 0; i < numparams; i++) {
    // find parameter
    if (copyParameter(id[i], wp, &error)) {
      // get timeline
      tlidx = wp.TimeLineIndex();
      timeLine.Timeline(tlidx, tline);
      // Insert time-points
      times.insert(tline);
      // extract data from wp's
      for (j = 0; j < wp.Npoints(); j++)
        modelIn[tline[j]] = wp.Data(j);

      // build parameter vectors
      modelUse.AddPara(num[i], modelIn, glat);
      modelIn.clear();
      modelVec.push_back(modelUse);
    }
  }
  // get time-union of all parameters
  tlidx = 0;
  times.Timeline(tlidx, termin);

  int dmin = 0;
  symbols = wsymbols.compute(modelVec, termin, dmin / 2, dmin);

  termin.erase(termin.begin(), termin.end());
  if (symbols.size() > 0) {
    wp.setDims(symbols.size(), 1);
    for (i = 0; i < symbols.size(); i++) {
      int cn = symbols[i].customNumber();
      if (cn == 999) {
        cerr << i << " Symbolmaker Error:" << symbols[i].customName()
                                                            << " number:" << symbols[i].customNumber() << endl;
        cn = 0;
      }
      termin.push_back(symbols[i].getTime());
      wp.setData(i, 0, static_cast<float> (cn));
      //wp.setData(i,1,symbols[i].lightStatus());
    }
    // check if new timeline already exist, add it
    int tlidx;
    if ((tlidx = timeLine.Exist(termin)) == -1)
      tlidx = addTimeLine(termin);
    wp.setTimeLineIndex(tlidx);
    wp.setId(p);
    wp.setType(SYMBOL);
    wp.calcAllProperties();
    addParameter(wp);
  }
}





void ptDiagramData::makeYrWeatherSymbols(ParId p)
{
  // start by checking the precipitation frequency
/*
  ErrorFlag error;
  WeatherParameter rr;
  TimeLine times;
  vector<miutil::miTime> weatherTimeline;
  vector<int> weatherPeriod;
  vector<miutil::miTime> precipitationTimeline;
  vector<weather_symbol::WeatherData> weatherData;

  if (copyParameter(17, rr, &error)) {
    int tlidx = rr.TimeLineIndex();
    timeLine.Timeline(tlidx, precipitationTimeline);

    if(precipitationTimeline.size() < 2 )
      return;

    for(int i=1;i<precipitationTimeline.size();i++) {
        // weathersymbol is dependend on precipitation and the period of the precipitation.
        // Therefore the symbol generator timeline is given by the precipitation timeline,
        // but caveat, the 1st time has no period - and gets no symbol!

        weatherTimeline.push_back(precipitationTimeline[i]);
        weatherPeriod.push_back(miutil::miTime::hourDiff(precipitationTimeline[i],precipitationTimeline[i-1]));
        weather_symbol::WeatherData singleData;
        singleData.precipitation= rr.Data(i);
        weatherData.push_back(singleData);
    }


  }



  unsigned int i;
  int j;
  int tlidx;
  const unsigned int numparams = 7;
  ErrorFlag error;
  vector<miutil::miTime> termin, tline;
  vector<ParId> id(numparams);
  vector<int> num(numparams, 0);
  vector<miSymbol> symbols;

  map<miutil::miTime, float> modelIn;
  vector<paramet> modelVec;
  paramet modelUse;


  float glat;
  WeatherParameter wp;

  glat = station.lat();

  for (i = 0; i < numparams; i++)
    id[i] = p;
  id[0].alias = "FG";
  id[1].alias = "CL";
  id[2].alias = "CM";
  id[3].alias = "CH";
  id[4].alias = "CC";
  id[5].alias = "RR";
  id[6].alias = "TT";


  for (i = 0; i < numparams; i++) {
    // find parameter
    if (copyParameter(id[i], wp, &error)) {

      // get timeline




      tlidx = wp.TimeLineIndex();
      timeLine.Timeline(tlidx, tline);
      // Insert time-points
      times.insert(tline);
      // extract data from wp's
      for (j = 0; j < wp.Npoints(); j++)
        modelIn[tline[j]] = wp.Data(j);

      // build parameter vectors
      modelUse.AddPara(num[i], modelIn, glat);
      modelIn.clear();
      modelVec.push_back(modelUse);
    }
  }





  // get time-union of all parameters
  tlidx = 0;
  times.Timeline(tlidx, termin);

  int dmin = 0;
  symbols = wsymbols.compute(modelVec, termin, dmin / 2, dmin);

  termin.erase(termin.begin(), termin.end());
  if (symbols.size() > 0) {
    wp.setDims(symbols.size(), 1);
    for (i = 0; i < symbols.size(); i++) {
      int cn = symbols[i].customNumber();
      if (cn == 999) {
        cerr << i << " Symbolmaker Error:" << symbols[i].customName()
                                                            << " number:" << symbols[i].customNumber() << endl;
        cn = 0;
      }
      termin.push_back(symbols[i].getTime());
      wp.setData(i, 0, static_cast<float> (cn));
      //wp.setData(i,1,symbols[i].lightStatus());
    }






    // check if new timeline already exist, add it
    int tlidx;
    if ((tlidx = timeLine.Exist(termin)) == -1)
      tlidx = addTimeLine(termin);
    wp.setTimeLineIndex(tlidx);
    wp.setId(p);
    wp.setType(SYMBOL);
    wp.calcAllProperties();
    addParameter(wp);
  }
  */
}











void ptDiagramData::makeWeatherSymbols_new_(ParId p, bool update)
{
  unsigned int i;
  int j;
  int tlidx;
  const unsigned int numparams = 5;
  ErrorFlag error;
  TimeLine times;
  vector<miutil::miTime> termin, tline;
  vector<ParId> id(numparams);
  vector<int> num(numparams, 0);
  vector<miSymbol> symbols;

  map<miutil::miTime, float> modelIn;
  vector<paramet> modelVec;
  paramet modelUse;

  float glat;
  WeatherParameter wp;
  int wpidx;

  //   WeatherParameter foiwp;
  //   ParId foiid= p;
  //   foiid.alias= "FOI";

  if (update) {
    if (!findParameter(p, wpidx, &error)) {
      cerr << "_makeWeatherSymbols_new: WeatherParameter:" << p << " not found"
          << endl;
      return;
    }
  }

  glat = station.lat();

  for (i = 0; i < numparams; i++)
    id[i] = p;
  id[0].alias = "CC";
  id[1].alias = "RR";
  id[2].alias = "LII";
  id[3].alias = "AGR";
  id[4].alias = "FOI";
  num[0] = 25;
  num[1] = 17;
  num[2] = 661;
  num[3] = 170;
  num[4] = 665;

  for (i = 0; i < numparams; i++) {
    // find parameter
    if (copyParameter(id[i], wp, &error)) {
      // get timeline
      tlidx = wp.TimeLineIndex();
      timeLine.Timeline(tlidx, tline);
      // Insert time-points
      times.insert(tline);
      // extract data from wp's
      for (j = 0; j < wp.Npoints(); j++) {
        modelIn[tline[j]] = wp.Data(j);
      }

      // build parameter vectors
      modelUse.AddPara(num[i], modelIn, glat);
      modelIn.clear();
      modelVec.push_back(modelUse);
    }
  }
  if (!update) {
    // get time-union of all parameters
    tlidx = 0;
    times.Timeline(tlidx, termin);
  } else {
    // get original timeline from wp
    tlidx = parList[wpidx].TimeLineIndex();
    timeLine.Timeline(tlidx, termin);
  }

  // if fewer symbols than data-points - use automatic
  // min/max-time algorithm in symbolMaker
  int tmin, tmax;
  if (termin.size() == times.size()) {
    tmin = 0;
    tmax = 0;
    symbols = wsymbols.compute_new(modelVec, termin, tmin, tmax, false);
  } else {
    if (termin.size() > 0) {
      tmin = tmax = 3;
      if (termin[0].hour() == 0)
        tmin = tmax = 6;
      symbols = wsymbols.compute_new(modelVec, termin, tmin, tmax, true);
    }
  }

  if (symbols.size() > 0) {

    if (update) {
      if (termin.size() != symbols.size()) {
        cerr << "Catastrophic: termin.size()!=symbols.size()" << endl;
        return;
      }
      j = termin.size();
      for (i = 0; i < j; i++)
        parList[wpidx].setData(i, 0, symbols[i].customNumber());
      wp.calcAllProperties();

    } else {
      termin.clear();
      wp.setDims(symbols.size(), 1);
      for (i = 0; i < symbols.size(); i++) {
        termin.push_back(symbols[i].getTime());
        wp.setData(i, 0, symbols[i].customNumber());
      }
      // check if new timeline already exist, add it
      int tlidx;
      if ((tlidx = timeLine.Exist(termin)) == -1)
        tlidx = addTimeLine(termin);
      wp.setTimeLineIndex(tlidx);
      wp.setId(p);
      wp.setType(SYMBOL);
      wp.calcAllProperties();
      addParameter(wp);
    }
  }
}

// fetch the parameters specified by inPars. outPars are the parameters
// actually fetched. If inPars is an empty vector, all parameters are
// fetched
// bug: if no parameters are added (because they don't exist on file),
// a timeline is still added
bool ptDiagramData::fetchDataFromFile(DataStream* pfile,
    const miPosition& stat, const ParId& modelid, const Model& renamemodelid,
    const miutil::miTime& start, const miutil::miTime& stop, const vector<ParId>& inPars,
    int* first, int* last, vector<ParId>& outPars, bool append, ErrorFlag* ef)
{
  int nread = 0, i;
  WeatherParameter wp;
  vector<miutil::miTime> tline;
  vector<int> pline;
  int index = 0;
  Range range;

#ifdef DEBUG
  cerr << "ptDiagramData::fetchDataFromFile" << endl;
#endif

  if (!append)
    cleanDataStructure_();

  // check if file is already opened, and open if not
  if (!pfile->isOpen())
    if (!pfile->openStream(ef))
      return false;

  // clean up the file object
  pfile->clean();

  // find station and read in data block
  int statIndex;
  if ((statIndex = pfile->findStation(stat.Name())) == -1) {
    *ef = DF_STATION_NOT_FOUND;
    return false;
  }
  if (!pfile->readData(statIndex, modelid, start, stop, ef))
    return false;

  // fetch textlines
  vector<std::string> tl;
  pfile->getTextLines(modelid, tl);
  textLines[modelid.model] = tl;

  // set station
  station = stat;

  vector<int> newtimelines;
  int tlIndex;
  if (inPars.size() == 0) { // get all parameters
    for (i = 0; i < pfile->numParameters(); i++) {
      if (pfile->getOnePar(i, wp, ef)) {
        if (renamemodelid != M_UNDEF){
          ParId id = wp.Id();
          id.model = renamemodelid;
          wp.setId(id);
        }
        // add timeline and progline
        if (!pfile->getTimeLine(wp.TimeLineIndex(), tline, pline, ef))
          break;
        if ((tlIndex = timeLine.Exist(tline)) == -1) {
          tlIndex = addTimeLine(tline);
          if (tlIndex == -1) {
            cerr << "Too many timelines! giving up this parameter" << endl;
            continue;
          }
          progLines.push_back(pline);
          newtimelines.push_back(tlIndex);
        }
        wp.setTimeLineIndex(tlIndex);
        index = addParameter(wp);
        ++nread;
        outPars.push_back(wp.Id());
      }
    }
  } else { // get the parameters specified in inPars
    int parIndex;
    for (i = 0; i < inPars.size(); i++) {
      if ((parIndex = pfile->findDataPar(inPars[i])) != -1) {
        if (pfile->getOnePar(parIndex, wp, ef)) {
          if (renamemodelid != M_UNDEF){
            ParId id = wp.Id();
            id.model = renamemodelid;
            wp.setId(id);
          }
          // add timeline and progline
          if (!pfile->getTimeLine(wp.TimeLineIndex(), tline, pline, ef))
            break;
          if ((tlIndex = timeLine.Exist(tline)) == -1) {
            tlIndex = addTimeLine(tline);
            if (tlIndex == -1) {
              cerr << "Too many timelines! giving up this parameter" << endl;
              continue;
            }
            progLines.push_back(pline);
            newtimelines.push_back(tlIndex);
          }
          wp.setTimeLineIndex(tlIndex);

          index = addParameter(wp);
          ++nread;
          outPars.push_back(wp.Id());
        }
      }
    }
  }
  // if no parameters are found, delete the timeline we added previously
  if (nread == 0) {
    for (i = 0; i < newtimelines.size(); i++) {
      deleteTimeLine(newtimelines[i]);
      progLines.pop_back();
    }
    *ef = DD_NO_PARAMETERS_FOUND;
    return false;
  }

  // set the index of the first and last elements appended
  *first = range.first = index + 1 - nread;
  *last = range.last = index;
  fetchRange.push_back(range);
  ++nfetches;

  if (nread < inPars.size()) { // check if all parameters are found
    *ef = DD_SOME_PARAMETERS_NOT_FOUND;
    return false;
  }

  *ef = OK;
  return true;
}

// fetch all parameters for first model for this station
bool ptDiagramData::fetchDataFromFile(DataStream* pfile,
    const miPosition& stat, const ParId& modelid, const Model& renamemodelid,
    const miutil::miTime& start, const miutil::miTime& stop, int* first, int* last,
    vector<ParId>& outPars, bool append, ErrorFlag* ef)
{
  vector<ParId> emptyvec;
  return fetchDataFromFile(pfile, stat, modelid, renamemodelid, start, stop,
      emptyvec, first, last, outPars, append, ef);
}

bool ptDiagramData::writeAllToFile(DataStream* pf, const std::string& modelName,
    ErrorFlag* ef)
{
  return false;
}

bool ptDiagramData::writeWeatherparametersToFile(DataStream* pfile,
    const miPosition& stat, const vector<int>& wpindexes, bool append,
    ErrorFlag* ef, bool complete_write, bool write_submodel)
{
  int i;
  // first save timelines, parameterlist, modellist and position-info
  // to DataStream. Then call DataStream.writeData


  if (!pfile->isOpen())
    if (!pfile->openStreamForWrite(ef)) {
      cerr << "DiagramData: feilet i stream->openStreamForWrite" << endl;
      return false;
    }

  // clean up data contents
  if (!append)
    pfile->clean();
  else
    pfile->cleanParData();

  // no data to write
  if (parList.size() == 0)
    return true;

  int statIndex = pfile->putStation(stat, ef);

  vector<int> pline;
  if (progLines.size())
    pline = progLines[0];

  if (!pfile->putTimeLine(timeLine, pline, ef)) {
    cerr << "DiagramData: Feilet i putTimeline" << endl;
    return false;
  }
  if (wpindexes.size() == 0) { // write all parameters
    for (i = 0; i < parList.size(); i++) {
      if (!pfile->putOnePar(parList[i], ef)) {
        cerr << "Kunne ikke skrive ut parameter:" << parList[i] << endl;
      }
    }
  } else { // write the parameters specified in wpindexes
    for (i = 0; i < wpindexes.size(); i++) {
      if (wpindexes[i] < 0 || wpindexes[i] >= parList.size()) {
        cerr << "Illegal parameterindex:" << wpindexes[i] << endl;
        continue;
      }
      if (!pfile->putOnePar(parList[wpindexes[i]], ef)) {
        cerr << "Kunne ikke skrive ut parameter:" << parList[wpindexes[i]]
                                                             << endl;
      }
    }
  }

  int modIndex = 0;
  if (!pfile->writeData(statIndex, modIndex, ef, complete_write, write_submodel)) {
    cerr << "DiagramData: Feilet i writeData" << endl;
    return false;
  }

  *ef = OK;
  return true;
}

// returns all timePoints belonging to timeline idx
void ptDiagramData::getTimeLine(const int idx, vector<miutil::miTime>& timePoints)
{
  timeLine.Timeline(idx, timePoints);
}

// returns all timePoints in the timeLine
void ptDiagramData::getTimeLine(vector<miutil::miTime>& timePoints)
{
  timePoints.clear();

  for (int i = 0; i < timeLine.size(); i++)
    timePoints.push_back(timeLine[i]);
}

// returns the indexes and the timepoints of the timeLine lying between
// start and stop.
void ptDiagramData::getTimeLine(const miutil::miTime& start, const miutil::miTime& stop,
    vector<int>& indexes, vector<miutil::miTime>& timePoints, int skip)
{
  // clear the input vectors if they're not empty
  indexes.clear();
  timePoints.clear();

  if (start < stop)
    return;

  // find index of the first element greater or equal to start
  int pos;

  if ((pos = timeLine.find_greater_or_equal(start)) != -1) {
    do {
      indexes.push_back(pos);
      timePoints.push_back(timeLine[pos]);
      pos += skip;
    } while (timeLine[pos] <= stop && pos < timeLine.size());
  }
}

// addTimeLine returns index of the new timeLine
int ptDiagramData::addTimeLine(const vector<miutil::miTime>& tl)
{
  return timeLine.addTimeline(tl);
}

// bug: the actual timepoints belonging to this timeline EXCLUSIVELY are
// NOT deleted, only the flag variables are set to false
void ptDiagramData::deleteTimeLine(int index)
{
  timeLine.deleteTimeline(index);
}

bool ptDiagramData::getProgLine(int i, vector<int>& prog, ErrorFlag* ef)
{
  if (i < 0 || i >= progLines.size()) {
    *ef = DD_RANGE_ERROR;
    return false;
  }
  prog = progLines[i];
  *ef = OK;
  return true;
}

bool ptDiagramData::addTimePoint(const miutil::miTime& tp, int tlIndex, ErrorFlag *ef)
{
  *ef = OK;
  bool ok = timeLine.insert(tp, tlIndex);
  if (!ok)
    *ef = DD_RANGE_ERROR;
  return ok;
}



// make a vector from two components
bool ptDiagramData::makeVector(const ParId& comp1, const ParId& comp2,
    const ParId& result, bool polar)
{
  int cidx1, cidx2, i;
  ErrorFlag ef;
  WeatherParameter wp1, wp2, wpresult;

  if (findParameter(comp1, cidx1, &ef) && findParameter(comp2, cidx2, &ef)) {
    if (copyParameter(cidx1, wp1, &ef) && copyParameter(cidx2, wp2, &ef)) {
      if (wp1.size() != wp2.size()) {
        cerr << "wp sizes not matching making vector [" << result << "] from ["
            << comp1 << "] (size:" << wp1.size() << ") and [" << comp2
            << "] (size:" << wp2.size() << ")" << endl;
        return false;
      }
      wpresult.setDims(wp1.size(), 2);
      for (i = 0; i < wp1.size(); i++)
        wpresult.setData(i, 0, wp1.Data(i));
      for (i = 0; i < wp2.size(); i++)
        wpresult.setData(i, 1, wp2.Data(i));
      ParId pid = result;
      if (pid.run == R_UNDEF) // set actual run from first component
        pid.run = wp1.Id().run;
      wpresult.setId(pid);
      wpresult.setPolar(polar);
      wpresult.setTimeLineIndex(wp1.TimeLineIndex());

      wpresult.calcAllProperties();
      addParameter(wpresult);
      return true;
    }
  }
  return false;
}

// make a polar vector from two cartesian components.
// zangle is startangle and rotsign is sign of rotation (+1=clockwise)
bool ptDiagramData::makePolarVector(const ParId& comp1, const ParId& comp2,
    const ParId& result, const float& zangle, const int& rotsign)
{
  int cidx1, cidx2, i;
  ErrorFlag ef;
  WeatherParameter wp1, wp2, wpresult;

  if (findParameter(comp1, cidx1, &ef) && findParameter(comp2, cidx2, &ef)) {
    if (copyParameter(cidx1, wp1, &ef) && copyParameter(cidx2, wp2, &ef)) {
      if (wp1.size() != wp2.size()) {
        cerr << "wp sizes not matching making polar vector [" << result
            << "] from [" << comp1 << "] (size:" << wp1.size() << ") and ["
            << comp2 << "] (size:" << wp2.size() << ")" << endl;
        return false;
      }
      wpresult.setDims(wp1.size(), 2);
      for (i = 0; i < wp1.size(); i++)
        wpresult.setData(i, 0, wp1.Data(i));
      for (i = 0; i < wp2.size(); i++)
        wpresult.setData(i, 1, wp2.Data(i));
      ParId pid = result;
      if (pid.run == R_UNDEF) // set actual run from first component
        pid.run = wp1.Id().run;
      wpresult.setId(pid);
      wpresult.setPolar(true);
      wpresult.setTimeLineIndex(wp1.TimeLineIndex());
      // transform to polar coordinates
      int nump = wpresult.Npoints();
      float dd, ff, uu, vv;
      for (i = 0; i < nump; i++) {
        uu = wpresult.Data(i);
        vv = wpresult.Data(i + nump);
        ff = sqrt(uu * uu + vv * vv);
        dd = atan2f(uu, vv) + M_PI;
        if (dd < 0)
          dd += M_PI * 2;
        else if (dd > M_PI*2)
          dd -= M_PI * 2;
        dd *= 180 / M_PI;
        wpresult.setData(i,0, ff );
        wpresult.setData(i,1, dd );
      }
      wpresult.calcAllProperties();
      addParameter(wpresult);
      return true;
    }
  }
  return false;
}

// make a cartesian vector from two polar components.
// zangle is startangle and rotsign is sign of rotation (+1=clockwise)
bool ptDiagramData::makeCartesianVector(const ParId& comp1, const ParId& comp2,
    const ParId& result, const float& zangle, const int& rotsign)
{
  //cerr << "Making cartesian vector:" << result << endl;

  int cidx1, cidx2, i;
  ErrorFlag ef;
  WeatherParameter wp1, wp2, wpresult;

  if (findParameter(comp1, cidx1, &ef) && findParameter(comp2, cidx2, &ef)) {
    if (copyParameter(cidx1, wp1, &ef) && copyParameter(cidx2, wp2, &ef)) {
      if (wp1.size() != wp2.size()) {
        cerr << "wp sizes not matching making cartesian vector [" << result
            << "] from [" << comp1 << "] (size:" << wp1.size() << ") and ["
            << comp2 << "] (size:" << wp2.size() << ")" << endl;
        return false;
      }
      //cerr << "here we go.." << endl;
      wpresult.setDims(wp1.size(), 2);
      for (i = 0; i < wp1.size(); i++)
        wpresult.setData(i, 0, wp1.Data(i));
      for (i = 0; i < wp2.size(); i++)
        wpresult.setData(i, 1, wp2.Data(i));
      ParId pid = result;
      if (pid.run == R_UNDEF) // set actual run from first component
        pid.run = wp1.Id().run;
      wpresult.setId(pid);
      wpresult.setPolar(false);
      wpresult.setTimeLineIndex(wp1.TimeLineIndex());
      // transform to cartesian coordinates
      int nump = wpresult.Npoints();
      float dd, ff, uu, vv;
      for (i = 0; i < nump; i++) {
        ff = wpresult.Data(i);
        dd = zangle + rotsign * wpresult.Data(i + nump) * M_PI / 180.0;
        uu = -cosf(dd) * ff;
        vv = -sinf(dd) * ff;
        wpresult.setData(i, uu);
        wpresult.setData(i + nump, vv);
      }
      wpresult.calcAllProperties();
      addParameter(wpresult);
      return true;
    }
  }
  return false;
}

bool ptDiagramData::splitVector(const ParId& vect, const ParId& comp1,
    const ParId& comp2)
{
  int cidx, i, npoints;
  ErrorFlag ef;
  WeatherParameter wp, wpvect;

  if (findParameter(vect, cidx, &ef)) {
    if (copyParameter(cidx, wpvect, &ef)) {
      if (wpvect.Ndim() == 2) {
        npoints = wpvect.Npoints();
        wp.setDims(npoints, 1);
        wp.setPolar(false);
        wp.setTimeLineIndex(wpvect.TimeLineIndex());

        ParId pid = comp1;
        if (pid.run == R_UNDEF) // set actual run from component
          pid.run = wpvect.Id().run;
        wp.setId(pid);
        for (i = 0; i < npoints; i++)
          wp.setData(i, wpvect.Data(i));
        wp.calcAllProperties();
        addParameter(wp);

        pid = comp2;
        if (pid.run == R_UNDEF) // set actual run from component
          pid.run = wpvect.Id().run;
        wp.setId(pid);
        for (i = 0; i < npoints; i++)
          wp.setData(i, wpvect.Data(i + npoints));
        wp.calcAllProperties();
        addParameter(wp);
        return true;
      }
    }
  }
  return false;
}

set<Model> ptDiagramData::allModels()
{
  set<Model> models;

  int n = parList.size();
  for (int i = 0; i < n; i++)
    models.insert(parList[i].Id().model);

  return models;
}

vector<std::string> ptDiagramData::getAllTextLines()
{
  vector<std::string> str;
  map<std::string, vector<std::string> >::iterator itr;
  for (itr = textLines.begin(); itr != textLines.end(); itr++)
    str.insert(str.end(), itr->second.begin(), itr->second.end());
  return str;
}

vector<std::string> ptDiagramData::getTextLines(const std::string& modelname)
{
  vector<std::string> str = textLines[modelname];
  return str;
}



bool ptDiagramData::fetchDataFromWDB(pets::WdbStream* wdb,float lat, float lon,
    const std::string& model, miutil::miTime run,vector<ParId>& inpars, vector<ParId>& outpars,
    unsigned long& readtime, const std::string& stationname)
{


  int nread = 0, i;

  vector<miutil::miTime> tline;
  vector<int> pline;
  int index = 0;
  Range range;

  cleanDataStructure_();


  wdb->clean();

  // find station and read in data block
  try {
    if (!wdb->readWdbData(lat,lon, model,run,inpars,outpars,readtime))
      return false;
  } catch(exception& e) {
    cerr << "WDB::READDATA FAILED: " << e.what() << endl;
    return false;
  }


  vector<int> newtimelines;
  int tlIndex;
  // get all parameters
  for (i = 0; i < wdb->numParameters(); i++) {
    WeatherParameter wp;
    if (wdb->getOnePar(i, wp)) {

      // add timeline and progline
      if (!wdb->getTimeLine(wp.TimeLineIndex(), tline, pline))
        break;

      if ((tlIndex = timeLine.Exist(tline)) == -1) {
        tlIndex = addTimeLine(tline);
        if (tlIndex == -1) {
          cerr << "Too many timelines! giving up this parameter" << endl;
          continue;
        }
        progLines.push_back(pline);
        newtimelines.push_back(tlIndex);
      }
      wp.setTimeLineIndex(tlIndex);
      index = addParameter(wp);
      ++nread;
    }
  }

  // if no parameters are found, delete the timeline we added previously
  if (nread == 0) {
    for (i = 0; i < newtimelines.size(); i++) {
      deleteTimeLine(newtimelines[i]);
      progLines.pop_back();
    }
    return false;
  }

  // set the index of the first and last elements appended
  //*first = range.first = index + 1 - nread;
  //*last = range.last = index;
  range.first = index + 1 - nread;
  range.last = index;

  fetchRange.push_back(range);
  ++nfetches;

  station.setLat(lat);
  station.setLon(lon);
  miCoordinates c=station.Coordinates();
  if(not stationname.empty())
    station.setName(stationname);
  else
    station.setName( c.str() );

  return true;
}

bool ptDiagramData::fetchDataFromKlimaDB(pets::KlimaStream* klima,
    vector<ParId>& inpars, vector<ParId>& outpars, miutil::miTime fromTime, miutil::miTime toTime)
{

  vector<miutil::miTime> tline;
  vector<int> pline;

  klima->clean();

  // find station and read in data block
  try {
    if (!klima->readKlimaData(inpars,outpars,fromTime,toTime))
      return false;
  } catch(exception& e) {
    cerr << "KLIMA::READDATA FAILED: " << e.what() << endl;
    return false;
  }


  vector<int> newtimelines;
  int tlIndex,index;
  int nread=0;
  // get all parameters
  for (signed int i = 0; i < klima->numParameters(); i++) {
    WeatherParameter wp;
    if (klima->getOnePar(i, wp)) {

      // add timeline and progline
      if (!klima->getTimeLine(wp.TimeLineIndex(), tline, pline))
        break;

      if ((tlIndex = timeLine.Exist(tline)) == -1) {
        tlIndex = addTimeLine(tline);
        if (tlIndex == -1) {
          cerr << "Too many timelines! giving up this parameter" << endl;
          continue;
        }
        progLines.push_back(pline);
        newtimelines.push_back(tlIndex);
      }
      wp.setTimeLineIndex(tlIndex);
      index = addParameter(wp);
      ++nread;
    }
  }



  // set the index of the first and last elements appended
  //first = range.first = index + 1 - nread;
  //last = range.last = index;
  Range range;
  range.first = index + 1 - nread;
  range.last = index;

  fetchRange.push_back(range);
  ++nfetches;

  return true;
}



bool ptDiagramData::fetchDataFromFimex(pets::FimexStream* fimex, double lat, double lon, miutil::miString stationname,
    std::vector<ParId>& inpars, std::vector<ParId>& outpars)
{

  int nread = 0, i;

  vector<miutil::miTime> tline;
  vector<int> pline;
  int index = 0;
  Range range;


  vector<int> newtimelines;

  // get all parameters

  int tlIndex;
  // get all parameters
  for (i = 0; i < fimex->numParameters(); i++) {
    WeatherParameter wp;
    if (fimex->getOnePar(i, wp)) {
      // add timeline and progline
      if (!fimex->getTimeLine(wp.TimeLineIndex(), tline, pline))
        break;

      if ((tlIndex = timeLine.Exist(tline)) == -1) {
        tlIndex = addTimeLine(tline);
        if (tlIndex == -1) {
          cerr << "Too many timelines! giving up this parameter" << endl;
          continue;
        }
        progLines.push_back(pline);
      }
      wp.setTimeLineIndex(tlIndex);
      index = addParameter(wp);
      ++nread;
    }
  }
  // set the index of the first and last elements appended
  //*first = range.first = index + 1 - nread;
  //*last = range.last = index;
  range.first = index + 1 - nread;
  range.last = index;

  fetchRange.push_back(range);
  ++nfetches;

  station.setLat(lat);
  station.setLon(lon);
  miCoordinates c=station.Coordinates();
  if(!stationname.empty())
    station.setName(stationname);
  else
    station.setName( c.str() );

  return true;

}

