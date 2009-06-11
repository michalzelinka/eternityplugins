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
// File name      : $URL$
// Revision       : $Revision$
// Last change on : $Date$
// Last change by : $Author$
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#include <win2k.h>

extern BOOL bPopUpService;

static const char* szLogLevelDescr[] = {
	LPGEN("Display all problems"),
	LPGEN("Display problems causing possible loss of data"),
	LPGEN("Display explanations for disconnection"),
	LPGEN("Display problems requiring user intervention"),
	LPGEN("Do not display any problems (not recommended)")
};

static BOOL (WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

static BOOL LoadDBCheckState(CIcqProto* ppro, HWND hwndDlg, int idCtrl, const char* szSetting, BYTE bDef)
{
	BOOL state = ppro->getSettingByte(NULL, szSetting, bDef);
	CheckDlgButton(hwndDlg, idCtrl, state);
	return state;
}

static BOOL StoreDBCheckState(CIcqProto* ppro, HWND hwndDlg, int idCtrl, const char* szSetting)
{
	BOOL state = IsDlgButtonChecked(hwndDlg, idCtrl);
	ppro->setSettingByte(NULL, szSetting, (BYTE)state);
	return state;
}

static void OptDlgChanged(HWND hwndDlg)
{
	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

TCHAR HttpUserAgents[][255] = 
{
  _T( "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; SV1; .NET CLR 1.1.4322)" ),
  _T( "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)" ),
  _T( "Mozilla/5.0 (Windows; U; Windows NT 5.1; ru; rv:1.8.1.9) Gecko/20071025 Firefox/2.0.0.9" ),
  _T( "Opera/9.23 (Windows NT 5.1; U; ru)" ),
  _T( "Opera/9.27 (Windows NT 5.1; U; en)" ),
  _T( "Opera/8.01 (J2ME/MIDP; Opera Mini/3.0.6306/1528; nb; U; ssr)" ),
  _T( "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.2) Gecko/20070221 SeaMonkey/1.1.1" ),
  _T( "Mozilla/5.0 (compatible; Konqueror/3.5; Linux 2.6.21-rc1; x86_64; cs, en_US) KHTML/3.5.6 (like Gecko)" ),
  _T( "Lynx/2.8.4rel.1 libwww-FM/2.14" ),
  _T( "Mozilla/4.08 [en] (WinNT; U ;Nav)" )
};

/////////////////////////////////////////////////////////////////////////////////////////
// standalone option pages

static INT_PTR CALLBACK DlgProcIcqOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		{
			DWORD dwUin = ppro->getContactUin(NULL);
			if (dwUin)
				SetDlgItemInt(hwndDlg, IDC_ICQNUM, dwUin, FALSE);
			else // keep it empty when no UIN entered
				SetDlgItemTextA(hwndDlg, IDC_ICQNUM, "");

			char pszPwd[16];
			if (!ppro->getSettingStringStatic(NULL, "Password", pszPwd, sizeof(pszPwd)))
			{
				CallService(MS_DB_CRYPT_DECODESTRING, strlennull(pszPwd) + 1, (LPARAM)pszPwd);

				//bit of a security hole here, since it's easy to extract a password from an edit box
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, pszPwd);
			}

			LoadDBCheckState(ppro, hwndDlg, IDC_MD5LOGIN, "SecureLogin", DEFAULT_SECURE_LOGIN);
			LoadDBCheckState(ppro, hwndDlg, IDC_SSL, "SecureConnection", DEFAULT_SECURE_CONNECTION);

			{
				char DBModule[32], Setting[32];
				SendDlgItemMessage(hwndDlg, IDC_ICQSERVER, CB_RESETCONTENT, 0, 0);
				mir_snprintf(DBModule, 32, "%sSrvs", ppro->m_szModuleName);

				for (int i = 1; i <= DBGetContactSettingWord(NULL, DBModule, "SrvCount", 0); i++)
				{
					DBVARIANT dbv = {DBVT_TCHAR};
					mir_snprintf(Setting, 32, "server%luhost", i);
					DBGetContactSettingTString(NULL, DBModule, Setting, &dbv);
					SendDlgItemMessage(hwndDlg, IDC_ICQSERVER, CB_ADDSTRING, 0, (LPARAM)dbv.ptszVal);
				}
			}
			{
				DBVARIANT dbv = {DBVT_TCHAR};
				DBGetContactSettingTString(NULL, ppro->m_szModuleName, "OscarServer", &dbv);
				if (dbv.ptszVal)
					SetDlgItemText(hwndDlg, IDC_ICQSERVER, dbv.ptszVal);
				else
					SetDlgItemTextA(hwndDlg, IDC_ICQSERVER, IsDlgButtonChecked(hwndDlg, IDC_SSL) ? DEFAULT_SERVER_HOST_SSL : DEFAULT_SERVER_HOST);

				SetDlgItemInt(hwndDlg, IDC_ICQPORT, ppro->getSettingWord(NULL, "OscarPort", DEFAULT_SERVER_PORT), FALSE);
			}
			{
				DBVARIANT dbv = {0};
				ppro->getSettingString(NULL, "HttpUserAgent", &dbv);
				SendDlgItemMessage(hwndDlg, IDC_HTTPUSERAGENT, CB_RESETCONTENT, 0, 0);
				for (int i = 0; i < SIZEOF(HttpUserAgents); i++)
					SendDlgItemMessage(hwndDlg, IDC_HTTPUSERAGENT, CB_ADDSTRING, 0, (LPARAM)HttpUserAgents[i]);

				if (dbv.pszVal)
					SetDlgItemTextA(hwndDlg, IDC_HTTPUSERAGENT, dbv.pszVal);
				else
					SetDlgItemTextA(hwndDlg, IDC_HTTPUSERAGENT, DEFAULT_HTTP_AGENT);
			}

			LoadDBCheckState(ppro, hwndDlg, IDC_KEEPALIVE, "KeepAlive", 1);
			ppro->m_bServerAutoChange = LoadDBCheckState(ppro, hwndDlg, IDC_AUTOCHANGE, "ServerAutoChange", DEFAULT_SERVER_AUTOCHANGE);
			SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_SETRANGE, FALSE, MAKELONG(0, 4));
			SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_SETPOS, TRUE, 4-ppro->getSettingByte(NULL, "ShowLogLevel", LOG_WARNING));
			{
				char buf[MAX_PATH];
				SetDlgItemTextUtf(hwndDlg, IDC_LEVELDESCR, ICQTranslateUtfStatic(szLogLevelDescr[4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_GETPOS, 0, 0)], buf, MAX_PATH));
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_HIDE);
			LoadDBCheckState(ppro, hwndDlg, IDC_NOERRMULTI, "IgnoreMultiErrorBox", 0);
		}
		return TRUE;

	case WM_HSCROLL:
		{
			char str[MAX_PATH];

			SetDlgItemTextUtf(hwndDlg, IDC_LEVELDESCR, ICQTranslateUtfStatic(szLogLevelDescr[4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL,TBM_GETPOS, 0, 0)], str, MAX_PATH));
			OptDlgChanged(hwndDlg);
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDC_LOOKUPLINK:
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_FORGOT_PASSWORD);
				return TRUE;

			case IDC_NEWUINLINK:
				CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_REGISTER);
				return TRUE;

			case IDC_RESETSERVER:
				SetDlgItemInt(hwndDlg, IDC_ICQPORT, DEFAULT_SERVER_PORT, FALSE);
				// no break!

			case IDC_SSL:
				SetDlgItemTextA(hwndDlg, IDC_ICQSERVER, IsDlgButtonChecked(hwndDlg, IDC_SSL) ? DEFAULT_SERVER_HOST_SSL : DEFAULT_SERVER_HOST);
				OptDlgChanged(hwndDlg);
				return TRUE;

			case IDC_REGUIN:
				ppro->ShowRegUINDialog(hwndDlg);
				return TRUE;

			case IDC_SERVERBOX:
				ppro->ShowSrvsListDialog(hwndDlg);
				return TRUE;

			case IDC_ICQSERVER:
				switch(HIWORD(wParam))
				{

				case CBN_SELCHANGE:
					{
						char DBModule[64], buf[64];
						int iIndex = (int)SendDlgItemMessage(hwndDlg, IDC_ICQSERVER, CB_GETCURSEL, 0, 0);
						if(iIndex < 0)
							return FALSE;
						mir_snprintf(DBModule, 64, "%sSrvs", ppro->m_szModuleName);
						mir_snprintf(buf, 64, "server%luport", iIndex + 1);
						DBWriteContactSettingWord(NULL, DBModule, "CurrServ", (WORD)(iIndex + 1));
						SetDlgItemInt(hwndDlg, IDC_ICQPORT, DBGetContactSettingWord(NULL, DBModule, buf, DEFAULT_SERVER_PORT), FALSE);
						OptDlgChanged(hwndDlg);
					}
				}
				return TRUE;

			case IDC_HTTPUSERAGENT:
				switch(HIWORD(wParam))
				{

				case CBN_SELCHANGE:
					OptDlgChanged(hwndDlg);
				}
				return TRUE;
			}

			if (ppro->icqOnline() && LOWORD(wParam) != IDC_NOERRMULTI)
			{
				char szClass[80];
				GetClassNameA((HWND)lParam, szClass, sizeof(szClass));

				if (stricmpnull(szClass, "EDIT") || HIWORD(wParam) == EN_CHANGE)
					ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_SHOW);
			}

			if ((LOWORD(wParam)==IDC_ICQNUM || LOWORD(wParam)==IDC_PASSWORD || LOWORD(wParam)==IDC_ICQSERVER || LOWORD(wParam)==IDC_ICQPORT || LOWORD(wParam)==IDC_HTTPUSERAGENT) &&
				(HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
			{
				return 0;
			}

			OptDlgChanged(hwndDlg);
			break;
		}

	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{

			case PSN_APPLY:
				{
					char str[128];
					TCHAR tstr[128];

					ppro->setSettingDword(NULL, UNIQUEIDSETTING, GetDlgItemInt(hwndDlg, IDC_ICQNUM, NULL, FALSE));
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, sizeof(ppro->m_szPassword));
					if (strlennull(str))
					{
						strcpy(ppro->m_szPassword, str);
						ppro->m_bRememberPwd = TRUE;
					}
					else
						ppro->m_bRememberPwd = ppro->getSettingByte(NULL, "RememberPass", 0);

					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(ppro->m_szPassword), (LPARAM)str);
					ppro->setSettingString(NULL, "Password", str);
					GetDlgItemText(hwndDlg,IDC_ICQSERVER, tstr, sizeof(tstr));
					DBWriteContactSettingTString(NULL, ppro->m_szModuleName, "OscarServer", tstr);
					GetDlgItemTextA(hwndDlg,IDC_HTTPUSERAGENT, str, sizeof(str));
					ppro->setSettingString(NULL, "HttpUserAgent", str);
					ppro->setSettingWord(NULL, "OscarPort", (WORD)GetDlgItemInt(hwndDlg, IDC_ICQPORT, NULL, FALSE));
					ppro->m_bServerAutoChange = StoreDBCheckState(ppro, hwndDlg, IDC_AUTOCHANGE, "ServerAutoChange");
					StoreDBCheckState(ppro, hwndDlg, IDC_KEEPALIVE, "KeepAlive");
					StoreDBCheckState(ppro, hwndDlg, IDC_MD5LOGIN, "SecureLogin");
					StoreDBCheckState(ppro, hwndDlg, IDC_SSL, "SecureConnection");
					StoreDBCheckState(ppro, hwndDlg, IDC_NOERRMULTI, "IgnoreMultiErrorBox");
					ppro->setSettingByte(NULL, "ShowLogLevel", (BYTE)(4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_GETPOS, 0, 0)));

					return TRUE;
				}
			}
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const UINT icqPrivacyControls[] = {
	IDC_DCALLOW_ANY, IDC_DCALLOW_CLIST, IDC_DCALLOW_AUTH, IDC_ADD_ANY, IDC_ADD_AUTH, 
	IDC_WEBAWARE, IDC_PUBLISHPRIMARY, IDC_STATIC_DC1, IDC_STATIC_DC2, IDC_STATIC_CLIST
};

static INT_PTR CALLBACK DlgProcIcqPrivacyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		{
			int nDcType = ppro->getSettingByte(NULL, "DCType", 0);
			int nAddAuth = ppro->getSettingByte(NULL, "Auth", 1);

			if (!ppro->icqOnline())
			{
				icq_EnableMultipleControls(hwndDlg, icqPrivacyControls, sizeof(icqPrivacyControls)/sizeof(icqPrivacyControls[0]), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_NOTONLINE), SW_SHOW);
			}
			else 
				ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_NOTONLINE), SW_HIDE);

			CheckDlgButton(hwndDlg, IDC_DCALLOW_ANY, (nDcType == 0));
			CheckDlgButton(hwndDlg, IDC_DCALLOW_CLIST, (nDcType == 1));
			CheckDlgButton(hwndDlg, IDC_DCALLOW_AUTH, (nDcType == 2));
			CheckDlgButton(hwndDlg, IDC_ADD_ANY, (nAddAuth == 0));
			CheckDlgButton(hwndDlg, IDC_ADD_AUTH, (nAddAuth == 1));
			LoadDBCheckState(ppro, hwndDlg, IDC_WEBAWARE, "WebAware", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_PUBLISHPRIMARY, "PublishPrimaryEmail", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_NOSTATUSREPLY, "NoStatusReply", DEFAULT_NO_STATUS_REPLY);
			LoadDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_CLIST, "StatusMsgReplyCList", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible", 0);
			if (!ppro->getSettingByte(NULL, "StatusMsgReplyCList", 0))
				EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DCALLOW_ANY:
		case IDC_DCALLOW_CLIST:
		case IDC_DCALLOW_AUTH:
		case IDC_ADD_ANY:
		case IDC_ADD_AUTH:
		case IDC_WEBAWARE:
		case IDC_PUBLISHPRIMARY:
		case IDC_NOSTATUSREPLY:
			if (IsDlgButtonChecked(hwndDlg, IDC_NOSTATUSREPLY))
				ppro->icq_sendSetAimAwayMsgServ(""); // set blank AIM message
		case IDC_STATUSMSG_VISIBLE:
			if ((HWND)lParam != GetFocus())  return 0;
			break;
		case IDC_STATUSMSG_CLIST:
			if (IsDlgButtonChecked(hwndDlg, IDC_STATUSMSG_CLIST)) 
			{
				EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, TRUE);
				LoadDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible", 0);
			}
			else 
			{
				EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
				CheckDlgButton(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
			}
			break;
		default:
			return 0;
		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			StoreDBCheckState(ppro, hwndDlg, IDC_WEBAWARE, "WebAware");
			StoreDBCheckState(ppro, hwndDlg, IDC_PUBLISHPRIMARY, "PublishPrimaryEmail");
			ppro->m_bNoStatusReply = StoreDBCheckState(ppro, hwndDlg, IDC_NOSTATUSREPLY, "NoStatusReply");
			StoreDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_CLIST, "StatusMsgReplyCList");
			StoreDBCheckState(ppro, hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible");
			if (IsDlgButtonChecked(hwndDlg, IDC_DCALLOW_AUTH))
				ppro->setSettingByte(NULL, "DCType", 2);
			else if (IsDlgButtonChecked(hwndDlg, IDC_DCALLOW_CLIST))
				ppro->setSettingByte(NULL, "DCType", 1);
			else 
				ppro->setSettingByte(NULL, "DCType", 0);
			StoreDBCheckState(ppro, hwndDlg, IDC_ADD_AUTH, "Auth");

			if (ppro->icqOnline())
			{
				PBYTE buf=NULL;
				int buflen=0;

				/*ppro->ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail", (BYTE)!ppro->getSettingByte(NULL, "PublishPrimaryEmail", 0), TLV_EMAIL);
				ppro->ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail0", 0, TLV_EMAIL);
				ppro->ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail1", 0, TLV_EMAIL);*/

				ppackTLVWord(&buf, &buflen, 0x19A, (WORD)!ppro->getSettingByte(NULL, "Auth", 1));
				ppackTLVByte(&buf, &buflen, 0x212, ppro->getSettingByte(NULL, "WebAware", 0));

				ppackTLVWord(&buf, &buflen, 0x1F9, ppro->getSettingByte(NULL, "PrivacyLevel", 1));

				ppro->icq_changeUserDirectoryInfoServ(buf, (WORD)buflen, DIRECTORYREQUEST_UPDATEPRIVACY);

				SAFE_FREE((void**)&buf);

				// Send a status packet to notify the server about the webaware setting
				{
					WORD wStatus = MirandaStatusToIcq(ppro->m_iStatus);

					if (ppro->m_iStatus == ID_STATUS_INVISIBLE)
					{
						if (ppro->m_bSsiEnabled)
							ppro->updateServVisibilityCode(3);
						ppro->icq_setstatus(wStatus, NULL);
					}
					else
					{
						ppro->icq_setstatus(wStatus, NULL);
						if (ppro->m_bSsiEnabled)
							ppro->updateServVisibilityCode(4);
					}
				}
			}
			return TRUE;
		}
		break;
	}

	return FALSE;  
}

/////////////////////////////////////////////////////////////////////////////////////////

static HWND hCpCombo;

struct CPTABLE {
	WORD cpId;
	char *cpName;
};

struct CPTABLE cpTable[] = {
	{  874,  LPGEN("Thai") },
	{  932,  LPGEN("Japanese") },
	{  936,  LPGEN("Simplified Chinese") },
	{  949,  LPGEN("Korean") },
	{  950,  LPGEN("Traditional Chinese") },
	{  1250, LPGEN("Central European") },
	{  1251, LPGEN("Cyrillic") },
	{  1252, LPGEN("Latin I") },
	{  1253, LPGEN("Greek") },
	{  1254, LPGEN("Turkish") },
	{  1255, LPGEN("Hebrew") },
	{  1256, LPGEN("Arabic") },
	{  1257, LPGEN("Baltic") },
	{  1258, LPGEN("Vietnamese") },
	{  1361, LPGEN("Korean (Johab)") },
	{   -1,  NULL}
};

const char* CIconSlotComboBox[] =
{
	"E-mail", "Protocol", "Phone/SMS", "Advanced #1", "Advanced #2",
	"Web page", "Client", "Visibility", "Advanced #3", "Advanced #4"
};

static BOOL CALLBACK FillCpCombo(LPSTR str)
{
	int i;
	UINT cp;

	cp = atoi(str);
	for (i=0; cpTable[i].cpName != NULL && cpTable[i].cpId!=cp; i++);
	if (cpTable[i].cpName) 
		ComboBoxAddStringUtf(hCpCombo, cpTable[i].cpName, cpTable[i].cpId);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const UINT icqUnicodeControls[] = {IDC_UTFALL,IDC_UTFSTATIC,IDC_UTFCODEPAGE};
static const UINT icqDCMsgControls[] = {IDC_DCPASSIVE,IDC_DCICON,IDC_DC_ICON_POS_TEXT,IDC_DC_ICON_POS};
static const UINT icqXStatusControls[] = {IDC_XSTATUSAUTO,IDC_XSTATUSRESET,IDC_XSTATUSICON,IDC_ICON_POS_TEXT,IDC_XSTATUS_ICON_POS,IDC_REPLACEXTEXT,IDC_FORCEXSTATUS,IDC_XSTNONSTD,IDC_XSTUPDATE,IDC_XSTUPDATERATE};
static const UINT icqAimControls[] = {IDC_AIMENABLE};
static const UINT icqDCIconControls[] = {IDC_DC_ICON_POS_TEXT,IDC_DC_ICON_POS};
static const UINT icqXStatusIconControls[] = {IDC_ICON_POS_TEXT,IDC_XSTATUS_ICON_POS};
static const UINT icqAdvStatusControls[] = {IDC_ADVSTATUSICON,IDC_ADVSTATUS_ICON_POS_TEXT,IDC_ADVSTATUS_ICON_POS};
static const UINT icqAdvStatusIconControls[] = {IDC_ADVSTATUS_ICON_POS_TEXT,IDC_ADVSTATUS_ICON_POS};

static INT_PTR CALLBACK DlgProcIcqFeaturesOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		{
			BYTE bData = ppro->getSettingByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
			CheckDlgButton(hwndDlg, IDC_UTFENABLE, bData?TRUE:FALSE);
			CheckDlgButton(hwndDlg, IDC_UTFALL, bData==2?TRUE:FALSE);
			icq_EnableMultipleControls(hwndDlg, icqUnicodeControls, SIZEOF(icqUnicodeControls), bData?TRUE:FALSE);
			LoadDBCheckState(ppro, hwndDlg, IDC_RTF, "RTF", DEFAULT_RTF);
			LoadDBCheckState(ppro, hwndDlg, IDC_TEMPVISIBLE, "TempVisListEnabled",DEFAULT_TEMPVIS_ENABLED);
			LoadDBCheckState(ppro, hwndDlg, IDC_SLOWSEND, "SlowSend", DEFAULT_SLOWSEND);
			LoadDBCheckState(ppro, hwndDlg, IDC_ONLYSERVERACKS, "OnlyServerAcks", DEFAULT_ONLYSERVERACKS);

			bData = ppro->getSettingByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
			CheckDlgButton(hwndDlg, IDC_DCENABLE, bData?TRUE:FALSE);
			CheckDlgButton(hwndDlg, IDC_DCPASSIVE, bData==1?TRUE:FALSE);
			icq_EnableMultipleControls(hwndDlg, icqDCMsgControls, SIZEOF(icqDCMsgControls), bData?TRUE:FALSE);
			icq_EnableMultipleControls(hwndDlg, icqDCIconControls, SIZEOF(icqDCIconControls), bData?TRUE:FALSE);
			BOOL state = LoadDBCheckState(ppro, hwndDlg, IDC_DCICON, "DCIconShow", DEFAULT_DC_ICON_SHOW);
			if (bData)
				icq_EnableMultipleControls(hwndDlg, icqDCIconControls, SIZEOF(icqDCIconControls), state?TRUE:FALSE);

			bData = ppro->getSettingByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);
			icq_EnableMultipleControls(hwndDlg, icqXStatusControls, SIZEOF(icqXStatusControls), bData);
			CheckDlgButton(hwndDlg, IDC_XSTATUSENABLE, bData);
			LoadDBCheckState(ppro, hwndDlg, IDC_XSTATUSAUTO, "XStatusAuto", DEFAULT_XSTATUS_AUTO);
			LoadDBCheckState(ppro, hwndDlg, IDC_XSTATUSRESET, "XStatusReset", DEFAULT_XSTATUS_RESET);
			SetDlgItemInt(hwndDlg, IDC_XSTUPDATERATE, ppro->getSettingWord(NULL, "XStatusUpdatePeriod", 15), FALSE);
			LoadDBCheckState(ppro, hwndDlg, IDC_REPLACEXTEXT, "ShowMyXText", 0);
			LoadDBCheckState(ppro, hwndDlg, IDC_FORCEXSTATUS, "ForceXStatus", 1);
			LoadDBCheckState(ppro, hwndDlg, IDC_XSTNONSTD, "NonStandardXstatus", 0);
			state = LoadDBCheckState(ppro, hwndDlg, IDC_XSTATUSICON, "XStatusIconShow", DEFAULT_XSTATUS_ICON_SHOW);
			if (bData)
				icq_EnableMultipleControls(hwndDlg, icqXStatusIconControls, SIZEOF(icqXStatusIconControls), state);

			state = LoadDBCheckState(ppro, hwndDlg, IDC_XSTUPDATE, "UpdateXStatus", 0);
			if (bData) EnableDlgItem(hwndDlg, IDC_XSTUPDATERATE, state);

			LoadDBCheckState(ppro, hwndDlg, IDC_KILLSPAMBOTS, "KillSpambots", DEFAULT_KILLSPAM_ENABLED);
			LoadDBCheckState(ppro, hwndDlg, IDC_KILLUNKNOWN, "KillUnknown", DEFAULT_KILLUNKNOWN_ENABLED);
			LoadDBCheckState(ppro, hwndDlg, IDC_AIMENABLE, "AimEnabled", DEFAULT_AIM_ENABLED);

			icq_EnableMultipleControls(hwndDlg, icqAimControls, SIZEOF(icqAimControls), ppro->icqOnline()?FALSE:TRUE);

			for (int i = 0; i < SIZEOF(CIconSlotComboBox); i++)
			{
				ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_DC_ICON_POS), CIconSlotComboBox[i], i);
				ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_XSTATUS_ICON_POS), CIconSlotComboBox[i], i);
			}
			SendDlgItemMessage(hwndDlg, IDC_DC_ICON_POS, CB_SETCURSEL, ppro->getSettingByte(NULL, "DCIconPosition", 5) - 1, 0);
			SendDlgItemMessage(hwndDlg, IDC_XSTATUS_ICON_POS, CB_SETCURSEL, ppro->getSettingByte(NULL, "XStatusIconPosition", 4) - 1, 0);

			hCpCombo = GetDlgItem(hwndDlg, IDC_UTFCODEPAGE);
			int sCodePage = ppro->getSettingWord(NULL, "AnsiCodePage", CP_ACP);
			ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_UTFCODEPAGE), LPGEN("System default codepage"), 0);
			EnumSystemCodePagesA(FillCpCombo, CP_INSTALLED);
			if(sCodePage == 0)
				SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_SETCURSEL, (WPARAM)0, 0);
			else 
			{
				for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETCOUNT, 0, 0); i++) 
				{
					if (SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETITEMDATA, (WPARAM)i, 0) == sCodePage)
					{
						SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_SETCURSEL, (WPARAM)i, 0);
						break;
					}
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_UTFENABLE:
			icq_EnableMultipleControls(hwndDlg, icqUnicodeControls, SIZEOF(icqUnicodeControls), IsDlgButtonChecked(hwndDlg, IDC_UTFENABLE));
			break;

		case IDC_DCENABLE:
			icq_EnableMultipleControls(hwndDlg, icqDCMsgControls, SIZEOF(icqDCMsgControls),
			    IsDlgButtonChecked(hwndDlg, IDC_DCENABLE));
			// no break!

		case IDC_DCICON:
			icq_EnableMultipleControls(hwndDlg, icqDCIconControls, SIZEOF(icqDCIconControls),
			    IsDlgButtonChecked(hwndDlg, IDC_DCENABLE) &&
			    IsDlgButtonChecked(hwndDlg, IDC_DCICON));
			break;

		case IDC_XSTATUSENABLE:
			icq_EnableMultipleControls(hwndDlg, icqXStatusControls, SIZEOF(icqXStatusControls),
			    IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE));
			// no break!

		case IDC_XSTATUSICON:
			icq_EnableMultipleControls(hwndDlg, icqXStatusIconControls, SIZEOF(icqXStatusIconControls),
			    IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE) &&
			    IsDlgButtonChecked(hwndDlg, IDC_XSTATUSICON));
			// no break!

		case IDC_XSTUPDATE:
			EnableDlgItem(hwndDlg, IDC_XSTUPDATERATE,
			    IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE) &&
			    IsDlgButtonChecked(hwndDlg, IDC_XSTUPDATE));
			break;

		case IDC_RTF:
			MessageBox(hwndDlg, LPGENT("To toggle RTF text receiving you have to reconnect ICQ protocol"), LPGENT("Warning"), MB_OK | MB_ICONWARNING);
			break;

		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			if (IsDlgButtonChecked(hwndDlg, IDC_UTFENABLE))
				ppro->m_bUtfEnabled = IsDlgButtonChecked(hwndDlg, IDC_UTFALL)?2:1;
			else
				ppro->m_bUtfEnabled = 0;
			{
				int i = SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETCURSEL, 0, 0);
				ppro->m_wAnsiCodepage = (WORD)SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETITEMDATA, (WPARAM)i, 0);
				ppro->setSettingWord(NULL, "AnsiCodePage", ppro->m_wAnsiCodepage);
			}
			ppro->setSettingByte(NULL, "UtfEnabled", ppro->m_bUtfEnabled);
			ppro->m_bRTFEnabled = StoreDBCheckState(ppro, hwndDlg, IDC_RTF, "RTF");
			ppro->m_bTempVisListEnabled = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TEMPVISIBLE);
			ppro->setSettingByte(NULL, "TempVisListEnabled", ppro->m_bTempVisListEnabled);
			StoreDBCheckState(ppro, hwndDlg, IDC_SLOWSEND, "SlowSend");
			StoreDBCheckState(ppro, hwndDlg, IDC_ONLYSERVERACKS, "OnlyServerAcks");
			if (IsDlgButtonChecked(hwndDlg, IDC_DCENABLE))
				ppro->m_bDCMsgEnabled = IsDlgButtonChecked(hwndDlg, IDC_DCPASSIVE)?1:2;
			else
				ppro->m_bDCMsgEnabled = 0;
			ppro->setSettingByte(NULL, "DirectMessaging", ppro->m_bDCMsgEnabled);
			ppro->m_bXStatusEnabled = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE);
			ppro->setSettingByte(NULL, "XStatusEnabled", ppro->m_bXStatusEnabled);
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTATUSAUTO, "XStatusAuto");
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTATUSRESET, "XStatusReset");
			StoreDBCheckState(ppro, hwndDlg, IDC_REPLACEXTEXT, "ShowMyXText");
			StoreDBCheckState(ppro, hwndDlg, IDC_FORCEXSTATUS, "ForceXStatus");
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTNONSTD, "NonStandardXstatus");
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTUPDATE, "UpdateXStatus");
			StoreDBCheckState(ppro, hwndDlg, IDC_DCICON, "DCIconShow");
			StoreDBCheckState(ppro, hwndDlg, IDC_XSTATUSICON, "XStatusIconShow");
			StoreDBCheckState(ppro, hwndDlg, IDC_KILLSPAMBOTS , "KillSpambots");
			StoreDBCheckState(ppro, hwndDlg, IDC_KILLUNKNOWN, "KillUnknown");
			StoreDBCheckState(ppro, hwndDlg, IDC_AIMENABLE, "AimEnabled");
			{
				int value = SendDlgItemMessage(hwndDlg, IDC_DC_ICON_POS, CB_GETCURSEL, 0, 0);
				ppro->setSettingByte(NULL, "DCIconPosition", ++value);
				value = SendDlgItemMessage(hwndDlg, IDC_XSTATUS_ICON_POS, CB_GETCURSEL, 0, 0);
				ppro->setSettingByte(NULL, "XStatusIconPosition", ++value);
				value = GetDlgItemInt(hwndDlg, IDC_XSTUPDATERATE, FALSE, FALSE);
				ppro->setSettingWord(NULL, "XStatusUpdatePeriod", value);
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const UINT icqContactsControls[] = {IDC_ADDSERVER,IDC_LOADFROMSERVER,IDC_SAVETOSERVER,IDC_UPLOADNOW,IDC_FORCEREFRESH};
static const UINT icqAvatarControls[] = {IDC_AUTOLOADAVATARS,IDC_BIGGERAVATARS,IDC_STRICTAVATARCHECK};
static const UINT icqTmpContactsControls[] = {IDC_TMP_CONTACTS_GROUP,IDC_ADDTEMP,IDC_REMOVETEMP,IDC_TMPSNDADDED,IDC_TMPREQAUTH};

static INT_PTR CALLBACK DlgProcIcqContactsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	BOOL state;

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

		LoadDBCheckState(ppro, hwndDlg, IDC_ENABLE, "UseServerCList", DEFAULT_SS_ENABLED);
		LoadDBCheckState(ppro, hwndDlg, IDC_ADDSERVER, "ServerAddRemove", DEFAULT_SS_ADDSERVER);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOADFROMSERVER, "LoadServerDetails", DEFAULT_SS_LOAD);
		LoadDBCheckState(ppro, hwndDlg, IDC_SAVETOSERVER, "StoreServerDetails", DEFAULT_SS_STORE);
		LoadDBCheckState(ppro, hwndDlg, IDC_ENABLEAVATARS, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
		LoadDBCheckState(ppro, hwndDlg, IDC_AUTOLOADAVATARS, "AvatarsAutoLoad", DEFAULT_LOAD_AVATARS);
		LoadDBCheckState(ppro, hwndDlg, IDC_BIGGERAVATARS, "AvatarsAllowBigger", DEFAULT_BIGGER_AVATARS);
		LoadDBCheckState(ppro, hwndDlg, IDC_STRICTAVATARCHECK, "StrictAvatarCheck", DEFAULT_AVATARS_CHECK);
		icq_EnableMultipleControls(hwndDlg, icqContactsControls, SIZEOF(icqContactsControls), 
			ppro->getSettingByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED)?TRUE:FALSE);
		icq_EnableMultipleControls(hwndDlg, icqAvatarControls, SIZEOF(icqAvatarControls), 
			ppro->getSettingByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED)?TRUE:FALSE);
		state = ppro->getSettingByte(NULL, "TempContacts", DEFAULT_ADD_TEMP_CONTACTS);
		CheckDlgButton(hwndDlg, state ? IDC_ADD_TMP_CONTACTS : IDC_DELETE_TMP_CONTACTS, 1 );
		icq_EnableMultipleControls(hwndDlg, icqTmpContactsControls, SIZEOF(icqTmpContactsControls), state?TRUE:FALSE);
		if ( IsWindowEnabled( GetWindow( hwndDlg, IDC_FORCEREFRESH ) ) )
			EnableDlgItem( hwndDlg, IDC_FORCEREFRESH, ppro->getSettingDword( NULL, "SrvLastUpdate", 0 ) );

		{
			DBVARIANT dbv = {DBVT_TCHAR};
			DBGetContactSettingTString( NULL, ppro->m_szModuleName, "TmpContactsGroup", &dbv );
			if ( dbv.ptszVal && lstrlen( dbv.ptszVal ) > 0 )
			SetDlgItemText( hwndDlg, IDC_TMP_CONTACTS_GROUP, dbv.ptszVal );
		}

		LoadDBCheckState(ppro, hwndDlg, IDC_ADDTEMP, "AddTemp", DEFAULT_ADD_TEMPORARILY_ONLY);
		LoadDBCheckState(ppro, hwndDlg, IDC_REMOVETEMP, "RemoveTemp", DEFAULT_REMOVE_TEMP_ON_STARTUP);
		LoadDBCheckState(ppro, hwndDlg, IDC_TMPSNDADDED, "TmpSendAdded", DEFAULT_TMP_SEND_YOU_WERE_ADDED);
		LoadDBCheckState(ppro, hwndDlg, IDC_TMPREQAUTH, "TmpRequireAuth", DEFAULT_TMP_REQUIRE_AUTHORIZATION);

		if (ppro->icqOnline())
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_OFFLINETOENABLE), SW_SHOW);
			EnableDlgItem(hwndDlg, IDC_ENABLE, FALSE);
			EnableDlgItem(hwndDlg, IDC_ENABLEAVATARS, FALSE);
		}
		else
			EnableDlgItem(hwndDlg, IDC_UPLOADNOW, FALSE);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UPLOADNOW:
			ppro->ShowUploadContactsDialog();
			return TRUE;
		case IDC_ENABLE:
			icq_EnableMultipleControls(hwndDlg, icqContactsControls, SIZEOF(icqContactsControls), IsDlgButtonChecked(hwndDlg, IDC_ENABLE));
			if (ppro->icqOnline()) 
				ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_SHOW);
			else 
				EnableDlgItem(hwndDlg, IDC_UPLOADNOW, FALSE);
			if (IsWindowEnabled(GetWindow(hwndDlg, IDC_FORCEREFRESH)))
				EnableDlgItem(hwndDlg, IDC_FORCEREFRESH, ppro->getSettingDword(NULL, "SrvLastUpdate", 0));
			break;
		case IDC_ENABLEAVATARS:
			icq_EnableMultipleControls(hwndDlg, icqAvatarControls, SIZEOF(icqAvatarControls), IsDlgButtonChecked(hwndDlg, IDC_ENABLEAVATARS));
			break;
		case IDC_FORCEREFRESH:
			EnableDlgItem(hwndDlg, IDC_FORCEREFRESH, 0);
			ppro->setSettingWord(NULL, "SrvRecordCount", 0);
			ppro->setSettingDword(NULL, "SrvLastUpdate", 0);
			break;
		case IDC_ADD_TMP_CONTACTS:
		case IDC_DELETE_TMP_CONTACTS:
			icq_EnableMultipleControls(hwndDlg, icqTmpContactsControls, SIZEOF(icqTmpContactsControls),
			    IsDlgButtonChecked(hwndDlg, IDC_ADD_TMP_CONTACTS));
			break;
		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			StoreDBCheckState(ppro, hwndDlg, IDC_ENABLE, "UseServerCList");
			StoreDBCheckState(ppro, hwndDlg, IDC_ADDSERVER, "ServerAddRemove");
			StoreDBCheckState(ppro, hwndDlg, IDC_LOADFROMSERVER, "LoadServerDetails");
			StoreDBCheckState(ppro, hwndDlg, IDC_SAVETOSERVER, "StoreServerDetails");
			StoreDBCheckState(ppro, hwndDlg, IDC_ENABLEAVATARS, "AvatarsEnabled");
			StoreDBCheckState(ppro, hwndDlg, IDC_AUTOLOADAVATARS, "AvatarsAutoLoad");
			StoreDBCheckState(ppro, hwndDlg, IDC_BIGGERAVATARS, "AvatarsAllowBigger");
			StoreDBCheckState(ppro, hwndDlg, IDC_STRICTAVATARCHECK, "StrictAvatarCheck");
			{
				TCHAR buf[255];
				GetDlgItemText(hwndDlg, IDC_TMP_CONTACTS_GROUP, buf, sizeof(buf));
				DBWriteContactSettingTString(NULL, ppro->m_szModuleName, "TmpContactsGroup", sizeof(buf) > 0 ? buf : _T(""));
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_DELETE_TMP_CONTACTS))
				ppro->setSettingByte(NULL, "TempContacts", 0);
			else
				ppro->setSettingByte(NULL, "TempContacts", 1);
			StoreDBCheckState(ppro, hwndDlg, IDC_ADDTEMP, "AddTemp");
			StoreDBCheckState(ppro, hwndDlg, IDC_REMOVETEMP, "RemoveTemp");
			StoreDBCheckState(ppro, hwndDlg, IDC_TMPSNDADDED, "TmpSendAdded");
			StoreDBCheckState(ppro, hwndDlg, IDC_TMPREQAUTH, "TmpRequireAuth");
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcIcqASDOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		LoadDBCheckState(ppro, hwndDlg, IDC_ASDSTARTUP, "ASDStartup", DEFAULT_ASD_ON_STARTUP);
		LoadDBCheckState(ppro, hwndDlg, IDC_NOASD, "NoASDInInvisible", DEFAULT_NO_ASD_IN_INVISIBLE);
		LoadDBCheckState(ppro, hwndDlg, IDC_ASD_FOR_OFFLINE, "ASDForOffline", DEFAULT_ASD_FOR_OFFLINE);
		LoadDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_STATUS_MESSAGE, "ASDViaAwayMsg", DEFAULT_DETECT_VIA_STATUS_MESSAGE);
		LoadDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_XTRAZ, "ASDViaXtraz", DEFAULT_DETECT_VIA_XTRAZ);
		LoadDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_URL, "ASDViaURL", DEFAULT_DETECT_VIA_URL);
		LoadDBCheckState(ppro, hwndDlg, IDC_DETECT_UNAUTHORIZED, "ASDUnauthorized", DEFAULT_DETECT_UNAUTHORIZED);
		LoadDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_AUTH, "ASDViaAuth", DEFAULT_DETECT_VIA_AUTH);
		EnableWindow(ppro->m_hASDDialogCaller, FALSE);
		{
			LOGFONT lf = {0};
			HFONT hfnt = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_GETFONT, 0, 0);
			GetObject(hfnt, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);
		}
		break;

	case WM_COMMAND:

		switch ( LOWORD(wParam) )
		{

		case IDOK:
			StoreDBCheckState(ppro, hwndDlg, IDC_ASDSTARTUP, "ASDStartup");
			ppro->m_bNoASDInInvisible = StoreDBCheckState(ppro, hwndDlg, IDC_NOASD, "NoASDInInvisible");
			ppro->m_bASDForOffline = StoreDBCheckState(ppro, hwndDlg, IDC_ASD_FOR_OFFLINE, "ASDForOffline");
			ppro->m_bASDViaAwayMsg = StoreDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_STATUS_MESSAGE, "ASDViaAwayMsg");
			ppro->m_bASDViaXtraz = StoreDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_XTRAZ, "ASDViaXtraz");
			ppro->m_bASDViaURL = StoreDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_URL, "ASDViaURL");
			ppro->m_bASDUnauthorized = StoreDBCheckState(ppro, hwndDlg, IDC_DETECT_UNAUTHORIZED, "ASDUnauthorized");
			ppro->m_bASDViaAuth = StoreDBCheckState(ppro, hwndDlg, IDC_DETECT_VIA_AUTH, "ASDViaAuth");

		case IDCANCEL:
			EnableWindow(ppro->m_hASDDialogCaller, TRUE);
			EndDialog(hwndDlg, 0);
			break;
		}
		break;

	case WM_CTLCOLORSTATIC:
		if (((HWND)lParam == GetDlgItem(hwndDlg, IDC_WHITERECT)) ||
		    ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TITLEICON)) ||
		    ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TITLE)) ||
		    ((HWND)lParam == GetDlgItem(hwndDlg, IDC_DESC)))
			return (BOOL)GetStockObject(WHITE_BRUSH);
		return FALSE;
		break;

	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcIcqFeaturesAdvOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {

	case WM_INITDIALOG:
		BOOL state;
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		state = LoadDBCheckState(ppro, hwndDlg, IDC_ASD, "ASD", 0);
		EnableDlgItem(hwndDlg, IDC_ASDOPTIONS, state);
		LoadDBCheckState(ppro, hwndDlg, IDC_PSD, "PSD", 0);
		LoadDBCheckState(ppro, hwndDlg, IDC_NOPSD_FOR_HIDDEN, "NoPSDForHidden", 0);
		if (!ppro->getSettingByte(NULL, "IncognitoGlobal", 0))
			CheckDlgButton(hwndDlg, IDC_INCUSER, 1);
		else
			CheckDlgButton(hwndDlg, IDC_INCGLOBAL, 1);
		if (!ppro->getSettingByte(NULL, "PrivacyMenuPlacement", 0))
			CheckDlgButton(hwndDlg, IDC_INMAINMENU, 1);
		else
			CheckDlgButton(hwndDlg, IDC_INSTATUSMENU, 1);
		LoadDBCheckState(ppro, hwndDlg, IDC_USERCHKPOS, "UserScanPos", 0);
		state = LoadDBCheckState(ppro, hwndDlg, IDC_ADVSTATUSENABLE, "AdvStatusEnabled", 0);
		LoadDBCheckState(ppro, hwndDlg, IDC_ADVSTATUSICON, "AdvStatusIconShow", 0);
		EnableDlgItem(hwndDlg, IDC_ADVSTATUSICON, state);
		state = IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ADVSTATUSICON)) && IsDlgButtonChecked(hwndDlg, IDC_ADVSTATUSICON);
		for (int i = 0; i < SIZEOF(CIconSlotComboBox); i++)
			ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_ADVSTATUS_ICON_POS), CIconSlotComboBox[i], i);
		SendDlgItemMessage(hwndDlg, IDC_ADVSTATUS_ICON_POS, CB_SETCURSEL, ppro->getSettingByte(NULL, "AdvStatusIconPosition", 9) - 1, 0);
		EnableDlgItem(hwndDlg, IDC_ADVSTATUS_ICON_POS, state);
		LoadDBCheckState(ppro, hwndDlg, IDC_TZER, "EnableTzers", 0);
		LoadDBCheckState(ppro, hwndDlg, IDC_SHOW_AUTH, "AlwaysShowAuthItems", 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_ASD:
			EnableDlgItem(hwndDlg, IDC_ASDOPTIONS, IsDlgButtonChecked(hwndDlg, IDC_ASD));
			break;

		case IDC_ASDOPTIONS:
			ppro->m_hASDDialogCaller = GetParent(hwndDlg);
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ASD_OPTIONS), NULL, DlgProcIcqASDOpts, LPARAM(ppro));
			break;

		case IDC_ADVSTATUSENABLE:
			icq_EnableMultipleControls(hwndDlg, icqAdvStatusControls, SIZEOF(icqAdvStatusControls),
			    IsDlgButtonChecked(hwndDlg, IDC_ADVSTATUSENABLE));
			// no break!

		case IDC_ADVSTATUSICON:
			icq_EnableMultipleControls(hwndDlg, icqAdvStatusIconControls, SIZEOF(icqAdvStatusIconControls),
			    IsDlgButtonChecked(hwndDlg, IDC_ADVSTATUSENABLE) &&
			    IsDlgButtonChecked(hwndDlg, IDC_ADVSTATUSICON));
			break;
		}

		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			ppro->m_bASD = StoreDBCheckState(ppro, hwndDlg, IDC_ASD, "ASD");
			ppro->m_bPSD = StoreDBCheckState(ppro, hwndDlg, IDC_PSD, "PSD");
			ppro->m_bNoPSDForHidden = StoreDBCheckState(ppro, hwndDlg, IDC_NOPSD_FOR_HIDDEN, "NoPSDForHidden");
			state = IsDlgButtonChecked(hwndDlg, IDC_INCGLOBAL);
			ppro->m_bIncognitoGlobal = state;
			ppro->setSettingByte(NULL, "IncognitoGlobal", state);
			ppro->setSettingByte(NULL, "PrivacyMenuPlacement", IsDlgButtonChecked(hwndDlg, IDC_INSTATUSMENU));
			StoreDBCheckState(ppro, hwndDlg, IDC_USERCHKPOS, "UserScanPos");
			StoreDBCheckState(ppro, hwndDlg, IDC_ADVSTATUSENABLE, "AdvStatusEnabled");
			StoreDBCheckState(ppro, hwndDlg, IDC_ADVSTATUSICON, "AdvStatusIconShow");
			int value = SendDlgItemMessage(hwndDlg, IDC_ADVSTATUS_ICON_POS, CB_GETCURSEL, 0, 0);
			ppro->setSettingByte(NULL, "AdvStatusIconPosition", ++value);
			ppro->m_bTzersEnabled = StoreDBCheckState(ppro, hwndDlg, IDC_TZER, "EnableTzers");
			StoreDBCheckState(ppro, hwndDlg, IDC_SHOW_AUTH, "AlwaysShowAuthItems");
		}
		break;

	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcIcqEventsLogOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {

	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_REMOVE_FILE, "LogSelfRemoveFile", DEFAULT_SELFREMOVE_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_IGNORECHECK_FILE, "LogIgnoreCheckFile", DEFAULT_IGNORECHECK_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_CHECKSTATUS_FILE, "LogStatusCheckFile", DEFAULT_STATUSCHECK_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_CLIENTCHANGE_FILE, "LogClientChangeFile", DEFAULT_CLIENTCHANGE_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_AUTH_FILE, "LogAuthFile", DEFAULT_AUTH_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_INFOREQUEST_FILE, "LogInfoRequestFile", DEFAULT_INFOREQUEST_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_ASD_FILE, "LogASDFile", DEFAULT_ASD_LOG_TO_FILE);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_REMOVE_HISTORY, "LogSelfRemoveHistory", DEFAULT_SELFREMOVE_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_IGNORECHECK_HISTORY, "LogIgnoreCheckHistory", DEFAULT_IGNORECHECK_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_CHECKSTATUS_HISTORY, "LogStatusCheckHistory", DEFAULT_STATUSCHECK_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_CLIENTCHANGE_HISTORY, "LogClientChangeHistory", DEFAULT_CLIENTCHANGE_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_AUTH_HISTORY, "LogAuthHistory", DEFAULT_AUTH_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_INFOREQUEST_HISTORY, "LogInfoRequestHistory", DEFAULT_INFOREQUEST_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_ASD_HISTORY, "LogASDHistory", DEFAULT_ASD_LOG_TO_HISTORY);
		LoadDBCheckState(ppro, hwndDlg, IDC_LOG_TO_CONTACTS_HISTORY, "LogToContactsHistory", DEFAULT_LOG_TO_CONTACTS_HISTORY);
		{
			DBVARIANT dbv = { 0 };
			DBGetContactSettingTString(NULL, ppro->m_szModuleName, "EventsLogFile", &dbv);
			if (dbv.ptszVal && lstrlen(dbv.ptszVal) > 0)
				SetDlgItemText(hwndDlg, IDC_FILEPATH, dbv.ptszVal);
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_BROWSE), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_BROWSE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_FILE));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BROWSE:
			TCHAR str[MAX_PATH+2];
			OPENFILENAME ofn = { 0 };
			TCHAR filter[512], *pfilter;

			GetWindowText (GetWindow((HWND)lParam, GW_HWNDPREV), str, SIZEOF(str));
			ofn.lStructSize = CDSIZEOF_STRUCT(OPENFILENAME, lpTemplateName);
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_HIDEREADONLY;
			if (LOWORD(wParam) == IDC_BROWSE)
				ofn.lpstrTitle = TranslateT("Select the location of the file where events will be logged");
			else {
				ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.lpstrTitle = _T("");
			}
			_tcscpy(filter, TranslateT("All Files"));
			_tcscat(filter, _T(" (*.*)"));
			pfilter = filter + lstrlen(filter) + 1;
			_tcscpy(pfilter, _T("*"));
			pfilter = pfilter + lstrlen(pfilter) + 1;
			*pfilter = '\0';
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.nMaxFile = SIZEOF(str) - 2;
			ofn.nMaxFileTitle = MAX_PATH;
			if (LOWORD(wParam) == IDC_BROWSE) {
				if (!GetSaveFileName(&ofn))
					return 1;
			}
			else {
				if(!GetOpenFileName(&ofn))
				return 1;
			}
			SetWindowText(GetWindow((HWND)lParam, GW_HWNDPREV), str);
			break;

		}
		OptDlgChanged(hwndDlg);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			ppro->m_bLogSelfRemoveFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_REMOVE_FILE, "LogSelfRemoveFile");
			ppro->m_bLogIgnoreCheckFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_IGNORECHECK_FILE, "LogIgnoreCheckFile");
			ppro->m_bLogStatusCheckFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_CHECKSTATUS_FILE, "LogStatusCheckFile");
			ppro->m_bLogClientChangeFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_CLIENTCHANGE_FILE, "LogClientChangeFile");
			ppro->m_bLogAuthFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_AUTH_FILE, "LogAuthFile");
			ppro->m_bLogInfoRequestFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_REQUEST_FILE, "LogInfoRequestFile");
			ppro->m_bLogASDFile = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_ASD_FILE, "LogASDFile");
			ppro->m_bLogSelfRemoveHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_REMOVE_HISTORY, "LogSelfRemoveHistory");
			ppro->m_bLogIgnoreCheckHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_IGNORECHECK_HISTORY, "LogIgnoreCheckHistory");
			ppro->m_bLogStatusCheckHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_CHECKSTATUS_HISTORY, "LogStatusCheckHistory");
			ppro->m_bLogClientChangeHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_CLIENTCHANGE_HISTORY, "LogClientChangeHistory");
			ppro->m_bLogAuthHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_AUTH_HISTORY, "LogAuthHistory");
			ppro->m_bLogInfoRequestHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_REQUEST_HISTORY, "LogInfoRequestHistory");
			ppro->m_bLogASDHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_ASD_HISTORY, "LogASDHistory");
			ppro->m_bLogToContactsHistory = StoreDBCheckState(ppro, hwndDlg, IDC_LOG_TO_CONTACTS_HISTORY, "LogToContactsHistory");
			{
				TCHAR buf[255];
				GetDlgItemText(hwndDlg, IDC_FILEPATH, buf, sizeof(buf) - 1);
				DBWriteContactSettingTString(NULL, ppro->m_szModuleName, "EventsLogFile", lstrlen(buf) > 0 ? buf : _T(""));
			}
			return TRUE;
		}
		break;
	}
return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

const TCHAR* CIdComboBox[] =
{
	_T("Miranda IM"), _T("Unknown"), _T("QiP 2005a"), _T("YSM"), _T("pyICQ"), _T("&RQ"),
	_T("Jimm"), _T("Trillian"), _T("Licq"), _T("Kopete"), _T("ICQ for MAC"),
	_T("Miranda IM 6.6.6 [evil]"), _T("ICQ 5 (Rambler)"), _T("ICQ 5.1"), _T("ICQ 5 (abv)"),
	_T("ICQ (Netvigator)"), _T("Sim/MacOS X"), _T("Sim/Win32"), _T("Centericq"),
	_T("libicq2k"), _T("mChat"), _T("stICQ"), _T("KXicq2"), _T("QIP PDA (Windows)"),
	_T("QIP Mobile (Java)"), _T("ICQ 2002"), _T("ICQ 6"), _T("ICQ for Pocket PC"),
	_T("Anastasia"), _T("Virus"), _T("alicq"), _T("mICQ"), _T("StrICQ"), _T("vICQ"),
	_T("IM2"), _T("GAIM"), _T("ICQ99"), _T("WebICQ"), _T("SmartICQ"), _T("IM+"),
	_T("uIM"), _T("TICQClient"), _T("IC@"), _T("PreludeICQ"), _T("Qnext"), _T("ICQ Lite"),
	_T("QIP Infium"), _T("JICQ"),_T("SpamBot"), _T("MIP"), _T("Trillian Astra"),
	_T("R&Q"), _T("NanoICQ"), _T("IMadering"), _T("MirandaMobile")
};

const TCHAR* ModIdComboBox[] =
{
	_T("Regular by Joe@Whale"), _T(ICQ_BM_NAME), _T(ICQ_S7SSS_NAME),
	_T(CIQ_SIN_NAME), _T(ICQ_PLUG_NAME), _T(ICQ_PLUSPLUS_NAME)
};

static INT_PTR CALLBACK DlgProcIcqClientIDOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {

	case WM_INITDIALOG:
		int i;
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		LoadDBCheckState(ppro, hwndDlg, IDC_SHOWREALID, "HideRealID", DEFAULT_HIDE_REAL_ID);
		LoadDBCheckState(ppro, hwndDlg, IDC_VERSION_CHECKBOX, "ProtoVersionUseOwn", DEFAULT_PROTO_VERSION_USE_OWN);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SET_VERSION), IsDlgButtonChecked(hwndDlg, IDC_VERSION_CHECKBOX));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SET_VERSION_RANGE), IsDlgButtonChecked(hwndDlg, IDC_VERSION_CHECKBOX));
		SetDlgItemInt(hwndDlg, IDC_SET_VERSION, ppro->getSettingWord(NULL, "ProtoVersion", ICQ_VERSION), FALSE);
		LoadDBCheckState(ppro, hwndDlg, IDC_CUSTOM_CAP, "CustomCaps", DEFAULT_USE_CUSTOM_CAPS);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CAPSBOX), IsDlgButtonChecked(hwndDlg, IDC_CUSTOM_CAP));

		SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_RESETCONTENT, 0, 0);
		for (i = 0; i < SIZEOF(CIdComboBox); i++)
		{
			SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_ADDSTRING, 0, (LPARAM)CIdComboBox[i]);
		}
		SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_SETCURSEL, ppro->getSettingByte(NULL, "CurrentID", DEFAULT_CLIENT_ID), 0);

		SendDlgItemMessage(hwndDlg, IDC_MODID, CB_RESETCONTENT, 0, 0);
		for (i = 0; i < SIZEOF(ModIdComboBox); i++)
		{
			SendDlgItemMessage(hwndDlg, IDC_MODID, CB_ADDSTRING, 0, (LPARAM)ModIdComboBox[i]);
		}
		SendDlgItemMessage(hwndDlg, IDC_MODID, CB_SETCURSEL, ppro->getSettingByte(NULL, "CurrentModID", DEFAULT_MOD_ID), 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CHANGEID:
			{
				unsigned int id = SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_GETCURSEL, 0, 0);
				EnableDlgItem(hwndDlg, IDC_MODID, id == 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MODID_DESC), id == 0);
				SetDlgItemText(hwndDlg, IDC_FAKEWARNING, (id > 0) ?
					TranslateT("You've changed your Client ID!\nThis can result in serious messaging problems!\n\nIf you experience such problems, set your Client ID back to \"Miranda IM\"") :
					TranslateT("Because of stability reasons, be aware before changing your Client ID.\nSome configurations are not healthy and may lead to the messaging problems.\n\nDo NOT touch it before you know what you are doing :)")
				);
			}
			break;

		case IDC_VERSION_CHECKBOX:
			{
				BOOL ticked = IsDlgButtonChecked( hwndDlg, IDC_VERSION_CHECKBOX );
				EnableWindow( GetDlgItem( hwndDlg, IDC_SET_VERSION ), ticked );
				EnableWindow( GetDlgItem( hwndDlg, IDC_SET_VERSION_RANGE ), ticked );
			}
			break;

		case IDC_CUSTOM_CAP:
			{
				BOOL ticked = IsDlgButtonChecked( hwndDlg, IDC_CUSTOM_CAP );
				EnableWindow( GetDlgItem( hwndDlg, IDC_CAPSBOX ), ticked );
			}
			break;

		case IDC_CAPSBOX:
			ppro->ShowCapsListDialog( hwndDlg );
			break;
		}
		OptDlgChanged( hwndDlg );
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{

		case PSN_APPLY:
			UINT protoVer = 0;
			ppro->setSettingByte( NULL, "CurrentID", ( BYTE )SendDlgItemMessage( hwndDlg, IDC_CHANGEID, CB_GETCURSEL, 0, 0 ) );
			ppro->setSettingByte( NULL, "CurrentModID", ( BYTE )SendDlgItemMessage( hwndDlg, IDC_MODID, CB_GETCURSEL, 0, 0 ) );
			ppro->m_bHideIdEnabled = StoreDBCheckState( ppro, hwndDlg, IDC_SHOWREALID, "HideRealID" );
			ppro->m_bCustomProtoVersionEnabled = StoreDBCheckState( ppro, hwndDlg, IDC_VERSION_CHECKBOX, "ProtoVersionUseOwn" );
			protoVer = GetDlgItemInt( hwndDlg, IDC_SET_VERSION, 0, 0 );
			ppro->setSettingWord( NULL, "ProtoVersion", protoVer != 0 ? protoVer : ICQ_VERSION );
			ppro->m_bCustomCapsEnabled = StoreDBCheckState( ppro, hwndDlg, IDC_CUSTOM_CAP, "CustomCaps" );

			ppro->setUserInfo();
			break;
		}
		return TRUE;

	}
return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcIcqPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcIcqPopupMoreOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int CIcqProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	HMODULE hUxTheme = 0;

	if (IsWinVerXPPlus())
	{
		hUxTheme = GetModuleHandleA("uxtheme.dll");
		if (hUxTheme) 
			pfnEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
	}

	odp.cbSize = sizeof(odp);
	odp.position = -800000000;
	odp.hInstance = hInst;
	odp.ptszGroup = LPGENT("Network");
	odp.dwInitParam = LPARAM(this);
	odp.ptszTitle = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQ);
	odp.pfnDlgProc = DlgProcIcqOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQCONTACTS);
	odp.pfnDlgProc = DlgProcIcqContactsOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Main Features");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQFEATURES);
	odp.pfnDlgProc = DlgProcIcqFeaturesOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Advanced Features");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQFEATURESADV);
	odp.pfnDlgProc = DlgProcIcqFeaturesAdvOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Privacy");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQPRIVACY);
	odp.pfnDlgProc = DlgProcIcqPrivacyOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Log");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQEVENTSLOG);
	odp.pfnDlgProc = DlgProcIcqEventsLogOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("Client ID");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICQCLIENTID);
	odp.pfnDlgProc = DlgProcIcqClientIDOpts;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

	if (bPopUpService)
	{
		odp.position = 100000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUPS);
		odp.groupPosition = 900000000;
		odp.pfnDlgProc = DlgProcIcqPopupOpts;
		odp.ptszGroup = LPGENT("Popups");
		odp.ptszTab = LPGENT("Basic Popups");
		CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

		odp.position = 100000001;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUPS_MORE);
		odp.groupPosition = 900000001;
		odp.pfnDlgProc = DlgProcIcqPopupMoreOpts;
		odp.ptszTab = LPGENT("Additional Popups");
		CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	}
	return 0;
}
