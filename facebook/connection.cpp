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
//	LOG("***** Executing APC");
}

void FacebookProto::SignOn(void*)
{
	LOG("***** Beginning SignOn process");
	WaitForSingleObject(&signon_lock_,INFINITE);

	// Kill the old threads if they are still around
	if(m_hMsgLoop)
	{
		LOG("***** Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback,m_hMsgLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old MessageLoop to exit");
		WaitForSingleObject(m_hMsgLoop,INFINITE);
		CloseHandle(m_hMsgLoop);
	}
	if(m_hUpdLoop)
	{
		LOG("***** Requesting UpdateLoop to exit");
		QueueUserAPC(APC_callback,m_hUpdLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old UpdateLoop to exit");
		WaitForSingleObject(m_hUpdLoop,INFINITE);
		CloseHandle(m_hUpdLoop);
	}
	if ( NegotiateConnection( ) ) // Could this be? The legendary Go Time??
	{
		m_hUpdLoop = ForkThreadEx( &FacebookProto::UpdateLoop, this );
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
	LOG("***** Negotiating connection with Facebook");

	int old_status = m_iStatus;
	std::string user, pass;
	DBVARIANT dbv = {0};

	if( !DBGetContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_LOGIN,&dbv) )
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
		success = facy.login( user, pass );
		// TODO: Following functions here, or in the ::SignOn( ) ?
		success = facy.popout( );
		success = facy.update( );
		success = facy.reconnect( );
		success = facy.settings( );
	}

	if(!success)
	{
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_FAILED,
			(HANDLE)old_status,m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;

		SetAllContactStatuses(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		facy.logout( );

		return false;
	}
	else
	{
		m_iStatus = m_iDesiredStatus;

		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		return true;
	}
}

void FacebookProto::UpdateLoop(void *)
{
	LOG( ">>>>> Entering Facebook::UpdateLoop" );

	BYTE poll_rate = getByte( FACEBOOK_KEY_POLL_RATE, FACEBOOK_DEFAULT_POLL_RATE );

	for ( WORD i = 0; ; i++ )
	{
		if ( m_iStatus != ID_STATUS_ONLINE )
			goto exit;
//		if ( i % 2 == 0 )
			facy.update( );

		if ( m_iStatus != ID_STATUS_ONLINE )
			goto exit;
		LOG( "***** FacebookProto::UpdateLoop going to sleep..." );
		if ( SleepEx( poll_rate * 1000, true ) == WAIT_IO_COMPLETION )
			goto exit;
		LOG( "***** FacebookProto::UpdateLoop waking up..." );
	}

exit:
	{
		ScopedLock s(facebook_lock_);
		facy.logout( );
	}
	LOG( "<<<<< Exiting FacebookProto::UpdateLoop" );
}

void FacebookProto::MessageLoop(void *)
{
	LOG( ">>>>> Entering Facebook::MessageLoop" );

	for ( WORD i = 0; ; i++ )
	{
		if ( m_iStatus != ID_STATUS_ONLINE )
			goto exit;
		facy.channel( );

		LOG( "***** FacebookProto::MessageLoop refreshing..." );
	}

exit:
	{
		ScopedLock s(facebook_lock_);
		facy.logout( );
	}
	LOG( "<<<<< Exiting FacebookProto::MessageLoop" );
}

void FacebookProto::ProcessUpdates( void* data )
{
	try
	{
		LOG("***** Starting processing updates");
		facebook_json_parser* fbjp = new facebook_json_parser( FB_PARSE_UPDATES, this );
		std::map< std::string, facebook_user* > friends;
		fbjp->parseFriends( &friends, data );
		delete fbjp;

		for(std::map<std::string, facebook_user*>::iterator i=friends.begin(); i!=friends.end(); ++i)
		{
			if(i->first == this->facy.user_id_)
			{
				DBWriteContactSettingUTF8String(NULL,m_szModuleName,FACEBOOK_KEY_NAME,i->second->real_name.c_str());
				DBWriteContactSettingUTF8String(NULL,"CList","StatusMsg",i->second->status.c_str());
				continue;
			}

			HANDLE hContact = AddToClientList(i->second);
			//UpdateAvatar(hContact,i->second->profile_image_url); // TODO
		}

		SetAllContactUpdates( friends );
		LOG("***** Updates processed");
	}
	catch(const std::exception &e)
	{
		LOG("***** Error processing updates: %s", e.what());
	}
}

void FacebookProto::ProcessMessages( void* data )
{
	try
	{
		LOG("***** Starting processing messages");
		facebook_json_parser* fbjp = new facebook_json_parser( FB_PARSE_MESSAGES, this );
		std::vector< facebook_message* > messages;
		fbjp->parseMessages( &messages, data );
		delete fbjp;

/*		facebook_message* msg = new facebook_message( );
		msg->message_text = "TESTING";
		msg->time = 1250289476;
		msg->user_id = "573295254";
		messages.push_back( msg );
*/
		for(unsigned int i = 0; i < messages.size( ); i++)
		{
			if ( messages[i]->user_id == this->facy.user_id_ )
				continue;

			facebook_user fbu;
			fbu.user_id = messages[i]->user_id;

			HANDLE hContact = AddToClientList(&fbu);

			PROTORECVEVENT recv = {};
			CCSDATA ccs = {};

			recv.flags = PREF_UTF;
			//if(pre_read)
			//	recv.flags |= PREF_CREATEREAD;
			recv.szMessage = const_cast<char*>(messages[i]->message_text.c_str());
			recv.timestamp = static_cast<DWORD>(messages[i]->time);

			ccs.hContact = hContact;
			ccs.szProtoService = PSR_MESSAGE;
			ccs.wParam = ID_STATUS_ONLINE;
			ccs.lParam = reinterpret_cast<LPARAM>(&recv);
			CallService(MS_PROTO_CHAINRECV,0,reinterpret_cast<LPARAM>(&ccs));


		}

		LOG("***** Messages processed");
	}
	catch(const std::exception &e)
	{
		LOG("***** Error processing messages: %s", e.what());
	}
}
