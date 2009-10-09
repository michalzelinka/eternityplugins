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

File name      : $URL$
Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

#pragma once

#define FORCE_DISCONNECT true

struct facebook_user
{
	HANDLE handle;

	std::string user_id;
	std::string real_name;

	unsigned int status_id;
	std::string status;
	bool is_idle;

	std::string image_url;

	bool passed;
	bool just_added;

	facebook_user( )
	{
		this->handle = NULL;
		this->user_id = this->real_name = this->status = this->image_url = "";
		this->is_idle = this->passed = this->just_added = false;
		this->status_id = ID_STATUS_OFFLINE;
	}
};

struct facebook_message
{
	std::string user_id;
	std::string message_text;
	time_t time;

	facebook_message( )
	{
		this->user_id = this->message_text = "";
		this->time = 0;
	}

	facebook_message( const facebook_message& msg )
	{
		this->user_id = msg.user_id;
		this->message_text = msg.message_text;
		this->time = msg.time;
	}
};

class facebook_client
{
public:

	////////////////////////////////////////////////////////////

	// Client definition

	facebook_client( )
	{
		username_ = password_ = \
		post_form_id_ = dtsg_ = \
		logout_action_ = chat_channel_num_ = \
		log = "";

		chat_sequence_num_ = error_count_ = 0;

		chat_first_touch_ = false;
	}

	// Parent handle

	FacebookProto*  parent;

	// User data

	facebook_user   self_;

	std::string username_;
	std::string password_;

	std::string post_form_id_;
	std::string dtsg_;
	std::string logout_action_;
	std::string chat_channel_num_;
	unsigned int    chat_sequence_num_;
	bool    chat_first_touch_;

	std::string log;

	// Session, Cookies, Data storage

	std::map< string, string >   cookies;

	char*   get_user_agent( );
	char*   load_cookies( );
	void    store_cookies( NETLIBHTTPHEADER* headers, int headers_count );
	void    clear_cookies( );

	// Connection handling

	unsigned int error_count_;

	bool    validate_response( http::response* );
	bool    handle_success( );
	bool    handle_error( std::string method, bool force = false );
	void __inline increment_error( ) { this->error_count_++; }
	void __inline decrement_error( ) { if ( error_count_ > 0 ) error_count_--; }
	void __inline reset_error( ) { error_count_ = 0; }

	// Login handling

	bool    login( const std::string &username, const std::string &password );
	bool    logout( );

	const std::string & get_username() const;

	// Session handling

	bool    home( );
	bool    reconnect( );

	// Updates handling

	std::map< std::string, facebook_user* > buddies;

	bool    buddy_list( );

	// Messages handling

	bool    channel( );
	bool    send_message( string message_recipient, string message_text );

	// User details handling

	bool    get_profile(facebook_user* fbu);

	// Status handling

	bool    set_status(const std::string &text);

	////////////////////////////////////////////////////////////

	// HTTP communication

	http::response flap( const int request_type, char* request_data = NULL );

	DWORD   choose_security_level( int );
	int     choose_method( int );
	char*   choose_server( int, char* data = NULL );
	char*   choose_action( int, char* data = NULL );
	char*   choose_request_url( int, char* data = NULL );

	NETLIBHTTPHEADER*   get_request_headers( int request_type, int* headers_count );
	void    set_header( NETLIBHTTPHEADER* header, char* header_name, char* header_value );

	// Netlib handle

	HANDLE handle_;

	void set_handle(HANDLE h)
	{
		handle_ = h;
	}
};