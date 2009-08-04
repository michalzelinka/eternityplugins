// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
// Copyright © 2006-2009 BM, SSS, jarvis, S!N, persei and others
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
// File name      : $URL: http://sss.chaoslab.ru:81/svn/icqjplus/branches/0.5_branch/icq_popups.h $
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
// DESCRIPTION:
//
//  Headers for PopUp Plugin support
//
// -----------------------------------------------------------------------------

#ifndef __ICQ_POPUPS_H
#define __ICQ_POPUPS_H


#define POPTYPE_SPAM            254  // this is for spambots
#define POPTYPE_UNKNOWN         253
#define POPTYPE_VISIBILITY      252
#define POPTYPE_INFOREQUEST     251
#define POPTYPE_CLIENTCHANGE    250
#define POPTYPE_IGNORECHECK     249
#define POPTYPE_SELFREMOVE      248
#define POPTYPE_AUTH            247
#define POPTYPE_XUPDATER        246
#define POPTYPE_FOUND           245
#define POPTYPE_SCAN            244

void InitPopUps();


#endif /* __ICQ_POPUPS_H */
