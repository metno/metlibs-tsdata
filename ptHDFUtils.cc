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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <ctype.h>
#include <hdf.h>

#include "ptHDFUtils.h"

/* Function rtrim:
   Remove trailing blanks from a string and pad with \0s. */

void rtrim(char *s)
{
  int i;

  for (i=strlen(s)-1; isspace(*(s+i)) && i>=0; *(s+i--)='\0');
}

/* HDF functions */

/* Function findVG:
   Search sequentially for vgroup named `name' and return its
   reference number. */

int32 findVG(int32 fid, const char *name)
{
  int32 vgref=-1, vgid;
  char  vgname[VGNAMELENMAX+1];

  while((vgref=Vgetid(fid,vgref))!=-1) {
    vgid=Vattach(fid,vgref,"r");
    Vgetname(vgid,vgname);
    Vdetach(vgid);
    if (strcmp(vgname,name)==0)
      break;
  }
  return vgref;
}

/* Function copyVGtree:
   Traverse a branched Vgroup structure (with innode in file infid as
   the top node) recursively and make a copy to outnode in file
   outfid */

void copyVGtree(int32 infid, int32 innode, int32 outfid, int32 outnode)
{
  char  vgname[VGNAMELENMAX+1];
  int   i;
  int32 vgin, vgout;
  int32 vdin, vdout;
  int32 npairs;
  int32 *tags, *refs;

  npairs=Vntagrefs(innode);
  tags=(int32*)malloc(npairs*sizeof(int32));
  refs=(int32*)malloc(npairs*sizeof(int32));
  Vgettagrefs(innode,tags,refs,npairs);

  for (i=0; i<npairs; i++) {
    if (tags[i]==DFTAG_VG) {
      Vgetname(vgin=Vattach(infid,refs[i],"r"),vgname);
      vgout=Vattach(outfid,-1,"w");
      Vsetname(vgout,vgname);
      Vinsert(outnode,vgout);

      copyVGtree(infid,vgin,outfid,vgout);
    }
     else if (tags[i]==DFTAG_VH) {
       vdin=VSattach(infid,refs[i],"r");
       vdout=VSattach(outfid,-1,"w");
       copyVD(vdin,vdout);
       Vinsert(outnode,vdout);
       VSdetach(vdout);
       VSdetach(vdin);
     }
  }

  Vdetach(outnode);
  Vdetach(innode);
  free(refs);
  free(tags);
}

int copyVD(int32 in, int32 out)
{
  int   i, fnsize=0, nfields=VFnfields(in);
  int32 nrec, interlace, recsz;
  char  *fields, fname[FNLENMAX], vdname[VSNAMELENMAX+1];
  uint8 *buf;

  for(i=0; i<nfields; i++) {
    fnsize+=strlen(strncpy(fname,VFfieldname(in,i),FNLENMAX))+1;
    if (VSfdefine(out,fname,VFfieldtype(in,i),VFfieldorder(in,i))==-1)
      return -1;
  }
  fields=(char*)malloc(fnsize*sizeof(char));

  VSinquire(in,&nrec,&interlace,fields,&recsz,vdname);
  buf=(uint8*)malloc(nrec*recsz*sizeof(uint8));

  VSsetfields(in,fields);
  VSread(in,buf,nrec,interlace);

  VSsetfields(out,fields);
  VSwrite(out,buf,nrec,interlace);
  VSsetname(out,vdname);

  free(buf);
  free(fields);

  return 0;
}

