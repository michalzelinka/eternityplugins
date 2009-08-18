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
extern DWORD g_mirandaVersion;

facebook_communication::facebook_communication( )
{
	username_ = password_ = user_id_ = post_form_id_ = "";
	chat_sequence_num_ = 0;
	first_touch_ = false;
}

http::response facebook_communication::flap( const int request_type, char* request_data )
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

	http::response resp;

	if ( pnlhr )
	{
		store_cookies( pnlhr->headers, pnlhr->headersCount );
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	}
	else
		resp.code = 0; // Better have something set explicitely as this value
	                   // is compaired in all communication requests

	// free data
	utils::mem::detract( &nlhr.szUrl );

//// TODO: headers leak
//	for ( int i = 0; i < nlhr.headersCount; i++ )
//	{
//		if ( !nlhr.headers[i].szName && !nlhr.headers[i].szValue )
//			break;
//
//		utils::mem::detract( &nlhr.headers[i].szName );
//		utils::mem::detract( &nlhr.headers[i].szValue );
//	}

	return resp;
}

bool facebook_communication::validate_response( http::response* resp )
{
	if ( resp->code == 0 )
		return false;
	
	if ( resp->data.find( "\"errorSummary\":\"Not Logged In\"" ) != string::npos )
	{
		resp->code = 0;
		return false;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////

DWORD facebook_communication::choose_security_level( int request_type )
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

int facebook_communication::choose_method( int request_type )
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

char* facebook_communication::choose_server( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
		return FACEBOOK_SERVER_LOGIN;

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	{
		char* chatServer = ( char* )utils::mem::allocate( 64 * sizeof( char ) );
		mir_snprintf( chatServer, 64, FACEBOOK_SERVER_CHAT, 1, chat_channel_num_ );
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

char* facebook_communication::choose_action( int request_type )
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
		char* reconn_action = ( char* )utils::mem::allocate( 128 * sizeof( char ) );
		mir_snprintf( reconn_action, 128, "ajax/presence/reconnect.php?reason=%d&post_form_id=%s", 3, post_form_id_.c_str( ) );
		// TODO: What is reason "3"?
		return reconn_action;
	}

	case FACEBOOK_REQUEST_SETTINGS:
		return "ajax/chat/settings.php";

	case FACEBOOK_REQUEST_STATUS_SET:
		return "ajax/updatestatus.php";

	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return "ajax/chat/send.php";

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	{
		char* chat_action = ( char* )utils::mem::allocate( 64 * sizeof( char ) );
//		std::string is_idle = ( this->parent->m_iStatus == ID_STATUS_AWAY ) ? "false" : "true";
		std::string first_time; // DAMN
		if ( this->first_touch_ )
		{
			first_time = "true";
			this->first_touch_ = false;
		}
		else
			first_time = "false";
		mir_snprintf( chat_action, 64, "x/%s/%s/p_%s=%d", utils::time::unix_timestamp().c_str(), first_time.c_str(), user_id_.c_str(), chat_sequence_num_ );
		return chat_action;
	}

	default:
	case FACEBOOK_REQUEST_STATUS_GET:
		return "";
	}
}

char* facebook_communication::choose_request_url( int request_type )
{
	char* url = ( char* )utils::mem::allocate( 255 * sizeof( char ) );
	char* server = choose_server( request_type );
	char* action = choose_action( request_type );
	mir_snprintf( url, 255, "%s%s", server, action );
//// TODO: server + action leaks
//	utils::mem::detract( &server );
//	utils::mem::detract( &action );
	return url;
}


NETLIBHTTPHEADER* facebook_communication::get_request_headers( int request_type, int* headers_count )
{
	// TODO: cookies leak
	// TODO: user-agent leak
	// TODO: headers could be static

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_UPDATE:
	case FACEBOOK_REQUEST_SETTINGS:
	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		*( headers_count ) = 7;
		break;
	case FACEBOOK_REQUEST_POPOUT:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	case FACEBOOK_REQUEST_NOTIFICATIONS_RECEIVE:
	default:
		*( headers_count ) = 6;
		break;
	}

	NETLIBHTTPHEADER* headers = ( NETLIBHTTPHEADER* )utils::mem::allocate( sizeof( NETLIBHTTPHEADER )*( *headers_count ) );

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_UPDATE:
	case FACEBOOK_REQUEST_SETTINGS:
	case FACEBOOK_REQUEST_STATUS_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		set_header( &headers[6], "Content-Type", "application/x-www-form-urlencoded" );
	case FACEBOOK_REQUEST_POPOUT:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	case FACEBOOK_REQUEST_NOTIFICATIONS_RECEIVE:
	default:
		set_header( &headers[0], "Connection", "close" );
		set_header( &headers[1], "User-Agent", get_user_agent( ) );
		set_header( &headers[2], "Accept", "*/*" );
		set_header( &headers[3], "Cookie", load_cookies( ) );
		set_header( &headers[4], "Accept-Encoding", "gzip" );
		set_header( &headers[5], "Accept-Language", "en, C" );
		break;
	}

	return headers;
}

void facebook_communication::set_header( NETLIBHTTPHEADER* header, char* header_name, char* header_value )
{
	header->szName  = header_name;
	header->szValue = header_value;
}

// TODO: Make default user-agent string global (avoid periodical leak), move generation elsewhere (Load() +
//       Unload()), maybe add additional info
char* facebook_communication::get_user_agent( )
{
	BYTE user_agent = DBGetContactSettingByte(NULL, parent->m_szModuleName, FACEBOOK_KEY_USER_AGENT, 0);
	if (user_agent)
	{
		return user_agents[user_agent];
	}
	else
	{
		string user_agent = "Miranda IM/";

//		DWORD mir_ver = ( DWORD )CallService( MS_SYSTEM_GETVERSION, NULL, NULL );

		user_agent += ( char )((( g_mirandaVersion >> 24 ) & 0xFF ) + 0x30);
		user_agent += ".";
		user_agent += ( char )((( g_mirandaVersion >> 16 ) & 0xFF ) + 0x30);

		char* user_agent_c = ( char* )utils::mem::allocate( user_agent.length( ) * sizeof( char ) );
		strcpy( user_agent_c, user_agent.c_str( ) );

		return user_agent_c;
	}
}

char* facebook_communication::load_cookies( )
{
	// TODO: rewrite to use input string pointer (partially/fully avoid leak?)

	string cookieString = "isfbe=false;";

	for ( map< string, string >::iterator iter = cookies.begin(); iter != cookies.end(); ++iter )
		cookieString += ( *iter ).first + "=" + ( *iter ).second + ";";

	char* cookieCharString = ( char* )utils::mem::allocate( ( cookieString.length( ) ) * sizeof( char ) );
	strcpy( cookieCharString, cookieString.c_str( ) );
	return cookieCharString;
}

void facebook_communication::store_cookies( NETLIBHTTPHEADER* headers, int headersCount )
{
	for ( int i = 0; i < headersCount; i++ )
	{
		if ( strcmpi( headers[i].szName, "Set-Cookie" ) == 0 )
		{
			string cookieString = headers[i].szValue;
			string cookieName = cookieString.substr( 0, cookieString.find( "=" ) );
			string cookieValue = cookieString.substr( cookieString.find( "=" ) + 1, cookieString.find( ";" ) - cookieString.find( "=" ) - 1 );
//			cookies[cookieName] = cookieValue;
			cookies.insert( make_pair( cookieName, cookieValue ) );
		}
	}
}

void facebook_communication::clear_cookies( )
{
	cookies.clear( );
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_communication::login(const std::string &username,const std::string &password)
{
	username_ = username;
	password_ = password;

	// Prepare validation data
	clear_cookies( );
//	cookies["isfbe"] = "false";
	cookies.insert( make_pair( "isfbe", "false" ) );
//	cookies["test_cookie"] = "1";
	cookies.insert( make_pair( "test_cookie", "1" ) );

	string data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&email=";
	data += utils::url::encode( username );
	data += "&pass=";
	data += utils::url::encode( password );
	data += "&pass_placeHolder=Password&persistent=1&login=Login&charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84";

	// Send validation

	http::response resp = flap( FACEBOOK_REQUEST_LOGIN, (char*)data.c_str( ) );

	// Process result data

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200: // OK page returned, but that is regular login page we don't want in fact
	case 403: // Forbidden
	case 404: // Not Found
	default:
		parent->LOG("!!!!! login: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	case 302: // Found and redirected to Home, Logged in, everything is OK
		user_id_ = cookies["c_user"]; // TODO: Buffer overrun fix?
		return true;

	}
}

bool facebook_communication::logout( )
{
	username_ = password_ = user_id_ = "";

	// TODO: Logout process if applicable
	// TODO:RE: Yeah, applicable, sniffed

	return true;
}

bool facebook_communication::popout( )
{
	http::response resp = flap( FACEBOOK_REQUEST_POPOUT );

	// Process result data

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200:
		post_form_id_ = resp.data.substr( resp.data.find( "post_form_id" ) + 14, 32 );
		return true;

	default:
		parent->LOG("!!!!! popout: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}
}

bool facebook_communication::reconnect( )
{
	http::response resp = flap( FACEBOOK_REQUEST_RECONNECT );

	// Process result data

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200: {
		// TODO: Optimize to use std::string constructor instead of C-like conversations?
		int channel_num_begin = resp.data.find( "\"host\":" ) + 8 + 7;
		int channel_num_length = resp.data.substr( channel_num_begin ).find( "\"" );
		string channel_num = resp.data.substr( channel_num_begin, channel_num_length );

		int sequence_num_begin = resp.data.find( "\"seq\":" ) + 6;
		int sequence_num_length = resp.data.substr( sequence_num_begin ).find( "," );
		string sequence_num = resp.data.substr( sequence_num_begin, sequence_num_length );

		this->chat_channel_num_ = atoi( channel_num.c_str( ) );
		this->chat_sequence_num_ = atoi( sequence_num.c_str( ) ); }

		return true;

	default:
		parent->LOG("!!!!! reconnect: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}
}

bool facebook_communication::settings( )
{
	// Prepare settings data

	string data = "visibility=true&post_form_id=" + this->post_form_id_;

	// Send settings

	http::response resp = flap( FACEBOOK_REQUEST_SETTINGS, (char*)data.c_str( ) );

	// Process result data

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		parent->LOG("!!!!! settings: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}
}

bool facebook_communication::update( )
{
	// Prepare update data

	// TODO: Request & process notifications by adding &notifications=1
	string data = "user=" + this->user_id_ + "&popped_out=true&force_render=true&buddy_list=1&notifications=0";

	// Get update

	http::response resp = flap( FACEBOOK_REQUEST_UPDATE, (char*)data.c_str( ) );

	// Process result data

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200: {
		std::string* response_data = new std::string( resp.data );
		ForkThreadEx( &FacebookProto::ProcessUpdates, this->parent, ( void* )response_data ); }
		return true;

	default:
		parent->LOG("!!!!! update: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}
}

bool facebook_communication::channel( )
{
	// Get update

	http::response resp = flap( FACEBOOK_REQUEST_MESSAGES_RECEIVE );

	// Process result data

	validate_response(&resp);

	if ( resp.code != 200 )
	{
		// Something went wrong
	}
	else if ( resp.data.find( "\"t\":\"continue\"" ) != string::npos )
	{
		// Everything is OK, no new message received
	}
	else if ( resp.data.find( "\"t\":\"refresh\"" ) != string::npos )
	{
		// Something went wrong with the session, refresh it

		this->reconnect( );
	}
	else
	{
		// Something has been received, throw to new thread to process

		string* response_data = new std::string( resp.data );
		ForkThreadEx( &FacebookProto::ProcessMessages, this->parent, ( void* )response_data );

		// Increment sequence number

		// TODO: This should be incremented elsewhere, in order of the received
		//       messages count, but not possible right now when we know the
		//       messages count mouch later (in thread) than we need as we need
		//       this number for a following MessageLoop instantly
		// TODO:RE:
		//       Maybe via recursive find() :)
		this->chat_sequence_num_++;
	}

	// Return

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		parent->LOG("!!!!! channel: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}
}

bool facebook_communication::send_message( string message_recipient, string message_text )
{
	string data = "msg_text=";
	data += utils::url::encode( message_text );
	data += "&msg_id=";
	data += utils::number::random( );
	data += "&to=";
	data += message_recipient;
	data += "&client_time=";
	data += utils::time::unix_timestamp( ) + "000"; // TODO: Vary
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";

	http::response resp = flap( FACEBOOK_REQUEST_MESSAGE_SEND, (char*)data.c_str( ) );

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		parent->LOG("!!!!! send_message: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}

}

bool facebook_communication::set_status(const std::string &status_text)
{
	string data = "action=HOME_UPDATE&home_tab_id=1&profile_id=";
	data += this->user_id_;
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

	http::response resp = flap( FACEBOOK_REQUEST_STATUS_SET, (char*)data.c_str( ) );

	validate_response(&resp);

	switch ( resp.code )
	{

	case 200:
		return true;

	default:
		parent->LOG("!!!!! set_status: Something went wrong...");
		parent->SetStatus(ID_STATUS_OFFLINE);
		return false;

	}
}
