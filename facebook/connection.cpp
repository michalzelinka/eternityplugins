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
	_APP("APC_callback");
}

void FacebookProto::KillThreads( )
{
	_APP("KillThreads");
	// Kill the old threads if they are still around
	if(m_hUpdLoop)
	{
		_APP("KillThreads::before_upd_kill");
		LOG("***** Requesting UpdateLoop to exit");
		QueueUserAPC(APC_callback,m_hUpdLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old UpdateLoop to exit");
		WaitForSingleObject(m_hUpdLoop,IGNORE);
		CloseHandle(m_hUpdLoop);
		m_hUpdLoop = NULL;
		_APP("KillThreads::after_upd_kill");
	}
	if(m_hMsgLoop)
	{
		_APP("KillThreads::before_msg_kill");
		LOG("..... Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback,m_hMsgLoop,(ULONG_PTR)this);
		LOG("***** Waiting for old MessageLoop to exit");
		WaitForSingleObject(m_hMsgLoop,IGNORE);
		CloseHandle(m_hMsgLoop);
		m_hMsgLoop = NULL;
		_APP("KillThreads::after_msg_kill");
	}
}

void FacebookProto::SignOn(void*)
{
	WaitForSingleObject(&signon_lock_,INFINITE);
	LOG("***** Beginning SignOn process");

	KillThreads( );

	if ( NegotiateConnection( ) )
	{
		setDword( "LogonTS", (DWORD)time(NULL) );
		m_hUpdLoop = ForkThreadEx( &FacebookProto::UpdateLoop,  this );
		m_hMsgLoop = ForkThreadEx( &FacebookProto::MessageLoop, this );
	}
	ToggleStatusMenuItems(isOnline());

	LOG("***** SignOn complete");
	ReleaseMutex(signon_lock_);
}

void FacebookProto::SignOff(void*)
{
	WaitForSingleObject(&signon_lock_,INFINITE);
	LOG("##### Beginning SignOff process");

	deleteSetting( "LogonTS" );
	SetAllContactStatuses( ID_STATUS_OFFLINE );

	{
		ScopedLock s(facebook_lock_);
		facy.logout( );
		facy.clear_cookies( );
		facy.buddies.clear( );
	}

	if ( !getByte(FACEBOOK_KEY_ENABLE_REAL_LOGOUT_SIGNAL, 0) )
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	                                       //  ^
	ToggleStatusMenuItems(isOnline());     //  |  customizable real
	KillThreads( );                        //  |  logout signalization
	                                       //  v
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	LOG("##### SignOff complete");
	ReleaseMutex(signon_lock_);
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
		_APP("UpdateLoop");
		if ( !isOnline( ) )
			goto exit;
		if ( !facy.buddy_list( ) )
			goto exit;
		if ( !isOnline( ) )
			goto exit;
//		if ( i % 30 == 29 )
//			if ( !facy.keep_alive( ) )
//				goto exit;
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
		_APP("MessageLoop");
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

	return (
	    ( poll_rate >= FACEBOOK_MINIMAL_POLL_RATE &&
	      poll_rate <= FACEBOOK_MAXIMAL_POLL_RATE )
	    ? poll_rate : FACEBOOK_DEFAULT_POLL_RATE );
}
