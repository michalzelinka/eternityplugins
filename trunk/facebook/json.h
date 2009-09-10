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

#pragma once

// Parser controllers

#define strsame( a, b ) !lstrcmpA( a, b )
#define lltoa _i64toa

#define FB_PARSE_BUDDY_LIST 1
#define FB_PARSE_MESSAGES   2

#define FB_PARSE_UPDATE_NOWAVAILABLE 1
#define FB_PARSE_UPDATE_WASAVAILABLE 2
#define FB_PARSE_UPDATE_USERINFOS    3

#define FB_PARSE_UPDATE_IDLE 1
#define FB_PARSE_UPDATE_NAME 2
#define FB_PARSE_UPDATE_THMB 3
#define FB_PARSE_UPDATE_STAT 4

#define FB_PARSE_MESSAGE_TYPE 1
#define FB_PARSE_MESSAGE_BODY 2
#define FB_PARSE_MESSAGE_TEXT 3
#define FB_PARSE_MESSAGE_TIME 4
#define FB_PARSE_MESSAGE_FROM 5

// Parser front-end

class facebook_json_parser
{
public:
	facebook_client*    parent;

	std::vector< facebook_message* >* messages;
	facebook_message* currentMessage;

	std::map< std::string, facebook_user* >* buddies;
	facebook_user* currentFriend;
	std::string  currentFriendStr;

	// TODO: Convert to bitmasks
	unsigned int    parserType;
	unsigned int    level;
	unsigned int section;
	unsigned int valueType;
	bool    isKey;

	int parse_data( void*, void* );

	static int parse(void* ctx, int type, const JSON_value* value);

	facebook_json_parser( unsigned int pType, facebook_client* fbc )
	{
		this->level = 0;
		this->isKey = false;
		this->valueType = 0;
		this->parserType = pType;
		this->section = 0;
		this->parent = fbc;
		this->currentFriendStr = "";
		if ( this->parserType != pType || this->valueType != 0 || this->section != 0 )
			throw std::exception( std::string(("error creating facebook_json_parser")).c_str() );
	};
};
