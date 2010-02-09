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
	time_t in = ::time( NULL );
	return utils::conversion::to_string( ( void* )&in, UTILS_CONV_TIME_T );
}

std::string utils::time::mili_timestamp( )
{
	SYSTEMTIME st;
	std::string timestamp = utils::time::unix_timestamp();
	GetSystemTime(&st);
	timestamp.append(utils::conversion::to_string( ( void* )&st.wMilliseconds, UTILS_CONV_UNSIGNED_NUMBER ));
	return timestamp;
}

std::string utils::conversion::to_string( void* data, WORD type )
{
	std::stringstream out;

	switch ( type )
	{

	case UTILS_CONV_BOOLEAN:
		if ( (*( bool* )data) == true ) return "true";
		else return "false";

	case UTILS_CONV_TIME_T:
		out << (*( time_t* )data);
		break;

	case UTILS_CONV_SIGNED_NUMBER:
		out << (*( signed long long* )data);
		break;

	case UTILS_CONV_UNSIGNED_NUMBER:
		out << (*( unsigned long long* )data);
		break;

	}

	return out.str( );
}

unsigned int utils::text::find_matching_bracket( std::string msg, unsigned int start_bracket_position )
{
	std::string beginBracket = msg.substr( start_bracket_position, 1 );
	std::string endBracket = "";
	std::string textIdentifier = "";
	bool   insideText = false;

	if      ( beginBracket == "(" ) endBracket = ")";
	else if ( beginBracket == "[" ) endBracket = "]";
	else if ( beginBracket == "{" ) endBracket = "}";
	else if ( beginBracket == "<" ) endBracket = ">";
	else return 0;

	int depthLevel = 1;

	for ( unsigned int i = start_bracket_position + 1; i < msg.length( ); i++ )
	{
		std::string currChar = msg.substr( i, 1 );
		std::string prevChar = msg.substr( i - 1, 1 );

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
	std::string beginEndQuote = msg.substr( start_quote_position, 1 );

	if ( beginEndQuote != "\"" && beginEndQuote != "'" )
		return 0;

	for ( unsigned int i = start_quote_position + 1; i < msg.length( ); i++ )
	{
		std::string currChar = msg.substr( i, 1 );

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

void utils::text::replace_first( std::string* data, std::string from, std::string to )
{
	std::string::size_type position = 0;

	if ( ( position = data->find(from, position) ) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

void utils::text::replace_all( std::string* data, std::string from, std::string to )
{
	std::string::size_type position = 0;

	while ( ( position = data->find( from, position ) ) != std::string::npos )
	{
		data->replace( position, from.size(), to );
		position++;
	}
}

unsigned int utils::text::count_all( std::string* data, std::string term )
{
	unsigned int count = 0;
	std::string::size_type position = 0;

	while ( ( position = data->find( term, position ) ) != std::string::npos )
	{
		count++;
		position++;
	}

	return count;
}

std::string utils::text::special_expressions_decode( std::string data )
{
	utils::text::replace_all( &data, "&amp;", "&" );
	utils::text::replace_all( &data, "&quot;", "\"" );
	utils::text::replace_all( &data, "&#039;", "'" );
	utils::text::replace_all( &data, "&lt;", "<" );
	utils::text::replace_all( &data, "&gt;", ">" );

	utils::text::replace_all( &data, "&hearts;", "\xE2\x99\xA5" ); // direct byte replacement
//	utils::text::replace_all( &data, "&hearts;", "\\u2665" );      // indirect slashu replacement

	utils::text::replace_all( &data, "\\/", "/" );
	utils::text::replace_all( &data, "\\\\", "\\" );

	// TODO: Add more to comply general usage
	// http://www.utexas.edu/learn/html/spchar.html
	// http://www.webmonkey.com/reference/Special_Characters
	// http://www.degraeve.com/reference/specialcharacters.php
	// http://www.chami.com/tips/internet/050798i.html
	// http://www.w3schools.com/tags/ref_entities.asp
	// http://www.natural-innovations.com/wa/doc-charset.html
	// http://webdesign.about.com/library/bl_htmlcodes.htm
	return data;
}

std::string utils::text::remove_html( std::string data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '<' && data.at(i+1) != ' ' )
		{
			i = data.find( ">", i );
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

std::string utils::text::slashu_to_utf8( std::string data )
{
	std::string new_string = "";

	for ( std::string::size_type i = 0; i < data.length( ); i++ )
	{
		if ( data.at(i) == '\\' && data.at(i+1) == 'u' )
		{
			unsigned int udn = strtol( data.substr( i + 2, 4 ).c_str(), NULL, 16 );

			if ( udn >= 128 && udn <= 2047 ) // U+0080 .. U+07FF
			{
				new_string += ( char )( 192 + ( udn / 64 ) );
				new_string += ( char )( 128 + ( udn % 64 ) );
			}
			else if ( udn >= 2048 && udn <= 65535 ) // U+0800 .. U+FFFF
			{
				new_string += ( char )( 224 + ( udn / 4096 ) );
				new_string += ( char )( 128 + ( ( udn / 64 ) % 64 ) );
				new_string += ( char )( 128 + ( udn % 64  ) );
			}
			else if ( udn <= 127 ) // U+0000 .. U+007F (should not appear)
				new_string += ( char )udn;

			i += 5;
			continue;
		}

		new_string += data.at(i);
	}

	return new_string;
}

int utils::number::random( )
{
	srand( ::time( NULL ) );
	return rand( );
}

void utils::debug::info( const char* info, HWND parent )
{
	CreateDialogParam( g_hInstance, MAKEINTRESOURCE( IDD_INFO ),
		parent, FBInfoDialogProc, ( LPARAM )mir_strdup(info) );
}

void utils::debug::test( FacebookProto* fbp )
{
	return;
}

int utils::debug::log(std::string file_name, std::string text)
{
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInstance, szFile, SIZEOF(szFile));
	std::string path = szFile;
	path = path.substr( 0, path.rfind( "\\" ) );
	path = path.substr( 0, path.rfind( "\\" ) + 1 );
	path = path + file_name.c_str() + ".txt";

	std::ofstream out( path.c_str(), std::ios_base::out | std::ios_base::app | std::ios_base::ate );
	out << text << std::endl;
	out.close( );

	return EXIT_SUCCESS;
}

void __fastcall utils::mem::detract(char** str )
{
	utils::mem::detract( ( void** )str );
}

void __fastcall utils::mem::detract(void** p)
{
	utils::mem::detract((void*)(*p));
}

void __fastcall utils::mem::detract(void* p)
{
	mir_free(p);
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

void MB( const char* m )
{
	MessageBoxA( NULL, m, NULL, MB_OK );
}

void MBI( int a )
{
	char b[32];
	itoa( a, b, 10 );
	MB( b );
}
