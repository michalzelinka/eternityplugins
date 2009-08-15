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

bool FacebookProto::IsMyContact(HANDLE hContact)
{
	const char *proto = reinterpret_cast<char*>( CallService(MS_PROTO_GETCONTACTBASEPROTO,
		reinterpret_cast<WPARAM>(hContact),0) );

	if(proto && strcmp(m_szModuleName,proto) == 0)
		return true;
	else
		return false;
}

HANDLE FacebookProto::UserIDToHContact(std::string user_id)
{
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
		hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0) )
	{
		if(!IsMyContact(hContact))
			continue;

		DBVARIANT dbv;
		if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			if(strcmp(user_id.c_str(),dbv.pszVal) == 0)
			{
				DBFreeVariant(&dbv);
				return hContact;
			}
			else
				DBFreeVariant(&dbv);
		}
	}

	return 0;
}

HANDLE FacebookProto::AddToClientList(facebook_user* fbu)
{
	// First, check if this contact exists
	HANDLE hContact = UserIDToHContact(fbu->user_id);
	if(hContact)
		return hContact;

	// If not, make a new contact!
	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if(hContact)
	{
		if(CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact,(LPARAM)m_szModuleName) == 0)
		{
			DBWriteContactSettingString    (hContact,m_szModuleName,FACEBOOK_KEY_ID,fbu->user_id.c_str());
			DBWriteContactSettingUTF8String(hContact,"CList","MyHandle",fbu->real_name.c_str());

			return hContact;
		}
		else
			CallService(MS_DB_CONTACT_DELETE,(WPARAM)hContact,0);
	}

	return 0;
}

void FacebookProto::SetAllContactStatuses(int status)
{
	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
		hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0) )
	{
		if(!IsMyContact(hContact))
			continue;

		DBWriteContactSettingWord(hContact,m_szModuleName,"Status",status);
	}
}

void FacebookProto::SetAllContactUpdates(std::map<std::string, facebook_user*> friends)
{
	if ( !this->isOnline( ) )
		return;

	for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
		hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0) )
	{
		if(!IsMyContact(hContact))
			continue;

		DBVARIANT dbv;
		if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			if ( friends[ dbv.pszVal ] )
			{
				facebook_user* fbu = friends[ dbv.pszVal ];
				DBWriteContactSettingUTF8String(hContact,m_szModuleName,FACEBOOK_KEY_NAME,fbu->real_name.c_str());
				DBWriteContactSettingUTF8String(hContact,"CList","StatusMsg",fbu->status.c_str());
				if ( fbu->is_idle )
					DBWriteContactSettingWord(hContact,m_szModuleName,"Status",ID_STATUS_AWAY);
				else
					DBWriteContactSettingWord(hContact,m_szModuleName,"Status",ID_STATUS_ONLINE);
			}
			else
				DBWriteContactSettingWord(hContact,m_szModuleName,"Status",ID_STATUS_OFFLINE);
		}
	}
}

void FacebookProto::GetAwayMsgWorker(void *hContact)
{
	if(hContact == 0)
		return;

	DBVARIANT dbv;
	if( !DBGetContactSettingString(hContact,"CList","StatusMsg",&dbv) )
	{
		ProtoBroadcastAck(m_szModuleName,hContact,ACKTYPE_AWAYMSG,ACKRESULT_SUCCESS,
			(HANDLE)1,(LPARAM)dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else
	{
		ProtoBroadcastAck(m_szModuleName,hContact,ACKTYPE_AWAYMSG,ACKRESULT_FAILED,
			(HANDLE)1,(LPARAM)0);
	}
}

HANDLE FacebookProto::GetAwayMsg(HANDLE hContact)
{
	ForkThread(&FacebookProto::GetAwayMsgWorker, this,hContact);
	return (HANDLE)1;
}

int FacebookProto::RecvMsg(HANDLE hContact,PROTORECVEVENT *pre)
{
	CCSDATA ccs = { hContact,PSR_MESSAGE,0,reinterpret_cast<LPARAM>(pre) };
	return CallService(MS_PROTO_RECVMSG,0,reinterpret_cast<LPARAM>(&ccs));
}

struct send_direct
{
	send_direct(HANDLE hContact,const std::string &msg) : hContact(hContact),msg(msg) {}
	HANDLE hContact;
	std::string msg;
};

void FacebookProto::SendSuccess(void *p)
{
	if(p == 0)
		return;
	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(data->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
	{
		ScopedLock s(facebook_lock_);
		facy.send_message(dbv.pszVal, data->msg);

		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_MESSAGE,ACKRESULT_SUCCESS,
			(HANDLE)1,0);
		DBFreeVariant(&dbv);
	}

	delete data;
}

int FacebookProto::SendMsg(HANDLE hContact,int flags,const char *msg)
{
	if(m_iStatus != ID_STATUS_ONLINE)
		return 0;

	ForkThread(&FacebookProto::SendSuccess, this,new send_direct(hContact,msg));
	return 1;
}
