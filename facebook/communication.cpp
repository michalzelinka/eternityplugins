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

facebook::facebook( )
{
	username_ = "";
	password_ = "";
	post_form_id_ = "";
}

http::response facebook::flap( const int request_type, char* request_data )
{
	NETLIBHTTPREQUEST nlhr = {sizeof( nlhr )};
	nlhr.requestType = choose_method( request_type );
	nlhr.szUrl = this->choose_request_url( request_type );
	nlhr.flags = NLHRF_NODUMP | choose_security_level( request_type ) | NLHRF_GENERATEHOST;
	nlhr.headers = get_request_headers( request_type, &nlhr.headersCount );

	if ( request_data != NULL )
	{
		nlhr.pData = request_data;
		nlhr.dataLength = strlen( nlhr.pData );
	}

	NETLIBHTTPREQUEST* pnlhr = ( NETLIBHTTPREQUEST* )CallService( MS_NETLIB_HTTPTRANSACTION, (WPARAM)handle_, (LPARAM)&nlhr );

	store_cookies( pnlhr->headers, pnlhr->headersCount );

	// free data
	utils::mem::detract( &nlhr.szUrl );

	// TODO: free headers
	//for ( int i = 0; i < nlhr.headersCount; i++ )
	//{
	//	if ( !nlhr.headers[i].szName && !nlhr.headers[i].szValue )
	//		break;

	//	utils::mem::detract( &nlhr.headers[i].szName );
	//	utils::mem::detract( &nlhr.headers[i].szValue );
	//}

	http::response resp;

	if ( pnlhr )
	{
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	}

	return resp;
}

DWORD facebook::choose_security_level( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
		return NLHRF_SSL;

	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_POST_FORM_ID_GET:
	default:
		return ( DWORD )0;
	}
}

int facebook::choose_method( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return REQUEST_POST;

	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_POST_FORM_ID_GET:
	default:
		return REQUEST_GET;
	}
}

char* facebook::choose_server( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
		return FACEBOOK_SERVER_LOGIN"%s";

	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_POST_FORM_ID_GET:
	default:
		return FACEBOOK_SERVER_REGULAR"%s";
	}
}

char* facebook::choose_action( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
		return "login.php?login_attempt=1";

	case FACEBOOK_REQUEST_STATUS_SET:
		return "ajax/updatestatus.php";

	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return "ajax/chat/send.php";

	case FACEBOOK_REQUEST_POST_FORM_ID_GET:
		return "presence/popout.php";

	default:
	case FACEBOOK_REQUEST_STATUS_GET:
		return "";
	}
}

char* facebook::choose_request_url( int request_type )
{
	char* url = ( char* )utils::mem::allocate( 64 * sizeof( char ) );
	mir_snprintf( url, 64, choose_server( request_type ), choose_action( request_type ) );
	return url;
}


NETLIBHTTPHEADER* facebook::get_request_headers( int request_type, int* headers_count )
{
	// TODO: cookies leak, user-agent leak

	NETLIBHTTPHEADER* headers = NULL;

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_POST_FORM_ID_GET:
		*( headers_count ) = 7;
		headers = ( NETLIBHTTPHEADER* )utils::mem::allocate( sizeof( NETLIBHTTPHEADER )*( *headers_count ) );
		set_header( &headers[0], "Connection", "close" );
		set_header( &headers[1], "User-Agent", get_user_agent( ) );
		set_header( &headers[2], "Content-Type", "application/x-www-form-urlencoded" );
		set_header( &headers[3], "Accept", "*/*" );
		set_header( &headers[4], "Cookie", load_cookies( ) );
		set_header( &headers[5], "Accept-Encoding", "gzip" );
		set_header( &headers[6], "Accept-Language", "en, C" );
		break;

	default:
		*( headers_count ) = 0;
		break;
	}

	return headers;
}

void facebook::set_header( NETLIBHTTPHEADER* header, char* header_name, char* header_value )
{
	header->szName  = header_name;
	header->szValue = header_value;
}


char* facebook::get_user_agent( )
{
	string user_agent = "Miranda IM/";

	DWORD mir_ver = ( DWORD )CallService( MS_SYSTEM_GETVERSION, NULL, NULL );

	user_agent += ( char )((( mir_ver >> 24 ) & 0xFF ) + 0x30);
	user_agent += ".";
	user_agent += ( char )((( mir_ver >> 16 ) & 0xFF ) + 0x30);

	char* user_agent_c = ( char* )utils::mem::allocate( user_agent.length( ) * sizeof( char ) );
	strcpy( user_agent_c, user_agent.c_str( ) );

	return user_agent_c;
}

char* facebook::load_cookies( )
{ // TODO: rewrite to use input string pointer (partially/fully avoid leak?)
	string cookieString = "";

	for ( map< string, string >::iterator iter = cookies.begin(); iter != cookies.end(); ++iter )
		cookieString += ( *iter ).first + "=" + ( *iter ).second + ";";

	char* cookieCharString = ( char* )utils::mem::allocate( ( cookieString.length( ) ) * sizeof( char ) );
	strcpy( cookieCharString, cookieString.c_str( ) );
	return cookieCharString;
}

void facebook::store_cookies( NETLIBHTTPHEADER* headers, int headersCount )
{
	for ( int i = 0; i < headersCount; i++ )
	{
		if ( strcmpi( headers[i].szName, "Set-Cookie" ) == 0 )
		{
			string cookieString = headers[i].szValue;
			string cookieName = cookieString.substr( 0, cookieString.find( "=" ) );
			string cookieValue = cookieString.substr( cookieString.find( "=" ) + 1, cookieString.find( ";" ) - cookieString.find( "=" ) - 1 );
			cookies[cookieName] = cookieValue;
		}
	}
}

void facebook::clear_cookies( )
{
	cookies.clear( );
}

//////////////////////////////////////////////////////////////////////////////

bool facebook::validate_user(const std::string &username,const std::string &password,bool test)
{
	username_ = username;
	password_ = password;

	// Prepare validation data
	clear_cookies( );
	cookies["isfbe"] = "false";
	cookies["test_cookie"] = "1";

	string data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&email=";
	data += utils::url::encode( username );
	data += "&pass=";
	data += utils::url::encode( password );
	data += "&pass_placeHolder=Password&persistent=1&login=Login&charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84";

	// Send validation
	if ( test )
	{
		switch ( flap( FACEBOOK_REQUEST_LOGIN, (char*)data.c_str( ) ).code )
		{

		case 200: // OK page returned, but that is regular login page we don't want in fact
		case 403: // Forbidden
		case 404: // Not Found
		default:
			ShowPopup(TranslateT("Something went wrong..."));
			return false;

		case 302: // Found and redirected to Home, Logged in, everything is OK
			return true;

		}
	}
	else
		return true;
}

bool facebook::devalidate_user( )
{
	username_ = password_ = "";

	// TODO: Logout process

	return true;
}

bool facebook::get_post_form_id( )
{
	http::response resp = flap( FACEBOOK_REQUEST_POST_FORM_ID_GET );

	// Process result data

	post_form_id_ = resp.data.substr( resp.data.find( "post_form_id" ) + 14, 32 );

	switch ( resp.code )
	{

	case 200: // OK page returned, but that is regular login page we don't want in fact
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}

}

bool facebook::set_status(const std::string &status_text)
{
	string data = "action=HOME_UPDATE&home_tab_id=1&profile_id=";
	data += cookies["c_user"];
	if ( status_text.length( ) )
	{
		data += "&status=";
		data += utils::url::encode( status_text );
	}
	else
	{
		data += "&clear=1";
	}
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";
	data += "&target_id=0&app_id=&post_form_id_source=AsyncRequest";

	switch ( flap( FACEBOOK_REQUEST_STATUS_SET, (char*)data.c_str( ) ).code )
	{

	case 200: // OK page returned, but that is regular login page we don't want in fact
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}
}

bool facebook::send_message( string message_recipient, string message_text )
{
	string data = "msg_text=";
	data += utils::url::encode( message_text );
	data += "&msg_id=";
	data += rand( ); // message ID = random INT x)
	data += "&to=";
	data += message_recipient;
	data += "&client_time=";
	data += time( NULL );
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : 0;

	switch ( flap( FACEBOOK_REQUEST_MESSAGE_SEND, (char*)data.c_str( ) ).code )
	{

	case 200: // OK page returned, but that is regular login page we don't want in fact
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}

}