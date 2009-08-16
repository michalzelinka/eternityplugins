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

File name      : $URL$
Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

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
		if( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,&dbv) )
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
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,str);

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
		{
			DBVARIANT dbv = { DBVT_TCHAR };
			DBGetContactSettingTString(NULL,proto->m_szModuleName,FACEBOOK_KEY_NAME,&dbv);
			SetDlgItemText(hwnd,IDC_NAME,dbv.ptszVal);
			DBFreeVariant( &dbv );
		}

		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDOK )
		{
			TCHAR mindMessage[FACEBOOK_MIND_LIMIT+1];
			proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWL_USERDATA));

			GetDlgItemText(hwnd,IDC_MINDMSG,mindMessage,SIZEOF(mindMessage));
			ShowWindow(hwnd,SW_HIDE);

			char *narrow = mir_t2a_cp(mindMessage,CP_UTF8); // TODO: narrow leak
			ForkThread(&FacebookProto::SendMindWorker, proto,narrow);

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
			TCHAR str[4];
			_sntprintf( str, 4, TEXT( "%d" ), FACEBOOK_MIND_LIMIT-len );
			SetDlgItemText(hwnd,IDC_CHARACTERS,str);

			SetDlgItemText(hwnd,IDOK, (len > 0) ? TEXT("Share") : TEXT("Clear"));

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	FacebookProto *proto;

	switch ( message )
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<FacebookProto*>(lparam);
		SetWindowLong(hwnd,GWL_USERDATA,lparam);

		DBVARIANT dbv;
		if( !DBGetContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,&dbv) )
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

//		SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, 8, 0); // TODO: Is there any length limit?

		if( !DBGetContactSettingTString(0,proto->ModuleName(),FACEBOOK_KEY_DEF_GROUP,&dbv) )
		{
			SetDlgItemText(hwnd,IDC_GROUP,dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		for(size_t i=0; i<SIZEOF(user_agents); i++)
		{
			SendDlgItemMessageA(hwnd,IDC_AGENT,CB_INSERTSTRING,i,
				reinterpret_cast<LPARAM>(user_agents[i]));
		}
		SendDlgItemMessage(hwnd, IDC_AGENT, CB_SETCURSEL,
		    DBGetContactSettingByte(NULL, proto->m_szModuleName, "UserAgent", 0), 0);

		return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDC_NEWACCOUNTLINK )
		{
			CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>
				( FACEBOOK_URL_HOMEPAGE ) );
			return TRUE;
		}

		if ( ( HIWORD( wparam ) == EN_CHANGE ) ||
		     ( HIWORD( wparam ) == CBN_EDITCHANGE )
		     && reinterpret_cast<HWND>(lparam) == GetFocus() )
		{
			SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
		}
		break;

	case WM_NOTIFY:
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			proto = reinterpret_cast<FacebookProto*>(GetWindowLong(hwnd,GWL_USERDATA));
			char str[128]; TCHAR tstr[128];

			GetDlgItemTextA(hwnd,IDC_UN,str,sizeof(str));
			DBWriteContactSettingString(0,proto->ModuleName(),FACEBOOK_KEY_LOGIN,str);

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			DBWriteContactSettingString(NULL,proto->m_szModuleName,FACEBOOK_KEY_PASS,str);

			GetDlgItemText(hwnd,IDC_GROUP,tstr,sizeof(tstr));
			if ( lstrlen( tstr ) > 0 )
			{
				DBWriteContactSettingTString(NULL,proto->m_szModuleName,FACEBOOK_KEY_DEF_GROUP,tstr);
				CallService( MS_CLIST_GROUPCREATE, 0, (LPARAM)tstr );
			}
			else
				DBDeleteContactSetting(NULL,proto->m_szModuleName,FACEBOOK_KEY_DEF_GROUP);

			DBWriteContactSettingByte(NULL, proto->m_szModuleName, "UserAgent",
			    SendDlgItemMessage(hwnd, IDC_AGENT, CB_GETCURSEL, 0, 0));

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK FBPopupsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	return FALSE;
}

INT_PTR CALLBACK FBDebugDialogProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	HFONT hFont = NULL;

	switch(message) 
	{

	case WM_INITDIALOG: {
		TranslateDialogDefault(hwnd);

		hFont = CreateFont( 14, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 2, TEXT( "Courier New" ) );
		SendMessage(GetDlgItem( hwnd, IDC_DEBUGINFO  ), WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

		char* debugInfo = reinterpret_cast< char* >(lparam);
		SetDlgItemTextA( hwnd, IDC_DEBUGINFO, debugInfo );

		SetClipboardData( CF_TEXT, ( HANDLE )debugInfo );
		} return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDCLOSE )
		{
			CloseWindow( hwnd );
			DestroyWindow( hwnd );
		}
		break;

	case WM_DESTROY:
		DeleteObject( hFont );
		break;

	}

	return FALSE;
}
