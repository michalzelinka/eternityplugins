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

#pragma once

// Parser controllers

#define strsame( a, b ) !lstrcmpA( a, b )
#define lltoa _i64toa

// Parser type flags
#define P_BUD_LIST   0x1000
#define P_MESSAGES   0x2000
#define P_TYPELESS   0x0FFF

// Section flags
#define P_BUD_NOWAVAILABLE 0x0100
#define P_BUD_WASAVAILABLE 0x0200
#define P_BUD_USERINFOS    0x0400
#define P_MSG_BODY         0x0800
#define P_SECTIONLESS      0xF0FF

// Value flags
#define P_BUD_IDLE  0x0001
#define P_BUD_NAME  0x0002
#define P_BUD_THMB  0x0004
#define P_MSG_TYPE  0x0010
#define P_MSG_TEXT  0x0020
#define P_MSG_TIME  0x0040
#define P_MSG_FROM  0x0080
#define P_VALUELESS 0xFF00

// Parser front-end

class facebook_json_parser
{
public:
	facebook_client*    parent;

	std::vector< facebook_message* >* messages;
	facebook_message* current_message;

	std::map< std::string, facebook_user* >* buddies;
	facebook_user* current_friend;

	// TODO: Convert to bitmasks
	unsigned int    flag; // [TYPE][SECT][VALUE][VALUE]
	unsigned int    lvl;
	bool    key;

	int parse_data( void*, void* );

	static int parse(void* ctx, int type, const JSON_value* value);

	facebook_json_parser( unsigned int type, facebook_client* fbc )
	{
		this->lvl = 0;
		this->key = false;
		this->flag = type;
		this->parent = fbc;
	};
};
