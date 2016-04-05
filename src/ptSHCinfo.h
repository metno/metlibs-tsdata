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


#ifndef PTSHCINFO_H
#define PTSHCINFO_H

#include <string>
#include <vector>

class SHCdir {
public:
  float low;
  float high;
  float value;
};

class SHClevel {
public:
  int level;
  std::vector<SHCdir> dirs;
  int dirIndex(const float& angle);
};

class SHCinfo {
public:
  std::string name;
  std::vector<SHClevel> levels;
  int levelIndex(const int& level);
  int lowLevel();
  int highLevel();
};

class SHCcollection {
public:
  std::vector<SHCinfo> list;
  SHCinfo total;
  bool readList(const std::string& filename);
  SHCinfo totalSHC(){return total;}
};

#endif
