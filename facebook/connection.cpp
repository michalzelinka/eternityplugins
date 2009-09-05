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

void CALLBACK FacebookProto::APC_callback(ULONG_PTR p)
{
//	LOG("***** Executing APC");
}

void FacebookProto::KillThreads( )
{
	// Kill the old threads if they are still around
	if(m_hUpdLoop)
	{
		LOG("***** Requesting UpdateLoop to exit");
		QueueUserAPC(APC_callback,m_hUpdLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old UpdateLoop to exit");
		WaitForSingleObject(m_hUpdLoop,INFINITE);
		CloseHandle(m_hUpdLoop);
		m_hUpdLoop = NULL;
	}
	if(m_hMsgLoop)
	{
		LOG("..... Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback,m_hMsgLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old MessageLoop to exit");
		WaitForSingleObject(m_hMsgLoop,INFINITE);
		CloseHandle(m_hMsgLoop);
		m_hMsgLoop = NULL;
	}
}

void FacebookProto::SignOn(void*)
{
	LOG("***** Beginning SignOn process");
	WaitForSingleObject(&signon_lock_,INFINITE);

	KillThreads( );

	if ( NegotiateConnection( ) ) // Could this be? The legendary Go Time??
	{
		m_hUpdLoop = ForkThreadEx( &FacebookProto::UpdateLoop, this );
		m_hMsgLoop = ForkThreadEx( &FacebookProto::MessageLoop, this );
	}
	ToggleStatusMenuItems(isOnline());

	ReleaseMutex(signon_lock_);
	LOG("..... SignOn complete");
}

void FacebookProto::SignOff(void*)
{
	LOG("xxxxx Beginning SignOff process");
	WaitForSingleObject(&signon_lock_,INFINITE);

	KillThreads( );

	ToggleStatusMenuItems(isOnline());

	{
		ScopedLock s(facebook_lock_);
		facy.logout( );
		facy.clear_cookies( );
		facy.buddies.clear( );
	}

	ReleaseMutex(signon_lock_);
	LOG("xxxxx SignOff complete");
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
		facy.first_touch_ = true;
		success = facy.login( user, pass );
		if (success) success = facy.home( );
		if (success) success = facy.reconnect( );
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

	for ( WORD i = 0; ; i++ )
	{
		if ( !isOnline( ) )
			goto exit;
		if ( !facy.buddy_list( ) )
			goto exit;

// TODO: Dummy for notifications
//		if ( i % 2 == 0 )
//			facy.update( );

		if ( !isOnline( ) )
			goto exit;
		LOG( "***** FacebookProto::UpdateLoop going to sleep..." );
		if ( SleepEx( GetPollRate( ) * 1000, true ) == WAIT_IO_COMPLETION )
			goto exit;
		LOG( "***** FacebookProto::UpdateLoop waking up..." );
	}

exit:
	LOG( "<<<<< Exiting FacebookProto::UpdateLoop" );
}

void FacebookProto::MessageLoop(void *)
{
	LOG( ">>>>> Entering Facebook::MessageLoop" );

	for ( WORD i = 0; ; i++ )
	{
		if ( !isOnline( ) )
			goto exit;
		if ( !facy.channel( ) )
			goto exit;

		LOG( "***** FacebookProto::MessageLoop refreshing..." );
	}

exit:
	LOG( "<<<<< Exiting FacebookProto::MessageLoop" );
}

BYTE FacebookProto::GetPollRate( )
{
	BYTE poll_rate = getByte( FACEBOOK_KEY_POLL_RATE, FACEBOOK_DEFAULT_POLL_RATE );

	return ( ( poll_rate >= FACEBOOK_MINIMAL_POLL_RATE && poll_rate <= FACEBOOK_MAXIMAL_POLL_RATE ) ? poll_rate : FACEBOOK_DEFAULT_POLL_RATE );
}
