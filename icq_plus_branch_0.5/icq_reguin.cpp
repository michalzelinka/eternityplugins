// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
// Copyright © 2008 SSS, jarvis
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
// UIN Registration Implementation
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

static INT_PTR CALLBACK DlgProcRegDialog( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	CIcqProto* ppro = ( CIcqProto* )GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch ( message )
	{

	case WM_INITDIALOG:
		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		ICQTranslateDialog( hwndDlg );
		break;

	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{

		case IDC_LOOKUPLINK:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_FORGOT_PASSWORD);
			return TRUE;
			
		case IDC_NEWUINLINK:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_REGISTER);
			return TRUE;

		case IDC_PICTURE:
			if ( ppro->icqOnline( ) )
			{
              if (MessageBox(NULL, TranslateT("If you continue, you will lose current connection with a server.\nContinue?"), TranslateT("Warning"), MB_YESNO) == IDYES)
				ppro->icq_requestRegImage(hwndDlg);
			}
			else
                ppro->icq_requestRegImage(hwndDlg);
			return TRUE;

		case IDC_REGISTER:
			{
			    char password[128];
				char regimage[128];
				GetDlgItemTextA(hwndDlg, IDC_NEWUIN_PASS, password, sizeof(password));
				GetDlgItemTextA(hwndDlg, IDC_PICTURE_CONTENT, regimage, sizeof(regimage));
				ppro->icq_registerNewUin(password, regimage);
			}
			return TRUE;

		}
		break;

	case WM_CLOSE:
		DestroyWindow( hwndDlg );
		break;

	case WM_DESTROY:
		ppro->m_hRegDialog = NULL;
		EnableWindow( ppro->m_hRegDialogCaller, TRUE );
		break;

	}
	return FALSE;
}

void CIcqProto::ShowRegUINDialog( HWND hwndCaller )
{
	if ( m_hRegDialog == NULL )
	{
		m_hRegDialog = CreateDialogParam( hInst, MAKEINTRESOURCE( IDD_ICQREGUIN ), NULL, DlgProcRegDialog, LPARAM( this ) );
		m_hRegDialogCaller = GetParent( hwndCaller );
		EnableWindow( m_hRegDialogCaller, FALSE );
	}
	SetForegroundWindow( m_hRegDialog );
}
