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
// File name      : $URL: http://sss.chaoslab.ru:81/svn/icqjplus/branches/0.5_branch/capabilities.h $
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
// DESCRIPTION:
//
//  Contains helper functions to handle oscar user capabilities.
//
// -----------------------------------------------------------------------------

#ifndef __CAPABILITIES_H
#define __CAPABILITIES_H 1

typedef struct 
{
	HANDLE hIcon;
	int type;
	char name[128];
	char caps[128];
} ICQ_CAPINFO;

typedef struct tagSTDCAPINFO
{
	const char *name;
	int type;
	const BYTE *caps;
	int capSize;
} STDCAPINFO;

extern const capstr capXStatus[XSTATUS_COUNT];
extern const char *nameXStatus[XSTATUS_COUNT];
extern const int moodXStatus[XSTATUS_COUNT];

extern const capstr capShortCaps;

extern const capstr capSrvRelay;
extern const capstr capNewCap;
extern const capstr capUTF;
extern const capstr capTyping;
extern const capstr capXtraz;
extern const capstr capOscarFile;
extern const capstr capPush2Talk;
extern const capstr capRTF;
extern const capstr capXtrazChat;
extern const capstr capVoiceChat;
extern const capstr capIcqDevil;
extern const capstr capIcqDirect;

extern const capstr capMirandaIm;
extern const capstr capMimMobile;
extern const capstr capMimPack;
extern const capstr capIcqJen;
extern const capstr capIcqJs7;
extern const capstr capIcqJp;
extern const capstr capIcqJs7s;
extern const capstr capIcqJs7old;
extern const capstr capIcqJSin;
extern const capstr capAimOscar;
extern const capstr capTrillian;
extern const capstr capTrilCrypt;
extern const capstr capSim;
extern const capstr capSimOld;
extern const capstr capLicq;
extern const capstr capKopete;
extern const capstr capmIcq;
extern const capstr capClimm;
extern const capstr capAndRQ;
extern const capstr capRAndQ;
extern const capstr capmChat;
extern const capstr capJimm;
extern const capstr capwJimm;
extern const capstr capBayan;
extern const capstr capCorePager;
extern const capstr capDiChat;
extern const capstr capNaim;
extern const capstr capAnastasia;
extern const capstr capQip;
extern const capstr capQipPDA;
extern const capstr capQipMobile;
extern const capstr capQipInfium;
extern const capstr capQipPlugins;
extern const capstr capQip_1;
extern const capstr capQipSymbian;
extern const capstr capVmIcq;
extern const capstr capSmapeR;
extern const capstr capPigeon;
extern const capstr capDigsbyBeta;
extern const capstr capDigsby;
extern const capstr capJapp;
extern const capstr capIMPlus;
extern const capstr capYapp;
extern const capstr capQutIm;
extern const capstr capJabberJIT;
extern const capstr capIcqKid2;
extern const capstr capWebIcqPro;
extern const capstr capIm2;
extern const capstr capMacIcq;
extern const capstr capIs2001;
extern const capstr capIs2002;
extern const capstr capComm20012;
extern const capstr capStrIcq;
//extern const capstr capAimIcon;
//extern const capstr capAimDirect;
//extern const capstr capAimFileShare;
extern const shortcapstr capAimIcon;
extern const shortcapstr capAimDirect;
extern const shortcapstr capAimFileShare;
extern const shortcapstr capAimSmartCaps;
extern const capstr capIcqLite;
extern const capstr capXtrazVideo; // TODO: dummy of IcqLite
extern const capstr capIcqLiteNew;
extern const capstr capAimChat;
extern const capstr capOscarChat; // TODO: dummy of capAimChat
extern const capstr capUim;
extern const capstr capRambler;
extern const capstr capAbv;
extern const capstr capNetvigator;
extern const capstr captZers;
extern const capstr capHtmlMsgs;
extern const capstr capLiveVideo;
extern const capstr capSimpLite;
extern const capstr capSimpPro;
extern const capstr capIMsecure;
extern const capstr capMsgType2;
extern const capstr capAimIcq;
extern const capstr capAimVoice;
extern const capstr capAimAudio;
extern const capstr capPalmJicq;
extern const capstr capInluxMsgr;
extern const capstr capMipClient;
extern const capstr capIMadering;
extern const capstr capAimAddins;
extern const capstr capAimContactSnd;
extern const capstr capAimUnk2;
extern const capstr capAimSendbuddylist;
//
extern const capstr capIMSecKey1;
extern const capstr capIMSecKey2;
extern const capstr capUnknown1;

extern char* cliLibicq2k;
extern char* cliLicqVer;
extern char* cliCentericq;
extern char* cliLibicqUTF;
extern char* cliTrillian;
extern char* cliTrillian4;
extern char* cliQip;
extern char* cliIM2;
extern char* cliSpamBot;

#endif /* __CAPABILITIES_H */