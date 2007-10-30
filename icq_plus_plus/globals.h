// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin ÷berg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006,2007 Joe Kucera
// Copyright © 2006,2007 [sss], chaos.persei, [sin], Faith Healer, Theif, nullbie
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
// File name      : $Source$
// Revision       : $Revision: 48 $
// Last change on : $Date: 2007-08-26 16:12:47 +0300 (–í—Å, 26 –∞–≤–≥ 2007) $
// Last change by : $Author: sss123next $
//
// DESCRIPTION:
//
//  Contains global variables declarations.
//
// -----------------------------------------------------------------------------


#ifndef __GLOBALS_H
#define __GLOBALS_H

typedef char uid_str[MAX_PATH];

// Defaults
#define DEFAULT_QUICK_LOGIN         1
#define DEFAULT_DCTYPE              2 //auth req
#define DEFAULT_VISIBILITY          0
#define DEFAULT_OEM_CONV            0
#define DEFAULT_RENAME_SERVER_NICKS 1
#define DEFAULT_UPLOAD_DELAY        2
#define DEFAULT_PRIVACY_ENABLED     1
#define DEFAULT_SHOW_IDLE_TIME      1
#define DEFAULT_EXTSEARCH_ENABLED   1
#define DEFAULT_TOOLS               0
#define DEFAULT_VISIBILITY_TOOLS    0
#define DEFAULT_EXT_STATUS          0
#define DEFAULT_VISIBLEMODE         4
#define DEFAULT_INVISIBLEMODE       3

// from init.c
HINSTANCE hInst;
char gpszICQProtoName[MAX_PATH];

HANDLE ghServerNetlibUser;
HANDLE ghDirectNetlibUser;

// from init.h
BYTE gbGatewayMode;
BYTE gbSecureLogin;
BYTE gbAimEnabled;
BYTE gbUtfEnabled;
WORD gwAnsiCodepage;
BYTE gbDCMsgEnabled;
BYTE gbTempVisListEnabled;
BYTE gbSsiEnabled;
BYTE gbAvatarsEnabled;
BYTE gbXStatusEnabled;
BYTE gbTzerEnabled;
DWORD MIRANDA_VERSION;

int gpiXStatusIconsIdx[38];
BOOL gpbXStatusIconsValid[38];

// from icqosc_svcs.c
int gnCurrentStatus;
DWORD dwLocalUIN;

char gpszPassword[16];
BYTE gbRememberPwd;

BYTE gbUnicodeAPI;
BYTE gbUnicodeCore;

// Globals
BYTE gbAimEnabled;
BYTE gbSsiEnabled;
BYTE gbShowIdle;
BYTE gbVisibility;
BYTE gbMtnEnabled;
BYTE gbAdvSearch;
BYTE gbWebAware;
//BYTE gbTools;
//BYTE gbVTools;
BYTE gbExtStatus;
BYTE gbRcvUnicode;
BYTE gbSendUnicode;
BYTE gbUnicodeDecode;
BYTE gbVisibleMode;
BYTE gwVersion;
BYTE gbQuickLogin;
BYTE gbSecureLogin;
BYTE gbSavePass;
BYTE gbUseServerNicks;
BYTE gbRenameServerNicks;
BYTE gbServerAddRemove;
BYTE gbSlowSend;
BYTE gbSetStatus;
BYTE gbCustomCapEnabled;
BYTE gbHideIdEnabled;
BYTE gbRTFEnabled;
BYTE gbVerEnabled;
BYTE gbScan;


DWORD gdwFP1;
DWORD gdwFP2;
DWORD gdwFP3;

DWORD gdwUpdateThreshold;

// from fam_04message.c
typedef struct icq_mode_messages_s
{
  char* szOnline;
  char* szAway;
  char* szNa;
  char* szDnd;
  char* szOccupied;
  char* szFfc;
} icq_mode_messages;

icq_mode_messages modeMsgs;
CRITICAL_SECTION modeMsgsMutex;

struct LIST_INTERFACE listInterface;
SortedList *lstCustomCaps;



#endif /* __GLOBALS_H */
