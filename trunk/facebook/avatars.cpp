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

void FacebookProto::UpdateAvatarWorker(void *p)
{
	if(p == NULL) return;

	std::auto_ptr<update_avatar> data( static_cast<update_avatar*>(p) );
	DBVARIANT dbv;

	if(!DBGetContactSettingString(data->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv))
	{
		std::string new_url = data->url;
		std::string ext = new_url.substr(new_url.rfind('.'));
		if ( new_url == FACEBOOK_DEFAULT_AVATAR_URL )
			new_url = new_url.replace( new_url.rfind( "/q" ), 2, "/d" );
		std::string filename = GetAvatarFolder() + '\\' + dbv.pszVal + ext;
		DBFreeVariant(&dbv);

		PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
		ai.hContact = data->hContact;
		ai.format = ext_to_format(ext);
		strncpy(ai.filename,filename.c_str(),MAX_PATH);

		ScopedLock s( avatar_lock_ );
		LOG("***** Updating avatar: %s",data->url.c_str());
		if (CallService(MS_SYSTEM_TERMINATED,0,0))
			LOG("***** Terminating avatar update early: %s",data->url.c_str());
		else
		{
			if(facy.save_url(new_url,filename))
				ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_AVATAR,
					ACKRESULT_SUCCESS,&ai,0);
			else
				ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_AVATAR,
					ACKRESULT_FAILED, &ai,0);
			LOG("***** Done avatar: %s",data->url.c_str());
		}
	}
}

std::string FacebookProto::GetAvatarFolder()
{
	char path[MAX_PATH];
	if(hAvatarFolder_ && FoldersGetCustomPath(hAvatarFolder_,path,sizeof(path), "") == 0)
		return path;
	else
		return def_avatar_folder_;
}

INT_PTR FacebookProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam) return -3;

	DBVARIANT dbv;
	std::string avatar_url;

	if ( !getString( FACEBOOK_KEY_AV_URL,&dbv ) )
	{
		if ( strlen( dbv.pszVal ) == 0 )
			return -2; // No avatar set

		std::string avatar_url = dbv.pszVal;
		DBFreeVariant(&dbv);

		if ( !getString( FACEBOOK_KEY_ID,&dbv ) )
		{
			// Return whether exists or not
			int ret = ( AvatarExists( dbv.pszVal ) ) ? 0 : -1;
			DBFreeVariant(&dbv);
			return ret;
		}
	}
	return -2; // No avatar set
}

bool FacebookProto::AvatarExists(std::string user_id)
{
	std::string base = GetAvatarFolder() + '\\' + user_id;

	for ( BYTE i = 0; i < SIZEOF(extensions); i++ ) {
		std::string file_name = base + extensions[i];
		if (_access(file_name.c_str(), 0))
			return true; } // Avatar file exists, we doesn't need refresh

	return false; // Avatar file doesn't exist, we need refresh
}
