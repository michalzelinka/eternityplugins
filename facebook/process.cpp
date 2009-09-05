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

void facebook_client::process_updates( void* data )
{
	try
	{
		parent->LOG("***** Starting processing updates");
		facebook_json_parser* fbjp = new facebook_json_parser( FB_PARSE_UPDATES, this );
		fbjp->parseFriends( &buddies, data );
		delete fbjp;
		delete ( std::string* )data;

		for(std::map<std::string, facebook_user*>::iterator i=buddies.begin(); i!=buddies.end(); )
		{
			if ( i->second->passed ) // This contact has been processed already
			{
				i++;
				continue;
			}

			parent->LOG("      Now %s: %s", (i->second->status_id==ID_STATUS_ONLINE)?"online":"offline", i->first.c_str());
			i->second->passed = true;
			if ( i->second->status_id == ID_STATUS_OFFLINE )
			{
				std::map< std::string, facebook_user*>::iterator j = i++;
				DBWriteContactSettingDword( j->second->handle, parent->m_szModuleName, "Status", ID_STATUS_OFFLINE );
				DBDeleteContactSetting(j->second->handle,parent->m_szModuleName,"IdleTS");
				this->buddies.erase( j );
			}
			else
			{
				i->second->handle = parent->AddToClientList(i->second);
				DBWriteContactSettingDword( i->second->handle, parent->m_szModuleName, "Status", ID_STATUS_ONLINE );
				parent->UpdateContact( i->second );
				i->second->just_added = false;
				i++;
			}
		}

		parent->LOG("***** Updates processed");
	}
	catch(const std::exception &e)
	{
		parent->LOG("***** Error processing updates: %s", e.what());
	}
}

void facebook_client::process_messages( void* data )
{
	try
	{
		parent->LOG("***** Starting processing messages");
		facebook_json_parser* fbjp = new facebook_json_parser( FB_PARSE_MESSAGES, this );
		std::vector< facebook_message* > messages;
		fbjp->parseMessages( &messages, data );
		delete fbjp;
		delete ( std::string* )data;

		for(size_t i = 0; i < messages.size( ); i++)
		{
			if ( messages[i]->user_id == this->user_id_ )
				continue;

			parent->LOG("      Got message: %s", messages[i]->message_text.c_str());
			facebook_user fbu;
			fbu.user_id = messages[i]->user_id;

			HANDLE hContact = parent->AddToClientList(&fbu);

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

		parent->LOG("***** Messages processed");
	}
	catch(const std::exception &e)
	{
		parent->LOG("***** Error processing messages: %s", e.what());
	}
}