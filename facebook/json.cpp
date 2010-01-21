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

#define SUPERLOG parent->parent->Log

int facebook_json_parser::parse_data( void* structure, void* data )
{
	std::string json_data = (*( std::string* )data).substr( 9 );

	if      ( flag & P_BUD_LIST ) buddies = ( std::map< std::string, facebook_user* >* )structure;
	else if ( flag & P_MESSAGES ) messages = ( std::vector< facebook_message* >* )structure;

	JSON_config config;
	init_JSON_config(&config);
    config.depth                  = 20;
    config.callback               = &facebook_json_parser::parse;
    config.callback_ctx           = ( void* )this;
    config.allow_comments         = 1;
    config.handle_floats_manually = 0;

	JSON_parser_struct* jc = new_JSON_parser(&config);

	SUPERLOG("===== Parser starts walking through the data");

	for (std::string::size_type i = 0; i < json_data.length( ); i++ )
	{
		int next_char = json_data.at( i );
        if (!JSON_parser_char(jc, next_char)) {
            delete_JSON_parser(jc);
			SUPERLOG("===== Parsing failed: Syntax error at %d", i);
            return EXIT_FAILURE;
        }
    }
	if (!JSON_parser_done(jc)) {
		delete_JSON_parser(jc);
		SUPERLOG("===== Parsing failed: End syntax error");
		return EXIT_FAILURE;
	}

	SUPERLOG("===== Parsing finished");

	delete ( std::string* )data;

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse(void* ctx, int type, const JSON_value* value)
{
	facebook_json_parser* p = ( facebook_json_parser* )ctx;

	switch(type) {

	case JSON_T_ARRAY_BEGIN: p->key = false; p->lvl++; break;

	case JSON_T_ARRAY_END:
		assert(!p->key);
		if (p->lvl > 0) p->lvl--;
		if ( (p->flag&(P_BUD_LIST|P_BUD_WASAVAILABLE))==(P_BUD_LIST|P_BUD_WASAVAILABLE) && p->lvl == 3 ) p->flag &= P_SECTIONLESS;
		break;

	case JSON_T_OBJECT_BEGIN: p->key = false; p->lvl++; break;

	case JSON_T_OBJECT_END:
		assert(!p->key);
		if (p->lvl > 0) p->lvl--;
		if ( p->flag & P_BUD_LIST ) {
			if ( p->flag & P_BUD_NOWAVAILABLE && p->lvl == 4 )
			{
				p->SUPERLOG("      Got user '%s'", p->current_friend->user_id.c_str() );
				p->current_friend = NULL;
			}
			else if ( (p->flag&(P_BUD_NOWAVAILABLE|P_BUD_WASAVAILABLE))==(P_BUD_NOWAVAILABLE|P_BUD_WASAVAILABLE) && p->lvl == 3 )
			{
				p->flag &= P_SECTIONLESS;
			}
			else if ( p->flag & P_BUD_USERINFOS && p->lvl == 3 )
			{
				p->flag &= P_SECTIONLESS;
				p->current_friend = NULL;
			}
		}
		else if ( (p->flag&(P_MESSAGES|P_MSG_BODY))==(P_MESSAGES|P_MSG_BODY) && p->lvl == 2 ) {
			(*p->messages).push_back( new facebook_message( *p->current_message ) );
			p->current_message = NULL;
			p->flag &= P_SECTIONLESS;
		}
		break;

	case JSON_T_INTEGER:
		p->key = false;
		if ( (p->flag&(P_MESSAGES|P_MSG_BODY|P_MSG_FROM))==(P_MESSAGES|P_MSG_BODY|P_MSG_FROM) ) {
			char time[32];
			lltoa( value->vu.integer_value, time, 10 );
			p->current_message->user_id = time;
		}
		else if ( (p->flag&(P_MESSAGES|P_MSG_BODY|P_MSG_TIME))==(P_MESSAGES|P_MSG_BODY|P_MSG_TIME) ) {
//			JSON_int_t timestamp = value->vu.integer_value;
//			if ( timestamp > 0xffffffff ) // fix for clients not sending proper
//				timestamp /= 1000;        // timestamp, resp. microtimestamp
//			p->current_message->time = timestamp;
// TODO: Check this code
			p->current_message->time = ::time( NULL );
		}
		else if ( (p->flag&(P_BUD_LIST|P_BUD_WASAVAILABLE))==(P_BUD_LIST|P_BUD_WASAVAILABLE) && p->lvl == 4 ) {
			char was_id[32];
			lltoa( value->vu.integer_value, was_id, 10 );
			if ( (*p->buddies)[was_id]->status_id & ID_STATUS_ONLY_ONCE )
				break; // user is in both nowavail and wasavail
			(*p->buddies)[was_id]->status_id = ID_STATUS_OFFLINE;
			p->SUPERLOG("      Lost user '%s'", was_id );
		}
		p->flag &= P_VALUELESS;
		break;

	case JSON_T_FLOAT: p->key = false; break;
	case JSON_T_NULL: p->key = false; break;

	case JSON_T_TRUE:
		p->key = false;
		if ( ( p->flag&(P_BUD_LIST|P_BUD_NOWAVAILABLE|P_BUD_IDLE))==(P_BUD_LIST|P_BUD_NOWAVAILABLE|P_BUD_IDLE) && p->lvl == 5 ){
			p->current_friend->is_idle = true; p->SUPERLOG("      %s is idle", p->current_friend->real_name.c_str());}
		p->flag &= P_VALUELESS;
		break;

	case JSON_T_FALSE:
		p->key = false;
		if ( ( p->flag&(P_BUD_LIST|P_BUD_NOWAVAILABLE|P_BUD_IDLE))==(P_BUD_LIST|P_BUD_NOWAVAILABLE|P_BUD_IDLE) && p->lvl == 5 ){
			p->current_friend->is_idle = false; p->SUPERLOG("      %s is normal", p->current_friend->real_name.c_str());}
		p->flag &= P_VALUELESS;
		break;

	case JSON_T_KEY:
		p->key = true;
		{
			const char* val = value->vu.str.value;
			if ( p->flag & P_BUD_LIST )
			{
				if ( strsame( val, "nowAvailableList" ) )
					p->flag |= P_BUD_NOWAVAILABLE;
				else if ( strsame( val, "wasAvailableIDs" ) )
					p->flag |= P_BUD_WASAVAILABLE;
				else if ( strsame( val, "userInfos" ) )
					p->flag |= P_BUD_USERINFOS;
				else if ( p->lvl == 4 && p->flag & P_BUD_NOWAVAILABLE )
				{
					std::map<std::string,facebook_user*>::iterator iter = p->buddies->find(val);
					if( iter == p->buddies->end() )
						p->buddies->insert( std::make_pair( val, new facebook_user( ) ) );
					p->current_friend = (*p->buddies)[val];
					p->current_friend->user_id = p->current_friend->real_name = val;
					p->current_friend->status_id = ID_STATUS_ONLINE|ID_STATUS_ONLY_ONCE;
				}
				else if ( p->lvl == 4 && p->flag & P_BUD_USERINFOS )
				{
					std::map<std::string,facebook_user*>::iterator iter = p->buddies->find(val);
					if( iter != p->buddies->end() )
						p->current_friend = (*p->buddies)[val];
					else
						p->current_friend = NULL;
				}
				else if ( strsame( val, "i" ) )        p->flag |= P_BUD_IDLE;
				else if ( strsame( val, "name" ) )     p->flag |= P_BUD_NAME;
				else if ( strsame( val, "thumbSrc" ) ) p->flag |= P_BUD_THMB;
			}
			else if ( p->flag & P_MESSAGES )
			{
				if      ( strsame( val, "type" ) )       p->flag |= P_MSG_TYPE;
				else if ( strsame( val, "text" ) )       p->flag |= P_MSG_TEXT;
				else if ( strsame( val, "time" ) )       p->flag |= P_MSG_TIME;
				else if ( strsame( val, "from" ) )       p->flag |= P_MSG_FROM;
			}
		}
		break;

	case JSON_T_STRING:
		p->key = false;
		if ( (p->flag&(P_BUD_LIST|P_BUD_USERINFOS))==(P_BUD_LIST|P_BUD_USERINFOS) && p->lvl == 5 )
		{
			if( p->current_friend )
			{
				if ( p->flag & P_BUD_NAME )
					p->current_friend->real_name = value->vu.str.value;
				else if ( p->flag & P_BUD_THMB )
					p->current_friend->image_url = value->vu.str.value;
			}
		}
		else if ( (p->flag&(P_MESSAGES|P_MSG_TYPE))==(P_MESSAGES|P_MSG_TYPE) && strsame(value->vu.str.value, "msg" ) )
		{
			p->current_message = new facebook_message( );
			p->flag |= P_MSG_BODY;
		}
		else if ( (p->flag&(P_MESSAGES|P_MSG_TEXT|P_MSG_BODY))==(P_MESSAGES|P_MSG_TEXT|P_MSG_BODY) )
			p->current_message->message_text = value->vu.str.value;

		p->flag &= P_VALUELESS;
		break;

	default:
		assert(0);
		break;

	}

	return 1;
}
