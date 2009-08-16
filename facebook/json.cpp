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

int facebook_json_parser::parseFriends( std::map< std::string, facebook_user* >* friends, void* data )
{
	parent->LOG("===== parseFriends started");
	this->friends = friends;
	std::string json_data = (*( std::string* )data).substr( 9 );
	std::string parsed = "";

	JSON_config config;

	init_JSON_config(&config);

    config.depth                  = 20;
    config.callback               = &facebook_json_parser::parse;
    config.callback_ctx           = ( void* )this;
    config.allow_comments         = 1;
    config.handle_floats_manually = 0;

	JSON_parser_struct* jc = new_JSON_parser(&config);

	parent->LOG("===== parseFriends starts walking thru the data...");

	for (size_t i = 0; i < json_data.length( ); i++ )
	{
		int next_char = json_data.at( i );
		parsed += json_data.at( i );
        if (!JSON_parser_char(jc, next_char)) {
            delete_JSON_parser(jc);
			parent->LOG( "===== parseFriends JSON_parser_char: Syntax error at %d", i );
            return EXIT_FAILURE;
        }
    }
	parent->LOG("===== parseFriends: parsed data:\r\n%s\r\n", parsed.c_str());
	if (!JSON_parser_done(jc)) {
		delete_JSON_parser(jc);
		parent->LOG( "===== parseFriends JSON_parser_end: Syntax error" );
		return EXIT_FAILURE;
	}
	parent->LOG("===== parseFriends finished, got items: %d", (*this->friends).size());

	return EXIT_SUCCESS;
}

int facebook_json_parser::parseMessages( std::vector< facebook_message* >* messages, void* data )
{
	parent->LOG("===== parseMessages started");
	this->messages = messages;
	std::string json_data = (*( std::string* )data).substr( 9 );
	std::string parsed = "";

	JSON_config config;

	init_JSON_config(&config);

	config.depth                  = 20;
	config.callback               = &facebook_json_parser::parse;
	config.callback_ctx           = ( void* )this;
	config.allow_comments         = 1;
	config.handle_floats_manually = 0;

	JSON_parser_struct* jc = new_JSON_parser(&config);

	parent->LOG("===== parseMessages starts walking thru the data...");

	for (size_t i = 0; i < json_data.length( ); i++ )
	{
		int next_char = json_data.at( i );
		parsed += json_data.at( i );
		if (!JSON_parser_char(jc, next_char)) {
			delete_JSON_parser(jc);
			parent->LOG( "===== parseMessages JSON_parser_char: Syntax error at %d", i );
			return EXIT_FAILURE;
		}
	}
	parent->LOG("===== parseMessages: parsed data:\r\n%s\r\n", parsed.c_str());
	if (!JSON_parser_done(jc)) {
		delete_JSON_parser(jc);
		parent->LOG( "===== parseMessages JSON_parser_end: Syntax error" );
		return EXIT_FAILURE;
	}
	parent->LOG("===== parseMessages finished, got items: %d", (*this->messages).size());

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
		break;

	case JSON_T_OBJECT_BEGIN:
		fbjp->isKey = false;
		++fbjp->level;
		break;

	case JSON_T_OBJECT_END:
		assert(!fbjp->isKey);
		if (fbjp->level > 0) --fbjp->level;
		if ( fbjp->parserType == FB_PARSE_UPDATES && fbjp->updatesSection == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 4 )
		{
			fbjp->parent->LOG("      Got user '%s'", fbjp->currentFriend->user_id.c_str() );
			(*fbjp->friends).insert( make_pair( fbjp->currentFriend->user_id, fbjp->currentFriend ) );
			fbjp->currentFriend = NULL;
		}
		if ( fbjp->parserType == FB_PARSE_UPDATES && fbjp->updatesSection == FB_PARSE_UPDATE_USERINFOS && fbjp->level == 4 )
		{
			fbjp->currentFriendStr = "";
		}
		else if ( fbjp->parserType == FB_PARSE_MESSAGES && fbjp->level == 2 )
		{
			(*fbjp->messages).push_back( new facebook_message( *fbjp->currentMessage ) );
			fbjp->currentMessage = NULL;
		}
		break;

	case JSON_T_INTEGER:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_MESSAGES )
		{
			if ( fbjp->valueType == FB_PARSE_MESSAGE_FROM )
			{
				char time[32];
				lltoa( value->vu.integer_value, time, 10 );
				fbjp->currentMessage->user_id = time;
			}
			else if ( fbjp->valueType == FB_PARSE_MESSAGE_TIME )
			{
				fbjp->currentMessage->time = value->vu.integer_value / 1000; // microtimestamp to timestamp
			}
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
		if ( fbjp->parserType == FB_PARSE_UPDATES && fbjp->updatesSection == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 5 )
			if ( fbjp->valueType == FB_PARSE_UPDATE_IDLE )
				fbjp->currentFriend->is_idle = true;
		fbjp->valueType = 0;
		break;

	case JSON_T_FALSE:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_UPDATES && fbjp->updatesSection == FB_PARSE_UPDATE_NOWAVAILABLE && fbjp->level == 5 )
			if ( fbjp->valueType == FB_PARSE_UPDATE_IDLE )
				fbjp->currentFriend->is_idle = false;
		fbjp->valueType = 0;
		break;

	case JSON_T_KEY:
		fbjp->isKey = true;
		{
			const char* val = value->vu.str.value;
			if ( fbjp->parserType == FB_PARSE_UPDATES )
			{
				if ( strsame( val, "nowAvailableList" ) )
					fbjp->updatesSection = FB_PARSE_UPDATE_NOWAVAILABLE;
				else if ( strsame( val, "userInfos" ) )
					fbjp->updatesSection = FB_PARSE_UPDATE_USERINFOS;
				else if ( fbjp->level == 4 && fbjp->updatesSection == FB_PARSE_UPDATE_NOWAVAILABLE )
				{
					fbjp->currentFriend = new facebook_user( );
					fbjp->currentFriend->user_id = fbjp->currentFriend->real_name = val;
				}
				else if ( fbjp->level == 4 && fbjp->updatesSection == FB_PARSE_UPDATE_USERINFOS )
				{
					fbjp->currentFriendStr = val;
				}
				else if ( strsame( val, "i" ) )        fbjp->valueType = FB_PARSE_UPDATE_IDLE;
				else if ( strsame( val, "name" ) )     fbjp->valueType = FB_PARSE_UPDATE_NAME;
				else if ( strsame( val, "thumbSrc" ) ) fbjp->valueType = FB_PARSE_UPDATE_THMB;
				else if ( strsame( val, "status" ) )   fbjp->valueType = FB_PARSE_UPDATE_STAT;
			}
			else if ( fbjp->parserType == FB_PARSE_MESSAGES )
			{
				if      ( strsame( val, "msg" ) )        fbjp->currentMessage = new facebook_message( );
				else if ( strsame( val, "text" ) )       fbjp->valueType = FB_PARSE_MESSAGE_TEXT;
				else if ( strsame( val, "clientTime" ) ) fbjp->valueType = FB_PARSE_MESSAGE_TIME;
				else if ( strsame( val, "from" ) )       fbjp->valueType = FB_PARSE_MESSAGE_FROM;
			}
		}
		break;

	case JSON_T_STRING:
		fbjp->isKey = false;
		if ( fbjp->parserType == FB_PARSE_UPDATES && fbjp->updatesSection == FB_PARSE_UPDATE_USERINFOS && fbjp->level == 5 )
		{
			std::map<std::string,facebook_user*>::iterator iter = fbjp->friends->find(fbjp->currentFriendStr);
			if( iter != fbjp->friends->end() )
			{
				if      ( fbjp->valueType == FB_PARSE_UPDATE_NAME )
					(*fbjp->friends)[ fbjp->currentFriendStr ]->real_name = value->vu.str.value;
				else if ( fbjp->valueType == FB_PARSE_UPDATE_THMB )
					(*fbjp->friends)[ fbjp->currentFriendStr ]->profile_image_url = value->vu.str.value;
				else if ( fbjp->valueType == FB_PARSE_UPDATE_STAT )
			   		(*fbjp->friends)[ fbjp->currentFriendStr ]->status = value->vu.str.value;
			}
		}
		else if ( fbjp->parserType == FB_PARSE_MESSAGES )
		{
			if      ( fbjp->valueType == FB_PARSE_MESSAGE_TEXT )
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
