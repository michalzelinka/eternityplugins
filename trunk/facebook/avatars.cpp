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

struct update_avatar
{
	update_avatar(HANDLE hContact,const std::string &url) : hContact(hContact),url(url) {}
	HANDLE hContact;
	std::string url;
};

void FacebookProto::UpdateAvatarWorker(void *p)
{
	if(p == 0)
		return;
	std::auto_ptr<update_avatar> data( static_cast<update_avatar*>(p) );
	DBVARIANT dbv;

	if(DBGetContactSettingString(data->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv))
		return;

	std::string new_url = data->url;
	std::string ext = new_url.substr(new_url.rfind('.'));
	if ( new_url == FACEBOOK_DEFAULT_AVATAR_URL )
		new_url = new_url.replace( new_url.rfind( "/q" ), 2, "/d" );
	else
		new_url = new_url.replace( new_url.rfind( "/q" ), 2, "/n" );
	std::string filename = GetAvatarFolder() + '\\' + dbv.pszVal + ext;
	DBFreeVariant(&dbv);

	PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
	ai.hContact = data->hContact;
	ai.format = ext_to_format(ext);
	strncpy(ai.filename,filename.c_str(),MAX_PATH);

	LOG("***** Updating avatar: %s",data->url.c_str());
	WaitForSingleObjectEx(avatar_lock_,INFINITE,true);
	if(CallService(MS_SYSTEM_TERMINATED,0,0))
	{
		LOG("***** Terminating avatar update early: %s",data->url.c_str());
		return;
	}

	if(save_url(this->m_hNetlibAvatar,new_url,filename))
		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_AVATAR,
			ACKRESULT_SUCCESS,&ai,0);
	else
		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_AVATAR,
			ACKRESULT_FAILED, &ai,0);
	ReleaseMutex(avatar_lock_);
	LOG("***** Done avatar: %s",data->url.c_str());
}

void FacebookProto::ProcessAvatar(HANDLE hContact,const std::string* url,bool force)
{
	ForkThread(&FacebookProto::UpdateAvatarWorker, this,
	    new update_avatar(hContact,(*url)));
}

std::string FacebookProto::GetAvatarFolder()
{
	char path[MAX_PATH];
	if(hAvatarFolder_ && FoldersGetCustomPath(hAvatarFolder_,path,sizeof(path), "") == 0)
		return path;
	else
		return def_avatar_folder_;
}
