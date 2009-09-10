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

FacebookProto::FacebookProto(const char* proto_name,const TCHAR* username)
{
	m_szProtoName  = mir_strdup( proto_name );
	m_szModuleName = mir_strdup( proto_name );
	m_tszUserName  = mir_tstrdup( username );

	this->facy.parent = this;

	CreateProtoService(m_szModuleName, PS_CREATEACCMGRUI, &FacebookProto::SvcCreateAccMgrUI, this);
	CreateProtoService(m_szModuleName, PS_GETNAME,        &FacebookProto::GetName,           this);
	CreateProtoService(m_szModuleName, PS_GETSTATUS,      &FacebookProto::GetStatus,         this);
	CreateProtoService(m_szModuleName, PS_SETSTATUS,      &FacebookProto::SetStatus,         this);
	CreateProtoService(m_szModuleName, PS_GETMYAWAYMSG,   &FacebookProto::GetMyAwayMsg,      this);
	CreateProtoService(m_szModuleName, PS_SETAWAYMSG,     &FacebookProto::SetMyAwayMsg,      this);
	CreateProtoService(m_szModuleName, PS_GETMYAVATAR,    &FacebookProto::GetMyAvatar,       this);

	HookProtoEvent(ME_DB_CONTACT_DELETED,       &FacebookProto::OnContactDeleted,     this);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &FacebookProto::OnBuildStatusMenu,    this);
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU,&FacebookProto::OnBuildContactMenu,   this);
	HookProtoEvent(ME_OPT_INITIALISE,           &FacebookProto::OnOptionsInit,        this);

	char *profile = Utils_ReplaceVars("%miranda_avatarcache%");
	def_avatar_folder_ = std::string(profile)+"\\"+m_szModuleName;
	mir_free(profile);
	hAvatarFolder_ = FoldersRegisterCustomPath(m_szModuleName,"Avatars",
	    def_avatar_folder_.c_str());

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses( ID_STATUS_OFFLINE );
}

FacebookProto::~FacebookProto( )
{
	if ( m_hNetlibUser )
		Netlib_CloseHandle( m_hNetlibUser );
	if ( m_hNetlibAvatar )
		Netlib_CloseHandle( m_hNetlibAvatar );

	KillThreads( );

	mir_free( m_tszUserName );
	mir_free( m_szModuleName );
	mir_free( m_szProtoName );
}

//////////////////////////////////////////////////////////////////////////////

DWORD FacebookProto::GetCaps( int type, HANDLE hContact )
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSG; // | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL; // TODO?
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_SUPPORTIDLE | PF4_IMSENDUTF | PF4_AVATARS; // | PF4_SUPPORTTYPING; // TODO?
	case PFLAG_MAXLENOFMESSAGE:
		return FACEBOOK_MESSAGE_LIMIT;
	case PFLAG_UNIQUEIDTEXT:
		return (int) "Facebook ID";
	case PFLAG_UNIQUEIDSETTING:
		return (int) FACEBOOK_KEY_ID;
	}
	return 0;
}

HICON FacebookProto::GetIcon(int index)
{
	if(LOWORD(index) == PLI_PROTOCOL)
	{
		HICON ico = (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)"Facebook_facebook");
		return CopyIcon(ico);
	}
	else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::SetStatus( int new_status )
{
	int old_status = m_iStatus;
	if ( new_status == m_iStatus )
		return 0;

	m_iDesiredStatus = new_status;

	if ( new_status == ID_STATUS_ONLINE )
	{
		if ( old_status == ID_STATUS_CONNECTING )
			return 0;

		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck( m_szModuleName, 0, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
			( HANDLE )old_status, m_iStatus );

		ForkThread( &FacebookProto::SignOn, this );
	}
	else if ( new_status == ID_STATUS_OFFLINE )
	{
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck( m_szModuleName, 0, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
			( HANDLE ) old_status, m_iStatus );

		ForkThread( &FacebookProto::SignOff, this );
	}

	// Routing statuses not supported by Facebook
	else if ( new_status == ID_STATUS_INVISIBLE )
		SetStatus( ID_STATUS_OFFLINE );
	else
		SetStatus( ID_STATUS_ONLINE );

	return 0;
}

int FacebookProto::SetAwayMsg( int status,const char *msg )
{
	if ( isOnline() && getByte( FACEBOOK_KEY_SET_MIRANDA_STATUS, 0 ) )
		ForkThread(&FacebookProto::SendMindWorker, this, (void*)msg);
	return 0;
}

void FacebookProto::SendMindWorker(void * data)
{
	const std::string new_status = ( char* )data;
	facy.set_status( new_status );
	// TODO: data leak?
	// TODO:RE:
	utils::mem::detract( ( void** )&data );
}

//////////////////////////////////////////////////////////////////////////////
// SERVICES

int FacebookProto::GetStatus( WPARAM wParam, LPARAM lParam )
{
	return m_iStatus;
}

int FacebookProto::SetStatus( WPARAM wParam, LPARAM lParam )
{
	return SetStatus( (int)wParam );
}

int FacebookProto::GetMyAwayMsg( WPARAM wParam, LPARAM lParam )
{
	DBVARIANT dbv = { DBVT_TCHAR };
	if ( !getTString( "StatusMsg", &dbv ) && lstrlen( dbv.ptszVal ) != 0 )
	{
		int res = ( int )mir_wstrdup( dbv.ptszVal );
		DBFreeVariant( &dbv );
		return res;
	}
	else
		return 0;
}

int FacebookProto::SetMyAwayMsg( WPARAM wParam, LPARAM lParam )
{
	return SetAwayMsg( (int)wParam, (const char*)lParam );
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::OnEvent(PROTOEVENTTYPE event,WPARAM wParam,LPARAM lParam)
{
	switch(event)
	{
	case EV_PROTO_ONLOAD:    return OnModulesLoaded(wParam,lParam);
	case EV_PROTO_ONEXIT:    return OnPreShutdown  (wParam,lParam);
	case EV_PROTO_ONOPTIONS: return OnOptionsInit  (wParam,lParam);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

int FacebookProto::GetName( WPARAM wParam, LPARAM lParam )
{
	lstrcpynA(reinterpret_cast<char*>(lParam),m_szProtoName,wParam);
	return 0;
}

int FacebookProto::SvcCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (int)CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_FACEBOOKACCOUNT), 
		 (HWND)lParam, FBAccountProc, (LPARAM)this );
}

int FacebookProto::OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	TCHAR descr[512];
    NETLIBUSER nlu = {sizeof(nlu)};
    nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
    nlu.szSettingsModule = m_szModuleName;

	// Create standard network connection
	mir_sntprintf(descr,SIZEOF(descr),TranslateT("%s server connection"),m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
	if(m_hNetlibUser == 0)
		MessageBoxA(0,"Unable to get Netlib connection for Facebook","",0);

	// Create avatar network connection (TODO: probably remove this)
	char module[512];
	mir_snprintf(module,SIZEOF(module),"%sAv",m_szModuleName);
	nlu.szSettingsModule = module;
	mir_sntprintf(descr,SIZEOF(descr),TranslateT("%s avatar connection"),m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	m_hNetlibAvatar = (HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
	if(m_hNetlibAvatar == 0)
		MessageBoxA(0,"Unable to get avatar Netlib connection for Facebook","",0);

	facy.set_handle(m_hNetlibUser);

	return 0;
}

int FacebookProto::OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	SetStatus( ID_STATUS_OFFLINE );
	Netlib_Shutdown( m_hNetlibAvatar );
	Netlib_Shutdown( m_hNetlibUser );
	return 0;
}

int FacebookProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {sizeof(odp)};
	odp.position    = 271828;
	odp.hInstance   = g_hInstance;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab     = LPGENT("Basic");
    odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc  = FBOptionsProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

//// TODO: Uncommend when popups working
//	if(ServiceExists(MS_POPUP_ADDPOPUPT))
//	{
//		odp.ptszTab     = LPGENT("Popups");
//		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_POPUPS);
//		odp.pfnDlgProc  = FBPopupsProc;
//		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
//	}

	return 0;
}

int FacebookProto::OnBuildStatusMenu(WPARAM wParam,LPARAM lParam)
{
	HGENMENU hRoot = pcli->pfnGetProtocolMenu(m_szModuleName);
	if (hRoot == NULL)
		return 0;

	CLISTMENUITEM mi = {sizeof(mi)};

	char text[200];
	strcpy(text,m_szModuleName);
	char *tDest = text+strlen(text);
	mi.pszService = text;

	mi.hParentMenu = hRoot;
	mi.flags = CMIF_ICONFROMICOLIB|CMIF_ROOTHANDLE|CMIF_HIDDEN; // TODO: CMIF_TCHAR
	mi.position = 1001;

	m_hMenuRoot = reinterpret_cast<HGENMENU>( CallService(
		MS_CLIST_ADDSTATUSMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );

	CreateProtoService(m_szModuleName,"/Mind",&FacebookProto::OnMind,this);
	strcpy(tDest,"/Mind");
	mi.pszName = LPGEN("Mind...");
	mi.popupPosition = 200001;
	mi.icolibItem = GetIconHandle("mind");
	m_hStatusMind = reinterpret_cast<HGENMENU>( CallService(
		MS_CLIST_ADDSTATUSMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );

	return 0;
}

void FacebookProto::ToggleStatusMenuItems( BOOL bEnable )
{
	CLISTMENUITEM clmi = { 0 };
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS | (( bEnable ) ? 0 : CMIF_HIDDEN);

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hMenuRoot,   ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hStatusMind, ( LPARAM )&clmi );
}

int FacebookProto::OnMind(WPARAM,LPARAM)
{
	if ( !isOnline( ) ) // TODO: remove this, toggle status menu item inside server negotiation proc
		return TRUE;

	HWND hDlg = CreateDialogParam( g_hInstance, MAKEINTRESOURCE( IDD_MIND ),
		 ( HWND )0, FBMindProc, reinterpret_cast<LPARAM>( this ) );
	ShowWindow( hDlg, SW_SHOW );
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::Test( WPARAM wparam, LPARAM lparam )
{
	return FALSE;
}

int FacebookProto::LOG(const char *fmt,...)
{
	if ( getByte( FACEBOOK_KEY_ENABLE_LOGGING, 0 ) != TRUE )
		return EXIT_SUCCESS;

	va_list va;
	char text[65535];
	ScopedLock s(log_lock_);

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	return utils::debug::log( m_szModuleName, text );
}
