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

struct facebook_status // TODO: Needed?
{
	std::string text;
	time_t time;
};

struct facebook_user
{
	std::string user_id;
	std::string real_name;
	std::string profile_image_url;
	facebook_status status;
};

struct facebook_message
{
	std::string user_id;
	std::string message_text;
	time_t time;
};

class facebook
{
public:

	facebook( );

	// User data

	std::string username_;
	std::string password_;
	std::string user_id_;
	std::string post_form_id_;
	unsigned int chat_channel_num_;
	unsigned int chat_sequence_num_;

	std::string log;

	// Session, Cookies, Data handling
	std::map< string, string >   cookies;

	char* get_user_agent( );
	char* load_cookies( );
	void store_cookies( NETLIBHTTPHEADER* headers, int headers_count );
	void clear_cookies( );

	// Login handling
	bool login( const std::string &username, const std::string &password, bool test = true ); // TODO: test??
	bool logout( );

	const std::string & get_username() const;

	// Chat session handling
	bool popout( );
	bool reconnect( );
	bool settings( );

	// Users handling

	bool update( );
	bool fetch( );
	//bool get_info(const std::string &name,facebook_user *);
	//bool get_info_by_email(const std::string &email,facebook_user *);
	//std::vector<facebook_user> get_friends();

	//facebook_user add_friend(const std::string &name);
	//void remove_friend(const std::string &name);

	// Status handling

	bool set_status(const std::string &text);
	//std::vector<facebook_user> get_statuses(int count=20,int id=0);

	// Messages handling

	bool send_message( string message_recipient, string message_text );
	bool channel( );

	////////////////////////////////////////////////////////////

	// HTTP communication

	http::response flap( const int request_type, char* request_data = NULL );

	DWORD choose_security_level( int );
	int choose_method( int );
	char* choose_server( int );
	char* choose_action( int );
	char* choose_request_url( int );

	NETLIBHTTPHEADER*   get_request_headers( int request_type, int* headers_count );
	void    set_header( NETLIBHTTPHEADER* header, char* header_name, char* header_value );

	// Netlib handle

	HANDLE handle_;

	void set_handle(HANDLE h)
	{
		handle_ = h;
	}
};