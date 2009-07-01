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

#include "common.h"

/*

	void __cdecl AddToListWorker(void *p);
	void __cdecl SendSuccess(void *);
	void __cdecl DoSearch(void *);
	void __cdecl SignOn(void *);
	void __cdecl MessageLoop(void *);
	void __cdecl SendMindWorker(void *);
	void __cdecl GetAwayMsgWorker(void *);
	void __cdecl UpdateAvatarWorker(void *);
	void __cdecl UpdateInfoWorker(void *);

*/

//int FacebookProto::SetAwayMsg( int iStatus, const char* msg )
//	HttpGetPost( FACEBOOK_REQUEST_STATUS_SET, szPostData );

void CALLBACK FacebookProto::APC_callback(ULONG_PTR p)
{
	LOG("***** Executing APC");
}

void FacebookProto::SignOn(void*)
{
	LOG("***** Beginning SignOn process");
	WaitForSingleObject(&signon_lock_,INFINITE);

	// Kill the old thread if it's still around
	if(m_hMsgLoop)
	{
		LOG("***** Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback,m_hMsgLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old MessageLoop to exit");
		WaitForSingleObject(m_hMsgLoop,INFINITE);
		CloseHandle(m_hMsgLoop);
	}
	if ( NegotiateConnection( ) ) // Could this be? The legendary Go Time??
	{
		m_hMsgLoop = ForkThreadEx( &FacebookProto::MessageLoop, this );
	}

	ReleaseMutex(signon_lock_);
	LOG("***** SignOn complete");
}

void FacebookProto::SignOff(void*)
{
	facy.clear_cookies( );
}

bool FacebookProto::NegotiateConnection( )
{
	LOG("***** Negotiating connection with Twitter");

	int old_status = m_iStatus;
	std::string user, pass;
	DBVARIANT dbv = {0};

	if( !DBGetContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
	{
		user = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else
	{
		ShowPopup(TranslateT("Please enter a username.")); // TODO: Refactor to Notify
		return false;
	}

	if( !DBGetContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_PASS,&dbv) )
	{
		CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
			reinterpret_cast<LPARAM>(dbv.pszVal));
		pass = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else
	{
		ShowPopup(TranslateT("Please enter a password."));
		return false;
	}

	bool success;
	{
		ScopedLock s(facebook_lock_);
		success = facy.validate_user( user, pass );
	}

	if(!success)
	{
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_FAILED,
			(HANDLE)old_status,m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		facy.devalidate_user( );

		return false;
	}
	else
	{
		m_iStatus = m_iDesiredStatus;
		// SetAllContactStatuses(ID_STATUS_ONLINE); // TODO: Implement as a useful tool

		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		return true;
	}
}

//void SignOn( void* )
//{
//	// prepare data
//
//	string user, pass;
//	DBVARIANT dbv = {0};
//
//	if( !DBGetContactSettingString(0,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
//	{
//		user = dbv.pszVal;
//		DBFreeVariant(&dbv);
//	}
//
//	if( !DBGetContactSettingString(0,m_szModuleName,FACEBOOK_KEY_PASS,&dbv) )
//	{
//		CallService(MS_DB_CRYPT_DECODESTRING,strlen(dbv.pszVal)+1,
//			reinterpret_cast<LPARAM>(dbv.pszVal));
//		pass = dbv.pszVal;
//		DBFreeVariant(&dbv);
//	}
//
//	// [ LOGIN ] login post data string urlencode(username, password), ...
//	string data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&email=";
//	data += utils::url::encode( user );
//	data += "&pass=";
//	data += utils::url::encode( pass );
//	data += "&pass_placeHolder=Password&persistent=1&login=Login&charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84\0";
//
//	char* datac = ( char* )utils::mem::malloc( data.size( ) * sizeof( char ) );
//	strcpy( datac, data.c_str( ) );
//
//	// send request and get response
//	NETLIBHTTPREQUEST* pnlhr = HttpGetPost( FACEBOOK_REQUEST_LOGIN, datac );
//
//	utils::mem::free( datac );
//
//	// process response
//
//	DebugInfo( pnlhr->pData );
//
//	/*if(!pnlhr || (pnlhr->resultCode != 200) || (pnlhr->dataLength != 1))
//	{
//        MessageBoxA( NULL, "Error", "", MB_OK );
//	}
//	else
//	{ // check if OK
//		MessageBoxA( NULL, pnlhr->pData, "OK", MB_OK );
//	}*/
//
//	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
//
//	return FALSE;
//}

//bool FacebookProto::NegotiateConnection( )
//{
//}

void FacebookProto::MessageLoop(void *)
{
}

