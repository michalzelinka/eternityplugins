/*
astyle --force-indent=tab=4 --brackets=linux --indent-switches
		--pad=oper --one-line=keep-blocks  --unpad=paren

Chat module plugin for Miranda IM

Copyright (C) 2003 J�rgen Persson

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

$Id: colorchooser.c 7032 2008-01-04 02:08:55Z nightwish2004 $

*/

// this color chooser window is inspired by PeaCow's smiley chooser window for the Smileyadd plugin

#include "../commonheaders.h"
#include <math.h>

static int CalculateCoordinatesToButton(COLORCHOOSER * pCC, POINT pt)
{
	int iSquareRoot = (int)sqrt(pCC->pModule->nColorCount);
	int nCols = iSquareRoot * iSquareRoot < pCC->pModule->nColorCount ? iSquareRoot + 1 : iSquareRoot;

	int col = pt.x / 25;
	int row = (pt.y - 20) / 20;
	int pos = nCols * row + col;

	if (pt.y < 20 && pos >= pCC->pModule->nColorCount)
		pos = -1;

	return pos;
}

static RECT CalculateButtonToCoordinates(COLORCHOOSER * pCC, int buttonPosition)
{
	RECT pt;
	int iSquareRoot = (int)sqrt(pCC->pModule->nColorCount);
	int nCols = iSquareRoot * iSquareRoot < pCC->pModule->nColorCount ? iSquareRoot + 1 : iSquareRoot;

	int row = buttonPosition / nCols;
	int col = buttonPosition % nCols;

	pt.left = col * 25 + 1;
	pt.top = row * 20 + 20;
	pt.right = pt.left + 25 - 1;
	pt.bottom = pt.top + 20;

	return pt;
}

BOOL CALLBACK DlgProcColorToolWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static COLORCHOOSER* pCC = NULL;
	static int iCurrentHotTrack;
	static BOOL bChoosing;
	static int iRows;
	static int iColumns;
	static HWND hPreviousActiveWindow;

	switch (msg) {
		case WM_INITDIALOG: {
			RECT rc;
			int iSquareRoot;
			int width ;
			int height;

			TranslateDialogDefault(hwndDlg);
			pCC = (COLORCHOOSER*) lParam;

			iCurrentHotTrack = -2;
			bChoosing = FALSE;

			iSquareRoot = (int)sqrt(pCC->pModule->nColorCount);

			iColumns = iSquareRoot * iSquareRoot == pCC->pModule->nColorCount ? iSquareRoot : iSquareRoot + 1;
			iRows = iSquareRoot;

			rc.top = rc.left = 100;
			rc.right =  100 +  iColumns * 25 + 1;
			rc.bottom = iRows * 20 + 100 + 20;

			AdjustWindowRectEx(&rc, GetWindowLong(hwndDlg, GWL_STYLE), FALSE, GetWindowLong(hwndDlg, GWL_EXSTYLE));

			width = rc.right - rc.left;
			height = rc.bottom - rc.top;

			pCC->yPosition -= height;


			SetDlgItemText(hwndDlg, IDC_COLORTEXT, pCC->bForeground ? TranslateT("Text colour") : TranslateT("Background colour"));
			SetWindowPos(GetDlgItem(hwndDlg, IDC_COLORTEXT), NULL,  0, 0, width, 20, 0);
			SetWindowPos(hwndDlg, NULL, pCC->xPosition, pCC->yPosition, width, height, SWP_SHOWWINDOW);
		}
		break;

		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_COLORTEXT)) {
				SetTextColor((HDC)wParam, RGB(60, 60, 150));
				SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
				return (BOOL)GetSysColorBrush(COLOR_WINDOW);
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					if (iCurrentHotTrack >= 0)
						PostMessage(hwndDlg, WM_LBUTTONUP, 0, 0);
					break;
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
			}
			break;

		case WM_LBUTTONUP:
			if (iCurrentHotTrack >= 0 && iCurrentHotTrack < pCC->pModule->nColorCount && pCC->hWndTarget != NULL) {
				HWND hWindow;
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = 0;
				cf.dwEffects = 0;
				hWindow = GetParent(pCC->hWndTarget);

				if (pCC->bForeground) {
					pCC->si->bFGSet = TRUE;
					pCC->si->iFG = iCurrentHotTrack;
					if (IsDlgButtonChecked(hWindow, IDC_COLOR)) {
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = pCC->pModule->crColors[iCurrentHotTrack];
						SendMessage(pCC->hWndTarget, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				} else {
					pCC->si->bBGSet = TRUE;
					pCC->si->iBG = iCurrentHotTrack;
					if (IsDlgButtonChecked(hWindow, IDC_BKGCOLOR)) {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = pCC->pModule->crColors[iCurrentHotTrack];
						SendMessage(pCC->hWndTarget, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
			}
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
				PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			else if ((wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE))
				hPreviousActiveWindow = (HWND)lParam;
			break;

		case WM_MOUSEMOVE: {
			HDC hdc = GetDC(hwndDlg);
			POINT pt;
			RECT rect;
			int but;

			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);

			if (iCurrentHotTrack == -2)
				return 0; // prevent focussing when not drawn yet!

			but = CalculateCoordinatesToButton(pCC, pt);

			// weird stuff
			if (but != iCurrentHotTrack) {
				if (iCurrentHotTrack >= 0) {
					rect = CalculateButtonToCoordinates(pCC, iCurrentHotTrack);
					DrawFocusRect(hdc, &rect);
					iCurrentHotTrack = -1;
				}
				iCurrentHotTrack = but;

				if (iCurrentHotTrack >= 0) {
					rect = CalculateButtonToCoordinates(pCC, iCurrentHotTrack);
					DrawFocusRect(hdc, &rect);
				}
			}
			ReleaseDC(hwndDlg, hdc);
		}
		break;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			int i = 0;
			int iThisRow = 1;
			int iThisColumn = 0;

			GetClientRect(hwndDlg, &rc);

			rc.top += 20;

			hdc = BeginPaint(hwndDlg, &ps);

			// fill background
			FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));

			for (i; i < pCC->pModule->nColorCount; i++) {
				HBRUSH hbr;

				// decide place to draw the color block in the window
				iThisColumn ++;
				if (iThisColumn > iColumns) {
					iThisColumn = 1;
					iThisRow++;
				}

				if (pCC->bForeground && pCC->si->bFGSet && pCC->si->iFG == i ||
						!pCC->bForeground && pCC->si->bBGSet && pCC->si->iBG == i) {
					rc.top = (iThisRow - 1) * 20 + 1 + 20 ;
					rc.left = (iThisColumn - 1) * 25 + 1 + 1 ;
					rc.bottom = iThisRow * 20 - 1 + 20 ;
					rc.right = iThisColumn * 25 - 1 ;

					DrawEdge(hdc, &rc, EDGE_RAISED, BF_TOP | BF_LEFT | BF_RIGHT | BF_BOTTOM);
				}

				rc.top = (iThisRow - 1) * 20 + 3 + 20 ;
				rc.left = (iThisColumn - 1) * 25 + 3 + 1 ;
				rc.bottom = iThisRow * 20 - 3 + 20 ;
				rc.right = iThisColumn * 25 - 3 ;

				FillRect(hdc, &rc, GetStockObject(BLACK_BRUSH));

				hbr = CreateSolidBrush(pCC->pModule->crColors[i]);

				rc.top = (iThisRow - 1) * 20 + 4 + 20;
				rc.left = (iThisColumn - 1) * 25 + 4 + 1;
				rc.bottom = iThisRow * 20 - 4 + 20;
				rc.right = iThisColumn * 25 - 4;

				FillRect(hdc, &rc, hbr);
				DeleteObject(hbr);
			}

			EndPaint(hwndDlg, &ps);
			iCurrentHotTrack = -1;
		}
		break;

		case WM_CLOSE:
			SetFocus(pCC->hWndTarget);
			DestroyWindow(hwndDlg);
			break;

		case WM_DESTROY:
			mir_free(pCC);
			return TRUE;
	}

	return FALSE;
}
