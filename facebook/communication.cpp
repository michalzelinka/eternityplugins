/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009 Michal Zelinka

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

//#include "tinyjson.hpp"
#include "common.h"

facebook::facebook( )
{
	username_ = password_ = user_id_ = post_form_id_ = "";
	chat_sequence_num_ = 0;
}

http::response facebook::flap( const int request_type, char* request_data )
{
	NETLIBHTTPREQUEST nlhr = {sizeof( nlhr )};
	nlhr.requestType = choose_method( request_type );
	nlhr.szUrl = choose_request_url( request_type );
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

	case FACEBOOK_REQUEST_POPOUT:
	case FACEBOOK_REQUEST_UPDATE:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_SETTINGS:
	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	default:
		return ( DWORD )0;
	}
}

int facebook::choose_method( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_UPDATE:
	case FACEBOOK_REQUEST_SETTINGS:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return REQUEST_POST;

	case FACEBOOK_REQUEST_POPOUT:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_STATUS_GET:
	default:
		return REQUEST_GET;
	}
}

char* facebook::choose_server( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
		return FACEBOOK_SERVER_LOGIN;

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	{
		char* chatServer = ( char* )utils::mem::allocate( 64 * sizeof( char ) );
		mir_snprintf( chatServer, 64, FACEBOOK_SERVER_CHAT, 0, chat_channel_num_ );
		return chatServer;
	}

	case FACEBOOK_REQUEST_SETTINGS:
		return FACEBOOK_SERVER_APPS;

	case FACEBOOK_REQUEST_POPOUT:
	case FACEBOOK_REQUEST_UPDATE:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	default:
		return FACEBOOK_SERVER_REGULAR;
	}
}

char* facebook::choose_action( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
		return "login.php?login_attempt=1";

	case FACEBOOK_REQUEST_POPOUT:
		return "presence/popout.php";

	case FACEBOOK_REQUEST_UPDATE:
		return "ajax/presence/update.php";

	case FACEBOOK_REQUEST_RECONNECT:
	{
		
		char* reconnAction = ( char* )utils::mem::allocate( 128 * sizeof( char ) );
		mir_snprintf( reconnAction, 128, "ajax/presence/reconnect.php?reason=%d&post_form_id=%s", 3, post_form_id_.c_str( ) );
		// TODO: What is reason "3"?
		return reconnAction;
	}

	case FACEBOOK_REQUEST_SETTINGS:
		return "ajax/chat/settings.php";

	case FACEBOOK_REQUEST_STATUS_SET:
		return "ajax/updatestatus.php";

	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return "ajax/chat/send.php";

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	{
		
		char* chatAction = ( char* )utils::mem::allocate( 64 * sizeof( char ) );
		mir_snprintf( chatAction, 64, "x/%s/false/p_%s=%d", utils::time::unix_timestamp( ).c_str( ), user_id_.c_str( ), chat_sequence_num_ );
		return chatAction;
	}

	default:
	case FACEBOOK_REQUEST_STATUS_GET:
		return "";
	}
}

char* facebook::choose_request_url( int request_type )
{
	char* url = ( char* )utils::mem::allocate( 255 * sizeof( char ) );
	char* server = choose_server( request_type );
	char* action = choose_action( request_type );
	mir_snprintf( url, 255, "%s%s", server, action );
	utils::mem::detract( &server );
	utils::mem::detract( &action );
	return url;
}


NETLIBHTTPHEADER* facebook::get_request_headers( int request_type, int* headers_count )
{
	// TODO: cookies leak, user-agent leak, should be static

	NETLIBHTTPHEADER* headers = NULL;

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_POPOUT:
	case FACEBOOK_REQUEST_UPDATE:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_SETTINGS:
	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	case FACEBOOK_REQUEST_NOTIFICATIONS_RECEIVE:
	default:
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

	case 0:
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

bool facebook::login(const std::string &username,const std::string &password,bool test)
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

	http::response resp = flap( FACEBOOK_REQUEST_LOGIN, (char*)data.c_str( ) );

	// Process response data

	user_id_ = cookies["c_user"];

	if ( test )
	{
		switch ( resp.code )
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

bool facebook::logout( )
{
	username_ = password_ = user_id_ = "";

	// TODO: Logout process

	return true;
}

bool facebook::popout( )
{
	http::response resp = flap( FACEBOOK_REQUEST_POPOUT );

	// Process result data

	post_form_id_ = resp.data.substr( resp.data.find( "post_form_id" ) + 14, 32 );

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}
}

bool facebook::reconnect( )
{
	http::response resp = flap( FACEBOOK_REQUEST_RECONNECT );

	// Process result data

	int channel_num_begin = resp.data.find( "\"host\":" ) + 8 + 7;
	int channel_num_length = resp.data.substr( channel_num_begin ).find( "\"" );
	string channel_num = resp.data.substr( channel_num_begin, channel_num_length );

	int sequence_num_begin = resp.data.find( "\"seq\":" ) + 6;
	int sequence_num_length = resp.data.substr( sequence_num_begin ).find( "," );
	string sequence_num = resp.data.substr( sequence_num_begin, sequence_num_length );

	this->chat_channel_num_ = atoi( channel_num.c_str( ) );
	this->chat_sequence_num_ = atoi( sequence_num.c_str( ) );

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}
}

bool facebook::settings( )
{
	// Prepare settings data

	string data = "visibility=true&post_form_id=" + this->post_form_id_;

	// Send settings

	http::response resp = flap( FACEBOOK_REQUEST_SETTINGS, (char*)data.c_str( ) );

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}
}

bool facebook::update( )
{
	// Prepare update data

	string data = "user=" + this->user_id_ + "&popped_out=true&force_render=true&buddy_list=1";
	// TODO: &notifications=1

	// Get update

	http::response resp = flap( FACEBOOK_REQUEST_UPDATE, (char*)data.c_str( ) );

	// Process result data

	utils::debug::info( ( char* )resp.data.c_str( ) );

	// Return

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}
}

bool facebook::channel( )
{
	return true;
}

bool facebook::send_message( string message_recipient, string message_text )
{
	string data = "msg_text=";
	data += utils::url::encode( message_text );
	data += "&msg_id=";
	data += utils::number::random( );
	data += "&to=";
	data += message_recipient;
	data += "&client_time=";
	data += utils::time::unix_timestamp( );
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";

	switch ( flap( FACEBOOK_REQUEST_MESSAGE_SEND, (char*)data.c_str( ) ).code )
	{

	case 200:
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
		data += "&clear=1"; // TODO: Remove "0" length limit in dialog
	}
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";
	data += "&target_id=0&app_id=&post_form_id_source=AsyncRequest";

	switch ( flap( FACEBOOK_REQUEST_STATUS_SET, (char*)data.c_str( ) ).code )
	{

	case 200:
		return true;

	default:
		ShowPopup(TranslateT("Something went wrong..."));
		return false;

	}
}
