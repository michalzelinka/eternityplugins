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

void FacebookProto::ProcessBuddyList( void* data )
{
	_APP("ProcessBuddyList");
	if (!isOnline())
		return;

	try
	{
		LOG("***** Starting processing buddy list");
		facebook_json_parser* jp = new facebook_json_parser( FB_PARSE_BUDDY_LIST, &this->facy );
		jp->parse_data( &facy.buddies, data );
		delete jp;

		for(std::map<std::string, facebook_user*>::iterator i=facy.buddies.begin(); i!=facy.buddies.end(); )
		{
			_APP("BuddyList::for");
			std::map< std::string, facebook_user*>::iterator this_one = i;
			++i;

			if ( this_one->second->passed ) { // This contact has been processed already
				continue; }

			this_one->second->passed = true;
			LOG("      Now %s: %s", (this_one->second->status_id==ID_STATUS_ONLINE)?"online":"offline", this_one->second->real_name.c_str());
			if ( this_one->second->status_id == ID_STATUS_OFFLINE )
			{
				DBWriteContactSettingWord(this_one->second->handle,m_szModuleName,"Status",ID_STATUS_OFFLINE );
				DBDeleteContactSetting(this_one->second->handle,m_szModuleName,"IdleTS");
				facy.buddies.erase( this_one );
			}
			else
			{
				_APP(this_one->second->image_url);
				this_one->second->handle = AddToContactList(this_one->second);
				DBWriteContactSettingWord(this_one->second->handle,m_szModuleName,"Status",ID_STATUS_ONLINE );
				ForkThreadEx(&FacebookProto::UpdateContactWorker, this, (void*)this_one->second);
				this_one->second->just_added = false;
			}
		}

		LOG("***** Buddy list processed");
	}
	catch(const std::exception &e)
	{
		LOG("***** Error processing buddy list: %s", e.what());
	}
}

void FacebookProto::ProcessMessages( void* data )
{
	_APP("ProcessMessages");
	if (!isOnline())
		return;

	try
	{
		LOG("***** Starting processing messages");
		facebook_json_parser* jp = new facebook_json_parser( FB_PARSE_MESSAGES, &this->facy );
		std::vector< facebook_message* > messages;
		jp->parse_data( &messages, data );
		delete jp;

		for(size_t i = 0; i < messages.size( ); i++)
		{
			_APP("Message::for");
			if ( messages[i]->user_id == facy.self_.user_id )
				continue;

			LOG("      Got message: %s", messages[i]->message_text.c_str());
			facebook_user fbu;
			fbu.user_id = messages[i]->user_id;

			HANDLE hContact = AddToContactList(&fbu);

			PROTORECVEVENT recv = {};
			CCSDATA ccs = {};

			recv.flags = PREF_UTF;
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

void FacebookProto::ProcessAvatar(HANDLE hContact,const std::string* url,bool force)
{
	_APP("ProcessAvatar");
	ForkThread(&FacebookProto::UpdateAvatarWorker, this,
	    new update_avatar(hContact,(*url)));
}
