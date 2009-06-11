// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
// Copyright © 2008 LubomirR, jarvis
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
// DESCRIPTION:
//
// tZers Implementation
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

TzerInfo tZers[TZER_COUNT] = {
	{ LPGENT("Gangsta'"), "gangSh", TZER_URL_GANGSTERSHEEP },
	{ LPGENT("Can't Hear U"), "cantH", TZER_URL_CANTHEARYOU },
	{ LPGENT("Scratch"), "scratch", TZER_URL_SCRATCH },
	{ LPGENT("Booooo"), "boo", TZER_URL_BOO },
	{ LPGENT("Kisses"), "kisses", TZER_URL_KISSES },
	{ LPGENT("Chill Out"), "rasta", TZER_URL_RASTAMAB },
	{ LPGENT("Akitaka"), "arakiri", TZER_URL_SAPPUKO },
	{ LPGENT("Hilaaarious"), "laugh", TZER_URL_LAUGH },
	{ LPGENT("Like Duh!"), "da", TZER_URL_DAHH },
	{ LPGENT("L8R"), "beback", TZER_URL_BEBACK },
	{ LPGENT("Like U!"), "ilikeu", TZER_URL_ILIKEYOU },
	{ LPGENT("I'm Sorry"), "sorry", TZER_URL_SORRY },
};

static INT_PTR CALLBACK SendTzerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int selection = 0;
	SendTzerDlgParam* dat = (SendTzerDlgParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {

	case WM_INITDIALOG:
		dat = (SendTzerDlgParam*)lParam;
		if (!dat->hContact || !dat->ppro->icqOnline())
			EndDialog(hwndDlg, 0);

		ICQTranslateDialog(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		SetDlgItemInt(hwndDlg, IDC_TZER_RECIPIENT, dat->ppro->getContactUin(dat->hContact), FALSE);
		SendDlgItemMessage(hwndDlg, IDC_TZER_EDIT, WM_SETTEXT, 0, (LPARAM)tZers[selection].ptszText);
		for (int i = 0; i < SIZEOF(tZers); i++)
			SendDlgItemMessage(hwndDlg, IDC_TZER_COMBO, CB_ADDSTRING, 0, (LPARAM)tZers[i].ptszText);

		SendDlgItemMessage(hwndDlg, IDC_TZER_COMBO, CB_SETCURSEL, 0, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_TZER_COMBO:
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				selection = SendDlgItemMessage(hwndDlg, IDC_TZER_COMBO, CB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_TZER_EDIT, WM_SETTEXT, 0, (LPARAM)tZers[selection].ptszText);
			}
			return TRUE;

		case IDOK:
			if (dat->ppro->icqOnline())
			{
				TZERTOSEND tts;
				CCSDATA ccs;
				PROTORECVEVENT pre = {0};

				selection = SendDlgItemMessage(hwndDlg, IDC_TZER_COMBO, CB_GETCURSEL, 0, 0);
				tts.hContact = dat->hContact;
				tts.pszText = (char *)malloc(1024 * sizeof(char)); // TODO: remove 1023 chars limit
				GetDlgItemTextA(hwndDlg, IDC_TZER_EDIT, tts.pszText, 1023);
				tts.pszText = MangleXml(tts.pszText, 1023);
				tts.pszURL = tZers[selection].pszURL;
				dat->ppro->SendTzer(&tts);

				{
					char* szStr = (char *)malloc(17 + strlen(tts.pszText) + strlen(tts.pszURL) + 1);
					sprintf(szStr, "You have tZed: %s\r\n%s", tts.pszText, tts.pszURL);
					ccs.szProtoService = PSR_MESSAGE;
					ccs.hContact = tts.hContact;
					ccs.wParam = 0;
					ccs.lParam = (LPARAM)&pre;
					pre.flags = 0; // how could I use TCHAR flag, when it accepts only (char) message? x)
					pre.timestamp = time(NULL);
					pre.szMessage = szStr;
					pre.lParam = 0;

					CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
				}
				//free(tts.pszText);
				SAFE_FREE((void**)&tts.pszText);

				EndDialog(hwndDlg, 0);
			}
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		return TRUE;
	}

	return FALSE;
}


int CIcqProto::SendTzerDialog(WPARAM wParam, LPARAM lParam)
{
	SendTzerDlgParam param = { this, (HANDLE)wParam };
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SENDTZER), NULL, SendTzerDlgProc, (LPARAM)&param);
	return 0;
}


int CIcqProto::SendTzerService(WPARAM wParam, LPARAM lParam)
{
	SendTzer((TZERTOSEND *)wParam);
	return 0;
}
