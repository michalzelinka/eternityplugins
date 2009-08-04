// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006,2007 Joe Kucera
// Copyright © 2006,2007,2008 [sss], chaos.persei, [sin], Faith Healer, Theif, nullbie, jarvis
// Copyright © 2008 [sss], chaos.persei, nullbie, baloo, jarvis
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
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
//
// DESCRIPTION:
//
//  Capabilities List GUI
//
// TODO:
//
//  Rewrite whole file into human-acceptable form :))
//  Add binary/BLOB capabilities support
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

static INT_PTR CALLBACK DlgProcCapsList( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	CIcqProto* ppro = ( CIcqProto* )GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	char buf2[64];
	TCHAR buf3[64];
	WORD m_wCapsCountNew = 0;
	int i;
	LVCOLUMN col;
	LVITEM item;
	HWND hwndList = GetDlgItem( hwndDlg, IDC_CAPABILITIES );
	NMLISTVIEW * hdr = ( NMLISTVIEW * )lParam;

	switch ( message )
	{

	case WM_INITDIALOG:
		{
			char szCapsDBKeyName[64];
			ppro = (CIcqProto*)lParam;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
			mir_snprintf( szCapsDBKeyName, 64, "%sCaps", ppro->m_szModuleName );
			ICQTranslateDialog( hwndDlg );
			col.pszText = TranslateT( "Capability" );
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.fmt = LVCFMT_LEFT;
			col.cx = 265;
			ListView_InsertColumn( hwndList, 0, &col );
			ListView_SetExtendedListViewStyleEx( hwndList, 0, LVS_EX_FULLROWSELECT );

			for ( i = 1; ; i++ )
			{
				DBVARIANT dbv = { DBVT_TCHAR };
				lstrcpy( dbv.ptszVal, _T( "" ) );
				mir_snprintf( buf2, 64, "cap%luname", i );
				DBGetContactSettingTString( NULL, szCapsDBKeyName, buf2, &dbv );
				if ( !dbv.ptszVal )
					break;
				else
				{
					item.mask = LVIF_TEXT;
					item.iItem = i;
					item.iSubItem = 0;
					item.pszText = _T( "<capability>" );
					int row = ListView_InsertItem( hwndList, &item );
					ListView_SetItemText( hwndList, row, 0, dbv.ptszVal );
				}
			}
			ppro->m_wCapsCount = ListView_GetItemCount( hwndList );
		}
		return TRUE;

	case WM_COMMAND:
		{
			ZeroMemory( &item, sizeof( LVITEM ) );
			switch ( LOWORD( wParam ) )
			{

			case IDC_CAPL_ADD:
				GetDlgItemText( hwndDlg, IDC_CAPL_EDIT, buf3, 64 );
				if( buf3[0] == 0 )
					break;
				item.mask = LVIF_TEXT;
				item.iItem = ListView_GetItemCount( hwndList );
				item.iSubItem = 0;
				item.pszText = NULL;
				i = ListView_InsertItem( hwndList, &item );
				ListView_SetItemText( hwndList, i, 0, buf3 );
				break;

			case IDC_CAPL_MODIFY:
				i = ListView_GetSelectionMark( hwndList );
				if ( i < 0 ) return FALSE;
				GetDlgItemText( hwndDlg, IDC_CAPL_EDIT, buf3 , 64 );
				ListView_SetItemText( hwndList, i, 0, buf3 );
				break;

			case IDC_CAPL_DELETE:
				i = ListView_GetSelectionMark( hwndList );
				if( i < 0 ) return FALSE;
				ListView_DeleteItem( hwndList, i );
				break;

			case IDOK:
			case IDCANCEL:
				DestroyWindow( hwndDlg );
				//SendMessage( hCaller, WM_INITDIALOG, 0, 0 );
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if ( hdr && hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible( hdr->hdr.hwndFrom ) && hdr->iItem != ( -1 ) )
		{
			i = hdr->iItem;
			if( i < 0 ) return FALSE;
			ListView_GetItemText( hwndList, i, 0, buf3, 64 );
			SetDlgItemText( hwndDlg, IDC_CAPL_EDIT, buf3 );
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;

	case WM_DESTROY:
		{
			char szCapsDBKeyName[64];

			m_wCapsCountNew = ListView_GetItemCount( hwndList );
			mir_snprintf( szCapsDBKeyName, 64, "%sCaps", ppro->m_szModuleName );

			for ( int i = 0; i < m_wCapsCountNew; i++ )
			{
				mir_snprintf( buf2, 64, "cap%luname", i + 1 );
				ListView_GetItemText( hwndList, i, 0, buf3, 64 )
				DBWriteContactSettingTString( NULL, szCapsDBKeyName, buf2, buf3 );
			}
			for ( int i = m_wCapsCountNew + 1; i <= ppro->m_wCapsCount; i++ )
			{
				mir_snprintf( buf2, 64, "cap%luname", i );
				DBDeleteContactSetting( NULL, szCapsDBKeyName, buf2 );
			}
			ppro->m_hCapsList = NULL;
			EnableWindow( ppro->m_hCapsListCaller, TRUE );
		}
		break;
	}
	return FALSE;
}

void CIcqProto::ShowCapsListDialog( HWND hwndCaller )
{
	if ( m_hCapsList == NULL )
	{
		m_hCapsList = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_CAPS_LIST ), NULL, DlgProcCapsList, LPARAM( this ) );
		m_hCapsListCaller = GetParent( hwndCaller );
		EnableWindow( m_hCapsListCaller, FALSE );
	}
	SetForegroundWindow( m_hCapsList );
}
