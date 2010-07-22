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

void facebook_client::client_notify( TCHAR* message )
{
	parent->NotifyEvent( parent->m_tszUserName, message );
}

http::response facebook_client::flap( const int request_type, std::string* request_data )
{
	NETLIBHTTPREQUEST nlhr = {sizeof( nlhr )};
	nlhr.requestType = choose_method( request_type );
	std::string url = choose_request_url( request_type, request_data );
	nlhr.szUrl = (char*)url.c_str( );
	nlhr.flags = NLHRF_NODUMP | choose_security_level( request_type ) | NLHRF_GENERATEHOST;
	nlhr.headers = get_request_headers( request_type, &nlhr.headersCount );

	if ( request_data != NULL )
	{
		nlhr.pData = (char*)(*request_data).c_str();
		nlhr.dataLength = request_data->length( );
	}

	parent->Log("@@@@@ Sending request to '%s'", nlhr.szUrl);

	NETLIBHTTPREQUEST* pnlhr = ( NETLIBHTTPREQUEST* )CallService( MS_NETLIB_HTTPTRANSACTION, (WPARAM)handle_, (LPARAM)&nlhr );

	http::response resp;

	if ( pnlhr )
	{
		parent->Log("@@@@@ Got response with code %d", pnlhr->resultCode);
		store_headers( &resp, pnlhr->headers, pnlhr->headersCount );
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	}
	else
	{
		parent->Log("!!!!! No response from server (time-out)");
		resp.code = HTTP_CODE_FAKE_DISCONNECTED;
		// Better to have something set explicitely as this value
	    // is compaired in all communication requests
	}

	return resp;
}

bool facebook_client::validate_response( http::response* resp )
{
	if ( resp->code == HTTP_CODE_FAKE_DISCONNECTED ) {
		parent->Log(" ! !  Request has timed out, connection or server error");
		return false; }

	if ( resp->data.find( "{\"error\":" ) != std::string::npos ) try
	{
		std::string error = resp->data.substr( resp->data.find( "{\"error\":" ) + 9, 64 );
		int error_num = atoi( error.substr( 0, error.find( "," ) ).c_str() );
		if ( error_num != 0 )
		{
			error = error.substr( error.find( "\"errorDescription\":\"" ) + 20 );
			error = error.substr( 0, error.find( "\"," ) );
			parent->Log(" ! !  Received Facebook error: %d -- %s", error_num, error.c_str());
			resp->code = HTTP_CODE_FAKE_LOGGED_OUT;
			return false;
		}
	} catch (const std::exception &e) {
		parent->Log(" @ @  validate_response: Exception: %s",e.what());return false; }

	return true;
}

bool facebook_client::handle_entry( std::string method )
{
	parent->Log("   >> Entering %s()", method.c_str());
	return true;
}

bool facebook_client::handle_success( std::string method )
{
	parent->Log("   << Quitting %s()", method.c_str());
	reset_error();
	return true;
}

bool facebook_client::handle_error( std::string method, bool force_disconnect )
{
	bool result;
	increment_error();
	parent->Log("!!!!! %s(): Something with Facebook went wrong", method.c_str());

	if ( force_disconnect )
		result = false;
	else if ( error_count_ <= (UINT)DBGetContactSettingByte(NULL,parent->m_szModuleName,FACEBOOK_KEY_TIMEOUTS_LIMIT,FACEBOOK_TIMEOUTS_LIMIT))
		result = true;
	else
		result = false;

	if ( result == false ) {
		reset_error(); parent->SetStatus(ID_STATUS_OFFLINE); }

	return result;
}

//////////////////////////////////////////////////////////////////////////////

DWORD facebook_client::choose_security_level( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return NLHRF_SSL;

	case FACEBOOK_REQUEST_API_CHECK:
	case FACEBOOK_REQUEST_LOGOUT:
	case FACEBOOK_REQUEST_KEEP_ALIVE:
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_FEEDS:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_PROFILE_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	default:
		return ( DWORD )0;
	}
}

int facebook_client::choose_method( int request_type )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
	case FACEBOOK_REQUEST_KEEP_ALIVE:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return REQUEST_POST;

	case FACEBOOK_REQUEST_API_CHECK:
	case FACEBOOK_REQUEST_LOGOUT:
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_FEEDS:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_PROFILE_GET:
	default:
		return REQUEST_GET;
	}
}

std::string facebook_client::choose_server( int request_type, std::string* data )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_API_CHECK:
		return "http://code.google.com/";

	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return FACEBOOK_SERVER_LOGIN;

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE: {
		std::string server = FACEBOOK_SERVER_CHAT;
		utils::text::replace_first( &server, "%s", "0" );
		utils::text::replace_first( &server, "%s", this->chat_channel_host_ );
		return server; }

	case FACEBOOK_REQUEST_PROFILE_GET:
		return FACEBOOK_SERVER_MOBILE;

	case FACEBOOK_REQUEST_LOGOUT:
	case FACEBOOK_REQUEST_KEEP_ALIVE:
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_FEEDS:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
	default:
		return FACEBOOK_SERVER_REGULAR;
	}
}

std::string facebook_client::choose_action( int request_type, std::string* data )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_API_CHECK:
		return "p/eternityplugins/wiki/FacebookProtocol_DevelopmentProgress";

	case FACEBOOK_REQUEST_LOGIN:
		return "login.php?login_attempt=1";

	case FACEBOOK_REQUEST_SETUP_MACHINE:
		return "loginnotify/setup_machine.php";

	case FACEBOOK_REQUEST_LOGOUT:
		return "logout.php";

	case FACEBOOK_REQUEST_KEEP_ALIVE:
		return "ajax/presence/update.php";

	case FACEBOOK_REQUEST_HOME:
		return "home.php?";

	case FACEBOOK_REQUEST_BUDDY_LIST:
		return "ajax/chat/buddy_list.php";

	case FACEBOOK_REQUEST_FEEDS: {
		// Filters: lf = live feed, h = news feed
		// TODO: Make filter selection customizable?
		std::string action = "ajax/intent.php?filter=lf&request_type=1&__a=1&newest=%s&ignore_self=true";
		std::string newest = utils::conversion::to_string((void*)&this->last_feeds_update_, UTILS_CONV_TIME_T);
		utils::text::replace_first( &action, "%s", newest );
		return action; }

	case FACEBOOK_REQUEST_RECONNECT: {
		std::string action = "ajax/presence/reconnect.php?reason=%s&iframe_loaded=false&post_form_id=%s&__a=1&nctr[n]=1";
		std::string reason = ( this->chat_first_touch_ ) ? FACEBOOK_RECONNECT_LOGIN : FACEBOOK_RECONNECT_KEEP_ALIVE;
		utils::text::replace_first( &action, "%s", reason );
		utils::text::replace_first( &action, "%s", this->post_form_id_ );
		return action; }

	case FACEBOOK_REQUEST_PROFILE_GET: {
		std::string action = "profile.php?id=%s&v=info";
		utils::text::replace_first( &action, "%s", (*data) );
		return action; }

	case FACEBOOK_REQUEST_STATUS_SET:
		return "ajax/updatestatus.php";

	case FACEBOOK_REQUEST_MESSAGE_SEND:
		return "ajax/chat/send.php";

	case FACEBOOK_REQUEST_MESSAGES_RECEIVE: {
		std::string action = "x/%s/%s/p_%s=%d";
		std::string first_time;
		if ( this->chat_first_touch_ ) { first_time = "true"; this->chat_first_touch_ = false; }
		else first_time = "false";
		utils::text::replace_first( &action, "%s", utils::time::unix_timestamp() );
		utils::text::replace_first( &action, "%s", first_time );
		utils::text::replace_first( &action, "%s", self_.user_id );
		utils::text::replace_first( &action, "%d", utils::conversion::to_string( (void*)&chat_sequence_num_, UTILS_CONV_UNSIGNED_NUMBER ) );
		return action; }

	default:
		return "";
	}
}

std::string facebook_client::choose_request_url( int request_type, std::string* data )
{
	std::string url = choose_server( request_type, data );
	url.append( choose_action( request_type, data ) );
	return url;
}


NETLIBHTTPHEADER* facebook_client::get_request_headers( int request_type, int* headers_count )
{
	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_PROFILE_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		*headers_count = 7;
		break;
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_FEEDS:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	default:
		*headers_count = 6;
		break;
	case FACEBOOK_REQUEST_API_CHECK:
		*headers_count = 5;
		break;
	}

	NETLIBHTTPHEADER* headers = ( NETLIBHTTPHEADER* )utils::mem::allocate( sizeof( NETLIBHTTPHEADER )*( *headers_count ) );

	refresh_headers( ); // TODO: Refresh only when required

	switch ( request_type )
	{
	case FACEBOOK_REQUEST_LOGIN:
	case FACEBOOK_REQUEST_SETUP_MACHINE:
	case FACEBOOK_REQUEST_BUDDY_LIST:
	case FACEBOOK_REQUEST_PROFILE_GET:
	case FACEBOOK_REQUEST_STATUS_SET:
	case FACEBOOK_REQUEST_MESSAGE_SEND:
		set_header( &headers[6], "Content-Type" );
	case FACEBOOK_REQUEST_HOME:
	case FACEBOOK_REQUEST_RECONNECT:
	case FACEBOOK_REQUEST_MESSAGES_RECEIVE:
	default:
		set_header( &headers[5], "Cookie" );
	case FACEBOOK_REQUEST_API_CHECK:
		set_header( &headers[4], "Connection" );
		set_header( &headers[3], "User-Agent" );
		set_header( &headers[2], "Accept" );
		set_header( &headers[1], "Accept-Encoding" );
		set_header( &headers[0], "Accept-Language" );
		break;
	}

	return headers;
}

void facebook_client::set_header( NETLIBHTTPHEADER* header, char* name )
{
	header->szName  = name;
	header->szValue = (char*)this->headers[name].c_str();
}

void facebook_client::refresh_headers( )
{
	if ( headers.size() < 5 )
	{
		this->headers["Connection"] = "close";
		this->headers["Accept"] = "*/*";
// TODO: Set Accept-Encoding: none as default for _DEBUG builds
//		this->headers["Accept-Encoding"] = "none";
		this->headers["Accept-Encoding"] = "deflate, gzip, x-gzip, identity, *;q=0";
		this->headers["Accept-Language"] = "en,en-US;q=0.9";
		this->headers["Content-Type"] = "application/x-www-form-urlencoded";
	}
	this->headers["User-Agent"] = get_user_agent( );
	this->headers["Cookie"] = load_cookies( );
}

std::string facebook_client::get_user_agent( )
{
	BYTE user_agent = DBGetContactSettingByte(NULL, parent->m_szModuleName, FACEBOOK_KEY_USER_AGENT, 0);
	if (user_agent > 0 && user_agent < SIZEOF(user_agents))
		return user_agents[user_agent];
	else return g_strUserAgent;
}

std::string facebook_client::load_cookies( )
{
	std::string cookieString = "isfbe=false;";

	if ( cookies.size() )
		for ( std::map< std::string, std::string >::iterator iter = cookies.begin(); iter != cookies.end(); ++iter ) {
			cookieString.append( iter->first );
			cookieString.append( "=" );
			cookieString.append( iter->second );
			cookieString.append( ";" ); }

	return cookieString;
}

void facebook_client::store_headers( http::response* resp, NETLIBHTTPHEADER* headers, int headersCount )
{
	for ( int i = 0; i < headersCount; i++ )
	{
		std::string header_name = headers[i].szName; // TODO: Casting?
		std::string header_value = headers[i].szValue; // TODO: Casting?

		if ( header_name == std::string( "Set-Cookie" ) ) // TODO: Is casting needed?
		{
			std::string cookie_name = header_value.substr( 0, header_value.find( "=" ) );
			std::string cookie_value = header_value.substr( header_value.find( "=" ) + 1, header_value.find( ";" ) - header_value.find( "=" ) - 1 );
			if ( cookie_value == std::string( "deleted" ) ) {
				parent->Log("      Deleted cookie '%s'", cookie_name.c_str());
				cookies.erase( cookie_name ); }
			else {
				parent->Log("      New cookie '%s': %s", cookie_name.c_str(), cookie_value.c_str());
				// TODO: append+set?
				cookies[cookie_name] = cookie_value; }
		}
		else
		{
			parent->Log("----- Got header '%s': %s", header_name.c_str(), header_value.c_str() );
			resp->headers[header_name] = header_value;
		}
	}
}

void facebook_client::clear_cookies( )
{
	if ( cookies.size() )
		cookies.clear( );
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_client::api_check( )
{
	handle_entry( "api_check" );

	http::response resp = flap( FACEBOOK_REQUEST_API_CHECK );

	// Process result

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		std::string key = "</h1><p><var>";
		std::string::size_type start = resp.data.find( key ) + key.length( );
		std::string::size_type end = resp.data.find( "</var>", start );
		std::string api_version_latest = resp.data.substr( start, end - start );

		if ( start > 0 && end > 0 && std::string( __API_VERSION_STRING ) != api_version_latest )
			client_notify( TEXT( "Facebook API version has changed, wait and watch for the Facebook protocol update." ) );

	}

	return handle_success( "api_check" );
}

bool facebook_client::login(const std::string &username,const std::string &password)
{
	handle_entry( "login" );

	username_ = username;
	password_ = password;

	// Prepare validation data
	clear_cookies( );

	// Access homepage to get initial cookies
	flap( FACEBOOK_REQUEST_HOME, NULL );

	// Prepare login data
	std::string data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&email=";
	data += utils::url::encode( username );
	data += "&pass=";
	data += utils::url::encode( password );
	data += "&pass_placeHolder=Password&persistent=1&login=Login&charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84";

	// Send validation
	http::response resp = flap( FACEBOOK_REQUEST_LOGIN, &data );

	// Process result data
	validate_response(&resp);

	// Check whether setting Machine name is required
	if ( resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end() && resp.headers["Location"].find("loginnotify/setup_machine.php") != std::string::npos ) {
		data = "charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&locale=en&machinename=";
		data += g_strUserAgent;
		flap( FACEBOOK_REQUEST_SETUP_MACHINE, &data ); }

	switch ( resp.code )
	{

	case HTTP_CODE_OK: // OK page returned, but that is regular login page we don't want in fact
		client_notify(TranslateT("Probably wrong password entered, please try again."));
	case HTTP_CODE_FORBIDDEN: // Forbidden
	case HTTP_CODE_NOT_FOUND: // Not Found
	default:
		return handle_error( "login", FORCE_DISCONNECT );

	case HTTP_CODE_FOUND: // Found and redirected to Home, Logged in, everything is OK
		if ( cookies.find("c_user") != cookies.end() ) {
			this->self_.user_id = cookies.find("c_user")->second;
			DBWriteContactSettingString(NULL,parent->m_szModuleName,FACEBOOK_KEY_ID,this->self_.user_id.c_str());
			parent->Log("      Got self user id: %s", this->self_.user_id.c_str());
			return handle_success( "login" );
		}
		else {
			client_notify(TranslateT("Login error, probably bad login credentials."));
			return handle_error( "login", FORCE_DISCONNECT );
		}
	}
}

bool facebook_client::logout( )
{
	if ( parent->isOffline() )
		return true;

	if ( DBGetContactSettingByte(NULL, parent->m_szModuleName, FACEBOOK_KEY_DISABLE_LOGOUT, 0) )
		return true;

	handle_entry( "logout" );

	http::response resp = flap( FACEBOOK_REQUEST_LOGOUT );

	// Process result
	username_ = password_ = self_.user_id = "";

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
	case HTTP_CODE_FOUND:
		return handle_success( "logout" );

	default:
		return handle_error( "logout", FORCE_DISCONNECT );

	}
}

bool facebook_client::keep_alive( )
{
	if ( parent->isOffline() )
		return false;

	handle_entry( "keep_alive" );

	std::string data = "user=" + this->self_.user_id + "&popped_out=false&force_render=true&buddy_list=1&notifications=0&post_form_id=" + this->post_form_id_ + "&fb_dtsg=" + this->dtsg_ + "&post_form_id_source=AsyncRequest&__a=1&nctr[n]=1";

	ScopedLock s(buddies_lock_);

	for (std::map< std::string, facebook_user* >::iterator i = buddies.begin(); i != buddies.end(); ++i )
	{
		data += "&available_list[";
		data += i->first;
		data += "][i]=";
		data += ( i->second->is_idle ) ? "1" : "0";
	}

	http::response resp = flap( FACEBOOK_REQUEST_KEEP_ALIVE, &data );

	// Process result
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
	case HTTP_CODE_FOUND:
		return handle_success( "keep_alive" );

	default:
		return handle_error( "keep_alive" );

	}
}

bool facebook_client::home( )
{
	handle_entry( "home" );

	http::response resp = flap( FACEBOOK_REQUEST_HOME );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		if ( resp.data.find( "id=\"navAccountName\"" ) != std::string::npos )
		{
			std::string::size_type start, end;

			// Get real_name
			start = resp.data.find( "id=\"navAccountName\"" );
			end = resp.data.find( "</a>", start );
			start = resp.data.rfind( "\">", end ) + 2;
			this->self_.real_name = resp.data.substr( start, end - start );
			this->self_.real_name = utils::text::special_expressions_decode( this->self_.real_name );
			DBWriteContactSettingUTF8String(NULL,parent->m_szModuleName,FACEBOOK_KEY_NAME,this->self_.real_name.c_str());
			DBWriteContactSettingUTF8String(NULL,parent->m_szModuleName,"Nick",this->self_.real_name.c_str());
			parent->Log("      Got self real name: %s", this->self_.real_name.c_str());

			// Get post_form_id
			this->post_form_id_ = resp.data.substr( resp.data.find( "post_form_id:" ) + 14, 32 );
			parent->Log("      Got self post form id: %s", this->post_form_id_.c_str());

			// If something would go wrong:
//			this->post_form_id_ = resp.data.substr( resp.data.find( "post_form_id:" ) + 14, 48 );
//			this->post_form_id_ = this->post_form_id_.substr( 0, this->post_form_id_.find( "\"" ) );

			// Get dtsg
			this->dtsg_ = resp.data.substr( resp.data.find( "fb_dtsg:" ) + 9, 48 );
			this->dtsg_ = this->dtsg_.substr( 0, this->dtsg_.find( "\"" ) );
			parent->Log("      Got self dtsg: %s", this->dtsg_.c_str());

			// Get friend requests count and messages count and notify it
			if ( DBGetContactSettingByte( NULL, parent->m_szModuleName, FACEBOOK_KEY_NOTIFICATIONS_ENABLE, 0 ) ) {
				start = resp.data.find( "<span id=\"jewelRequestCount\">" );
				if ( start != std::string::npos ) {
					start += 29;
					end = resp.data.find( "</span>", start );
					std::string str_count = resp.data.substr(start, end-start);
					if ( str_count != std::string( "0" ) ) {
						TCHAR* message = TranslateT( "Got new friend requests: " );
						TCHAR* count = mir_a2t_cp( str_count.c_str( ), CP_UTF8 );
						TCHAR* info = ( TCHAR* )malloc( ( lstrlen( message ) + lstrlen( count ) ) * sizeof( TCHAR ) );
						lstrcpy( info, message );
						lstrcat( info, count );
						this->parent->NotifyEvent( this->parent->m_tszUserName, info );
						mir_free( message );
						mir_free( count );
						mir_free( info ); } }

				start = resp.data.find( "<span id=\"jewelInnerUnseenCount\">" );
				if ( start != std::string::npos ) {
					start += 33;
					end = resp.data.find( "</span>", start );
					std::string str_count = resp.data.substr(start, end-start);
					if ( str_count != std::string( "0" ) ) {
						TCHAR* message = TranslateT( "Got new messages: " );
						TCHAR* count = mir_a2t_cp( str_count.c_str( ), CP_UTF8 );
						TCHAR* info = ( TCHAR* )malloc( ( lstrlen( message ) + lstrlen( count ) ) * sizeof( TCHAR ) );
						lstrcpy( info, message );
						lstrcat( info, count );
						this->parent->NotifyEvent( this->parent->m_tszUserName, info );
						mir_free( message );
						mir_free( count );
						mir_free( info ); } }
			}

			// Set first touch flag
			this->chat_first_touch_ = true;

			// Update self-contact
			ForkThreadEx(&FacebookProto::UpdateContactWorker, this->parent, (void*)&this->self_);

			return handle_success( "home" );
		}
		else {
			client_notify(TranslateT("Something happened to Facebook. Maybe there was some major update so you should wait for an update, or you have Facebook Lite set as default."));
			return handle_error( "home", FORCE_DISCONNECT );
		}

	case HTTP_CODE_FOUND:
		// Work-around for replica_down, f**king hell what's that?
		return this->home();

	default:
		return handle_error( "home", FORCE_DISCONNECT );

	}
}

bool facebook_client::reconnect( )
{
	handle_entry( "reconnect" );

	http::response resp = flap( FACEBOOK_REQUEST_RECONNECT );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK: {
		std::string channel_num = resp.data.substr( resp.data.find( "\"host\":" ) + 8, 16 );
		std::string::size_type end = channel_num.find( "\"" );
		this->chat_channel_host_ = channel_num.substr( 0, end );
		parent->Log("      Got self channel host: %s", this->chat_channel_host_.c_str());

		std::string sequence_num = resp.data.substr( resp.data.find( "\"seq\":" ) + 6, 16 );
		end = sequence_num.find( "," );
		sequence_num = sequence_num.substr( 0, end );
		this->chat_sequence_num_ = atoi( sequence_num.c_str( ) );
		parent->Log("      Got self sequence number: %d", this->chat_sequence_num_); }

		return handle_success( "reconnect" );

	default:
		return handle_error( "reconnect", FORCE_DISCONNECT );

	}
}

bool facebook_client::buddy_list( )
{
	handle_entry( "buddy_list" );

	// Prepare update data
	ScopedLock s(buddies_lock_);

	std::string data = "user=" + this->self_.user_id + "&popped_out=false&force_render=true&buddy_list=1&notifications=0&post_form_id=" + this->post_form_id_ + "&fb_dtsg=" + this->dtsg_ + "&post_form_id_source=AsyncRequest&__a=1&nctr[n]=1";

	for (std::map< std::string, facebook_user* >::iterator i = buddies.begin(); i != buddies.end(); ++i )
	{
		data += "&available_list[";
		data += i->first;
		data += "][i]=";
		data += ( i->second->is_idle ) ? "1" : "0";
	}

	// Get buddy list
	http::response resp = flap( FACEBOOK_REQUEST_BUDDY_LIST, &data );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		if ( resp.data.find( "\"listChanged\":true" ) != std::string::npos ) {
			std::string* response_data = new std::string( resp.data );
			ForkThreadEx( &FacebookProto::ProcessBuddyList, this->parent, ( void* )response_data ); }
		return handle_success( "buddy_list" );

	case HTTP_CODE_FAKE_LOGGED_OUT:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "buddy_list" );

	}
}

bool facebook_client::feeds( )
{
	handle_entry( "feeds" );

	// Get feeds
	http::response resp = flap( FACEBOOK_REQUEST_FEEDS );

	// Process result data
	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		if ( resp.data.find( "\"storyCount\":" ) != std::string::npos ) {
			std::string* response_data = new std::string( resp.data );
			ForkThreadEx( &FacebookProto::ProcessFeeds, this->parent, ( void* )response_data ); }
		return handle_success( "feeds" );

	case HTTP_CODE_FAKE_LOGGED_OUT:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "feeds" );

	}
}

bool facebook_client::channel( )
{
	handle_entry( "channel" );

	// Get update
	http::response resp = flap( FACEBOOK_REQUEST_MESSAGES_RECEIVE );

	// Process result data
	validate_response(&resp);

	if ( resp.code != HTTP_CODE_OK )
	{
		// Something went wrong
	}
	else if ( resp.data.find( "\"t\":\"continue\"" ) != std::string::npos )
	{
		// Everything is OK, no new message received
	}
	else if ( resp.data.find( "\"t\":\"refresh\"" ) != std::string::npos )
	{
		// Something went wrong with the session (has expired?), refresh it
		this->reconnect( );
	}
	else
	{
		// Something has been received, throw to new thread to process
		std::string* response_data = new std::string( resp.data );
		ForkThreadEx( &FacebookProto::ProcessMessages, this->parent, ( void* )response_data );

		// Increment sequence number
		this->chat_sequence_num_ += utils::text::count_all( &resp.data, "\"type\":\"" );
	}

	// Return
	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "channel" );

	case HTTP_CODE_FAKE_LOGGED_OUT:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "channel" );

	}
}

bool facebook_client::send_message( std::string message_recipient, std::string message_text )
{
	handle_entry( "send_message" );

	std::string data = "msg_text=";
	data += utils::url::encode( message_text );
	data += "&msg_id=";
	data += utils::time::unix_timestamp( );
	data += "&to=";
	data += message_recipient;
	data += "&client_time=";
	data += utils::time::mili_timestamp( );
	data += "&post_form_id=";
	data += ( post_form_id_.length( ) ) ? post_form_id_ : "0";

	http::response resp = flap( FACEBOOK_REQUEST_MESSAGE_SEND, &data );

	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "send_message" );

	case HTTP_CODE_FAKE_LOGGED_OUT:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "send_message" );

	}

}

bool facebook_client::get_profile(facebook_user* fbu)
{
	handle_entry( "get_profile" );

	http::response resp = flap( FACEBOOK_REQUEST_PROFILE_GET, &fbu->user_id );

	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		{
			std::string::size_type start = resp.data.find( "<div class=\"section_title\">" );
			start = resp.data.find( "<div id=\"anchor_fbid_", start );
			if ( start != std::string::npos ) {
				start = resp.data.find( "\">", start );
				start += 2;
				std::string::size_type end = resp.data.find( "&nbsp;<small>", start );
				if ( end != std::string::npos ) {
					if ( (BYTE)resp.data.at(end-1) == (BYTE)'.' ) end--; // This will hopefuly remove the "evil" dot ^^. ..Oh, no!. Stop!. Stop it!. .. Okay, you won :(               . (fuck, again!).
					fbu->status = resp.data.substr( start, end - start );
					fbu->status = utils::text::special_expressions_decode( fbu->status ); }
			}
			else fbu->status = "";
		}
		{
			std::string::size_type start = resp.data.find( "http://profile.ak.fbcdn.net" );
			if ( start != std::string::npos ) {
				std::string::size_type end = resp.data.find( "\"", start );
				if ( end != std::string::npos ) {
					fbu->image_url = resp.data.substr( start, end - start );
			} } else
				fbu->image_url = FACEBOOK_DEFAULT_AVATAR_URL;
		}
		// TODO: More items?
	case HTTP_CODE_FOUND:
		return handle_success( "get_profile" );

	case HTTP_CODE_FAKE_LOGGED_OUT:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "get_profile" );

	}
}

bool facebook_client::set_status(const std::string &status_text)
{
	handle_entry( "set_status" );

	std::string data = "post_form_id_source=AsyncRequest&post_form_id=";
	data += ( this->post_form_id_.length( ) ) ? this->post_form_id_ : "0";
	data += "&fb_dtsg=";
	data += ( this->dtsg_.length( ) ) ? this->dtsg_ : "0";
	data += "&target_id=";
	data += this->self_.user_id;

	if ( status_text.length( ) )
	{
		data += "&action=PROFILE_UPDATE&app_id=&hey_kid_im_a_composer=true&display_context=profile&_log_display_context=profile&ajax_log=1&status=";
		data += utils::url::encode( status_text );
		data += "&profile_id=";
		data += this->self_.user_id;
	}
	else
		data += "&clear=1&nctr[_mod]=pagelet_top_bar";

	http::response resp = flap( FACEBOOK_REQUEST_STATUS_SET, &data );

	validate_response(&resp);

	switch ( resp.code )
	{

	case HTTP_CODE_OK:
		return handle_success( "set_status" );

	case HTTP_CODE_FAKE_LOGGED_OUT:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "set_status" );

	}
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_client::save_url(const std::string &url,const std::string &filename)
{
	HANDLE hNetlib = this->parent->m_hNetlibAvatar;
	NETLIBHTTPREQUEST req = {sizeof(req)};
	NETLIBHTTPREQUEST *resp;
	req.requestType = REQUEST_GET;
	req.szUrl = const_cast<char*>(url.c_str());

	resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService( MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(hNetlib), reinterpret_cast<LPARAM>(&req) ));

	if(resp)
	{
		parent->Log( "@@@@@ Saving avatar URL %s to path %s", url.c_str(), filename.c_str() );

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
