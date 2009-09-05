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
	// If this is self contact and we don't want it added, return
	// TODO: Obsolete?
	if ( fbu->user_id == facy.user_id_ && !getByte(FACEBOOK_KEY_INC_SELF,0) )
		return 0;

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
			DBWriteContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,fbu->user_id.c_str());
			if (fbu->user_id == facy.user_id_)
				DBWriteContactSettingString(NULL,m_szModuleName,FACEBOOK_KEY_ID,fbu->user_id.c_str());
			DBVARIANT dbv;
			if( !DBGetContactSettingTString(NULL,m_szModuleName,FACEBOOK_KEY_DEF_GROUP,&dbv) )
			{
				DBWriteContactSettingTString(hContact,"CList","Group",dbv.ptszVal);
				DBFreeVariant(&dbv);
			}

			fbu->just_added = true;

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

void FacebookProto::UpdateContact(facebook_user* fbu)
{
	if ( !this->isOnline( ) )
		return;

	HANDLE hContact = fbu->handle;

	DBVARIANT dbv;
	bool update_data;

	// Update Real name, if allowed
	if ( fbu->just_added )
		update_data = true;
	else if ( getByte( FACEBOOK_KEY_UPD_NAMES, 0 ) )
	{
		if ( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			update_data = strcmp( dbv.pszVal, fbu->real_name.c_str() ) != 0;
			DBFreeVariant(&dbv);
		}
		else update_data = true;
	}
	else update_data = false;

	if ( update_data )
	{
		DBWriteContactSettingUTF8String(hContact,m_szModuleName,FACEBOOK_KEY_NAME,fbu->real_name.c_str());
	}

	// Update status message
	facy.get_status( fbu );
	if ( fbu->just_added )
		update_data = true;
	else if ( !DBGetContactSettingUTF8String(hContact,"CList","StatusMsg",&dbv) )
	{
		update_data = strcmp( dbv.pszVal, fbu->status.c_str() ) != 0;
		DBFreeVariant(&dbv);
	}
	else update_data = true;

	if ( update_data )
	{
		DBWriteContactSettingUTF8String(hContact,"CList","StatusMsg",fbu->status.c_str());
	}
	update_data = true;

	// Update idle flag
	if ( fbu->is_idle )
	{
		if ( !DBGetContactSettingDword(hContact,m_szModuleName,"IdleTS",0) )
			DBWriteContactSettingDword(hContact,m_szModuleName,"IdleTS",(DWORD)time(NULL));
	}
	else
	{
		if ( DBGetContactSettingDword(hContact,m_szModuleName,"IdleTS",0) )
			DBWriteContactSettingDword(hContact,m_szModuleName,"IdleTS",0);
	}

	// Update avatar
	if ( fbu->just_added )
		update_data = true;
	else if ( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_AV_URL,&dbv) )
	{
		update_data = strcmp( dbv.pszVal, fbu->image_url.c_str() ) != 0;
		DBFreeVariant(&dbv);
	}
	else update_data = true;

	if ( update_data )
	{
		DBWriteContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_AV_URL,fbu->image_url.c_str());
		ProcessAvatar(hContact,&fbu->image_url);
	}
	update_data = true;
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

int FacebookProto::OnBuildContactMenu(WPARAM,LPARAM)
{
	return NULL;
}

int FacebookProto::OnContactDeleted(WPARAM wparam,LPARAM)
{
	HANDLE hContact = (HANDLE)wparam;

	for (std::map< std::string, facebook_user* >::iterator i = facy.buddies.begin(); i != facy.buddies.end(); ++i)
		if (hContact == i->second->handle)
			{ facy.buddies.erase(i); break; }

	return NULL;
}
