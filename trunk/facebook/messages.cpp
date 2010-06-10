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

int FacebookProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	CCSDATA ccs = { hContact,PSR_MESSAGE,0,reinterpret_cast<LPARAM>(pre) };
	return CallService(MS_PROTO_RECVMSG,0,reinterpret_cast<LPARAM>(&ccs));
}

struct send_direct
{
	send_direct(HANDLE hContact,const std::string &msg) : hContact(hContact),msg(msg) {}
	HANDLE hContact;
	std::string msg;
};

void FacebookProto::SendMsgWorker(void *p)
{
	if(p == 0)
		return;

	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(data->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
	{
		facy.send_message(dbv.pszVal, data->msg);

		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_MESSAGE,ACKRESULT_SUCCESS,
			(HANDLE)1,0);
		DBFreeVariant(&dbv);
	}

	delete data;
}

int FacebookProto::SendMsg(HANDLE hContact, int flags, const char *msg)
{
	if ( !isOnline( ) )
		return 0;

	if ( DBGetContactSettingWord( hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE ) == ID_STATUS_OFFLINE )
		return 0;

	// TODO: msg comes as Unicode (retyped wchar_t*), why should we convert it as ANSI to UTF-8? o_O
	if ( flags & PREF_UNICODE )
        msg = mir_utf8encode(msg);


	ForkThread(&FacebookProto::SendMsgWorker, this,new send_direct(hContact,msg));

	// TODO: Free msg?

	return 1;
}
