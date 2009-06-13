/*
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
*/

#pragma once

#include "common.h"

class FacebookProto : public PROTO_INTERFACE
{
public:
	FacebookProto(const char *proto_name,const TCHAR *username);
	~FacebookProto( );

	////////////////////////

	virtual	HANDLE   __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr ) { return NULL; };
	virtual	HANDLE   __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent ) { return NULL; };

	virtual	int      __cdecl Authorize( HANDLE hDbEvent ) { return NULL; };
	virtual	int      __cdecl AuthDeny( HANDLE hDbEvent, const char* szReason ) { return NULL; };
	virtual	int      __cdecl AuthRecv( HANDLE hContact, PROTORECVEVENT* ) { return NULL; };
	virtual	int      __cdecl AuthRequest( HANDLE hContact, const char* szMessage ) { return NULL; };

	virtual	HANDLE   __cdecl ChangeInfo( int iInfoType, void* pInfoData ) { return NULL; };

	virtual	HANDLE   __cdecl FileAllow( HANDLE hContact, HANDLE hTransfer, const char* szPath ) { return NULL; };
	virtual	int      __cdecl FileCancel( HANDLE hContact, HANDLE hTransfer ) { return NULL; };
	virtual	int      __cdecl FileDeny( HANDLE hContact, HANDLE hTransfer, const char* szReason ) { return NULL; };
	virtual	int      __cdecl FileResume( HANDLE hTransfer, int* action, const char** szFilename ) { return NULL; };

	virtual	DWORD_PTR __cdecl GetCaps( int type, HANDLE hContact = NULL ) { return NULL; };
	virtual	HICON     __cdecl GetIcon( int iconIndex );
	virtual	int       __cdecl GetInfo( HANDLE hContact, int infoType ) { return NULL; };

	virtual	HANDLE    __cdecl SearchBasic( const char* id ) { return NULL; };
	virtual	HANDLE    __cdecl SearchByEmail( const char* email ) { return NULL; };
	virtual	HANDLE    __cdecl SearchByName( const char* nick, const char* firstName, const char* lastName ) { return NULL; };
	virtual	HWND      __cdecl SearchAdvanced( HWND owner ) { return NULL; };
	virtual	HWND      __cdecl CreateExtendedSearchUI( HWND owner ) { return NULL; };

	virtual	int       __cdecl RecvContacts( HANDLE hContact, PROTORECVEVENT* ) { return NULL; };
	virtual	int       __cdecl RecvFile( HANDLE hContact, PROTORECVFILE* ) { return NULL; };
	virtual	int       __cdecl RecvMsg( HANDLE hContact, PROTORECVEVENT* ) { return NULL; };
	virtual	int       __cdecl RecvUrl( HANDLE hContact, PROTORECVEVENT* ) { return NULL; };

	virtual	int       __cdecl SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList ) { return NULL; };
	virtual	HANDLE    __cdecl SendFile( HANDLE hContact, const char* szDescription, char** ppszFiles ) { return NULL; };
	virtual	int       __cdecl SendMsg( HANDLE hContact, int flags, const char* msg ) { return NULL; };
	virtual	int       __cdecl SendUrl( HANDLE hContact, int flags, const char* url ) { return NULL; };

	virtual	int       __cdecl SetApparentMode( HANDLE hContact, int mode ) { return NULL; };
	virtual	int       __cdecl SetStatus( int iNewStatus ) { return NULL; };

	virtual	HANDLE    __cdecl GetAwayMsg( HANDLE hContact ) { return NULL; };
	virtual	int       __cdecl RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt ) { return NULL; };
	virtual	int       __cdecl SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg ) { return NULL; };
	virtual	int       __cdecl SetAwayMsg( int iStatus, const char* msg ) { return NULL; };

	virtual	int       __cdecl UserIsTyping( HANDLE hContact, int type ) { return NULL; };

	virtual	int       __cdecl OnEvent( PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam ) { return NULL; };

	////////////////////////

	// Services
	int  __cdecl SvcCreateAccMgrUI(WPARAM,LPARAM);

	// Events
	int  __cdecl OnOptionsInit(WPARAM,LPARAM);

};


