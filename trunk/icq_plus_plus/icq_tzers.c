// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006,2007 Joe Kucera, LubomirR
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
// File name      : $Source: /cvsroot/miranda/miranda/protocols/IcqOscarJ/icq_servlist.c,v $
// Revision       : $Revision$
// Last change on : $Date$
// Last change by : $Author$
//
// DESCRIPTION:
//
//  tZers
//
// -----------------------------------------------------------------------------


#include "icqoscar.h"

CALLBACK tZersWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int DialogWidth = 0;
	int DialogHeight = 0;
	int x = GetSystemMetrics(SM_CXSCREEN);
	int y = GetSystemMetrics(SM_CYSCREEN);
	int newleft = 0;
	int newtop = 0;
	int curSel;
	int len;
	char * txt;
	char * txt2;
	char * tzid;
	char * tzurl;
	RECT rect;

	static HANDLE hContact;
	static DWORD dwUin;

	switch (uMsg) {
		case WM_INITDIALOG:
			hContact = (HANDLE)lParam;
			if (!hContact) {
				EndDialog(hWnd, 0);
				return FALSE;
			}

			if (!(dwUin = ICQGetContactSettingUIN(hContact))) {
				EndDialog(hWnd, 0);
				return FALSE;
			}

			GetWindowRect(hWnd, &rect);
			DialogWidth = rect.right - rect.left;
			DialogHeight = rect.bottom - rect.top;

			newleft = ((x + 1) - DialogWidth) / 2;
			newtop = ((y + 1) - DialogHeight) / 2;

			MoveWindow(hWnd, newleft, newtop, DialogWidth, DialogHeight, TRUE);

			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Gangsta"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Can't Hear U"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Scratch"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Booooo"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Kisses"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Chill Out!"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Akitaka"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("I'm Sorry"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Hilaaarious"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Like Duh!"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("L8R"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)("Like U!"));
			SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_SETCURSEL, 0, 0);

			SendDlgItemMessage(hWnd, IDC_TZER_NAME, WM_SETTEXT, 0, (LPARAM)"Gangsta");

			ShowWindow(hWnd, SW_SHOW);
			break;
		case WM_COMMAND:
			if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_TZER_COMBO) {
				curSel = SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_GETCURSEL, 0, 0);
				switch(curSel) {
					case 0:
						txt = "Gangsta";
						break;
					case 1:
						txt = "Can't Hear U";
						break;
					case 2:
						txt = "Scratch";
						break;
					case 3:
						txt = "Booooo";
						break;
					case 4:
						txt = "Kisses";
						break;
					case 5:
						txt = "Chill Out!";
						break;
					case 6:
						txt = "Akitaka";
						break;
					case 7:
						txt = "I'm Sorry";
						break;
					case 8:
						txt = "Hilaaarious";
						break;
					case 9:
						txt = "Like Duh!";
						break;
					case 10:
						txt = "L8R";
						break;
					case 11:
						txt = "Like U!";
						break;
				}
				SendDlgItemMessage(hWnd, IDC_TZER_NAME, WM_SETTEXT, 0, (LPARAM)txt);
			}

			if (LOWORD(wParam) == IDOK) {
				curSel = SendDlgItemMessage(hWnd, IDC_TZER_COMBO, CB_GETCURSEL, 0, 0);
				len = SendDlgItemMessage(hWnd, IDC_TZER_NAME, WM_GETTEXTLENGTH, 0, 0);
				txt = malloc(len + 1);
				if (txt) {
					SendDlgItemMessage(hWnd, IDC_TZER_NAME, WM_GETTEXT, (WPARAM)(len + 1), (LPARAM)txt);
					switch(curSel) {
          // eternity TODO: czech & slovak equivalents
          // slovak: common > distId/18113
          // czech:  not available? x))
						case 0:
							tzid = "gangSh";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/gangsterSheep.swf";
							break;
						case 1:
							tzid = "cantH";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/cant_hear.swf";
							break;
						case 2:
							tzid = "scratch";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/scratch.swf";
							break;
						case 3:
							tzid = "boo";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/boo.swf";
							break;
						case 4:
							tzid = "kisses";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/kisses.swf";
							break;
						case 5:
							tzid = "rasta";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/rastamab.swf";
							break;
						case 6:
							tzid = "arakiri";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/sappuko.swf";
							break;
						case 7:
							tzid = "sorry";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/sorry.swf";
							break;
						case 8:
							tzid = "laugh";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/laugh.swf";
							break;
						case 9:
							tzid = "da";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/dahh.swf";
							break;
						case 10:
							tzid = "beback";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/beBack.swf";
							break;
						case 11:
							tzid = "ilikeu";
							tzurl = "http://c.icq.com/xtraz/products/teaser/anims/common/iLikeU.swf";
							break;
					}
					txt2 = MangleXml(txt, len);
					free(txt);
					SendtZer(hContact, dwUin, "ID", txt2, tzurl);
					free(txt2);
					EndDialog(hWnd, 0);
				}
			}

			if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hWnd, 0);
			}
	}
	return FALSE;
}
