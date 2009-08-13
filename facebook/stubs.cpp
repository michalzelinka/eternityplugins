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

#include "common.h"

HANDLE FacebookProto::AddToListByEvent(int flags,int iContact,HANDLE hDbEvent)
{
	return 0;
};

int FacebookProto::Authorize(HANDLE hContact)
{
	return 0;
}

int FacebookProto::AuthDeny(HANDLE hContact,const char *reason)
{
	return 0;
}

int FacebookProto::AuthRecv(HANDLE hContact,PROTORECVEVENT *)
{
	return 0;
}

int FacebookProto::AuthRequest(HANDLE hContact,const char *message)
{
	return 0;
}

HANDLE FacebookProto::ChangeInfo(int type,void *info_data)
{
	MessageBoxA(0,"ChangeInfo","",0);
	return 0;
}

HANDLE FacebookProto::FileAllow(HANDLE hContact,HANDLE hTransfer,const char *path)
{
	return 0;
}

int FacebookProto::FileCancel(HANDLE hContact,HANDLE hTransfer)
{
	return 0;
}

int FacebookProto::FileDeny(HANDLE hContact,HANDLE hTransfer,const char *reason)
{
	return 0;
}

int FacebookProto::FileResume(HANDLE hTransfer,int *action,const char **filename)
{
	return 0;
}

HANDLE FacebookProto::SearchByName(const char *nick,const char *first_name,const char *last_name)
{
	return 0;
}

HWND FacebookProto::SearchAdvanced(HWND owner)
{
	return 0;
}

HWND FacebookProto::CreateExtendedSearchUI(HWND owner)
{
	return 0;
}

int FacebookProto::RecvContacts(HANDLE hContact,PROTORECVEVENT *)
{
	return 0;
}

int FacebookProto::RecvFile(HANDLE hContact,PROTORECVFILE *)
{
	return 0;
}

int FacebookProto::RecvUrl(HANDLE hContact,PROTORECVEVENT *)
{
	return 0;
}

int FacebookProto::SendContacts(HANDLE hContact,int flags,int nContacts,HANDLE *hContactsList)
{
	return 0;
}

HANDLE FacebookProto::SendFile(HANDLE hContact,const char *desc, char **files)
{
	return 0;
}

int FacebookProto::SendUrl(HANDLE hContact,int flags,const char *url)
{
	return 0;
}

int FacebookProto::SetApparentMode(HANDLE hContact,int mode)
{
	return 0;
}

int FacebookProto::RecvAwayMsg(HANDLE hContact,int mode,PROTORECVEVENT *evt)
{
	return 0;
}

int FacebookProto::SendAwayMsg(HANDLE hContact,HANDLE hProcess,const char *msg)
{
	return 0;
}

int FacebookProto::SetAwayMsg(int status,const char *msg)
{
	return 0;
}

int FacebookProto::UserIsTyping(HANDLE hContact,int type)
{ 
	return 0;
}
