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
//  Code for User details ICQ specific pages
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#define SVS_NORMAL        0
#define SVS_GENDER        1
#define SVS_ZEROISUNSPEC  2
#define SVS_IP            3
#define SVS_COUNTRY       4
#define SVS_MONTH         5
#define SVS_SIGNED        6
#define SVS_TIMEZONE      7
#define SVS_ICQVERSION    8
#define SVS_TIMESTAMP     9
#define SVS_STATUSID      10
#define SVS_ADVSTATUSID   11

#define SVS_FLAGS         0xFF
#define SVSF_GRAYED       0x100
#define SVSF_NORMAL       0x200
#define AVATAR_HASH_FLASH 0x08

#define HM_DBCHANGE     (WM_USER+200)
#define HM_PROTOACK     (WM_USER+201)
#define TIMERID         1
#define TIMEOUT_IGNORE  2
#define TIMEOUT_CAPS    3
#define TIMEOUT_IP      4

char* MirandaVersionToString(char* szStr, int bUnicode, int v, int m);

/////////////////////////////////////////////////////////////////////////////////////////

// TODO: Check TCHAR issues (especially this whole method (its string outputs) should be fixed in ICQOscarJ)
static BYTE SetValue(CIcqProto* ppro, HWND hwndDlg, int idCtrl, HANDLE hContact, char* szModule, char* szSetting, WORD type)
{
	DBVARIANT dbv = {0};
	char str[MAX_PATH];
	char* pstr = NULL;
	int unspecified = 0;
	int special = type & SVS_FLAGS;
	int bUtf = 0, bDbv = 0, bAlloc = 0;

	dbv.type = DBVT_DELETED;

	if ((hContact == NULL) && ((int)szModule<0x100))
	{
		dbv.type = (BYTE)szModule;

		switch((int)szModule) {
		case DBVT_BYTE:
			dbv.cVal = (BYTE)szSetting;
			break;
		case DBVT_WORD:
			dbv.wVal = (WORD)szSetting;
			break;
		case DBVT_DWORD:
			dbv.dVal = (DWORD)szSetting;
			break;
		case DBVT_ASCIIZ:
			dbv.pszVal = pstr = szSetting;
			break;
		default:
			unspecified = 1;
			dbv.type = DBVT_DELETED;
		}
	}
	else
	{
		if (szModule == NULL)
			unspecified = 1;
		else
		{
			unspecified = DBGetContactSetting(hContact, szModule, szSetting, &dbv);
			bDbv = 1;
		}
	}

	if (!unspecified)
	{
		switch (dbv.type) {
		case DBVT_BYTE:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
			pstr = _itoa(special == SVS_SIGNED ? dbv.cVal:dbv.bVal, str, 10);
			break;

		case DBVT_WORD:
			if (special == SVS_ICQVERSION)
			{
				if (dbv.wVal != 0)
				{
					char szExtra[80];

					null_snprintf(str, 250, "%d", dbv.wVal);
					pstr = str;

					if (hContact && ppro->IsDirectConnectionOpen(hContact, DIRECTCONN_STANDARD, 1))
					{
						ICQTranslateUtfStatic(LPGEN(" (DC Established)"), szExtra, 80);
						strcat(str, (char*)szExtra);
						bUtf = 1;
					}
				}
				else
					unspecified = 1;
			}
			else if (special == SVS_STATUSID)
			{
				char *pXName;
				char *pszStatus;
				BYTE bXStatus = ppro->getContactXStatus(hContact);

				pszStatus = MirandaStatusToStringUtf(dbv.wVal);
				if (bXStatus)
				{
					pXName = ppro->getSettingStringUtf(hContact, DBSETTING_XSTATUS_NAME, NULL);
					if (!strlennull(pXName))
					{ // give default name
						pXName = ICQTranslateUtf(nameXStatus[bXStatus-1]);
					}
					null_snprintf(str, sizeof(str), "%s (%s)", pszStatus, pXName);
					SAFE_FREE((void**)&pXName);
				}
				else
					null_snprintf(str, sizeof(str), (char*)pszStatus);

				bUtf = 1;
				SAFE_FREE((void**)&pszStatus);
				pstr = str;
				unspecified = 0;
			}
			else if (special == SVS_ADVSTATUSID)
			{
				switch (dbv.wVal)
				{
				case ICQ_STATUSF_DEPRESS:
					pstr = Translate(LPGEN("Depression"));
					break;
				case ICQ_STATUSF_EVIL:
					pstr = Translate(LPGEN("Evil"));
					break;
				case ICQ_STATUSF_LUNCH:
					pstr = Translate(LPGEN("Lunch"));
					break;
				case ICQ_STATUSF_WORK:
					pstr = Translate(LPGEN("@ Work"));
					break;
				case ICQ_STATUSF_HOME:
					pstr = Translate(LPGEN("@ Home"));
					break;
				default:
					pstr = Translate(LPGEN("None"));
				}
			}
			else
			{
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
				pstr = _itoa(special == SVS_SIGNED ? dbv.sVal:dbv.wVal, str, 10);
			}
			break;

		case DBVT_DWORD:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
			if (special == SVS_IP)
			{
				struct in_addr ia;
				ia.S_un.S_addr = htonl(dbv.dVal);
				pstr = inet_ntoa(ia);
				if (dbv.dVal == 0)
					unspecified=1;
			}
			else if (special == SVS_TIMESTAMP)
			{
				if (dbv.dVal == 0)
					unspecified = 1;
				else
					pstr = time2text(dbv.dVal);
			}
			else
				pstr = _itoa(special == SVS_SIGNED ? dbv.lVal:dbv.dVal, str, 10);
			break;

		case DBVT_ASCIIZ:
		case DBVT_WCHAR:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			if (!unspecified && pstr != szSetting)
			{
				pstr = ppro->getSettingStringUtf(hContact, szModule, szSetting, NULL);
				bUtf = 1;
				bAlloc = 1;
			}
			if (idCtrl == IDC_UIN)
				SetDlgItemTextUtf(hwndDlg, IDC_UINSTATIC, ICQTranslateUtfStatic(LPGEN("ScreenName:"), str, MAX_PATH));
			break;

		default:
			pstr = str;
			strcpy(str,"???");
			break;
		}
	}

	EnableDlgItem(hwndDlg, idCtrl, !unspecified);
	if (unspecified)
		SetDlgItemTextUtf(hwndDlg, idCtrl, ICQTranslateUtfStatic(LPGEN("<not specified>"), str, MAX_PATH));
	else if (bUtf)
		SetDlgItemTextUtf(hwndDlg, idCtrl, pstr);
	else
		SetDlgItemTextA(hwndDlg, idCtrl, pstr);

	if (bDbv)
		ICQFreeVariant(&dbv);

	if (bAlloc)
		SAFE_FREE((void**)&pstr);

	if (type > SVS_FLAGS)
		EnableWindow(GetDlgItem(hwndDlg, idCtrl), !(type & SVSF_GRAYED));

	return !unspecified;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK IcqDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (( PSHNOTIFY* )lParam )->lParam );
				break;

			case PSN_INFOCHANGED:
				{
					CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

          if (!ppro)
            break;

					char* szProto;
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;

					if (hContact == NULL)
						szProto = ppro->m_szModuleName;
					else
						szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

					if (!szProto)
						break;

					SetValue(ppro, hwndDlg, IDC_UIN, hContact, szProto, UNIQUEIDSETTING, SVS_NORMAL);
					SetValue(ppro, hwndDlg, IDC_ONLINESINCE, hContact, szProto, "LogonTS", SVS_TIMESTAMP);
					SetValue(ppro, hwndDlg, IDC_IDLETIME, hContact, szProto, "IdleTS", SVS_TIMESTAMP);
					SetValue(ppro, hwndDlg, IDC_IP, hContact, szProto, "IP", SVS_IP);
					SetValue(ppro, hwndDlg, IDC_REALIP, hContact, szProto, "RealIP", SVS_IP);

					if (hContact)
					{
						SetValue(ppro, hwndDlg, IDC_PORT, hContact, szProto, "UserPort", SVS_ZEROISUNSPEC);
						if (ppro->getSettingByte(hContact, "ClientID", 0))
							ppro->setSettingDword(hContact, "TickTS", 0);
						SetValue(ppro, hwndDlg, IDC_SYSTEMUPTIME, hContact, szProto, "TickTS", SVS_TIMESTAMP);
						SetValue(ppro, hwndDlg, IDC_STATUS, hContact, szProto, "Status", SVS_STATUSID);
						SetValue(ppro, hwndDlg, IDC_ADVSTATUS, hContact, szProto, "AdvStatus", SVS_ADVSTATUSID);
					}
					else
					{
						char str[MAX_PATH];

						SetValue(ppro, hwndDlg, IDC_PORT, hContact, (char*)DBVT_WORD, (char*)ppro->wListenPort, SVS_ZEROISUNSPEC);
						SetDlgItemTextUtf(hwndDlg, IDC_SUPTIME, ICQTranslateUtfStatic(LPGEN("Member since:"), str, MAX_PATH));
						SetValue(ppro, hwndDlg, IDC_SYSTEMUPTIME, hContact, szProto, "MemberTS", SVS_TIMESTAMP);
						SetValue(ppro, hwndDlg, IDC_STATUS, hContact, (char*)DBVT_WORD, (char*)ppro->m_iStatus, SVS_STATUSID);
					}
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg),msg,wParam,lParam);
			break;
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK IcqClientDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (( PSHNOTIFY* )lParam )->lParam );
				break;

			case PSN_INFOCHANGED:
				{
					CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

					if (!ppro)
						break;

					char* szProto;
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;

					if (hContact == NULL)
						szProto = ppro->m_szModuleName;
					else
						szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

					if (!szProto)
						break;

					if (hContact)
					{
						SetValue(ppro, hwndDlg, IDC_VERSION, hContact, szProto, "Version", SVS_ICQVERSION);
						SetValue(ppro, hwndDlg, IDC_MIRVER, hContact, szProto, "MirVer", SVS_ZEROISUNSPEC);
					}
					else
					{
						char str[MAX_PATH];

						SetValue(ppro, hwndDlg, IDC_VERSION, hContact, (char*)DBVT_WORD, (char*)ICQ_VERSION, SVS_ICQVERSION);
						SetValue(ppro, hwndDlg, IDC_MIRVER, hContact, (char*)DBVT_ASCIIZ, MirandaVersionToString((char*)str, gbUnicodeCore, ICQ_PLUG_VERSION, MIRANDA_VERSION), SVS_ZEROISUNSPEC); // TODO
					}

					{ // Plus fields // TODO: Check TCHAR issues
						DBVARIANT dbv = {0};
						DBCONTACTGETSETTING dbcgs = {0};
						dbcgs.szModule = ppro->m_szModuleName;
						dbcgs.szSetting = "CapBuf";
						dbcgs.pValue = &dbv;
						CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&dbcgs);

						// Set flags
						SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_RESETCONTENT, 0, 0);

						if ((dbv.type == DBVT_BLOB) && (dbv.cpbVal))
						{
							int bufsize = dbv.cpbVal;
							int capsize = 0x10;
							BYTE *buf = dbv.pbVal;

							SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_SETITEMDATA,
							    SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_ADDSTRING, 0, (LPARAM)"01."), 0);
							SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_SETITEMDATA,
							    SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_ADDSTRING, 0, (LPARAM)"02."), 0);
							SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_SETITEMDATA,
							    SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_ADDSTRING, 0, (LPARAM)"03."), 0);
							SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_SETITEMDATA,
							    SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_ADDSTRING, 0, (LPARAM)"04."), 0);
							SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_SETITEMDATA,
							    SendDlgItemMessage(hwndDlg, IDC_CLIENTCAPS, LB_ADDSTRING, 0, (LPARAM)"05."), 0);

							while (bufsize > 0) // search the buffer for a capability
							{
								ICQ_CAPINFO *info = (ICQ_CAPINFO *)malloc(sizeof(ICQ_CAPINFO));
								char* capName = ppro->GetCapabilityName(buf, info);
								SendDlgItemMessageA(hwndDlg, IDC_CLIENTCAPS, LB_SETITEMDATA,
								    SendDlgItemMessageA(hwndDlg, IDC_CLIENTCAPS, LB_ADDSTRING, 0, (LPARAM)capName), (LPARAM)info);
								buf += 0x10;
								bufsize -= 0x10;
							}
							//ListView_SortItems(GetDlgItem(hwndDlg, IDC_CLIENTCAPS), CompareFunc, (LPARAM)GetDlgItem(hwndDlg, IDC_CLIENTCAPS));
						}
						CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);

						if (ServiceExists(MS_FP_GETCLIENTICON))
						{
							DBCONTACTGETSETTING dbcgs = {0};
							dbcgs.szModule = ppro->m_szModuleName;
							dbcgs.szSetting = "MirVer";
							dbcgs.pValue = &dbv;
							dbv.type = DBVT_TCHAR; // To get proper DBVT string type
							CallService(MS_DB_CONTACT_GETSETTING_STR, (WPARAM)hContact, (LPARAM)&dbcgs);

							// DBVT "MirVer" option string type should be exact to plugin character set
							if (dbv.type == DBVT_TCHAR)
							{
								SendDlgItemMessage(hwndDlg, IDC_CLIENTICON, STM_SETICON,
								    (WPARAM)CallService(MS_FP_GETCLIENTICON, (WPARAM)mir_t2a(dbv.ptszVal), 1), 0);
							}
							else
							{
								SendDlgItemMessage(hwndDlg, IDC_CLIENTICON, STM_SETICON,
								    (WPARAM)CallService(MS_FP_GETCLIENTICON, (WPARAM)"[Undetected client]", 1), 0);
							}
							CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
						}
					}
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg),msg,wParam,lParam);
			break;
		}
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;
			if (lpmis->itemID == -1)
				return FALSE;
			lpmis->itemHeight = 18;
		}
		return TRUE;

	case WM_DRAWITEM: // TODO: Check TCHAR issues
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;

			if (lpdis->itemID == -1)
				return FALSE;

			HWND hwndList = GetDlgItem(hwndDlg, lpdis->CtlID);
			ICQ_CAPINFO* info = (ICQ_CAPINFO *)ListBox_GetItemData(hwndList, lpdis->itemID);

			if (info)
			{
				if (lpdis->itemState & ODS_SELECTED)
				{
					HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
					FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
					SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
					DeleteObject(hbr);
				}
				else
				{
					HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
					FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
					SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
					DeleteObject(hbr);
				}

				SIZE sz = {0};
				SetBkMode(lpdis->hDC, TRANSPARENT);
				GetTextExtentPoint32A(lpdis->hDC, info->name, lstrlenA(info->name), &sz);

				char str[256];
				ListBox_GetText(hwndList, lpdis->itemID, str);

				TextOutA(lpdis->hDC, lpdis->rcItem.left+3+16+1,
				    (lpdis->rcItem.top+lpdis->rcItem.bottom-sz.cy)/2,
				    info->name, lstrlenA(info->name));
			}
			else
			{
				char str[256];
				HBRUSH hbr = CreateSolidBrush(RGB(0x60, 0x60, 0x60));
				FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
				SetTextColor(lpdis->hDC, RGB(0xff, 0xff, 0xff));
				DeleteObject(hbr);

				ListBox_GetText(hwndList, lpdis->itemID, str);

				if (!lstrcmpA(str, "01."))
				{
					lstrcpyA(str, Translate("Universal Protocol Features"));
				}
				else if (!lstrcmpA(str, "02."))
				{
					lstrcpyA(str, Translate("Miranda Features"));
				}
				else if (!lstrcmpA(str, "03."))
				{
					lstrcpyA(str, Translate("Miranda Features (Unknown)"));
				}
				else if (!lstrcmpA(str, "04."))
				{
					lstrcpyA(str, Translate("Unknown Flags (Decoded)"));
				}
				else if (!lstrcmpA(str, "05."))
				{
					lstrcpyA(str, Translate("Unknown Flags (Other)"));
				}
				else
				{
					if (lpdis->itemState & ODS_SELECTED)
					{
						HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
						FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
						SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
						DeleteObject(hbr);
					}
					else
					{
						HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
						FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
						SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
						DeleteObject(hbr);
					}
				}

				{
					SIZE sz;
					SetBkMode(lpdis->hDC, TRANSPARENT);
					GetTextExtentPoint32A(lpdis->hDC, str, lstrlenA(str), &sz);
					TextOutA(lpdis->hDC, lpdis->rcItem.left+3,
					    (lpdis->rcItem.top+lpdis->rcItem.bottom-sz.cy)/2,
					    str, lstrlenA(str));
				}
			}
		}
		return TRUE;

	case WM_DELETEITEM:
		{
			DELETEITEMSTRUCT* lpdis = (DELETEITEMSTRUCT *)lParam;
			ICQ_CAPINFO* info = (ICQ_CAPINFO *)ListBox_GetItemData(lpdis->hwndItem, lpdis->itemID);
			if (info) free(info);
		}
		return TRUE;

	case WM_VKEYTOITEM: // TODO: Check TCHAR issues
		{
			char ch = (char)LOWORD(wParam);
			if (((ch == 'C') || (ch == VK_INSERT)) && (GetKeyState(VK_CONTROL) & 0x80))
			{
				int count = ListBox_GetCount((HWND)lParam);
				int bufsize = 1; // for the \0
				char *buf, *bufptr;

				for (int i = 0; i < count; ++i)
				{
					if (!ListBox_GetSel((HWND)lParam, i)) continue;
					ICQ_CAPINFO* info = (ICQ_CAPINFO *)ListBox_GetItemData((HWND)lParam, i);
					if (!info) continue;
					bufsize += lstrlenA(info->caps) + 5 + lstrlenA(info->name) + 2;
				}
				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bufsize);
				bufptr = buf = (char *)GlobalLock(hMem);
				for (int i = 0; i < count; ++i)
				{
					if (!ListBox_GetSel((HWND)lParam, i)) continue;
					ICQ_CAPINFO* info = (ICQ_CAPINFO *)ListBox_GetItemData((HWND)lParam, i);
					if (!info) continue;
					bufptr += mir_snprintf(bufptr, bufsize-(bufptr-buf), "%s, // %s\r\n", info->caps, info->name);
				}
				GlobalUnlock(hMem);
				OpenClipboard(hwndDlg);
				EmptyClipboard();
				SetClipboardData(CF_TEXT, hMem);
				CloseClipboard();
				return (INT_PTR)(-2);
			}
			else if ((ch == 'A') && (GetKeyState(VK_CONTROL) & 0x80))
			{
				SendMessage((HWND)lParam, LB_SELITEMRANGE, TRUE, MAKELPARAM(0, ListBox_GetCount((HWND)lParam)));
				return (INT_PTR)(-2);
			}
			return (INT_PTR)(-1);
		}

	}

	return FALSE;  
}

/////////////////////////////////////////////////////////////////////////////////////////

int CIcqProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if ((!IsICQContact((HANDLE)lParam)) && lParam)
		return 0;

	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.flags = ODPF_TCHAR|ODPF_USERINFOTAB;
	odp.hInstance = hInst;
	odp.dwInitParam = LPARAM(this);
	odp.pfnDlgProc = IcqDlgProc;
//	odp.position = -1900000000; // Obsolete, UInfo shows this dialog tabbed, UInfoEx has own implementation
	odp.ptszTitle = m_tszUserName;

	if (!lParam)
	{
		odp.ptszTab = LPGENT("Details");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_CHANGEINFO);
		odp.pfnDlgProc = ChangeInfoDlgProc;
		CallService( MS_USERINFO_ADDPAGE, wParam, ( LPARAM )&odp );
	}

	odp.ptszTab = LPGENT("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_ICQ);
	odp.pfnDlgProc = IcqDlgProc;
	CallService( MS_USERINFO_ADDPAGE, wParam, ( LPARAM )&odp );

	odp.ptszTab = LPGENT("User Client");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_ICQ_CLIENT);
	odp.pfnDlgProc = IcqClientDlgProc;
	CallService( MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp );

	return 0;
}
