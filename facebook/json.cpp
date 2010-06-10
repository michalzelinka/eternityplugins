/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-10 Michal Zelinka

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
#include "JSON_CAJUN/reader.h"
#include "JSON_CAJUN/writer.h"
#include "JSON_CAJUN/elements.h"

int facebook_json_parser::parse_buddy_list( void* data, std::map< std::string, facebook_user* >* buddy_list )
{
	using namespace json;

	try
	{
		facebook_user* current = NULL;
		std::string buddyData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( buddyData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Object& nowAvailableList = objRoot["payload"]["buddy_list"]["nowAvailableList"];

		for (Object::const_iterator itAvailable(nowAvailableList.Begin());
			itAvailable != nowAvailableList.End(); ++itAvailable)
		{
			const Object::Member& member = *itAvailable;

			std::map<std::string,facebook_user*>::iterator iter = buddy_list->find( member.name );
			if ( buddy_list->empty() || iter == buddy_list->end() )
				buddy_list->insert( std::make_pair( member.name, new facebook_user( ) ) );

			current = (*buddy_list)[member.name];
			const Object& objMember = member.element;
			const Boolean idle = objMember["i"];

			current->user_id = current->real_name = member.name;
			current->status_id = ID_STATUS_ONLINE | ID_STATUS_ONLY_ONCE;
			current->is_idle = ( idle.Value( ) == 1 );
		}

		const Array& wasAvailableIDs = objRoot["payload"]["buddy_list"]["wasAvailableIDs"];

		for ( Array::const_iterator itWasAvailable( wasAvailableIDs.Begin() );
			itWasAvailable != wasAvailableIDs.End(); ++itWasAvailable)
		{
			const Number& member = *itWasAvailable;
			char was_id[32];
			lltoa( member.Value(), was_id, 10 );

			current = (*buddy_list)[was_id];
			// work-around for idle change indicating user in both nowAvailable and wasAvailable
			if ( current->status_id & ID_STATUS_ONLY_ONCE ) current->status_id &= ~(ID_STATUS_ONLY_ONCE);
			else current->status_id = ID_STATUS_OFFLINE;
		}

		const Object& userInfosList = objRoot["payload"]["buddy_list"]["userInfos"];

		for (Object::const_iterator itUserInfo(userInfosList.Begin());
			itUserInfo != userInfosList.End(); ++itUserInfo)
		{
			const Object::Member& member = *itUserInfo;

			std::map<std::string,facebook_user*>::iterator iter = buddy_list->find( member.name );
			if ( iter == buddy_list->end() )
				continue;

			const Object& objMember = member.element;
			const String& realName = objMember["name"];
			const String& imageUrl = objMember["thumbSrc"];

			current = (*buddy_list)[member.name];
			current->real_name = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( realName.Value( ) ) );
			current->image_url = utils::text::slashu_to_utf8(
			    utils::text::special_expressions_decode( imageUrl.Value( ) ) );

		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log ( "!!!!! Caught json::Exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_messages( void* data, std::vector< facebook_message* >* messages, std::vector< facebook_notification* >* notifications )
{
	using namespace json;

	try
	{
		std::string messageData = static_cast< std::string* >( data )->substr( 9 );
		std::istringstream sDocument( messageData );
		Object objDocument;
		Reader::Read(objDocument, sDocument);

		const Object& objRoot = objDocument;
		const Array& messagesArray = objRoot["ms"];

		for (Array::const_iterator itMessage(messagesArray.Begin());
			itMessage != messagesArray.End(); ++itMessage)
		{
			const Object& objMember = *itMessage;

			const String& type = objMember["type"];

			if ( type.Value( ) == "msg" )
			{
				const Number& from = objMember["from"];
				char was_id[32];
				lltoa( from.Value(), was_id, 10 );
				
				const Object& messageContent = objMember["msg"];
				const String& text = messageContent["text"];

				facebook_message* message = new facebook_message( );
				message->message_text= utils::text::slashu_to_utf8(
					utils::text::special_expressions_decode( text.Value( ) ) );
				message->time = ::time( NULL );
				message->user_id = was_id;

				messages->push_back( message );
			}
			else if ( type.Value( ) == "app_msg" )
			{
				const String& text = objMember["response"]["payload"]["title"];

				facebook_notification* notification = new facebook_notification( );
				notification->text = utils::text::slashu_to_utf8(
					utils::text::special_expressions_decode(
						utils::text::remove_html( text.Value( ) ) ) );

				notifications->push_back( notification );
			}
			else if ( type.Value( ) == "inbox" )
			{
/*				const Number& unseen = objMember["unseen"];

				TCHAR* info = TranslateT("You have %d unseen messages");
				proto->ShowEvent( TranslateT("Unseen messages"), TC );
*/			}
			else
				continue;
		}
	}
	catch (Reader::ParseException& e)
	{
		proto->Log( "!!!!! Caught json::ParseException: %s", e.what() );
		proto->Log( "      Line/offset: %d/%d", e.m_locTokenBegin.m_nLine + 1, e.m_locTokenBegin.m_nLineOffset + 1 );
	}
	catch (const Exception& e)
	{
		proto->Log ( "!!!!! Caught json::Exception: %s", e.what() );
	}

	return EXIT_SUCCESS;
}
