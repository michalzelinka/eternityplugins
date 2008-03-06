/*
astyle	--force-indent=tab=4 --brackets=linux --indent-switches
		--pad=oper --one-line=keep-blocks  --unpad=paren

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

$Id: srmm.c 7266 2008-02-13 16:01:10Z nightwish2004 $
*/

#include "commonheaders.h"

int LoadSendRecvMessageModule(void);
int SplitmsgShutdown(void);
int LogErrorMessage(HWND hwndDlg, struct MessageWindowData *dat, int i, char *szMsg);
void Chat_Load(PLUGINLINK *link);
void Chat_Unload();

DWORD g_mirandaVersion = 0;

PLUGINLINK *pluginLink;
HINSTANCE g_hInst;
extern MYGLOBALS myGlobals;
struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;

pfnSetMenuInfo fnSetMenuInfo = NULL;

/*/ eternity changelog
2.0.0.31 - svn merge, no time to fix/create somethin' x)
2.0.0.21 - svn merge
2.0.0.11 - translation fixes, something more I forgot x)
2.0.0.10 - SVN merge, ended support for < 0.7 cores, icons and sounds for your comfort :)
1.1.1.204 - fixed statusbar button click fix, typing sounds in Chat module :)
1.1.1.203 - added new xstatus support (if your icq plugin supports it of course :]), Status bar Typing sounds button, Code revision x).. min mirver: 0.7.0.44 (pr8)
1.1.1.202 - 0.7 PR 8/0.8 alpha #1 SVN merge..
1.1.1.201 - SVN merge..
1.1.1.001 - Updater support (I hope :)), repaired broken General options dialog x)
1.1.1.000 - merged with latest SVN sources
1.1.0.204 - merged with latest SVN sources - important Unicode changes..
1.1.0.201 - translation support of some strings not tslt'd in original mod
1.1.0.200 - merged with latest SVN sources..
1.1.0.192 - added Mod's own MUUIDs, merged with latest SVN sources..
1.1.0.191 - merged with latest SVN sources..
1.1.0.186 - merged with latest SVN sources, minor fixes..
1.1.0.184 - merged with latest SVN sources..
1.1.0.183 - merged with latest SVN sources..
1.1.0.182 - merged with latest SVN sources..
1.1.0.181 - merged with latest SVN sources..
1.1.0.180 - merged with latest SVN sources..
1.1.0.175 - merged with latest SVN sources..
1.1.0.174 - merged with latest SVN sources..
1.1.0.173 - code reworked, now collisions of sounds between breaking new line and sending via Enter or Shift+Enter have gone :) .. but still have to think about Double Enter procedure xo)) ..
1.1.0.172 - Options->Message Sessions dialog slightly redesigned to fit all the content.. Typing Sound checkbox is now placed in its own Mod group :) ..
1.1.0.171 - merged with latest SVN sources..
1.1.0.162 - code cleaning, comments cleaning xo) ..
1.1.0.161 - merged with latest SVN sources..
1.1.0.151 - initial release, with working typing sound and backspace sound, new line sound doesn't work very well :o/ ..

// eternity mod versioning */
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
#ifdef _UNICODE
#ifdef __GNUWIN32__
	"tabSRMM eternity mod (Unicode - MINGW32)",
#else
	"tabSRMM eternity mod (Unicode)",
#endif
#else
#ifdef __GNUWIN32__
	"tabSRMM eternity mod (ANSI - MINGW32)",
#else
	"tabSRMM eternity mod (ANSI)",
#endif
#endif
	PLUGIN_MAKE_VERSION(2, 0, 1, 0),
	"Chat module for instant messaging and group chat, offering a tabbed interface and many advanced features.",
	"The Miranda developers team",
	"mike.taussick@seznam.cz, silvercircle@gmail.com",
	"© 2000-2008 Miranda Project",
	"http://dev.mirandaim.ru/jarvis/, http://tabsrmm.sourceforge.net/",
	UNICODE_AWARE,
	DEFMOD_SRMESSAGE,            // replace internal version (if any)
#ifdef _UNICODE
	{0xa4f2d0d4, 0x2c38, 0x439a, { 0xbd, 0xfb, 0x0b, 0x3e, 0xf1, 0x06, 0x2d, 0x7d }} //{a4f2d0d4-2c38-439a-bdfb-0b3ef1062d7d}
#else
  {0xeb96173f, 0x56cb, 0x46f4, { 0xb2, 0xfa, 0xa3, 0xe4, 0x6e, 0x19, 0xed, 0xff }} //{eb96173f-56cb-46f4-b2fa-a3e46e19edff}
#endif  
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
  if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0, 8, 0, 9 ) ) 
  {
    MessageBox( NULL, "tabSRMM plugin cannot be loaded. It requires Miranda IM 0.8 alpha build #9 or later.", "tabSRMM plugin", 
      MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST );
    return NULL;
  }
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_SRMM, MIID_CHAT, MIID_LAST};

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	pluginLink = link;

#ifdef _DEBUG //mem leak detector :-) Thanks Tornado!
	{
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag
		flag |= (_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF); // Turn on leak-checking bit
		_CrtSetDbgFlag(flag); // Set flag to the new value
	}
#endif

	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);

	if (!ServiceExists(MS_DB_EVENT_GETTEXT)) {
		MessageBox(0, _T("tabSRMM"), _T("Critical error. Unsupported database driver found. tabSRMM will be disabled"), MB_OK);
		return 1;
	}

	fnSetMenuInfo = (pfnSetMenuInfo)GetProcAddress(GetModuleHandleA("USER32.DLL"), "SetMenuInfo");

	Chat_Load(pluginLink);
	return LoadSendRecvMessageModule();
}

int __declspec(dllexport) Unload(void)
{
	Chat_Unload();
	return SplitmsgShutdown();
}

#if defined(_UNICODE)
int _DebugTraceW(const wchar_t *fmt, ...)
{
	wchar_t debug[2048];
	int     ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	lstrcpyW(debug, L"TABSRMM: ");

	_vsnwprintf(&debug[9], ibsize - 10, fmt, va);
#ifdef _DEBUG
	OutputDebugStringW(debug);
#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
			char *szDebug = Utf8_Encode(debug);
			fputs(szDebug, f);
			fputs("\n", f);
			fclose(f);
			if (szDebug)
				free(szDebug);
		}
	}
#endif
	return 0;
}
#endif

int _DebugTraceA(const char *fmt, ...)
{
	char    debug[2048];
	int     ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	lstrcpyA(debug, "TABSRMM: ");
	_vsnprintf(&debug[9], ibsize - 10, fmt, va);
#ifdef _DEBUG
	OutputDebugStringA(debug);
#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "tabsrmm_debug.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
			fputs(debug, f);
			fputs("\n", f);
			fclose(f);
		}
	}
#endif
	return 0;
}

/*
 * output a notification message.
 * may accept a hContact to include the contacts nickname in the notification message...
 * the actual message is using printf() rules for formatting and passing the arguments...
 *
 * can display the message either as systray notification (baloon popup) or using the
 * popup plugin.
 */

int _DebugPopup(HANDLE hContact, const TCHAR *fmt, ...)
{
	va_list	va;
	TCHAR		debug[1024];
	int			ibsize = 1023;

	va_start(va, fmt);
	_vsntprintf(debug, ibsize, fmt, va);

	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY tn;
		TCHAR	szTitle[128];

		tn.szProto = NULL;
		tn.cbSize = sizeof(tn);
		mir_sntprintf(szTitle, safe_sizeof(szTitle), TranslateT("tabSRMM Message (%s)"), (hContact != 0) ? (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR) : TranslateT("Global"));
#if defined(_UNICODE)
		tn.tszInfoTitle = szTitle;
		tn.tszInfo = debug;
#else
		tn.szInfoTitle = szTitle;
		tn.szInfo = debug;
#endif
		tn.dwInfoFlags = NIIF_INFO;
#if defined(_UNICODE)
		tn.dwInfoFlags |= NIIF_INTERN_UNICODE;
#endif
		tn.uTimeout = 1000 * 4;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & tn);
	}
	return 0;
}

BOOL CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	COLORREF url_visited = RGB(128, 0, 128);
	COLORREF url_unvisited = RGB(0, 0, 255);

	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			{
				int h;
				HFONT hFont;
				LOGFONT lf;

				hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TABSRMM, WM_GETFONT, 0, 0);
				GetObject(hFont, sizeof(lf), &lf);
				h = lf.lfHeight;
				lf.lfHeight = (int)(lf.lfHeight * 1.5);
				lf.lfWeight = FW_BOLD;
				hFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_TABSRMM, WM_SETFONT, (WPARAM)hFont, 0);
				lf.lfHeight = h;
				hFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_SETFONT, (WPARAM)hFont, 0);
			}
			{
				char str[64];
				DWORD v = pluginInfo.version;
				char szVersion[512], *found = NULL, buildstr[50] = "";
				UINT build_nr = 0;

				CallService(MS_SYSTEM_GETVERSIONTEXT, 500, (LPARAM)szVersion);
				if ((found = strchr(szVersion, '#')) != NULL) {
					build_nr = atoi(found + 1);
					mir_snprintf(buildstr, 50, "[Build #%d]", build_nr);
				}
#if defined(_UNICODE)
				mir_snprintf(str, sizeof(str), "%s %d.%d.%d.%d (Unicode) %s", Translate("Version"), HIBYTE(HIWORD(v)), LOBYTE(HIWORD(v)), HIBYTE(LOWORD(v)), LOBYTE(LOWORD(v)), buildstr);
#else
				mir_snprintf(str, sizeof(str), "%s %d.%d.%d.%d %s", Translate("Version"), HIBYTE(HIWORD(v)), LOBYTE(HIWORD(v)), HIBYTE(LOWORD(v)), LOBYTE(LOWORD(v)), buildstr);
#endif
				SetDlgItemTextA(hwndDlg, IDC_VERSION, str);
				mir_snprintf(str, sizeof(str), Translate("Built %s %s"), __DATE__, __TIME__);
				SetDlgItemTextA(hwndDlg, IDC_BUILDTIME, str);
			}
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)myGlobals.g_iconContainer);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_SUPPORT:
					CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://miranda.or.at/");
					break;
			}
			break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_WHITERECT)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_TABSRMM)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_VERSION)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_BUILDTIME)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_COPYRIGHT)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_SUPPORT)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_LOGO)) {
				if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_TABSRMM))
					SetTextColor((HDC)wParam, RGB(180, 10, 10));
				else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_VERSION))
					SetTextColor((HDC)wParam, RGB(70, 70, 70));
				else
					SetTextColor((HDC)wParam, RGB(0, 0, 0));
				SetBkColor((HDC)wParam, RGB(255, 255, 255));
				return (BOOL)GetStockObject(WHITE_BRUSH);
			}
			break;
		case WM_DESTROY: {
			HFONT hFont;

			hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TABSRMM, WM_GETFONT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_TABSRMM, WM_SETFONT, SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0), 0);
			DeleteObject(hFont);
			hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_GETFONT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_SETFONT, SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0), 0);
			DeleteObject(hFont);

		}
		break;
	}
	return FALSE;
}

#define _DBG_STR_LENGTH_MAX 2048
int _DebugMessage(HWND hwndDlg, struct MessageWindowData *dat, const char *fmt, ...)
{
	va_list	va;
	char		szDebug[_DBG_STR_LENGTH_MAX];

	va_start(va, fmt);
	_vsnprintf(szDebug, _DBG_STR_LENGTH_MAX, fmt, va);
	szDebug[_DBG_STR_LENGTH_MAX - 1] = '\0';

	LogErrorMessage(hwndDlg, dat, -1, szDebug);
	return 0;
}

// eternity: Release Notes Procedure
INT_PTR CALLBACK RelNotesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          DBWriteContactSettingWord( NULL, SRMSGMOD_T, "MOD_RelNotes", TABSRMM_RELNOTES );
          EndDialog(hwndDlg, LOWORD(wParam));
          break;
      }
    }
    break;
  }
  return FALSE;
}

