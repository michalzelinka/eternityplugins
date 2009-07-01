/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009 Michal Zelinka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

#pragma once

#include "common.h"

INT_PTR CALLBACK FBAccountProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto;

	switch ( message )
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWL_USERDATA,lparam);

		DBVARIANT dbv;
		if( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_ID,&dbv) )
		{
			SetDlgItemTextA(hwnd,IDC_UN,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		if ( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_PASS,&dbv) )
		{
			CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
				reinterpret_cast<LPARAM>(dbv.pszVal));
			SetDlgItemTextA(hwnd,IDC_PW,dbv.pszVal);
			DBFreeVariant(&dbv);
		}

		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDC_NEWACCOUNTLINK )
		{
			CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>
				( FACEBOOK_URL_HOMEPAGE ) );
			return TRUE;
		}

		if ( HIWORD( wparam ) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus() )
		{
			switch(LOWORD(wparam))
			{
			case IDC_UN:
			case IDC_PW:
				SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
			}
		}
		break;

	case WM_NOTIFY:
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWL_USERDATA));
			char str[128];

			GetDlgItemTextA(hwnd,IDC_UN,str,sizeof(str));
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_ID,str);

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_PASS,str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBMindProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto;

	switch(message)
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWL_USERDATA,lparam);
		SendDlgItemMessage(hwnd,IDC_MINDMSG,EM_LIMITTEXT,FACEBOOK_MIND_LIMIT,0);
		SetDlgItemText(hwnd,IDC_CHARACTERS,_T(FACEBOOK_MIND_LIMIT_TEXT));
		SetDlgItemText(hwnd,IDC_NAME,_T("<User's real name>")); // TODO

		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDOK )
		{
			char mindMessage[FACEBOOK_MIND_LIMIT+1];
			proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWL_USERDATA));

			GetDlgItemTextA(hwnd,IDC_MINDMSG,mindMessage,SIZEOF(mindMessage));
			ShowWindow(hwnd,SW_HIDE);

//			char *narrow = mir_t2a_cp(mindMessage,CP_UTF8); // TODO: Unicodization example
//			ForkThread(&TwitterProto::SendMindWorker, proto,narrow);

			proto->facy.set_status( mindMessage );

			EndDialog(hwnd, wparam); 
			return TRUE;
		}
		else if ( LOWORD( wparam ) == IDCANCEL )
		{
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		else if ( LOWORD( wparam ) == IDC_MINDMSG && HIWORD( wparam ) == EN_CHANGE )
		{
			size_t len = SendDlgItemMessage(hwnd,IDC_MINDMSG,WM_GETTEXTLENGTH,0,0);
			char str[4];
			_snprintf(str,sizeof(str),"%d",FACEBOOK_MIND_LIMIT-len);
			SetDlgItemTextA(hwnd,IDC_CHARACTERS,str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	return FALSE;
}

INT_PTR CALLBACK FBPopupsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	return FALSE;
}

INT_PTR CALLBACK FBDebugDialogProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	switch(message) 
	{

	case WM_INITDIALOG: {
		TranslateDialogDefault(hwnd);

		HFONT hFont = CreateFont( 14, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 2, TEXT( "Courier New" ) );
		SendMessage(GetDlgItem( hwnd, IDC_DEBUGINFO  ), WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		DeleteObject( hFont );

		char* debugInfo = reinterpret_cast< char* >(lparam);
		SetDlgItemTextA( hwnd, IDC_DEBUGINFO, debugInfo );
		} return TRUE;

	case WM_COMMAND:
		if(LOWORD(wparam) == IDCLOSE)
		{
			CloseWindow( hwnd );
			DestroyWindow( hwnd );
		}
		break;

	}

	return FALSE;
}