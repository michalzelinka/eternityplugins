// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
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
				if (MessageBox(NULL, TranslateT("If you continue, you will lose current connection with a server.\nContinue?"), TranslateT("Warning"), MB_YESNO) != IDYES)
					return TRUE;

			ppro->regUin_requestImage(hwndDlg);
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

void CIcqProto::regUin_sendImageRequest( )
{
	icq_packet packet;

	SetDlgItemText(m_hRegDialog, IDC_NEWUININFO, TranslateT("Requesting image..."));  
	
	packet.wLen = 4;
	write_flap(&packet, ICQ_LOGIN_CHAN);
	packDWord(&packet, 0x00000001);
	sendServPacket(&packet);  // greet login server
	
	serverPacketInit(&packet, (WORD)(18));
	packFNACHeader(&packet, ICQ_AUTHORIZATION_FAMILY, ICQ_SIGNON_REQUEST_IMAGE, 0, 0);
	packTLV(&packet, 0x0001, 0, 0);
	packDWord(&packet, 0x00000000);
	sendServPacket(&packet);  // request image
}

void CIcqProto::regUin_requestImage(HWND hwndDlg)
{
	icq_serverDisconnect(FALSE);
	m_iIcqNewStatus = ID_STATUS_OFFLINE;
	m_hRegDialog = hwndDlg;
	m_bRegImageRequested = 1;
	SetDlgItemText(m_hRegDialog, IDC_NEWUININFO, TranslateT("Connecting to server..."));  
	icq_login(NULL);
	icq_regNewUin = 1;
}
