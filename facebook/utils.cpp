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

Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

#include "common.h"

std::string utils::url::encode(const std::string &s)
{
	char *encoded = reinterpret_cast<char*>(CallService( MS_NETLIB_URLENCODE,
		0,reinterpret_cast<LPARAM>(s.c_str()) ));
	std::string ret = encoded;
	HeapFree(GetProcessHeap(),0,encoded);

	return ret;
}

std::string utils::time::unix_timestamp( )
{
	string timestamp = "";
	int in = ::time( NULL );
	std::stringstream out;
	out << in;
	timestamp = out.str();
	return timestamp;
}

std::string utils::number::random( )
{
	string number = "";
	int in = ::time( NULL );
	std::stringstream out;
	out << in;
	number = out.str();
	return number;
}

void utils::debug::info( const char* info )
{
	CreateDialogParam( g_hInstance, MAKEINTRESOURCE( IDD_DEBUGINFO ), 
		NULL, FBDebugDialogProc, ( LPARAM )info );
}

void utils::debug::test( FacebookProto* fbp )
{
	fbp->SignOn( fbp );

	return;
}

void __fastcall utils::mem::detract(char** str )
{
	detract( ( void** )str );
}

void __fastcall utils::mem::detract(void** p)
{
	if (*p)
	{
		free(*p);
		*p = NULL;
	}
}

void* __fastcall utils::mem::allocate(size_t size)
{
	void* p = NULL;

	if (size)
	{
		p = malloc(size);

		if (p)
			ZeroMemory(p, size);
	}
	return p;
}










void DebugInfo( const char* debugInfo ) // OBSOLETE
{
	utils::debug::info( debugInfo );
}

void NOTIFY( char* title, char* message )
{
	// Notify
	//POPUP( title, message );

	// Log
	string log_message = title;
	log_message += message;
	LOG( ( char* )log_message.c_str( ) );
}

void LOG( char* message )
{
}

void MB( char* m )
{
	MessageBoxA( NULL, m, NULL, MB_OK );
}

void ShowPopup( TCHAR* message )
{
}