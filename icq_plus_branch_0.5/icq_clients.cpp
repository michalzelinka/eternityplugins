// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
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
// File name      : $URL: http://sss.chaoslab.ru:81/svn/icqjplus/branches/0.5_branch/icq_clients.cpp $
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
// DESCRIPTION:
//
//  Provides capability & signature based client detection
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

capstr* MatchCap(BYTE* buf, int bufsize, const capstr* cap, int capsize)
{
	while (bufsize>0) // search the buffer for a capability
	{
		if (!memcmp(buf, cap, capsize))
		{
			return (capstr*)buf; // give found capability for version info
		}
		else
		{
			buf += 0x10;
			bufsize -= 0x10;
		}
	}
	return 0;
}

capstr* MatchShortCap(BYTE *buf, int bufsize, const shortcapstr *cap)
{
	capstr fullCap;

	memcpy(fullCap, capShortCaps, 0x10);
	fullCap[2] = (*cap)[0];
	fullCap[3] = (*cap)[1];

	return MatchCap(buf, bufsize, &fullCap, 0x10);
}

#define MIRANDA_VERSION_TYPE_REGULAR    0
#define MIRANDA_VERSION_TYPE_SPECIAL    1

static void makeClientVersion(char *szBuf, const char* szClient, unsigned v1, unsigned v2, unsigned v3, unsigned v4, int type = MIRANDA_VERSION_TYPE_REGULAR)
{
	BOOL isAlpha = ( (v1 >> 4) == 0x8 );
	BOOL isPreview = !isAlpha && v4;
	v1 &= 0x7F;

	if (type == MIRANDA_VERSION_TYPE_SPECIAL)
	{
		if (isAlpha)
		{
			null_snprintf(szBuf, 64, "%s%u.%u alpha build #%u", szClient, v1, v2, v4);
			return;
		}
		else if (isPreview)
		{
			if (v3)
				null_snprintf(szBuf, 64, "%s%u.%u.%u preview build #%u", szClient, v1, v2, v3, v4);
			else
				null_snprintf(szBuf, 64, "%s%u.%u preview build #%u", szClient, v1, v2, v4);
			return;
		}
	}

	if (v4)
		null_snprintf(szBuf, 64, "%s%u.%u.%u.%u", szClient, v1, v2, v3, v4);
	else if (v3)
		null_snprintf(szBuf, 64, "%s%u.%u.%u", szClient, v1, v2, v3);
	else
		null_snprintf(szBuf, 64, "%s%u.%u", szClient, v1, v2);

	if (isAlpha) strcat(szBuf, " alpha");
}

static void verToStr(char* szStr, int v, int type = MIRANDA_VERSION_TYPE_REGULAR)
{
	char szVer[64];

	makeClientVersion(szVer, "", (v>>24)&0xFF, (v>>16)&0xFF, (v>>8)&0xFF, v&0xFF, type); 
	strcat(szStr, szVer);
}

static char* MirandaVersionToStringEx(char* szStr, int bUnicode, const char* szPlug, int v, int m)
{
	if (!v) // this is not Miranda
		return NULL;

	strcpy(szStr, "Miranda IM ");

	if (!m && v == 1)
		verToStr(szStr, 0x80010200);
	else if (!m && (v&0x7FFFFFFF) <= 0x030301)
		verToStr(szStr, v);
	else {
		if (m) {
			verToStr(szStr, m, MIRANDA_VERSION_TYPE_SPECIAL);
			strcat(szStr, " ");
		}
		if (bUnicode)
			strcat(szStr, "Unicode ");

		strcat(szStr, "(");
		strcat(szStr, szPlug);

		if ( (v >> 24) == 0x81 ) // detect and reset BM Mod flag
		{
			strcat(szStr, " BM Mod");
			v = v & 0xF0FFFFFF;
		}

		strcat(szStr, " v");
		verToStr(szStr, v);
		strcat(szStr, ")");
	}

	return szStr;
}

char* MirandaVersionToString(char* szStr, int bUnicode, int v, int m)
{
	return MirandaVersionToStringEx(szStr, bUnicode, "ICQ", v, m);
}

char* MirandaModToString(char* szStr, capstr* capId, int bUnicode, const char* szModName)
{ // decode icqj mod version
	char* szClient;
	DWORD mver = (*capId)[0x4] << 0x18 | (*capId)[0x5] << 0x10 | (*capId)[0x6] << 8 | (*capId)[0x7];
	DWORD iver = (*capId)[0x8] << 0x18 | (*capId)[0x9] << 0x10 | (*capId)[0xA] << 8 | (*capId)[0xB];
	DWORD scode = (*capId)[0xC] << 0x18 | (*capId)[0xD] << 0x10 | (*capId)[0xE] << 8 | (*capId)[0xF];

	szClient = MirandaVersionToStringEx(szStr, bUnicode, szModName, iver, mver);
	if (scode == 0x5AFEC0DE)
	{
		strcat(szClient, " + SecureIM");
	}
	return szClient;
}

char* CIcqProto::detectUserClient(HANDLE hContact, DWORD dwUin, WORD wUserClass, WORD wVersion, DWORD dwFT1, DWORD dwFT2, DWORD dwFT3, DWORD dwOnlineSince, BYTE bDirectFlag, DWORD dwDirectCookie, DWORD dwWebPort, BYTE* caps, WORD wLen, BYTE* bClientId, char* szClientBuf)
{
	LPSTR szClient = NULL;
	int bMirandaIM = FALSE;

	*bClientId = CLID_ALTERNATIVE; // Most clients does not tick as MsgIDs

	// Is this a Miranda IM client?
	if (dwFT1 == 0xffffffff)
	{
		if (dwFT2 == 0xffffffff)
		{ // This is Gaim not Miranda
			szClient = "Gaim";
		}
		else if (!dwFT2 && wVersion == 7)
		{ // This is WebICQ not Miranda
			szClient = "WebICQ";
		}
		else if (!dwFT2 && dwFT3 == 0x3B7248ED)
		{ // And this is most probably Spam Bot
			szClient = cliSpamBot;
		}
		else 
		{ // Yes this is most probably Miranda, get the version info
			szClient = MirandaVersionToString(szClientBuf, 0, dwFT2, 0);
			*bClientId = CLID_MIRANDA;
			bMirandaIM = TRUE;
		}
	}
	else if (dwFT1 == 0x7fffffff)
	{ // This is Miranda with unicode core
		szClient = MirandaVersionToString(szClientBuf, 1, dwFT2, 0);
		*bClientId = CLID_MIRANDA;
		bMirandaIM = TRUE;
	}
	else if ((dwFT1 & 0xFF7F0000) == 0x7D000000)
	{ // This is probably an Licq client
		DWORD ver = dwFT1 & 0xFFFF;

		makeClientVersion(szClientBuf, cliLicqVer, ver / 1000, (ver / 10) % 100, ver % 10, 0);
		if (dwFT1 & 0x00800000)
			strcat(szClientBuf, "/SSL");

		szClient = szClientBuf;
	}
	else if (dwFT1 == 0xffffff8f)
	{
		szClient = "StrICQ";
	}
	else if (dwFT1 == 0xffffff42)
	{
		szClient = "mICQ";
	}
	else if (dwFT1 == 0xffffffbe)
	{
		unsigned ver1 = (dwFT2>>24)&0xFF;
		unsigned ver2 = (dwFT2>>16)&0xFF;
		unsigned ver3 = (dwFT2>>8)&0xFF;

		makeClientVersion(szClientBuf, "Alicq ", ver1, ver2, ver3, 0);

		szClient = szClientBuf;
	}
	else if (dwFT1 == 0xFFFFFF7F)
	{
		szClient = "&RQ";
	}
	else if (dwFT1 == 0xFFFFFFAB)
	{
		szClient = "YSM";
	}
	else if (dwFT1 == 0x04031980)
	{
		szClient = "vICQ";
	}
	else if ((dwFT1 == 0x3AA773EE) && (dwFT2 == 0x3AA66380))
	{
		szClient = cliLibicq2k;
	}
	else if (dwFT1 == 0x3B75AC09)
	{
		szClient = cliTrillian;
	}
	else if (dwFT1 == 0x3BA8DBAF) // FT2: 0x3BEB5373; FT3: 0x3BEB5262;
	{
		if (wVersion == 2)
			szClient = "stICQ";
	}
	else if (dwFT1 == 0xFFFFFFFE && dwFT3 == 0xFFFFFFFE)
	{
		szClient = "Jimm";
	}
	else if (dwFT1 == 0xEFFEEFFE && dwFT3 == 0xEFFEEFFE)
	{
		szClient = "wJimm";
	}
	else if (dwFT1 == 0x3FF19BEB && dwFT3 == 0x3FF19BEB)
	{
		szClient = cliIM2;
	}
	else if (dwFT1 == 0xDDDDEEFF && !dwFT2 && !dwFT3)
	{
		szClient = "SmartICQ";
	}
	else if ((dwFT1 & 0xFFFFFFF0) == 0x494D2B00 && !dwFT2 && !dwFT3)
	{ // last byte of FT1: (5 = Win32, 3 = SmartPhone, Pocket PC)
		szClient = "IM+";
	}
	else if (dwFT1 == 0x3B4C4C0C && !dwFT2 && dwFT3 == 0x3B7248ed)
	{
		szClient = "KXicq2";
	}
	else if (dwFT1 == 0xFFFFF666 && !dwFT3)
	{ // this is R&Q (Rapid Edition)
		null_snprintf(szClientBuf, 64, "R&Q %u", (unsigned)dwFT2);
		szClient = szClientBuf;   
	}
	else if (dwFT1 == 0x48151623 && dwFT3 == 0x48151623)
	{ // http://loc-id.ru
		makeClientVersion(szClientBuf, "LocID v.", (dwFT2 >> 8) & 0x0F, (dwFT2 >> 4) & 0x0F, 0, 0);
		szClient = szClientBuf;
	}
	else if (dwFT1 == 0x66666666 && dwFT3 == 0x66666666)
	{ // http://darkjimm.ucoz.ru/
		if (dwFT2 == 0x10000)
    {
      strcpy(szClientBuf, "D[i]Chat v.");
      strcat(szClientBuf, "0.1a");
    }
    else 
    {
      makeClientVersion(szClientBuf, "D[i]Chat v.", (dwFT2 >> 8) & 0x0F, (dwFT2 >> 4) & 0x0F, 0, 0);
      if ((dwFT2 & 0x0F) == 1)
        strcat(szClientBuf, " alpha");
      else if ((dwFT2 & 0x0F) == 2)
        strcat(szClientBuf, " beta");
      else if ((dwFT2 & 0x0F) == 3)
        strcat(szClientBuf, " final");
    }

		szClient = szClientBuf;
	}
	else if (dwFT1 == dwFT2 && dwFT2 == dwFT3 && wVersion == 8)
	{
		if ((dwFT1 < dwOnlineSince + 3600) && (dwFT1 > (dwOnlineSince - 3600)))
		{
			szClient = cliSpamBot;
		}
	}
	else if (!dwFT1 && !dwFT2 && !dwFT3 && !wVersion && !wLen && dwWebPort == 0x75BB)
	{
		szClient = cliSpamBot;
	}
	else if (dwFT1 == 0x44F523B0 && dwFT2 == 0x44F523A6 && dwFT3 == 0x44F523A6 && wVersion == 8)
	{
		szClient = "Virus";
	}

	{ // capabilities based detection
		capstr* capId;

		if (dwUin && caps)
		{
			// Plus variables for our purposes
			BOOL Unicode = (BOOL)(((dwFT3>>24)&0xFF) == 0x80 || dwFT1 == 0x7fffffff);

			// check capabilities for client identification
			if (capId = MatchCap(caps, wLen, &capMirandaIm, 8))
			{ // new Miranda Signature
				DWORD iver = (*capId)[0xC] << 0x18 | (*capId)[0xD] << 0x10 | (*capId)[0xE] << 8 | (*capId)[0xF];
				DWORD mver = (*capId)[0x8] << 0x18 | (*capId)[0x9] << 0x10 | (*capId)[0xA] << 8 | (*capId)[0xB];

				szClient = MirandaVersionToString(szClientBuf, dwFT1 == 0x7fffffff, iver, mver);

				if (MatchCap(caps, wLen, &capIcqJs7, 0x4))
				{ // detect mod
					strcat(szClient, " (S7 & SSS)");
					if (MatchCap(caps, wLen, &capIcqJs7, 0xE))
					{
						strcat(szClient, " + SecureIM");
					}
				}
				else if ((dwFT1 & 0x7FFFFFFF) == 0x7FFFFFFF)
				{
					if (MatchCap(caps, wLen, &capMimMobile, 0x10))
						strcat(szClient, " (Mobile)");

					if (dwFT3 == 0x5AFEC0DE)
						strcat(szClient, " + SecureIM");
				}
				*bClientId = CLID_MIRANDA;
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJp, 4))
			{ // detect icqj plus mod
				szClient = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_PLUG_NAME);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJen, 4))
			{ // detect icqj eternity/plusplus++ mod
				szClient = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_PLUSPLUS_NAME);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJSin, 4))
			{ // detect newer icqj mod
				szClient = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_SIN_NAME);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJs7, 4))
			{ // detect newer icqj mod
				szClient = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_S7SSS_NAME);
				bMirandaIM = TRUE;
			}
			else if (MatchCap(caps, wLen, &capTrillian, 0x10) || MatchCap(caps, wLen, &capTrilCrypt, 0x10))
			{ // this is Trillian, check for new versions
				if (CheckContactCapabilities(hContact, CAPF_RTF))
				{
					if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE))
						szClient = cliTrillian4;
					else
					{ // workaroud for a bug in Trillian - make it receive msgs, other features will not work!
						ClearContactCapabilities(hContact, CAPF_SRV_RELAY);
						szClient = "Trillian v3";
					}
				}
        else if (CheckContactCapabilities(hContact, CAPF_HTML) || CheckContactCapabilities(hContact, CAPF_OSCAR_FILE))
          szClient = cliTrillian4;
				else
					szClient = cliTrillian;
			}
			else if ((capId = MatchCap(caps, wLen, &capSimOld, 0xF)) && ((*capId)[0xF] != 0x92 && (*capId)[0xF] >= 0x20 || (*capId)[0xF] == 0))
			{
				int hiVer = (((*capId)[0xF]) >> 6) - 1;
				unsigned loVer = (*capId)[0xF] & 0x1F;

				if ((hiVer < 0) || ((hiVer == 0) && (loVer == 0))) 
					szClient = "Kopete";
				else
				{
					makeClientVersion(szClientBuf, "SIM ", (unsigned)hiVer, loVer, 0, 0);
					szClient = szClientBuf;
				}
			}
			else if (capId = MatchCap(caps, wLen, &capSim, 0xC))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				makeClientVersion(szClientBuf, "SIM ", ver1, ver2, ver3, ver4 & 0x0F);
				if (ver4 & 0x80) 
					strcat(szClientBuf,"/Win32");
				else if (ver4 & 0x40) 
					strcat(szClientBuf,"/MacOS X");

				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capLicq, 0xC))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD] % 100;
				unsigned ver3 = (*capId)[0xE];

				makeClientVersion(szClientBuf, cliLicqVer, ver1, ver2, ver3, 0);
				if ((*capId)[0xF]) 
					strcat(szClientBuf,"/SSL");

				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capKopete, 0xC))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				makeClientVersion(szClientBuf, "Kopete ", ver1, ver2, ver3, ver4);

				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capClimm, 0xC))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				makeClientVersion(szClientBuf, "climm ", ver1, ver2, ver3, ver4);
				if ((ver1 & 0x80) == 0x80) 
					strcat(szClientBuf, " alpha");
				if (dwFT3 == 0x02000020)
					strcat(szClientBuf, "/Win32");
				else if (dwFT3 == 0x03000800)
					strcat(szClientBuf, "/MacOS X");

				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capmIcq, 0xC))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				makeClientVersion(szClientBuf, "mICQ ", ver1, ver2, ver3, ver4);
				if ((ver1 & 0x80) == 0x80) strcat(szClientBuf, " alpha");

				szClient = szClientBuf;
			}
			else if (MatchCap(caps, wLen, &capIm2, 0x10))
			{ // IM2 v2 provides also Aim Icon cap
				szClient = cliIM2;
			}
			else if (capId = MatchCap(caps, wLen, &capAndRQ, 9))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xB];
				unsigned ver3 = (*capId)[0xA];
				unsigned ver4 = (*capId)[9];

				makeClientVersion(szClientBuf, "&RQ ", ver1, ver2, ver3, ver4);

				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capRAndQ, 9))
			{
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xB];
				unsigned ver3 = (*capId)[0xA];
				unsigned ver4 = (*capId)[9];

				makeClientVersion(szClientBuf, "R&Q ", ver1, ver2, ver3, ver4);

				szClient = szClientBuf;
			}
			else if (MatchCap(caps, wLen, &capIMadering, 0x10))
			{ // http://imadering.com
				szClient = "IMadering";
			}
			else if (MatchCap(caps, wLen, &capQipPDA, 0x10))
			{
				szClient = "QIP PDA (Windows)";
			}
			else if (MatchCap(caps, wLen, &capQipMobile, 0x10))
			{
				szClient = "QIP Mobile (Java)";
			}
			else if (MatchCap(caps, wLen, &capQipSymbian, 0x10))
			{
				szClient = "QIP PDA (Symbian)";
			}
			else if (MatchCap(caps, wLen, &capQipInfium, 0x10))
			{
				char ver[10];

				strcpy(szClientBuf, "QIP Infium");
				if (dwFT1)
				{ // add build
					null_snprintf(ver, 10, " (%d)", dwFT1);
					strcat(szClientBuf, ver);
				}
				if (dwFT2 == 0x0B)
					strcat(szClientBuf, " Beta");

				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capQip, 0xE))
			{
				char ver[10];

				if (dwFT3 == 0x0F)
					strcpy(ver, "2005");
				else
				{
					strncpy(ver, (char*)(*capId)+11, 5);
					ver[5] = '\0'; // fill in missing zero
				}

				null_snprintf(szClientBuf, 64, cliQip, ver);
				if (dwFT1 && dwFT2 == 0x0E)
				{ // add QIP build
					null_snprintf(ver, 10, " (%d%d%d%d)", dwFT1 >> 0x18, (dwFT1 >> 0x10) & 0xFF, (dwFT1 >> 0x08) & 0xFF, dwFT1 & 0xFF);
					strcat(szClientBuf, ver);
				}
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capmChat, 0xA))
			{
				strcpy(szClientBuf, "mChat ");
				strncat(szClientBuf, (char*)(*capId) + 0xA, 6);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capJimm, 5))
			{
				strcpy(szClientBuf, "Jimm ");
				strncat(szClientBuf, (char*)(*capId) + 5, 11);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capwJimm, 5))
			{
				strcpy(szClientBuf, "wJimm ");
				strncat(szClientBuf, (char*)(*capId) + 5, 11);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capCorePager, 0xA))
			{ // http://corepager.net.ru/index/0-2
				strcpy(szClientBuf, "CORE Pager");
				if (dwFT2 == 0x0FFFF0011 && dwFT3 == 0x1100FFFF && (dwFT1 >> 0x18))
				{
					char ver[16];

					null_snprintf(ver, 10, " %d.%d", dwFT1 >> 0x18, (dwFT1 >> 0x10) & 0xFF);
					if ((dwFT1 & 0xFF) == 0x0B)
						strcat(ver, " Beta");
					strcat(szClientBuf, ver);
				}
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capDiChat, 9))
			{ // http://darkjimm.ucoz.ru/
				strcpy(szClientBuf, "D[i]Chat");
				strncat(szClientBuf, (char*)(*capId) + 8, 8);
				szClient = szClientBuf;
			}
			else if (MatchCap(caps, wLen, &capMacIcq, 0x10))
			{
				szClient = "ICQ for Mac";
			}
			else if (MatchCap(caps, wLen, &capUim, 0x10))
			{
				szClient = "uIM";
			}
			else if (MatchCap(caps, wLen, &capAnastasia, 0x10))
			{ // http://chis.nnov.ru/anastasia
				szClient = "Anastasia";
			}
			else if (capId = MatchCap(caps, wLen, &capPalmJicq, 0xC))
			{ // http://www.jsoft.ru
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				makeClientVersion(szClientBuf, "JICQ ", ver1, ver2, ver3, ver4);

				szClient = szClientBuf;
			}
			else if (MatchCap(caps, wLen, &capInluxMsgr, 0x10))
			{ // http://www.inlusoft.com
				szClient = "Inlux Messenger";
			}
			else if (capId = MatchCap(caps, wLen, &capMipClient, 0xC))
			{ // http://mip.rufon.net
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				if (ver1 < 30)
				{
					makeClientVersion(szClientBuf, "MIP ", ver1, ver2, ver3, ver4);
				}
				else
				{
					strcpy(szClientBuf, "MIP ");
					strncat(szClientBuf, (char*)(*capId) + 11, 5);
				}
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capMipClient, 0x04))
			{ //http://mip.rufon.net - new signature
				strcpy(szClientBuf, "MIP ");
				strncat(szClientBuf, (char*)(*capId) + 4, 12);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capVmIcq, 0x06))
			{
				strcpy(szClientBuf, "VmICQ");
				strncat(szClientBuf, (char*)(*capId) + 5, 11);
				szClient = szClientBuf;
			}
			else if (!wVersion && CheckContactCapabilities(hContact, CAPF_ICQDIRECT|CAPF_RTF|CAPF_TYPING|CAPF_UTF))
			{
				szClient = "GlICQ";
			}
			else if (capId = MatchCap(caps, wLen, &capSmapeR, 0x07))
			{ // http://www.smape.com/smaper
				strcpy(szClientBuf, "SmapeR");
				strncat(szClientBuf, (char*)(*capId) + 6, 10);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capYapp, 0x04))
			{ // http://yapp.ru
				strcpy(szClientBuf, "Yapp! v");
				strncat(szClientBuf, (char*)(*capId) + 8, 5);
				szClient = szClientBuf;
			}
			else if (MatchCap(caps, wLen, &capDigsby, 0x06))
			{ // http://www.dibsby.com (newer builds)
				szClient = "Digsby";
			}
			else if (MatchCap(caps, wLen, &capDigsbyBeta, 0x10))
			{ // http://www.digsby.com - probably by mistake (feature detection as well)
				szClient = "Digsby";
			}
			else if (MatchCap(caps, wLen, &capJapp, 0x10))
			{ // http://www.japp.org.ua
				szClient = "japp";
			}
			else if (MatchCap(caps, wLen, &capPigeon, 0x07))
			{ // http://pigeon.vpro.ru
				szClient = "PIGEON!";
			}
			else if (capId = MatchCap(caps, wLen, &capQutIm, 0x05))
			{ // http://www.qutim.org
				if ((*capId)[0x6] == 0x2E)
				{ // old qutim id
					unsigned ver1 = (*capId)[0x5] - 0x30;
					unsigned ver2 = (*capId)[0x7] - 0x30;

					makeClientVersion(szClientBuf, "qutIM ", ver1, ver2, 0, 0);
				}
				else 
				{ // new qutim id
					unsigned ver1 = (*capId)[0x6];
					unsigned ver2 = (*capId)[0x7];
					unsigned ver3 = (*capId)[0x8];
					unsigned ver4 = ((*capId)[0x9] << 8) || (*capId)[0xA];

					makeClientVersion(szClientBuf, "qutIM ", ver1, ver2, ver3, ver4);

					switch ((*capId)[0x5])
					{
					case 'l':
						strcat(szClientBuf, "/Linux");
						break;
					case 'w':
						strcat(szClientBuf, "/Win32");
						break;
					case 'm':
						strcat(szClientBuf, "/MacOS X");
						break;
					}
				}
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capBayan, 8))
			{ // http://www.barobin.com/bayanICQ.html
				strcpy(szClientBuf, "bayanICQ ");
				strncat(szClientBuf, (char*)(*capId) + 8, 5);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capJabberJIT, 0x04))
			{
				szClient = "Jabber ICQ Transport";
			}
			else if (capId = MatchCap(caps, wLen, &capIcqKid2, 0x07))
			{ // http://sourceforge.net/projects/icqkid2
				unsigned ver1 = (*capId)[0x7];
				unsigned ver2 = (*capId)[0x8];
				unsigned ver3 = (*capId)[0x9];
				unsigned ver4 = (*capId)[0xA];

				makeClientVersion(szClientBuf, "IcqKid2 v", ver1, ver2, ver3, ver4);
				szClient = szClientBuf;
			}
			else if (capId = MatchCap(caps, wLen, &capWebIcqPro, 0x0A))
			{ // http://intrigue.ru/workshop/webicqpro/webicqpro.html
				szClient = "WebIcqPro";
			}
			else if (szClient == cliLibicq2k)
			{ // try to determine which client is behind libicq2000
				if (CheckContactCapabilities(hContact, CAPF_RTF))
					szClient = cliCentericq; // centericq added rtf capability to libicq2000
				else if (CheckContactCapabilities(hContact, CAPF_UTF))
					szClient = cliLibicqUTF; // IcyJuice added unicode capability to libicq2000
				// others - like jabber transport uses unmodified library, thus cannot be detected
			}
			else if (szClient == NULL) // HERE ENDS THE SIGNATURE DETECTION, after this only feature default will be detected
			{
				if (wVersion == 8 && CheckContactCapabilities(hContact, CAPF_XTRAZ) && (MatchCap(caps, wLen, &capIMSecKey1, 6) || MatchCap(caps, wLen, &capIMSecKey2, 6)))
				{ // ZA mangled the version, OMG!
					wVersion = 9;
				}
				if (wVersion == 8 && (MatchCap(caps, wLen, &capComm20012, 0x10) || CheckContactCapabilities(hContact, CAPF_SRV_RELAY)))
				{ // try to determine 2001-2003 versions
					if (MatchCap(caps, wLen, &capIs2001, 0x10))
					{
						if (!dwFT1 && !dwFT2 && !dwFT3)
						{
							if (CheckContactCapabilities(hContact, CAPF_RTF))
								szClient = "TICQClient"; // possibly also older GnomeICU
							else
								szClient = "ICQ for Pocket PC";
						}
						else
						{
							*bClientId = CLID_GENERIC;
							szClient = "ICQ 2001";
						}
					}
					else if (MatchCap(caps, wLen, &capIs2002, 0x10))
					{
						*bClientId = CLID_GENERIC;
						szClient = "ICQ 2002";
					}
					else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_RTF))
					{
						if (!dwFT1 && !dwFT2 && !dwFT3)
						{
							if (!dwWebPort)
								szClient = "GnomeICU 0.99.5+"; // no other way
							else
								szClient = "IC@";
						}
						else
						{
							*bClientId = CLID_GENERIC;
							szClient = "ICQ 2002/2003a";
						}
					}
					else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_TYPING))
					{
						if (!dwFT1 && !dwFT2 && !dwFT3)
						{
							szClient = "PreludeICQ";
						}
					}
				}
				else if (wVersion == 9)
				{ // try to determine lite versions
					if (CheckContactCapabilities(hContact, CAPF_XTRAZ))
					{
						*bClientId = CLID_GENERIC;
						if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE))
						{
							if (MatchCap(caps, wLen, &captZers, 0x10))
							{ // capable of tZers ?
								if (CheckContactCapabilities(hContact, CAPF_HTML))
								{
									if(MatchCap(caps, wLen, &capLiveVideo, 0x10))
									{
										strcpy(szClientBuf, "ICQ 6");
										*bClientId = CLID_ICQ6;
									}
								}
								else
								{
									strcpy(szClientBuf, "ICQ 5.1");
								}
								SetContactCapabilities(hContact, CAPF_STATUSMSG_EXT);
							}
							else
							{
								strcpy(szClientBuf, "icq5");
							}
							if (MatchCap(caps, wLen, &capRambler, 0x10))
							{
								strcat(szClientBuf, " (Rambler)");
							}
							else if (MatchCap(caps, wLen, &capAbv, 0x10))
							{
								strcat(szClientBuf, " (Abv)");
							}
							else if (MatchCap(caps, wLen, &capNetvigator, 0x10))
							{
								strcat(szClientBuf, " (Netvigator)");
							}
							szClient = szClientBuf;
						}
						else if (!CheckContactCapabilities(hContact, CAPF_ICQDIRECT))
						{
							if (CheckContactCapabilities(hContact, CAPF_RTF))
							{
								// most probably Qnext - try to make that shit at least receiving our msgs
								ClearContactCapabilities(hContact, CAPF_SRV_RELAY);
								NetLog_Server("Forcing simple messages (QNext client).");
								szClient = "QNext";
							}
							else if (CheckContactCapabilities(hContact, CAPF_ICQ_DEVIL | CAPF_XTRAZ| CAPF_SRV_RELAY | CAPF_TYPING | CAPF_XTRAZ_CHAT  | CAPF_UTF ) && MatchCap(caps, wLen, &capAimAudio, 0x10) && MatchCap(caps, wLen, &captZers, 0x10) && MatchCap(caps, wLen, &capHtmlMsgs, 0x10))
								szClient = "Mail.ru Agent (PC)";
							else if (CheckContactCapabilities(hContact, CAPF_HTML) && CheckContactCapabilities(hContact, CAPF_TYPING) && MatchCap(caps, wLen, &captZers, 0x10))
								szClient = "fring";
							else
								szClient = "pyICQ";
						}
						else
							szClient = "ICQ Lite v4";
					}
					else if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE | CAPF_UTF))
					{
						if (MatchShortCap(caps, wLen, &capAimIcon) && MatchCap(caps, wLen, &capAimContactSnd, 0x10) && MatchCap(caps, wLen, &capAimChat, 0x10) && MatchCap(caps, wLen, &capLiveVideo, 0x10) && MatchCap(caps, wLen, &capAimAudio, 0x10))
							szClient = "ICQ Lite v7";
					}
					else if (MatchCap(caps, wLen, &capIcqLiteNew, 0x10))
						szClient = "ICQ Lite"; // the new ICQ Lite based on ICQ6
					else if (!CheckContactCapabilities(hContact, CAPF_ICQDIRECT))
					{
						if (CheckContactCapabilities(hContact, CAPF_HTML) && MatchCap(caps, wLen, &capOscarChat, 0x10) && MatchShortCap(caps, wLen, &capAimSmartCaps))
							szClient = cliTrillian4;
						else if (CheckContactCapabilities(hContact, CAPF_UTF) && !CheckContactCapabilities(hContact, CAPF_RTF))
							szClient = "pyICQ";
					}
				}
				else if (wVersion == 7)
				{
					if (CheckContactCapabilities(hContact, CAPF_RTF))
						szClient = "GnomeICU"; // this is an exception
					else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY))
					{
						if (!dwFT1 && !dwFT2 && !dwFT3)
							szClient = "&RQ";
						else
						{
							*bClientId = CLID_GENERIC;
							szClient = "ICQ 2000";
						}
					}
					else if (CheckContactCapabilities(hContact, CAPF_UTF))
					{
						if (CheckContactCapabilities(hContact, CAPF_TYPING))
							szClient = "Icq2Go! (Java)";
						else if (wUserClass & CLASS_WIRELESS)
							szClient = "Pocket Web 1&1";
						else
							szClient = "Icq2Go! (Flash)";
					}
				}
				else if (wVersion == 0xA)
				{
					if (!CheckContactCapabilities(hContact, CAPF_RTF) && !CheckContactCapabilities(hContact, CAPF_UTF))
					{ // this is bad, but we must do it - try to detect QNext
						ClearContactCapabilities(hContact, CAPF_SRV_RELAY);
						NetLog_Server("Forcing simple messages (QNext client).");
						szClient = "QNext";
					}
					else if (!CheckContactCapabilities(hContact, CAPF_RTF) && CheckContactCapabilities(hContact, CAPF_UTF) && !dwFT1 && !dwFT2 && !dwFT3)
					{ // not really good, but no other option
						szClient = "NanoICQ";
					}
				}
				else if (wVersion == 0xB)
				{
					if (CheckContactCapabilities(hContact, CAPF_XTRAZ| CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_OSCAR_FILE | CAPF_TYPING | CAPF_UTF ) && MatchCap(caps, wLen, &capAimContactSnd, 0x10))
					{ 
						szClient = "Mail.ru Agent (Symbian)";
					}
					else if (CheckContactCapabilities(hContact, CAPF_XTRAZ| CAPF_SRV_RELAY | CAPF_TYPING | CAPF_OSCAR_FILE  | CAPF_UTF) && MatchCap(caps, wLen, &capIcqDevil, 0x10))
					{ 
						szClient = "Mail.ru Agent (Symbian)";
					}
				}
				else if (wVersion == 0x7A69)
				{
					szClient = "QIP Infium";
				}
				else if (wVersion == 0)
				{ // capability footprint based detection - not really reliable
					if (!dwFT1 && !dwFT2 && !dwFT3 && !dwWebPort && !dwDirectCookie)
					{ // DC info is empty
						if (CheckContactCapabilities(hContact, CAPF_TYPING) && MatchCap(caps, wLen, &capIs2001, 0x10) &&
							MatchCap(caps, wLen, &capIs2002, 0x10) && MatchCap(caps, wLen, &capComm20012, 0x10))
							szClient = cliSpamBot;
						else if (MatchShortCap(caps, wLen, &capAimIcon) && MatchShortCap(caps, wLen, &capAimDirect) && 
							CheckContactCapabilities(hContact, CAPF_OSCAR_FILE | CAPF_UTF))
						{ // detect libgaim/libpurple versions
							if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY))
								szClient = "Adium X"; // yeah, AFAIK only Adium has this fixed
							else if (CheckContactCapabilities(hContact, CAPF_TYPING))
								szClient = "libpurple";
							else
								szClient = "libgaim";
						}
						else if (MatchShortCap(caps, wLen, &capAimIcon) && MatchShortCap(caps, wLen, &capAimDirect) &&
							MatchCap(caps, wLen, &capOscarChat, 0x10) && CheckContactCapabilities(hContact, CAPF_OSCAR_FILE) && wLen == 0x40)
							szClient = "libgaim"; // Gaim 1.5.1 most probably
						else if (MatchCap(caps, wLen, &capOscarChat, 0x10) && CheckContactCapabilities(hContact, CAPF_OSCAR_FILE) && wLen == 0x20)
							szClient = "Easy Message";
						else if (MatchShortCap(caps, wLen, &capAimIcon) && MatchCap(caps, wLen, &capOscarChat, 0x10) && CheckContactCapabilities(hContact, CAPF_UTF) && wLen == 0x30)
							szClient = "Meebo";
						else if (MatchShortCap(caps, wLen, &capAimIcon) && CheckContactCapabilities(hContact, CAPF_UTF) && wLen == 0x20)
							szClient = "PyICQ-t Jabber Transport";
						else if (MatchShortCap(caps, wLen, &capAimIcon) && MatchCap(caps, wLen, &capXtrazVideo, 0x10) && CheckContactCapabilities(hContact, CAPF_UTF | CAPF_XTRAZ))
							szClient = "PyICQ-t Jabber Transport";
						else if (MatchShortCap(caps, wLen, &capAimIcon) && CheckContactCapabilities(hContact, CAPF_UTF | CAPF_TYPING | CAPF_SRV_RELAY | CAPF_XTRAZ))
							szClient = "PyICQ-t Jabber Transport";
						else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_TYPING) && wLen == 0x40)
							szClient = "Agile Messenger"; // Smartphone 2002
						else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_OSCAR_FILE) && MatchShortCap(caps, wLen, &capAimFileShare))
							szClient = "Slick"; // http://lonelycatgames.com/?app=slick
						else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_OSCAR_FILE | CAPF_CONTACTS) && MatchShortCap(caps, wLen, &capAimFileShare) && MatchShortCap(caps, wLen, &capAimIcon))
							szClient = "Digsby"; // http://www.digsby.com
						else if (MatchShortCap(caps, wLen, &capAimIcon) && CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_CONTACTS | CAPF_HTML))
							szClient = "mundu IM"; // http://messenger.mundu.com
						else if (!caps)
							szClient = "Icq2Go!"; // TODO: Really? x)
					}
				}
			}
		}
		else if (!dwUin)
		{ // detect AIM clients
			if (caps)
			{
				if (capId = MatchCap(caps, wLen, &capAimOscar, 8))
				{ // AimOscar Signature
					DWORD aver = (*capId)[0xC] << 0x18 | (*capId)[0xD] << 0x10 | (*capId)[0xE] << 8 | (*capId)[0xF];
					DWORD mver = (*capId)[0x8] << 0x18 | (*capId)[0x9] << 0x10 | (*capId)[0xA] << 8 | (*capId)[0xB];

					szClient = MirandaVersionToStringEx(szClientBuf, 0, "AimOscar", aver, mver);
					bMirandaIM = TRUE;
				}
				else if (capId = MatchCap(caps, wLen, &capSim, 0xC))
				{ // Sim is universal
					unsigned ver1 = (*capId)[0xC];
					unsigned ver2 = (*capId)[0xD];
					unsigned ver3 = (*capId)[0xE];

					makeClientVersion(szClientBuf, "SIM ", ver1, ver2, ver3, 0);
					if ((*capId)[0xF] & 0x80) 
						strcat(szClientBuf,"/Win32");
					else if ((*capId)[0xF] & 0x40) 
						strcat(szClientBuf,"/MacOS X");

					szClient = szClientBuf;
				}
				else if (capId = MatchCap(caps, wLen, &capKopete, 0xC))
				{
					unsigned ver1 = (*capId)[0xC];
					unsigned ver2 = (*capId)[0xD];
					unsigned ver3 = (*capId)[0xE];
					unsigned ver4 = (*capId)[0xF];

					makeClientVersion(szClientBuf, "Kopete ", ver1, ver2, ver3, ver4);

					szClient = szClientBuf;
				}
				else if (MatchCap(caps, wLen, &capIm2, 0x10))
				{ // IM2 extensions
					szClient = cliIM2;
				}
				else if (MatchCap(caps, wLen, &capNaim, 0x8))
				{
					szClient = "naim";
				}
				else if (MatchCap(caps, wLen, &capDigsby, 0x06))
				{ // http://www.dibsby.com (newer builds)
					szClient = "Digsby";
				}
				else if (MatchCap(caps, wLen, &capDigsbyBeta, 0x10))
				{ // http://www.digsby.com
					szClient = "Digsby";
				}
				else if (MatchShortCap(caps, wLen, &capAimIcon))
				{
					if (MatchCap(caps, wLen, &capOscarChat, 0x10) && CheckContactCapabilities(hContact, CAPF_UTF) && wLen == 0x30)
					{
						szClient = "Meebo";
					}
					else if (MatchCap(caps, wLen, &capAimAddins, 0x10) && MatchCap(caps, wLen, &capAimUnk2, 0x10))
					{
						szClient = "AIM 5";
					}
					else if (MatchCap(caps, wLen, &capAimAudio, 0x10) && MatchCap(caps, wLen, &capAimContactSnd, 0x10))
					{
						szClient = "AIM Pro";
					}
					else if (MatchCap(caps, wLen, &capAimAudio, 0x10))
					{
						szClient = "AIM 6";
					}
					else
						szClient = "AIM";
				}
				else
					szClient = "AIM";
			}
			else
				szClient = "AIM";
		}
	}

	if (caps && bMirandaIM)
	{
		capstr* capId;

		if ( capId = MatchCap( caps, wLen, &capMimPack, 4 ) )
		{ // if some pack capability is available, move pointer at it and begin searching
			for ( ;; )
			{
				if ( capId = MatchCap( *capId, wLen, &capMimPack, 4 ) )
				{ // if pointer points at something
					char szPack[16];
					null_snprintf( szPack, 16, " [%.12s]", ( *capId ) + 4 ); // take this capability ..
					lstrcatA( szClient, szPack ); // .. and add to client signature
					capId++; // move pointer one byte to the right to avoid infinite loop and/or repeats
				}
				else // else
					break; // safely quit infinite loop
			}
		}
	}

	if (!szClient)
	{
		if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_TYPING) && wVersion > 1000)
		{
			szClient = "QIP 2005a (as Unknown)";
		}
		else if (CheckContactCapabilities(hContact, CAPF_UTF)&&!CheckContactCapabilities(hContact, CAPF_SRV_RELAY)&&!CheckContactCapabilities(hContact, CAPF_TYPING)) // I hate Pocket Web ;)
		{
			szClient = "Pocket Web 1&1";
		}
	}

	if (!szClient)
	{
		NetLog_Server("No client identification, put default ICQ client for protocol.");

		*bClientId = CLID_GENERIC;

		switch (wVersion)
		{  // client detection failed, provide default clients
		case 6:
			szClient = "ICQ99";
			break;
		case 7:
			szClient = "ICQ 2000/Icq2Go";
			break;
		case 8: 
			szClient = "ICQ 2001-2003a";
			break;
		case 9: 
			szClient = "ICQ Lite";
			break;
		case 0xA:
			szClient = "ICQ 2003b";
			break;
		case 0xB:
			szClient = "QIP 2005a";
			break;
		}
	}
	else
	{
		NetLog_Server("Client identified as %s", szClient);
	}

	if (szClient)
	{
		char* szExtra = NULL;

		if (MatchCap(caps, wLen, &capSimpLite, 0x10))
			szExtra = " + SimpLite";
		if (MatchCap(caps, wLen, &capSimpPro, 0x10))
			szExtra = " + SimpPro";
		if (MatchCap(caps, wLen, &capIMsecure, 0x10) || MatchCap(caps, wLen, &capIMSecKey1, 6) || MatchCap(caps, wLen, &capIMSecKey2, 6))
			szExtra = " + IMsecure";

		// Plus: Check whether someone tries to fake his Miranda ID and find his real ID if possible

		// capabilities based detection, once again :)
		capstr* capId;
		char* szMirandaMod;

		if (!strstr(szClient, "Miranda") && ((capId = MatchCap(caps, wLen, &capIcqJp, 4)) || (capId = MatchCap(caps, wLen, &capIcqJen, 4)) || (capId = MatchCap(caps, wLen, &capIcqJs7, 4))))
		{
			//BOOL Unicode = (BOOL)(((dwFT3>>24)&0xFF) == 0x80 || dwFT1 == 0x7fffffff);
			BOOL Unicode = dwFT1 == 0x7fffffff;

			if (capId = MatchCap(caps, wLen, &capIcqJp, 4))
			{ // detect icqj plus mod
				szMirandaMod = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_PLUG_NAME);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJen, 4))
			{ // detect icqj eternity/plusplus++ mod
				szMirandaMod = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_PLUSPLUS_NAME);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJSin, 4))
			{ // detect newer icqj mod
				szMirandaMod = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_SIN_NAME);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCap(caps, wLen, &capIcqJs7, 4))
			{ // detect newer icqj mod
				szMirandaMod = MirandaModToString(szClientBuf, capId, Unicode, "ICQ "ICQ_S7SSS_NAME);
				bMirandaIM = TRUE;
			}
			MessageBoxA( NULL, szMirandaMod, "", MB_OK );
		}

		if (szExtra)
		{
			if (szClient != szClientBuf)
			{
				strcpy(szClientBuf, szClient);
				szClient = szClientBuf;
			}
			if (szClient != szExtra)
				strcat(szClient, szExtra);
		}

		//if (strlen(szMirandaMod))
		//{
		//	strcat(szClient, " (");
		//	strcat(szClient, szMirandaMod);
		//	strcat(szClient, ")");
		//}

	}
	return szClient;
}
