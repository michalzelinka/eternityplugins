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

#define strsame( a, b ) !strcmp( a, b )
#define lltoa _i64toa

#define FB_PARSE_UPDATES   1
#define FB_PARSE_MESSAGES  2

#define FB_PARSE_UPDATE_NOWAVAILABLE 1
#define FB_PARSE_UPDATE_USERINFOS    2

#define FB_PARSE_UPDATE_IDLE 1
#define FB_PARSE_UPDATE_NAME 2
#define FB_PARSE_UPDATE_THMB 3
#define FB_PARSE_UPDATE_STAT 4

#define FB_PARSE_MESSAGE_TEXT 1
#define FB_PARSE_MESSAGE_TIME 2
#define FB_PARSE_MESSAGE_FROM 3

// Parser front-end

class facebook_json_parser
{
public:
	FacebookProto*  parent;
	unsigned int    level;
	bool    isKey;
	unsigned int    parserType;
	std::string     local_user_id;

	std::vector< facebook_message* >* messages;
	facebook_message* currentMessage;

	std::map< std::string, facebook_user* >* friends;
	facebook_user* currentFriend;
	std::string  currentFriendStr;
	unsigned int updatesSection;

	unsigned int valueType;

	int parseFriends( std::map< std::string, facebook_user* >*, void* );
	int parseMessages( std::vector< facebook_message* >*, void* data );

	static int parse(void* ctx, int type, const JSON_value* value);

	facebook_json_parser( unsigned int pType, FacebookProto* fbp )
	{
		this->level = 0;
		this->isKey = false;
		this->valueType = 0;
		this->parserType = pType;
		this->updatesSection = 0;
		this->parent = fbp;
		this->currentFriendStr = "";
		this->local_user_id = fbp->facy.user_id_;
	};
};
