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

unsigned int utils::text::find_matching_bracket( std::string msg, unsigned int start_bracket_position )
{
	string beginBracket = msg.substr( start_bracket_position, 1 );
	string endBracket = "";
	string textIdentifier = "";
	bool   insideText = false;

	if      ( beginBracket == "(" ) endBracket = ")";
	else if ( beginBracket == "[" ) endBracket = "]";
	else if ( beginBracket == "{" ) endBracket = "}";
	else if ( beginBracket == "<" ) endBracket = ">";
	else return 0;

	int depthLevel = 1;

	for ( unsigned int i = start_bracket_position + 1; i < msg.length( ); i++ )
	{
		string currChar = msg.substr( i, 1 );
		string prevChar = msg.substr( i - 1, 1 );

		if ( currChar == "\"" || currChar == "'" )
		{
			if ( prevChar == "\\" )
				continue;
			else
			{
				if ( !insideText )
				{
					textIdentifier = currChar;
					insideText = !insideText;
				}
				else
				{
					if ( currChar == textIdentifier )
					{
						textIdentifier = "";
						insideText = !insideText;
					}
				}
				continue;
			}
		}
		else if ( currChar == beginBracket )
		{
			if ( insideText || prevChar == "\\" )
				continue;
			else
				depthLevel++;
		}
		else if ( currChar == endBracket )
		{
			if ( insideText || prevChar == "\\" )
				continue;
			else
				depthLevel--;
		}

		if ( depthLevel == 0 )
			return i;
	}
	return 0;
}

unsigned int utils::text::find_matching_quote( std::string msg, unsigned int start_quote_position )
{
	string beginEndQuote = msg.substr( start_quote_position, 1 );
	bool   insideText = false;

	if ( beginEndQuote != "\"" && beginEndQuote != "'" )
		return 0;

	for ( unsigned int i = start_quote_position + 1; i < msg.length( ); i++ )
	{
		string currChar = msg.substr( i, 1 );

		if ( currChar == "\"" || currChar == "'" )
		{
			if ( msg.substr( i - 1, 1 ) == "\\" )
				continue;

			else if ( currChar == beginEndQuote )
				return i;
		}
	}
	return 0;
}

void utils::text::replace_all( std::string* data, std::string from, std::string to )
{
	string::size_type position = 0;

	while ( ( position = data->find(from, position) ) != string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

unsigned int utils::text::find_all( std::string* data, std::string term )
{
	unsigned int count = 0;
	string::size_type position = 0;

	while ( true )
	{
		position = data->find( term, position );
		if ( position != string::npos )
		{
			count++;
			position++;
		}
		else break;
	}

	return count;
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
	return;
}

int utils::debug::log(std::string text)
{
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInstance, szFile, SIZEOF(szFile));
	std::string path = szFile;
	path = path.substr( 0, path.rfind( "\\" ) );
	path = path.substr( 0, path.rfind( "\\" ) + 1 );
	path += "log.txt";
	FILE* f = fopen( path.c_str(), "a" );
	fprintf( f, "%s\r\n", text.c_str() );
	fclose( f );
	return EXIT_SUCCESS;
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

bool save_url(HANDLE hNetlib,const std::string &url,const std::string &filename)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	NETLIBHTTPREQUEST *resp;
	req.requestType = REQUEST_GET;
	req.szUrl = const_cast<char*>(url.c_str());

	resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService( MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(hNetlib), reinterpret_cast<LPARAM>(&req) ));

	if(resp)
	{
		// Create folder if necessary
		std::string dir = filename.substr(0,filename.rfind('\\'));
		if(_access(dir.c_str(),0))
			CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)dir.c_str());

		// Write to file
		FILE *f = fopen(filename.c_str(),"wb");
		fwrite(resp->pData,1,resp->dataLength,f);
		fclose(f);

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)resp);
		return true;
	}
	else
		return false;
}

int ext_to_format(const std::string &ext)
{
	for(size_t i=0; i<SIZEOF(formats); i++)
	{
		if(ext == formats[i].ext)
			return formats[i].fmt;
	}
	
	return PA_FORMAT_UNKNOWN;
}


// OBSOLETE

void DebugInfo( const char* debugInfo )
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
	//LOG( ( char* )log_message.c_str( ) );
}

void MB( char* m )
{
	MessageBoxA( NULL, m, NULL, MB_OK );
}

void MBI( int a )
{
	char b[32];
	itoa( a, b, 10 );
	MB( b );
}

void ShowPopup( TCHAR* message )
{
	MessageBox( NULL, message, TEXT( "Facebook Protocol" ), MB_OK ); // TODO: Popup? Merge with events?
}
