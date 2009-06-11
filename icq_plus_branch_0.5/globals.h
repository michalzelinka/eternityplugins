// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $URL: https://miranda.svn.sourceforge.net/svnroot/miranda/trunk/miranda/protocols/IcqOscarJ/globals.h $
// Revision       : $Revision: 8822 $
// Last change on : $Date: 2009-01-11 18:17:05 +0100 (Sun, 11 Jan 2009) $
// Last change by : $Author: jokusoftware $
//
// DESCRIPTION:
//
//  Contains global variables declarations.
//
// -----------------------------------------------------------------------------


#ifndef __GLOBALS_H
#define __GLOBALS_H

typedef char uid_str[MAX_PATH];

// from init.cpp
extern HINSTANCE hInst;
extern DWORD MIRANDA_VERSION;
extern BYTE gbUnicodeCore;

extern IcqIconHandle hStaticIcons[];

extern const int moodXStatus[];

// from fam_04message.cpp
struct icq_mode_messages
{
  char *szOffline;
  char *szOnline;
  char *szAway;
  char *szNa;
  char *szDnd;
  char *szOccupied;
  char *szFfc;
};


#endif /* __GLOBALS_H */
