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
const char * VGPOSNAME = "List of positions";
const char * VGPARNAME = "Parameters";
const char * VGMODNAME = "Models";
const char * VGPOSGROUP = "Positions";
/* #define VGLEV1NAME "Level 1" */

/* Field names */
const char * FNPOSNAME = "Name";
const char * FNGEOPOS = "Position";
const char * FNPOSREF = "Reference";
const char * FNTOPOGRAPHY = "Topography";

const char * FNMODID = "Model ID";
const char * FNMODNAME = "Name";
const char * FNMODRUN = "Run";
const char * FNMODTXT = "Info";

const char * FNALIAS = "Short parameter name";
const char * FNNAME = "Parameter name";
const char * FNUNIT = "Parameter unit";
const char * FNNUM = "Parameter number";
const char * FNSCALE = "Scale";
const char * FNSIZE = "Size";
const char * FNORDER = "Order";
const char * FNTYPE = "Data type";
const char * FNPLOT = "Plot type";

const char * FNTIME = "Time";
const char * FNPROG = "Prog";
const char * FNLEV = "Level";
const char * FNSMOD = "Submodel";
const char * FNDATA = "Data";

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
