// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
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
// Revision       : $Revision: 36 $
// Last change on : $Date: 2007-08-05 03:45:10 +0300 (Ð’Ñ, 05 Ð°Ð²Ð³ 2007) $
// Last change by : $Author: sss123next $
//
//
// DESCRIPTION:
//
//  Server List/Manager GUI
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

static BOOL CALLBACK DlgProcSrvsList(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLong(hwndDlg, GWL_USERDATA);

	char buf2[64];
	TCHAR buf3[64];
	WORD m_wSrvsCount = 0;
	WORD m_wSrvsCountNew = 0;
	int iIndex, i, iRow;
	LVCOLUMN col;
	LVITEM item;
	HWND hwndList = GetDlgItem(hwndDlg, IDC_SERVERS);
	NMLISTVIEW * hdr = (NMLISTVIEW*)lParam;

	switch(message)
	{

	case WM_INITDIALOG:
		{
			char szSrvsDBKeyName[64];
			ppro = (CIcqProto*)lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			mir_snprintf(szSrvsDBKeyName, 64, "%sSrvs", ppro->m_szModuleName);
			ICQTranslateDialog(hwndDlg);
			col.pszText = TranslateT("Use");
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.fmt = LVCFMT_LEFT;
			col.cx = 33;
			ListView_InsertColumn(hwndList, 0, &col);
			col.pszText = TranslateT("Server");
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.fmt = LVCFMT_LEFT;
			col.cx = 214;
			ListView_InsertColumn(hwndList, 1, &col);
			col.pszText = TranslateT("Port");
			col.cx = 36;			
			ListView_InsertColumn(hwndList, 2, &col);
			ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

			m_wSrvsCount = DBGetContactSettingWord(NULL, szSrvsDBKeyName, "SrvCount", 0x0000);

			for(i = 1; i <= m_wSrvsCount; i++)
			{
				DBVARIANT dbv = { DBVT_TCHAR };
				lstrcpyA(dbv.pszVal, "");
				item.mask = LVIF_TEXT;
				item.iItem = i;
				item.iSubItem = 0;
				item.pszText = _T("<server>");
				iRow = ListView_InsertItem(hwndList, &item);
				ListView_SetItemText(hwndList, iRow, 0, NULL);
				mir_snprintf(buf2, 64, "server%luhost", i);
				DBGetContactSettingTString(NULL, szSrvsDBKeyName, buf2, &dbv);
				if (!dbv.ptszVal) break;
				ListView_SetItemText(hwndList, iRow, 1, dbv.ptszVal);
				mir_snprintf(buf2, 64, "server%luport", i);
				mir_sntprintf(buf3, sizeof(buf3), _T("%d"), DBGetContactSettingWord(NULL, szSrvsDBKeyName, buf2, 0));
				ListView_SetItemText(hwndList, iRow, 2, buf3);
				mir_snprintf(buf2, 64, "server%luuse", i);
				if (DBGetContactSettingByte(NULL, szSrvsDBKeyName, buf2, 1))
				ListView_SetItemState(hwndList, iRow, 0x2000, 0xF000);
			}
			//ListView_SetColumnWidth(hwndList, 0..2, LVSCW_AUTOSIZE);
			EnableWindow(ppro->m_hSrvsListCaller, FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		{
			ZeroMemory(&item, sizeof(LVITEM));
			switch (LOWORD(wParam))
			{
			case IDADD:
				GetDlgItemText(hwndDlg, IDC_EDIT_SERVER, buf3, 64);
				if(buf3[0] == 0)
					break;
				item.mask = LVIF_TEXT;
				item.iItem = ListView_GetItemCount(hwndList);
				item.iSubItem = 0;
				item.pszText = _T("<server>");
				iRow = ListView_InsertItem(hwndList, &item);
				ListView_SetItemText(hwndList, iRow, 0, NULL);
				ListView_SetItemText(hwndList, iRow, 1, buf3);
				GetDlgItemText(hwndDlg, IDC_EDIT_PORT, buf3, 64);
				if(buf3[0] == 0)
					mir_sntprintf(buf3, sizeof(buf3), _T("%d"), DEFAULT_SERVER_PORT);
				ListView_SetItemText(hwndList, iRow, 2, buf3);
				ListView_SetItemState(hwndList, iRow, 0x2000, 0xF000);
				break;

			case IDEDIT:
				iIndex = ListView_GetSelectionMark(hwndList);
				if (iIndex < 0) break;
				GetDlgItemText(hwndDlg, IDC_EDIT_SERVER, buf3 , 64);
				if(buf3[0] == 0)
					break;
				ListView_SetItemText(hwndList, iIndex, 1, buf3);
				GetDlgItemText(hwndDlg, IDC_EDIT_PORT, buf3 , 64);
				if(buf3[0] == 0)
					mir_sntprintf(buf3, sizeof(buf3), _T("%d"), DEFAULT_SERVER_PORT);
				ListView_SetItemText(hwndList, iIndex, 2, buf3);
				break;

			case IDDEL:
				iIndex = ListView_GetSelectionMark(hwndList);
				if(iIndex < 0) return FALSE;
				ListView_DeleteItem(hwndList, iIndex);
				break;        

			case IDCCLOSE:
				//SendMessage(hCaller, WM_INITDIALOG, 0, 0);
				SendMessage(GetParent(hwndDlg), WM_INITDIALOG, 0, 0);
				DestroyWindow(hwndDlg);
				break;

			}
		}
		break;

	case WM_NOTIFY:
		if (hdr && hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible(hdr->hdr.hwndFrom))
		{
			iIndex = hdr->iItem;
			if (iIndex == -1)
			{
				SetDlgItemText(hwndDlg, IDC_EDIT_SERVER, _T(""));
				SetDlgItemText(hwndDlg, IDC_EDIT_PORT, _T(""));
			}
			else {
				ListView_GetItemText(hwndList, iIndex, 1, buf3, 64);
				SetDlgItemText(hwndDlg, IDC_EDIT_SERVER, buf3);
				ListView_GetItemText(hwndList, iIndex, 2, buf3, 64);
				SetDlgItemText(hwndDlg, IDC_EDIT_PORT, buf3);
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		{
			char szSrvsDBKeyName[64];

			mir_snprintf(szSrvsDBKeyName, 64, "%sSrvs", ppro->m_szModuleName);
			m_wSrvsCount = DBGetContactSettingWord(NULL, szSrvsDBKeyName, "SrvCount", 0x0000);
			m_wSrvsCountNew = ListView_GetItemCount(hwndList);
			for (i = 0; i < m_wSrvsCount; i++)
			{
				mir_snprintf(buf2, 64, "server%luhost", i + 1);
				DBDeleteContactSetting(NULL, szSrvsDBKeyName, buf2);
				mir_snprintf(buf2, 64, "server%luport", i + 1);
				DBDeleteContactSetting(NULL, szSrvsDBKeyName, buf2);
				mir_snprintf(buf2, 64, "server%luuse", i + 1);
				DBDeleteContactSetting(NULL, szSrvsDBKeyName, buf2);
			}
			for (i = 0; i < m_wSrvsCountNew; i++)
			{
				int iState = ListView_GetItemState(hwndList, i, LVIS_STATEIMAGEMASK);
				mir_snprintf(buf2, 64, "server%luhost", i + 1);
				ListView_GetItemText(hwndList, i, 1, buf3, 64);
				DBWriteContactSettingTString(NULL, szSrvsDBKeyName, buf2, buf3);
				mir_snprintf(buf2, 64, "server%luport", i + 1);
				ListView_GetItemText(hwndList, i, 2, buf3, 64);
				SetDlgItemText(hwndDlg, IDC_EDIT_PORT, buf3);
				DBWriteContactSettingWord(NULL, szSrvsDBKeyName, buf2, GetDlgItemInt(hwndDlg, IDC_EDIT_PORT, 0, 0));
				mir_snprintf(buf2, 64, "server%luuse", i + 1);
				DBWriteContactSettingByte(NULL, szSrvsDBKeyName, buf2, (BYTE)(iState & 0x2000 ? 1 : 0));
			}
			DBWriteContactSettingWord(NULL, szSrvsDBKeyName, "SrvCount", m_wSrvsCountNew);
			ppro->m_hSrvsList = NULL;
		}
		EnableWindow(ppro->m_hSrvsListCaller, TRUE);
		// todo: reload combobox in main icq opts dialog
		break;

	}
	return FALSE;
}

void CIcqProto::ShowSrvsListDialog(HWND hwndCaller)
{
	if (m_hSrvsList == NULL)
	{
		m_hSrvsList = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SRVS_LIST), NULL, DlgProcSrvsList, LPARAM(this));
		m_hSrvsListCaller = GetParent(hwndCaller);
	}
	SetForegroundWindow(m_hSrvsList);
}
