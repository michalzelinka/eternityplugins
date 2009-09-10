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

int facebook_json_parser::parse_data( void* structure, void* data )
{
	std::string json_data = (*( std::string* )data).substr( 9 );
	std::string parser_name = ""; std::string parsed = "";

	if ( parserType == FB_PARSE_BUDDY_LIST ) {
		parser_name = "buddy list";
		this->buddies = ( std::map< std::string, facebook_user* >* )structure; }
	else if ( parserType = FB_PARSE_MESSAGES ) {
		parser_name = "messages";
		this->messages = ( vector< facebook_message* >* )structure; }
	else return EXIT_FAILURE;

	parent->parent->LOG("===== Parsing %s started", parser_name.c_str());

	JSON_config config;

	init_JSON_config(&config);

    config.depth                  = 20;
    config.callback               = &facebook_json_parser::parse;
    config.callback_ctx           = ( void* )this;
    config.allow_comments         = 1;
    config.handle_floats_manually = 0;

	JSON_parser_struct* jc = new_JSON_parser(&config);

	parent->parent->LOG("===== Parser of %s starts walking through the data", parser_name.c_str());

	for (size_t i = 0; i < json_data.length( ); i++ )
	{
		int next_char = json_data.at( i );
//		parsed += json_data.at( i );
        if (!JSON_parser_char(jc, next_char)) {
            delete_JSON_parser(jc);
			parent->parent->LOG("===== Parsing %s failed: Syntax error at %d", parser_name.c_str(), i);
            return EXIT_FAILURE;
        }
    }
//	parent->parent->LOG("\r\n===== Parsed %s data:\n%s\n", parser_name.c_str(), parsed.c_str());
	if (!JSON_parser_done(jc)) {
		delete_JSON_parser(jc);
		parent->parent->LOG("===== Parsing %s failed: End syntax error", parser_name.c_str());
		return EXIT_FAILURE;
	}
	if ( parserType == FB_PARSE_BUDDY_LIST )
		parent->parent->LOG("===== Parsing %s finished, got items: %d", parser_name.c_str(), buddies->size());
	else if ( parserType = FB_PARSE_MESSAGES )
		parent->parent->LOG("===== Parsing %s finished, got items: %d", parser_name.c_str(), messages->size());

	delete ( std::string* )data;

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse(void* ctx, int type, const JSON_value* value)
{
	facebook_json_parser* fbjp = ( facebook_json_parser* )ctx;

	switch(type) {

	case JSON_T_ARRAY_BEGIN:
		fbjp->isKey = false;
		++fbjp->level;
		break;

	case JSON_T_ARRAY_END:
		assert(!fbjp->isKey);
		if (fbjp->level > 0) --fbjp->level;
		if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_WASAVAILABLE && fbjp->level == 3 )
		{
			fbjp->section = 0;
		}
		break;

	case JSON_T_OBJECT_BEGIN:
		fbjp->isKey = false;
		++fbjp->level;
		break;

	case JSON_T_OBJECT_END:
		assert(!fbjp->isKey);
		if (fbjp->level > 0) --fbjp->level;
		if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 4 )
		{
			fbjp->parent->parent->LOG("      Got user '%s'", fbjp->currentFriend->user_id.c_str() );
		}
		else if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 3 )
		{
			fbjp->section = 0;
		}
		else if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_WASAVAILABLE && fbjp->level == 3 )
		{
			fbjp->section = 0;
		}
		else if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_USERINFOS && fbjp->level == 3 )
		{
			fbjp->section = 0;
		}
		else if ( fbjp->parserType == FB_PARSE_MESSAGES && fbjp->level == 2 && fbjp->section == FB_PARSE_MESSAGE_BODY )
		{
			(*fbjp->messages).push_back( new facebook_message( *fbjp->currentMessage ) );
			fbjp->currentMessage = NULL;
			fbjp->section = 0;
		}
		break;

	case JSON_T_INTEGER:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_MESSAGES && fbjp->section == FB_PARSE_MESSAGE_BODY )
		{
			if ( fbjp->valueType == FB_PARSE_MESSAGE_FROM )
			{
				char time[32];
				lltoa( value->vu.integer_value, time, 10 );
				fbjp->currentMessage->user_id = time;
			}
			else if ( fbjp->valueType == FB_PARSE_MESSAGE_TIME )
			{
				JSON_int_t timestamp = value->vu.integer_value;
				if ( timestamp > 0xffffffff ) // fix for clients not sending proper
					timestamp /= 1000;        // timestamp, resp. microtimestamp
				fbjp->currentMessage->time = timestamp;
			}
		}
		else if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_WASAVAILABLE && fbjp->level == 4 )
		{
			char was_id[32];
			lltoa( value->vu.integer_value, was_id, 10 );
			(*fbjp->buddies)[was_id]->status_id = ID_STATUS_OFFLINE;
			(*fbjp->buddies)[was_id]->passed = false;
			fbjp->parent->parent->LOG("      Lost user '%s'", was_id );
		}
		fbjp->valueType = 0;
		break;

	case JSON_T_FLOAT:
		fbjp->isKey = false;
		break;

	case JSON_T_NULL:
		fbjp->isKey = false;
		break;

	case JSON_T_TRUE:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 5 )
			if ( fbjp->valueType == FB_PARSE_UPDATE_IDLE )
				fbjp->currentFriend->is_idle = true;
		fbjp->valueType = 0;
		break;

	case JSON_T_FALSE:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 5 )
			if ( fbjp->valueType == FB_PARSE_UPDATE_IDLE )
				fbjp->currentFriend->is_idle = false;
		fbjp->valueType = 0;
		break;

	case JSON_T_KEY:
		fbjp->isKey = true;
		{
			const char* val = value->vu.str.value;
			if ( fbjp->parserType == FB_PARSE_BUDDY_LIST )
			{
				if ( strsame( val, "nowAvailableList" ) )
					fbjp->section = FB_PARSE_UPDATE_NOWAVAILABLE;
				else if ( strsame( val, "wasAvailableIDs" ) )
					fbjp->section = FB_PARSE_UPDATE_WASAVAILABLE;
				else if ( strsame( val, "userInfos" ) )
					fbjp->section = FB_PARSE_UPDATE_USERINFOS;
				else if ( fbjp->level == 4 && fbjp->section == FB_PARSE_UPDATE_NOWAVAILABLE )
				{
					std::map<std::string,facebook_user*>::iterator iter = fbjp->buddies->find(val);
					if( iter == fbjp->buddies->end() )
						fbjp->buddies->insert( make_pair( val, new facebook_user( ) ) );
					fbjp->currentFriend = (*fbjp->buddies)[val];
					fbjp->currentFriend->user_id = fbjp->currentFriend->real_name = val;
					fbjp->currentFriend->status_id = ID_STATUS_ONLINE;
					fbjp->currentFriend->passed = false;
				}
				else if ( fbjp->level == 4 && fbjp->section == FB_PARSE_UPDATE_USERINFOS )
				{
					fbjp->currentFriendStr = val;
				}
				else if ( strsame( val, "i" ) )        fbjp->valueType = FB_PARSE_UPDATE_IDLE;
				else if ( strsame( val, "name" ) )     fbjp->valueType = FB_PARSE_UPDATE_NAME;
				else if ( strsame( val, "thumbSrc" ) ) fbjp->valueType = FB_PARSE_UPDATE_THMB;
//				else if ( strsame( val, "status" ) )   fbjp->valueType = FB_PARSE_UPDATE_STAT;
			}
			else if ( fbjp->parserType == FB_PARSE_MESSAGES )
			{
				if      ( strsame( val, "type" ) )       fbjp->valueType = FB_PARSE_MESSAGE_TYPE;
//				else if ( strsame( val, "msg" ) )        fbjp->section   = FB_PARSE_MESSAGE_BODY;
				else if ( strsame( val, "text" ) )       fbjp->valueType = FB_PARSE_MESSAGE_TEXT;
				else if ( strsame( val, "clientTime" ) ) break;
				else if ( strsame( val, "time" ) )       fbjp->valueType = FB_PARSE_MESSAGE_TIME;
				else if ( strsame( val, "from" ) )       fbjp->valueType = FB_PARSE_MESSAGE_FROM;
			}
		}
		break;

	case JSON_T_STRING:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_BUDDY_LIST && fbjp->section == FB_PARSE_UPDATE_USERINFOS && fbjp->level == 5 )
		{
			std::map<std::string,facebook_user*>::iterator iter = fbjp->buddies->find(fbjp->currentFriendStr);
			if( iter != fbjp->buddies->end() )
			{
				if      ( fbjp->valueType == FB_PARSE_UPDATE_NAME )
					(*fbjp->buddies)[ fbjp->currentFriendStr ]->real_name = value->vu.str.value;
				else if ( fbjp->valueType == FB_PARSE_UPDATE_THMB )
					(*fbjp->buddies)[ fbjp->currentFriendStr ]->image_url = value->vu.str.value;
//				else if ( fbjp->valueType == FB_PARSE_UPDATE_STAT )
//			   		(*fbjp->buddies)[ fbjp->currentFriendStr ]->status = value->vu.str.value;
			}
		}
		else if ( fbjp->parserType == FB_PARSE_MESSAGES )
		{
			if      ( fbjp->valueType == FB_PARSE_MESSAGE_TYPE && strsame(value->vu.str.value, "msg" ) )
			{
				fbjp->currentMessage = new facebook_message( );
				fbjp->section = FB_PARSE_MESSAGE_BODY;
			}
			else if ( fbjp->valueType == FB_PARSE_MESSAGE_TEXT && fbjp->section == FB_PARSE_MESSAGE_BODY )
				fbjp->currentMessage->message_text = value->vu.str.value;
		}
		fbjp->valueType = 0;
		break;

	default:
		assert(0);
		break;

	}

	return 1;
}
