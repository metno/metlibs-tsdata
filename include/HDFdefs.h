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


#ifndef _HDFdefs_h
#define _HDFdefs_h

#ifdef __cplusplus
extern "C" {
#endif

/** HDFdefs.h
 ** 
 ** Definitions for standard pets HDF file.
 **
 ** LBS  5 Dec 1996
 ** LBS 13 Nov 1997 -- major revision
 ** LBS 17 Mar 1998 -- major revision
 **
 **/

/*
 * String constants
 */

/* Vgroups, vdatas */
#define VGPOSNAME  "List of positions"
#define VGPARNAME  "Parameters"
#define VGMODNAME  "Models"
#define VGPOSGROUP "Positions"
/* #define VGLEV1NAME "Level 1" */

/* Field names */
#define FNPOSNAME  "Name"
#define FNGEOPOS   "Position"
#define FNPOSREF   "Reference"
#define FNTOPOGRAPHY "Topography"

#define FNMODID    "Model ID"
#define FNMODNAME  "Name"
#define FNMODRUN   "Run"
#define FNMODTXT   "Info"

#define FNALIAS    "Short parameter name"
#define FNNAME     "Parameter name"
#define FNUNIT     "Parameter unit"
#define FNNUM      "Parameter number"
#define FNSCALE    "Scale"
#define FNSIZE     "Size"
#define FNORDER    "Order"
#define FNTYPE     "Data type"
#define FNPLOT     "Plot type"

#define FNTIME     "Time"
#define FNPROG     "Prog"
#define FNLEV      "Level"
#define FNSMOD     "Submodel"
#define FNDATA     "Data"

/*
 * Other constants
 */

#define MAXHDF 32  /* Max # of HDF input files */
#define MAXFN  30  /* Max length of a field name */

#define MAXPOS 20000
#define MAXMOD 100
#define MAXTIM 100
#define MAXPAR 500
#define MAXLEV 100
#define MAXSMO 100

#define undefValue -32767

#ifdef __cplusplus
}
#endif

#endif /* _HDFdefs_h */
