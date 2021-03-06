/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka

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

File name      : $HeadURL$
Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

#include "common.h"

void FacebookProto::ProcessBuddyList( void* data )
{
	if ( data == NULL ) return;

	std::string* resp = (std::string*)data;

	if ( isOffline( ) )
		goto exit;

	CODE_BLOCK_TRY

	LOG("***** Starting processing buddy list");

	ScopedLock s(facy.buddies_lock_);

	facebook_json_parser* p = new facebook_json_parser( this );
	p->parse_buddy_list( data, &facy.buddies );
	delete p;

	for ( List::Item< facebook_user >* i = facy.buddies.begin( ); i != NULL; )
	{
		LOG("      Now %s%s: %s", (i->data->status_id==ID_STATUS_ONLINE?"online":"offline"),(i->data->is_idle?" (idle)":""), i->data->real_name.c_str());

		facebook_user* fu;

		if ( i->data->status_id == ID_STATUS_OFFLINE ) {
			fu = new facebook_user(i->data);
			std::string to_delete( i->key );
			i = i->next;
			facy.buddies.erase( to_delete ); }
		else {
			fu = i->data;
			i = i->next; }

		ForkThread(&FacebookProto::UpdateContactWorker, this, (void*)fu);
	}

	LOG("***** Buddy list processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing buddy list: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::ProcessMessages( void* data )
{
	if ( data == NULL ) return;

	std::string* resp = (std::string*)data;

	if (!isOnline())
		goto exit;

	CODE_BLOCK_TRY

	LOG("***** Starting processing messages");

	std::vector< facebook_message* > messages;
	std::vector< facebook_notification* > notifications;

	facebook_json_parser* p = new facebook_json_parser( this );
	p->parse_messages( data, &messages, &notifications );
	delete p;

	for(std::vector<facebook_message*>::size_type i=0; i<messages.size( ); i++)
	{
		if ( messages[i]->user_id != facy.self_.user_id ) {
			LOG("      Got message: %s", messages[i]->message_text.c_str());
			facebook_user fbu;
			fbu.user_id = messages[i]->user_id;

			HANDLE hContact = AddToContactList(&fbu);
			DBWriteContactSettingWord(hContact,m_szModuleName,"Status",ID_STATUS_ONLINE);

			PROTORECVEVENT recv = {};
			CCSDATA ccs = {};

			recv.flags = PREF_UTF;
			recv.szMessage = const_cast<char*>(messages[i]->message_text.c_str());
			recv.timestamp = static_cast<DWORD>(messages[i]->time);

			ccs.hContact = hContact;
			ccs.szProtoService = PSR_MESSAGE;
			ccs.wParam = ID_STATUS_ONLINE;
			ccs.lParam = reinterpret_cast<LPARAM>(&recv);
			CallService(MS_PROTO_CHAINRECV,0,reinterpret_cast<LPARAM>(&ccs));
		}
		delete messages[i];
	}
	messages.clear();

	BYTE notify = getByte( FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE );
	for(std::vector<facebook_notification*>::size_type i=0; i<notifications.size( ); i++)
	{
		if ( notify ) {
			LOG("      Got notification: %s", notifications[i]->text.c_str());
			TCHAR* szTitle = mir_a2t_cp(this->m_szModuleName, CP_UTF8);
			TCHAR* szText = mir_a2t_cp(notifications[i]->text.c_str(), CP_UTF8);
			TCHAR* szUrl = mir_a2t_cp(notifications[i]->link.c_str(), CP_UTF8);
			NotifyEvent( szTitle, szText, ContactIDToHContact(notifications[i]->user_id), FACEBOOK_EVENT_NOTIFICATION, szUrl );
			mir_free( szTitle );
			mir_free( szText );
//			mir_free( szUrl ); // URL is free'd in popup procedure
		}
		delete notifications[i];
	}
	notifications.clear();

	LOG("***** Messages processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing messages: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::ProcessFeeds( void* data )
{
	if ( data == NULL ) return;

	std::string* resp = (std::string*)data;

	if (!isOnline())
		goto exit;

	CODE_BLOCK_TRY

	LOG("***** Starting processing feeds");

	std::vector< facebook_newsfeed* > news;

	std::string::size_type pos = 0;
	UINT limit = 0;

	while ( ( ( pos = resp->find( "\\u003ch6", pos ) ) != std::string::npos ) && ( limit <= 25 ) )
	{
		std::string post_content = resp->substr( pos, resp->find( "\\u003c\\/h6", pos ) - pos );
		std::string rest_content = resp->substr( resp->find( "class=\\\"uiStreamSource\\\"", pos ), resp->find( "<abbr title=", pos ) );

		facebook_newsfeed* nf = new facebook_newsfeed;

		nf->title = utils::text::source_get_value( &post_content, 3, "\\u003ca ", "\\\">", "\\u003c\\/a>" );

		nf->user_id = utils::text::source_get_value( &post_content, 2, "user.php?id=", "\"" );
		nf->text = utils::text::source_get_value( &post_content, 2, "\\u003cspan class=\\\"messageBody\\\">", "\\u003c\\/span" );
		nf->link = utils::text::source_get_value( &rest_content, 2, "href=\\\"", "\\\">" );

		if ( nf->title.length() && nf->text.length() )
		{
			nf->title = utils::text::trim(
				utils::text::remove_html(
					utils::text::slashu_to_utf8(
						utils::text::special_expressions_decode( nf->title ) ) ) );

			nf->text = utils::text::trim(
				utils::text::remove_html(
					utils::text::slashu_to_utf8(
						utils::text::special_expressions_decode( nf->text ) ) ) );
			nf->link = utils::text::special_expressions_decode( nf->link );

			if (nf->text.length() > 420) nf->text = nf->text.substr(0, 420);

			news.push_back( nf );
		}
		else
			delete nf;

		pos++;
		limit++;
	}

	for(std::vector<facebook_newsfeed*>::size_type i=0; i<news.size( ); i++)
	{
		LOG("      Got newsfeed: %s %s", news[i]->title.c_str(), news[i]->text.c_str());
		TCHAR* szTitle = mir_a2t_cp(news[i]->title.c_str(), CP_UTF8);
		TCHAR* szText = mir_a2t_cp(news[i]->text.c_str(), CP_UTF8);
		TCHAR* szUrl = mir_a2t_cp(news[i]->link.c_str(), CP_UTF8);
		NotifyEvent(szTitle,szText,this->ContactIDToHContact(news[i]->user_id),FACEBOOK_EVENT_NEWSFEED, szUrl);
		mir_free(szTitle);
		mir_free(szText);
//		mir_free(szUrl); // URL is free'd in popup procedure
		delete news[i];
	}
	news.clear();

	this->facy.last_feeds_update_ = ::time( NULL );
	setDword( "LastNotificationsUpdate", this->facy.last_feeds_update_ );

	LOG("***** Feeds processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing feeds: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::ProcessAvatar(HANDLE hContact,const std::string* url,bool force)
{
	ForkThread(&FacebookProto::UpdateAvatarWorker, this,
	    new update_avatar(hContact,(*url)));
}
