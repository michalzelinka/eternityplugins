/* ======================================================================== 78

                              Custom Status List
                              __________________

  Custom Status List plugin for Miranda-IM (www.miranda-im.org)
  Follower of Custom Status History List by HANAX
  Copyright © 2006-2008 HANAX
  Copyright © 2007-2009 jarvis

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  ============================================================================

  File name      : $URL$
  Revision       : $Rev$
  Last change on : $Date$
  Last change by : $Author$

  ============================================================================  


  DESCRIPTION:

  Offers List of your Custom Statuses.

  ============================================================================  


  HISTORY:

  TODO
  ----
  - more and more code revisioning :)
  - all TODOs listed in this source x)
  - feature requests maybe :)
  ! importing
  ! undo
  ! settings changes

  MAYBE-TODO
  ----------
  - advanced duplicities detection (between favs/regular)
  
  NOT-TODO
  --------
  - setting Away/NA/DND/Occ - there are already many status managers :)
  - watching statuses set by x-status menu

  0.1.0.3 - What's your favourite?
  -------
  - Bugfixes

  0.1.0.1 - After storm
  -------
  - Favourites
  - Virtualized structure
  - Code refactorization
  - Duplicities detection (partial)
  - Saving window position

  0.0.0.20 - calm before the storm Ox)
  --------
  - some corrections of working with strings (ANSI/Unicode <-> TCHAR)
  - simplified loading of list items :)
  - the Options button replaced by the Undo button

  0.0.0.19 - enjoy multi'ing, baby :)
  --------
  - status is changing on all ICQ based accounts/protos
  - when dialog is opened, and user tryes to open another one, forgotten dialog is brought to front instead
  - menu placement optimalization
  - removed old nasty GOTO because of double-clicking the list x)
  - active checking of input data length during editing custom status
  - optimalizations in translations
  - support for Variables plugin
  - other enhancements related to the possibility of using multiple ICQ accounts

  0.0.0.18 - fixing trip :)
  --------
  - icon selector shows only icons, that are supported by installed ICQ plugin
  - fixed memory leak and possible crash when modifying status and then importing 0 DB entries (helpItem not nulled)
  - merged Add/Modify processes together :)
  - Unicode build, finally :)

  0.0.0.17 - I become to be useful :)
  --------
  - CSList menu item can be placed elsewhere, also instead of Xstatus menu (currently ICQ eternity/PlusPlus++ mod only)
  - importing statuses from database (ICQ key)
  - list sorting (Procedure + calls)
  - dialog openable only once :)
  - new icons by inducti0n :)
  - no more infinite width when adding x-message
  - new Options dialog

  0.0.0.16 - rework
  --------
  - code redesigned
  - fix for Windows Vista crash (free) and ANSI Windows crash (init extended controls)
  - preimplementation for new features that will come (favourites, ...)
  
  0.0.0.15 - make me nice
  --------
  - stage 1 of code revision
  - Options dialog (in development) :)
  - IcoLib group renamed to CSLIST_MODULE_LONG_NAME
  - status titles in Add/Modify ComboBox were strangely corrected x)
  - possibility to turn off "Release Notes" dialog

  0.0.0.14 - will you use me? :)
  --------
  - new lines in message input creatable via Enter
  - NULLED XSTATUS now available :)
  
  0.0.0.13 - oooops x))
  --------
  - hotfix for Miranda cores < 0.7
    - CSList appears in Main Menu
    - fixed icons not loaded when IcoLib is missing
  
  0.0.0.12 - goin' my way now x)
  --------
  - DB saving method/format slightly modified
    [+] multi-line messages now working
    [-] LOST compatibility with HANAX's plugin (but is auto-imported if exists)
    NOTE: I think "importing-only" of older releases will stay,
          I have many Feature Requests that would need changing
          DB format anyway, so this problem looks solved x)
  - "Release notes" dialog on first run
  - Updater support
  
  0.0.0.11 - will this work? x)
  --------
  - some similar procedures merged together :P 
  - another procedures more generalized 8) 
  - separated Add/Modify dialog finally in use :)
    [+] solves some bahaviour problems x)
  
  0.0.0.10 - ohhh, basics ]:)
  --------
  - icons support - complete xstatus service :) 
    [+] no more static angry ;) :D
  - iconized buttons - supporting IcoLib :)
  - better method for DB entry format - old was little.."dizzy" x) :D
  - TEMPORARILY DISABLED modifying of items - needs some fixes x))
  
  0.0.0.9 - working a little? O:)
  -------
  - enhanced List Control behaviour (selecting, deselecting, editation, adding)
  - LOADING from + SAVING to DB!
    [+] my parser is also compatible with HANAX plugin's DB entries ;)
  
  0.0.0.8 - behaviour testing :o)
  -------
  - setting status by double-clicking list item
  - much better procedures, debugging, better exceptions, dialog behaviour
  
  0.0.0.7 - crash testing :o)
  -------
  - basic functions coding
  
  0.0.0.6 - resource testing
  -------
  - basic resources modeling


// ======================================================================== */

#include "cslist.h"
#include "strpos.h"
#include "legacy.h"


// ====[ MAIN ]===============================================================

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	CSList::handle = hinstDLL;
	return TRUE;
}


// ====[ PLUGIN INFO ]========================================================

extern "C" __declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion )
{
	CSList::dwMirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}

extern "C" __declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion )
{
	// check if core is equal or newer than 0.4.0.3
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0, 4, 0, 3 ) )
		return NULL;

	CSList::dwMirandaVersion = mirandaVersion;
	return &pluginInfo;
}


// ====[ INTERFACES ]=========================================================

extern "C" __declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void )
{
	return interfaces;
}


// ====[ LOADER ]=============================================================

extern "C" __declspec( dllexport ) int Load( PLUGINLINK *link )
{
	// link plugin
	pluginLink = link;

	// support for ComboBoxEx
	{
		INITCOMMONCONTROLSEX icc;
		icc.dwSize = sizeof( icc );
		icc.dwICC = ICC_USEREX_CLASSES;
		InitCommonControlsEx( &icc );
	}

	// set global variables
	// weneed 0.8.0.9 core which introduced accounts support for protocols
	CSList::bAccountsSupported = ( CSList::dwMirandaVersion >= PLUGIN_MAKE_VERSION( 0, 8, 0, 9 ) );

	// are we running under Unicode core?
	{
		char szVer[MAX_PATH];

		CallService( MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, ( LPARAM )szVer );
		_strlwr( szVer ); // make sure it is lowercase

		CSList::bUnicodeCore = ( strstr( szVer, "unicode" ) != NULL );
	}

	// TODO: Alerts with wrong combination of ANSI/Unicode Windows/core/plugin

	// create handler
	cslist = new CSList( );

	return FALSE;
}


// ====[ UNLOADER ]===========================================================

extern "C" __declspec( dllexport ) int Unload( void )
{
	delete cslist;

	return FALSE;
}


// ====[ FUN ]================================================================


CSList::CSList( )
{
	this->mainWindow = NULL;
	this->initIcoLib( );
	this->hHookOnPluginsLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, this->postLoading );
	this->hHookOnOptionsInit = HookEvent( ME_OPT_INITIALISE, this->initOptions );
	this->hHookOnDBSettingChanged = HookEvent( ME_DB_CONTACT_SETTINGCHANGED, this->respondDBChange );
	if ( ServiceExists( MS_CLIST_ADDSTATUSMENUITEM ) )
		this->hHookOnStatusMenuBuild = HookEvent( ME_CLIST_PREBUILDSTATUSMENU, this->createMenuItems );
	else
		this->hHookOnStatusMenuBuild = NULL && this->createMenuItems( NULL, NULL );
	this->hHookOnStatusMenuBuild = NULL;
	this->hServiceShowList = CreateServiceFunction( MS_CSLIST_SHOWLIST, this->showList );
	this->hMainMenuItem = this->hToolbarIcon = NULL;
}


CSList::~CSList( )
{
	// unhook events, destroy functions
	UnhookEvent( this->hHookOnStatusMenuBuild );
	UnhookEvent( this->hHookOnPluginsLoaded );
	UnhookEvent( this->hHookOnOptionsInit );
	UnhookEvent( this->hHookOnDBSettingChanged );
	DestroyServiceFunction( this->hServiceShowList );

	// clear IcoLib
	this->hToolbarIcon = ( HANDLE )CallService( MS_SKIN2_REMOVEICON, 0, ( LPARAM )"CSList_TB_Icon" );
	for ( int i = 0; i < SIZEOF( forms ); i++ )
	{
		char iconName[64];
		mir_snprintf( iconName, sizeof( iconName ), "%s_%s", PLUGIN_SHORT_NAME, forms[i].pszIconIcoLib );
		CallService( MS_SKIN2_REMOVEICON, 0, ( LPARAM )iconName );
	}
}


int CSList::postLoading( WPARAM wparam, LPARAM lparam )
{
	cslist->registerUpdater( );
	cslist->registerHotkeys( );
	cslist->registerToolbar( );
	cslist->initIcoLib( ); // for < 0.7, but probably useless though :/
	ForkThread( &CSList::showReleaseNotes, NULL );
	return FALSE;
}


void CSList::registerUpdater( )
{
	if ( ! ServiceExists( MS_UPDATE_REGISTER ) )
		return;

	static Update upd = { 0 };
	static char szCurrentVersion[30];
	static char *szVersionUrl = PLUGIN_UPD_VERURL;
	static char *szUpdateUrl = PLUGIN_UPD_UPDURL;
	static char *szFLVersionUrl = PLUGIN_UPD_FLVERURL;
	static char *szFLUpdateurl = PLUGIN_UPD_FLUPDURL;
	static char *szPrefix = PLUGIN_UPD_SZPREFIX;

	upd.cbSize = sizeof( upd );
	upd.szComponentName = pluginInfo.shortName;
	if ( this->dwMirandaVersion > PLUGIN_MAKE_VERSION( 0, 7, 0, 0 ) )
		upd.pbVersion = ( BYTE * )CreateVersionStringPluginEx( ( PLUGININFOEX* )&pluginInfoEx, szCurrentVersion );
	else
		upd.pbVersion = ( BYTE * )CreateVersionStringPlugin( ( PLUGININFO* )&pluginInfo, szCurrentVersion );
	upd.cpbVersion = strlen( ( char * )upd.pbVersion );
	upd.pbVersionPrefix = ( BYTE * )PLUGIN_UPD_SZPREFIX;
	upd.cpbVersionPrefix = strlen( ( char * )upd.pbVersionPrefix );
	upd.szBetaUpdateURL = szUpdateUrl;
	upd.szBetaVersionURL = szVersionUrl;
	upd.pbVersion = ( BYTE* )&szCurrentVersion;
	upd.cpbVersion = lstrlenA( szCurrentVersion );
	upd.pbBetaVersionPrefix = ( BYTE * )szPrefix;
	upd.cpbBetaVersionPrefix = strlen( ( char * )upd.pbBetaVersionPrefix );
	CallService( MS_UPDATE_REGISTER, 0, ( LPARAM )&upd );
}


void CSList::registerHotkeys( )
{
	// 0.8+ core hotkeys service
	if ( ServiceExists( MS_HOTKEY_REGISTER ) )
	{
		HOTKEYDESC hotkey = { 0 };
		hotkey.cbSize = sizeof( hotkey );
		hotkey.pszName = MS_CSLIST_SHOWLIST;
		hotkey.pszDescription = LPGEN( "Show Custom Status List" );
		hotkey.pszSection = "Status";
		hotkey.pszService = MS_CSLIST_SHOWLIST;
		hotkey.DefHotKey = MAKEWORD( 'L', HOTKEYF_CONTROL | HOTKEYF_SHIFT );
		CallService( MS_HOTKEY_REGISTER, 0, ( LPARAM )&hotkey );
	}
}


void CSList::registerToolbar( )
{
	// 0.7+ clist modern toolbar
	if ( ServiceExists( MS_TB_ADDBUTTON ) )
	{
		TBButton button = { 0 };
		SKINICONDESC sid = { 0 };
		TCHAR szFile[MAX_PATH];
		GetModuleFileName( this->handle, szFile, MAX_PATH );

		sid.cbSize = sizeof( sid );
		sid.flags = SIDF_ALL_TCHAR;
		sid.ptszSection = TranslateT( LPGEN( "ToolBar" ) );
		sid.ptszDefaultFile = ( TCHAR* )szFile;
		sid.cx = sid.cy = 16;

		sid.pszName = "CSList_TB_Icon";
		sid.ptszDescription = TranslateT( PLUGIN_LONG_NAME );
		sid.iDefaultIndex = -IDI_CSLIST;
		this->hToolbarIcon = ( HANDLE )CallService( MS_SKIN2_ADDICON, 0, ( LPARAM )&sid );

		button.cbSize = sizeof( button );
		button.pszButtonID = MS_CSLIST_SHOWLIST;
		button.pszTooltipUp = button.pszTooltipDn = button.pszButtonName = Translate( PLUGIN_LONG_NAME );
		button.pszServiceName = MS_CSLIST_SHOWLIST;
		button.defPos = 200;
		button.tbbFlags = TBBF_SHOWTOOLTIP | TBBF_VISIBLE;
		button.hPrimaryIconHandle = this->hToolbarIcon;
		button.hSecondaryIconHandle = button.hPrimaryIconHandle;
		CallService( MS_TB_ADDBUTTON, 0, ( LPARAM )&button );
	}
}


int CSList::initOptions( WPARAM wparam, LPARAM lparam )
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof( odp );
	odp.position = 955000000;
	odp.hInstance = CSList::handle;
	odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPTIONS );
	odp.ptszTitle = TranslateT( PLUGIN_LONG_NAME );
	odp.pfnDlgProc = ( DLGPROC )CSOptionsProc;
	odp.ptszGroup = TranslateT( "Status" );
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	CallService( MS_OPT_ADDPAGE, wparam, ( LPARAM )&odp );
	return FALSE;
}


int CSList::respondDBChange( WPARAM wparam, LPARAM lparam )
{
	HANDLE hContact = ( HANDLE )wparam;
	DBCONTACTWRITESETTING *cws = ( DBCONTACTWRITESETTING* )lparam;

	// if user changes his UIN on any account
	if ( hContact == NULL && !lstrcmpA( cws->szSetting, "UIN" ) )
	{
		cslist->rebuildMenuItems( );
		return 0;
	}

	// own setting or not a status-related setting
	else if ( hContact == NULL || lstrcmpA( cws->szSetting, "Status" ) )
		return 0;

	DWORD dwEasteredUin = getDword( "EasterEggedUIN", 0x0 );
	if ( dwEasteredUin == 0x0 ) // no spyed UIN found
		return 0;

	DBVARIANT dbv = { 0 };
	DBGetContactSettingString( hContact, "Protocol", "p", &dbv );

	DWORD dwUin = DBGetContactSettingDword( hContact, dbv.pszVal, "UIN", 0x0 );

	if ( dwUin != 0x0 ) // if this is an ICQ user
		if ( dwUin == dwEasteredUin ) // if spyed UIN is equivalent to this contact
			if ( DBGetContactSettingWord( hContact, "UserOnline", "OldStatus", 0 ) == ID_STATUS_OFFLINE )
				if ( cws->value.wVal > ID_STATUS_OFFLINE )
					cslist->setStatus( IDCANCEL, NULL );

	return FALSE;
}


int CSList::createMenuItems( WPARAM wparam, LPARAM lparam )
{
	int icqProtoCount = 0;

	ForAllProtocols( countICQProtos, &icqProtoCount );

	if ( icqProtoCount == 0 )
		return FALSE;

	CallService( MS_CLIST_REMOVEMAINMENUITEM, ( WPARAM )hMainMenuItem, 0 );

	BOOL service = ServiceExists( MS_CLIST_ADDSTATUSMENUITEM );

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof( mi );
	mi.flags = CMIF_TCHAR;

	if ( ServiceExists( MS_SKIN2_GETICON ) )
		mi.icolibItem = ( HANDLE )LoadIconExEx( "icon", IDI_CSLIST );
	else
		mi.hIcon = LoadIconExEx( "icon", IDI_CSLIST );

	mi.pszService = MS_CSLIST_SHOWLIST;

	BYTE placement = ( service ) ? getByte( "Placement", DEFAULT_PLUGIN_MENU_PLACEMENT ) : MENU_PLACEMENT_MAIN;
	
	switch ( placement )
	{

	case MENU_PLACEMENT_MAIN:
		mi.position = 2000040000;
		mi.ptszName = LPGENT( PLUGIN_LONG_NAME );
		hMainMenuItem = ( HANDLE )CallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
		break;

	case MENU_PLACEMENT_ICQ:
		if ( getByte( "HideCustomStatusUI", DEFAULT_PLUGIN_HIDE_CUSTOM_STATUS_UI ) )
		{
			mi.ptszName = TranslateT( LPGEN( "Custom status" ) );
			mi.position = 00000001; // instead of ICQ Custom Status UI
		}
		else
		{
			mi.ptszName = TranslateT( LPGEN( PLUGIN_LONG_NAME ) );
			mi.position = 2000040000; // default // TODO: smaller value?
		}

		ForAllProtocols( addProtoStatusMenuItem, ( void* )&mi );
		break;

	case MENU_PLACEMENT_GLOBAL_TOP:
		mi.ptszName = TranslateT( LPGEN( PLUGIN_LONG_NAME ) );
		mi.position = -0x7fffffff;
		CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );
		break;

	case MENU_PLACEMENT_GLOBAL_BOTTOM:
		mi.ptszName = TranslateT( LPGEN( PLUGIN_LONG_NAME ) );
		mi.position = 2000040000;
		CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );
		break;

	//case CSLIST_MENU_PLACEMENT_TRAY:
	//	mi.ptszName = TranslateT( LPGEN( PLUGIN_LONG_NAME ) );
	//	CallService( MS_CLIST_ADDTRAYMENUITEM, 0, ( LPARAM )&mi );
	//	break;

	}

	toggleCustomStatusUI( ( ( placement == MENU_PLACEMENT_MAIN || placement == MENU_PLACEMENT_ICQ ) &&
		getByte( "HideCustomStatusUI", DEFAULT_PLUGIN_HIDE_CUSTOM_STATUS_UI ) ) ? 0 : 1 );

	return FALSE;
}


void CSList::rebuildMenuItems( )
{
	CLIST_INTERFACE* pcli;
	pcli = ( CLIST_INTERFACE* )CallService( MS_CLIST_RETRIEVE_INTERFACE, 0, 0 );
	if ( pcli && pcli->version > 4 )
		pcli->pfnReloadProtoMenus( );
}


void CSList::setStatus( WORD code, StatusItem* item )
{
	if ( code == IDCLOSE )
		return;

	int statusToSet = -1;
	ICQ_CUSTOM_STATUS ics = { 0 };
	ics.cbSize = sizeof( ICQ_CUSTOM_STATUS );
	ics.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_TCHAR;

	if ( code == IDCANCEL )
	{
		statusToSet = 0;
		ics.ptszName = TEXT( "" );
		ics.ptszMessage = TEXT( "" );
	}
	else if ( code == IDOK && item != NULL )
	{
		statusToSet = item->iIcon + 1;
		ics.ptszName = variables_parsedup( item->tszTitle, NULL, NULL );
		ics.ptszMessage = variables_parsedup( item->tszMessage, NULL, NULL );
	}
	else return;

	ics.status = &statusToSet;

	ForAllProtocols( setStatusOnAllProtos, ( void* )&ics );
}


void CSList::toggleCustomStatusUI( BOOL enabled )
{
	WPARAM toggleUI = enabled ? 1 : 0;
	ICQ_CUSTOM_STATUS ics = { 0 };
	ics.cbSize = sizeof( ICQ_CUSTOM_STATUS );
	ics.flags = CSSF_DISABLE_MENU;
	ics.wParam = &toggleUI;

	ForAllProtocols( toggleCustomStatusUIOnAllProtos, ( void* )&ics );
}


void CSList::showReleaseNotes( void* arg )
{
	WORD lastReleaseNotes = getWord( "LastReleaseNotes", DEFAULT_RELNOTES_VERSION );
	BYTE show = getByte( "ShowReleaseNotes", DEFAULT_PLUGIN_SHOW_RELNOTES );
	if ( lastReleaseNotes != PLUGIN_RELNOTES && show )
	{
		setWord( "LastReleaseNotes", PLUGIN_RELNOTES );
		DialogBox( CSList::handle, MAKEINTRESOURCE( IDD_RELEASENOTES ), NULL, ( DLGPROC )CSRNWindowProc );
	}
}


void CSList::initIcoLib( )
{
	if ( !ServiceExists( MS_SKIN2_ADDICON ) )
		return;

	// init icons of buttons
	TCHAR tszFile[MAX_PATH];
	GetModuleFileName( CSList::handle, tszFile, MAX_PATH );

	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof( SKINICONDESC );
	sid.ptszDefaultFile = tszFile;
	sid.flags = SIDF_ALL_TCHAR;
	sid.cx = sid.cy = 16;
	sid.ptszSection = TranslateT( PLUGIN_LONG_NAME );

	for ( int i = 0; i < SIZEOF( forms ); i++ )
	{
		char szSettingName[64];
		mir_snprintf( szSettingName,
		              sizeof( szSettingName ),
		              "%s_%s",
		              PLUGIN_SHORT_NAME,
		              forms[i].pszIconIcoLib );

		sid.pszName = szSettingName;
		sid.ptszDescription = TranslateTS( forms[i].ptszDescr );
		sid.iDefaultIndex = -forms[i].iconNoIcoLib;
		forms[i].hIcoLibItem = ( HANDLE )CallService( MS_SKIN2_ADDICON, 0, ( LPARAM )&sid );
	}
}


INT_PTR __cdecl CSList::showList( WPARAM wparam, LPARAM lparam )
{
	if ( cslist->mainWindow == NULL )
	{
		cslist->mainWindow = new CSWindow( );
		ForkThread( &cslist->mainWindow->showWindow, cslist->mainWindow );
	}
	else
	{
		MessageBoxA( NULL, "Already opened!", "CSList", MB_OK );
		cslist->mainWindow->setForeground( );
	}

	return FALSE;
}


void CSList::closeList( HWND hwnd )
{
	ForkThread( &cslist->mainWindow->closeWindow, hwnd );
	delete cslist->mainWindow;
	cslist->mainWindow = NULL;
}


void CSList::ForAllProtocols( pForAllProtosFunc pFunc, void* arg )
{
	int protoCount;
	PROTOCOLDESCRIPTOR** pdesc;
	CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );

	for ( int i = 0; i < protoCount; i++ )
	{
		// legacy-compatible protocol check, unfortunately the only one :/
		if ( DBGetContactSettingDword( NULL, pdesc[i]->szName, "UIN", 0 ) == 0 )
			continue;

		pFunc( pdesc[i]->szName , arg );
	}
}


void CSList::addProtoStatusMenuItem( char* protoName, void* arg )
{
	CLISTMENUITEM* mi = ( CLISTMENUITEM* )arg;

#ifdef UNICODE
	TCHAR tProtoName[MAXMODULELABELLENGTH] = TEXT( "" );
	MultiByteToWideChar( CP_ACP, 0,
	                     protoName, lstrlenA( protoName ),
	                     tProtoName, MAXMODULELABELLENGTH );
	mi->ptszPopupName = tProtoName;
#else
	mi->ptszPopupName = protoName;
#endif

	CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )mi );
}


void CSList::countICQProtos( char* protoName, void* arg )
{
	int* protosEnabled = ( int* )arg;
	if ( DBGetContactSettingDword( NULL, protoName, "UIN", 0 ) )
		*protosEnabled = ( *protosEnabled )++;
}


void CSList::countPlusModProtos( char* protoName, void* arg )
{
	int* protosExtraEnabled = ( int* )arg;
	if ( DBGetContactSettingDword( NULL, protoName, "NonStandartXstatus", 0 ) ||
	     DBGetContactSettingDword( NULL, protoName, "NonStandardXstatus", 0 ) )
		*protosExtraEnabled = ( *protosExtraEnabled )++;
}


void CSList::getFirstProto( char* protoName, void* arg )
{
	if ( DBGetContactSettingDword( NULL, protoName, "UIN", 0 ) != 0 )
	{
		char* firstProtoName = ( char* )arg;
		if ( !lstrlenA( firstProtoName ) )
		{
			lstrcpyA( firstProtoName, protoName );
		}
	}
}


void CSList::setStatusOnAllProtos( char* protoName, void* arg )
{
	ICQ_CUSTOM_STATUS* ics = ( ICQ_CUSTOM_STATUS* )arg;
	char protoService[64];
	lstrcpyA( protoService, protoName );
	lstrcatA( protoService, PS_ICQ_SETCUSTOMSTATUSEX );
	CallService( protoService, 0, ( LPARAM )ics ); // f**king function, 4 hours of thinking xDD
}


void CSList::toggleCustomStatusUIOnAllProtos( char* protoName, void* arg )
{
	char protoService[64];
	ICQ_CUSTOM_STATUS* ics = ( ICQ_CUSTOM_STATUS* )arg;
	lstrcpyA( protoService, protoName );
	lstrcatA( protoService, PS_ICQ_SETCUSTOMSTATUSEX );
	CallService( protoService, 0, ( LPARAM )ics );
}


void CSList::importCustomStatusUIStatusesFromAllProtos( char* protoName, void* arg )
{
	int* result = ( int* )arg;

	for ( int i = 1; i <= 37; i++ )
	{
		DBVARIANT dbv = { 0 };
		char bufTitle[32], bufMessage[32];
		StatusItem* csi = new StatusItem( );

		csi->iIcon = i - 1;

		mir_snprintf( bufTitle, 32, "XStatus%luName", i );
		DBGetContactSettingTString( NULL, protoName, bufTitle, &dbv );
		lstrcpy( csi->tszTitle, dbv.ptszVal );

		mir_snprintf( bufMessage, 32, "XStatus%luMsg", i );
		DBGetContactSettingTString( NULL, protoName, bufMessage, &dbv );
		lstrcpy( csi->tszMessage, dbv.ptszVal );

		if ( lstrlen( csi->tszTitle ) || lstrlen( csi->tszMessage ) )
		{
			cslist->mainWindow->itemslist->list->add( csi );
			cslist->mainWindow->bSomethingChanged = TRUE;
		}
		else
			delete csi;

		if ( *result == IDYES )
		{
			DBDeleteContactSetting( NULL, protoName, bufTitle );
			DBDeleteContactSetting( NULL, protoName, bufMessage );
		}
	}
	cslist->mainWindow->listview->reinitItems( cslist->mainWindow->itemslist->list->getListHead( ) );
}


CSWindow::CSWindow( )
{
	this->handle = NULL;
	this->bExtraIcons = getByte( "AllowExtraIcons", DEFAULT_ALLOW_EXTRA_ICONS );
	this->itemslist = new CSItemsList( );
	this->listview = NULL;
	this->addModifyDlg = NULL;
	this->bSomethingChanged = FALSE;
	this->filterString = NULL;
}


CSWindow::~CSWindow( )
{
	delete this->itemslist;
	SAFE_FREE( ( void** )&this->filterString );
}


void CSWindow::showWindow( void* arg )
{
	CSWindow* csw = ( CSWindow* )arg;

	while ( csw == NULL )
		SleepEx( 10, FALSE );

	DialogBoxParam( CSList::handle, MAKEINTRESOURCE( IDD_CSLIST ), NULL, ( DLGPROC )CSWindowProc, ( LPARAM )csw );
}


void CSWindow::closeWindow( void* arg )
{
	HWND hwnd = ( HWND )arg;
	EndDialog( hwnd, FALSE );
}


void CSWindow::initIcons( )
{
	this->icons = ImageList_Create( 16, 16,  IsWinVerXPPlus( ) ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, XSTATUS_COUNT_EXTENDED, 0 );
	if ( this->icons )
	{
		char protoName[MAXMODULELABELLENGTH] = "";

		CSList::ForAllProtocols( CSList::getFirstProto, ( void* )&protoName );

		for ( int i = 0; i < ( ( this->bExtraIcons ) ? XSTATUS_COUNT_EXTENDED : XSTATUS_COUNT ); i++ )
		{
			char szTemp[MAX_PATH];
			mir_snprintf( szTemp, sizeof( szTemp ), "%s_%s%d", protoName, "xstatus", i );
			ImageList_AddIcon( this->icons, ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szTemp ) );
		}
	}
}


void CSWindow::deinitIcons( )
{
	ImageList_Destroy( this->icons );
}


void CSWindow::initButtons( )
{
	for ( int i = 0; i < SIZEOF( forms ); i++ )
	{
		if ( forms[i].idc < 0 )
			continue;

		SendDlgItemMessage( this->handle, forms[i].idc, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )LoadIconExEx( forms[i].pszIconIcoLib, forms[i].iconNoIcoLib ) );
		SendDlgItemMessage( this->handle, forms[i].idc, BUTTONSETASFLATBTN, 0, 0 ); //maybe set as BUTTONSETDEFAULT?
		SendDlgItemMessage( this->handle, forms[i].idc, BUTTONADDTOOLTIP, ( WPARAM )TranslateTS( forms[i].ptszTitle ), BATF_TCHAR );
	}
}


void CSWindow::setForeground( )
{
	SetForegroundWindow( this->handle );
}


void CSWindow::loadWindowPosition( )
{
	RECT rect = { 0 };
	int width = GetSystemMetrics( SM_CXSCREEN );
	int height = GetSystemMetrics( SM_CYSCREEN );
	GetWindowRect( this->handle, &rect );
	int x, y, defX, defY;
	defX = x = ( width  + 1 - ( rect.right  - rect.left ) ) >> 1;
	defY = y = ( height + 1 - ( rect.bottom - rect.top  ) ) >> 1;
	if ( getByte( "RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION ) )
	{
		x = getWord( "PositionX", defX );
		y = getWord( "PositionY", defY );
	}
	if ( x > width || y > height || x < 0 || y < 0 )
	{
		x = defX;
		y = defY;
  }

	MoveWindow( this->handle,
	            x, y,
	            ( rect.right - rect.left ),
	            ( rect.bottom - rect.top ),
	            TRUE );
}


void CSWindow::toggleEmptyListMessage( )
{
	HWND hwnd = GetDlgItem( this->handle, IDC_NO_ITEMS );
	ShowWindow( hwnd, ( ListView_GetItemCount( this->listview->handle ) > 0 ) ? FALSE : TRUE );
	SetForegroundWindow( hwnd );
	hwnd = GetDlgItem( this->handle, IDC_ADD_SAMPLE );
	ShowWindow( hwnd, ( ListView_GetItemCount( this->listview->handle ) > 0 ) ? FALSE : TRUE );
	SetForegroundWindow( hwnd );
}


BOOL CSWindow::itemPassedFilter( ListItem< StatusItem >* li )
{
	TCHAR filter[MAX_PATH];
	GetDlgItemText( this->handle, IDC_FILTER_FIELD, filter, MAX_PATH );

	if ( lstrlen( filter ) )
	{
		TCHAR title[XSTATUS_TITLE_LIMIT], message[XSTATUS_MESSAGE_LIMIT];
		lstrcpy( title, li->item->tszTitle ); lstrcpy( message, li->item->tszMessage );
		if ( strpos( tcslwr( title ), tcslwr( filter ) ) == -1 )
			if ( strpos( tcslwr( message ), tcslwr( filter ) ) == -1 )
				return FALSE;
	}

	return TRUE;
}


void CSWindow::toggleFilter( )
{
	HWND hFilter = GetDlgItem( this->handle, IDC_FILTER_FIELD );
	BOOL isEnabled = !IsWindowEnabled( hFilter ) ? TRUE : FALSE; // ! = + isEnabled = !isEnabled in one
	EnableWindow( hFilter, isEnabled );
	ShowWindow( hFilter, isEnabled );
	CheckDlgButton( this->handle, IDC_FILTER, isEnabled );
	SetForegroundWindow( hFilter );
	if ( isEnabled )
		SetFocus( hFilter );
	else
	{
		TCHAR filterText[255];
		GetDlgItemText( this->handle, IDC_FILTER_FIELD, filterText, SIZEOF( filterText ) );
		if ( lstrlen( filterText ) > 0 )
			SetDlgItemText( this->handle, IDC_FILTER_FIELD, TEXT( "" ) );
	}
}


BOOL CSWindow::toggleSelection( )
{
	int selection = ListView_GetSelectedItemMacro( this->listview->handle );
	BOOL validSelection = ( selection >= 0 && ( unsigned int )selection < this->itemslist->list->getCount( ) ) ? TRUE : FALSE;
	BOOL filterEnabled = IsWindowVisible( GetDlgItem( this->handle, IDC_FILTER_FIELD ) );
	BOOL somethingChanged = this->bSomethingChanged;

	EnableWindow( GetDlgItem( this->handle, IDC_ADD       ), !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_MODIFY    ), validSelection && !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_REMOVE    ), validSelection && !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_FAVOURITE ), validSelection && !filterEnabled );
//	EnableWindow( GetDlgItem( this->handle, IDC_UNDO      ), somethingChanged && !filterEnabled );
//	EnableWindow( GetDlgItem( this->handle, IDC_GLOBAL    ), !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDOK          ), validSelection );

	return validSelection;
}


CSAMWindow::CSAMWindow( WORD action, CSWindow* parent )
{
	this->action = action;
	this->parent = parent;
	this->bChanged = FALSE;
	this->hCombo = this->hTitle = this->hMessage = NULL;
	
	if ( this->action == IDC_ADD )
		this->item = new StatusItem( );
	else
		this->item = new StatusItem( *this->parent->itemslist->list->get( this->parent->listview->getPositionInList( ) ) );

}


CSAMWindow::~CSAMWindow( )
{
	if ( ! this->bChanged )
		delete this->item;
}


void CSAMWindow::exec( )
{
	DialogBoxParam( CSList::handle, MAKEINTRESOURCE( IDD_ADDMODIFY ), NULL, ( DLGPROC )CSAMWindowProc, ( LPARAM )this );
}


void CSAMWindow::setCombo( )
{
	SendMessage( this->hCombo, CBEM_SETIMAGELIST, 0, ( LPARAM )this->parent->icons );
	for ( int i = 0; i < ( ( this->parent->bExtraIcons ) ? XSTATUS_COUNT_EXTENDED : XSTATUS_COUNT ); i++ )
	{
		COMBOBOXEXITEM cbi = { 0 };
		cbi.mask = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
		cbi.pszText = TranslateTS( xstatus_names[i].ptszTitle );
		cbi.iImage = xstatus_names[i].iId - 1;
        cbi.iItem = i;
        cbi.iSelectedImage = i;
        SendMessage( this->hCombo, CBEM_INSERTITEM, 0, ( LPARAM )&cbi );
	}
	SendMessage( this->hCombo, CB_SETCURSEL, 0, 0 ); // first 0 sets selection to top
}


void CSAMWindow::switchTitle( )
{
	if ( this->action == IDC_MODIFY )
		return;

	int i = SendMessage( this->hCombo, CB_GETCURSEL, 0, 0 );
	SetDlgItemText( this->handle, IDC_TITLE, TranslateTS( xstatus_names[i].ptszTitle ) );
}


void CSAMWindow::fillDialog( )
{
	if ( this->action == IDC_ADD )
	{
        SetWindowText( this->handle, TranslateT( LPGEN( "Add new item" ) ) );
        SetDlgItemText( this->handle, IDOK, TranslateT( LPGEN( "Add" ) ) );
	}
	else
	{
        SetWindowText( this->handle, TranslateT( LPGEN( "Modify item" ) ) );
        SetDlgItemText( this->handle, IDOK, TranslateT( LPGEN( "Modify" ) ) );
	}

	SendMessage( this->hCombo, CB_SETCURSEL, this->item->iIcon, 0 );
	SetDlgItemText( this->handle, IDC_TITLE, this->item->tszTitle );
	SetDlgItemText( this->handle, IDC_MESSAGE, this->item->tszMessage );
}


void CSAMWindow::checkFieldLimit( WORD action, WORD item )
{
	BOOL type = ( item == IDC_MESSAGE ) ? TRUE : FALSE;
	unsigned int limit = type ? XSTATUS_MESSAGE_LIMIT : XSTATUS_TITLE_LIMIT;

	if ( action == EN_CHANGE )
	{
		TCHAR* ptszInputText = ( TCHAR* )malloc( ( limit + 8 ) * sizeof( TCHAR ) );

		GetDlgItemText( this->handle, item, ptszInputText, limit + 8 );

		if ( tcslen( ptszInputText ) > limit )
		{
#if ( WINVER >= 0x501 )
			TCHAR tszPopupTip[MAX_PATH];
			EDITBALLOONTIP ebt = { 0 };
			ebt.cbStruct = sizeof( ebt );
			ebt.pszTitle = TranslateT( LPGEN( "Warning" ) );
			mir_sntprintf( tszPopupTip, SIZEOF( tszPopupTip ), TranslateT( LPGEN( "This field doesn't accept string longer than %d characters. The string will be truncated." ) ), limit );
			ebt.pszText = tszPopupTip;
			ebt.ttiIcon = TTI_WARNING;
			SendMessage( GetDlgItem( this->handle, item ), EM_SHOWBALLOONTIP, 0, ( LPARAM )&ebt );
#endif
			TCHAR* ptszOutputText = ( TCHAR* )malloc( ( limit + 1 ) * sizeof( TCHAR ) );
			GetDlgItemText( this->handle, item, ptszOutputText, limit + 1 );
			SetDlgItemText( this->handle, item, ptszOutputText );
			free( ptszOutputText );
		}
		free( ptszInputText );
	}
}


void CSAMWindow::checkItemValidity( )
{
	COMBOBOXEXITEM cbi = { 0 };
	cbi.mask = CBEIF_IMAGE;
	cbi.iItem = SendDlgItemMessage( this->handle, IDC_COMBO, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( this->handle, IDC_COMBO, CBEM_GETITEM, 0, ( LPARAM )&cbi );

	if ( this->item->iIcon != cbi.iImage )
		( this->item->iIcon = cbi.iImage ) && ( this->bChanged = TRUE );

	TCHAR tszInputTitle[XSTATUS_TITLE_LIMIT];
	TCHAR tszInputMessage[XSTATUS_MESSAGE_LIMIT];

	GetDlgItemText( this->handle, IDC_TITLE, tszInputTitle, XSTATUS_TITLE_LIMIT );
	GetDlgItemText( this->handle, IDC_MESSAGE, tszInputMessage, XSTATUS_MESSAGE_LIMIT );

	if ( lstrcmp( this->item->tszTitle, tszInputTitle ) )
		( lstrcpy( this->item->tszTitle, tszInputTitle ) ) && ( this->bChanged = TRUE );

	if ( lstrcmp( this->item->tszMessage, tszInputMessage ) )
		( lstrcpy( this->item->tszMessage, tszInputMessage ) ) && ( this->bChanged = TRUE );
}


CSListView::CSListView( HWND hwnd, CSWindow* parent )
{
	this->handle = hwnd;
	this->parent = parent;

#if (_WIN32_WINNT >= 0x501)
	LVGROUP lg = { 0 };
	lg.cbSize = sizeof( LVGROUP );
	lg.mask = LVGF_HEADER | LVGF_GROUPID;

	lg.pszHeader = TranslateT( LPGEN( "Favourites" ) );
	lg.iGroupId = 0;
	ListView_InsertGroup( this->handle, -1, &lg );

	lg.pszHeader = TranslateT( LPGEN( "Regular statuses" ) );
	lg.iGroupId = 1;
	ListView_InsertGroup( this->handle, -1, &lg );
	ListView_EnableGroupView( this->handle, TRUE );
#endif

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 0x00;
	lvc.pszText = TEXT( "" );
	lvc.cx = 0x16;
	SendMessage( this->handle, LVM_INSERTCOLUMN, 0, ( LPARAM )&lvc );
	lvc.pszText = TranslateT( LPGEN( "Title" ) );
	lvc.cx = 0x64;
	SendMessage( this->handle, LVM_INSERTCOLUMN, 1, ( LPARAM )&lvc );
	lvc.pszText = TranslateT( LPGEN( "Message" ) );
	lvc.cx = 0xa8;
	SendMessage( this->handle, LVM_INSERTCOLUMN, 2, ( LPARAM )&lvc );

	//ListView_SetExtendedListViewStyle( this->handle, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
#if (_WIN32_IE >= 0x0400)
	ListView_SetExtendedListViewStyleEx( this->handle, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP );
#endif
	ListView_SetImageList( this->handle, this->parent->icons, LVSIL_SMALL );
}


void CSListView::addItem( StatusItem* item, int itemNumber )
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_IMAGE | LVIF_GROUPID | LVIF_PARAM;
	lvi.cchTextMax = 256;
	lvi.iItem = ListView_GetItemCount( this->handle );
	lvi.lParam = itemNumber;
#if ( WINVER >= 0x501 )
	lvi.iGroupId = ( item->bFavourite == TRUE ) ? 0 : 1;
#endif

	// first column
	lvi.iSubItem = 0;
	lvi.iImage = item->iIcon; // use selected xicon
	SendMessage( this->handle, LVM_INSERTITEM, 0, ( LPARAM )&lvi );

	// second column
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	lvi.pszText = item->tszTitle;
	SendMessage( this->handle, LVM_SETITEM, 0, ( LPARAM )&lvi );

	// third column
	lvi.iSubItem = 2;
	lvi.pszText = item->tszMessage;
	SendMessage( this->handle, LVM_SETITEM, 0, ( LPARAM )&lvi );
}


void CSListView::initItems( ListItem< StatusItem >* items )
{
	ListItem< StatusItem >* help = items;
	for ( int i = 0; help != NULL; help = help->next, i++ )
		if ( parent->itemPassedFilter( help ) )
			addItem( help->item, i );
}


void CSListView::reinitItems( ListItem< StatusItem >* items )
{
	EnableWindow( this->handle, FALSE );
	this->removeItems( );
	this->initItems( items );
	EnableWindow( this->handle, TRUE );
}


void CSListView::removeItems( )
{
	ListView_DeleteAllItems( this->handle );
}


int CSListView::getPositionInList( )
{
	LVITEM lvi = { 0 };
	lvi.iItem = ListView_GetSelectedItemMacro( this->parent->listview->handle );
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	ListView_GetItem( this->parent->listview->handle, &lvi );
	return lvi.lParam;
}


// ====[ THREAD FORK ]========================================================

void ForkThread( pThreadFunc pFunc, void* arg )
{
	//CloseHandle( ( HANDLE )mir_forkthread( pFunc, arg ) );
	mir_forkthread( pFunc, arg );
}


// ====[ LIST MANAGEMENT ]====================================================

CSItemsList::CSItemsList( )
{
	this->list = new List< StatusItem >( this->compareItems );
	this->loadItems( );
}


CSItemsList::~CSItemsList( )
{
	delete this->list;
}


int CSItemsList::compareItems( const StatusItem* p1, const StatusItem* p2 )
{
	int favRes = 0, icoRes = 0, ttlRes = 0, msgRes = 0;

	if ( p1->bFavourite < p2->bFavourite )
	    favRes = 1;
	else if ( p1->bFavourite > p2->bFavourite )
	    favRes = -1;

	int result;
	if ( p1->iIcon > p2->iIcon )
		icoRes = 1;
	else if ( p1->iIcon < p2->iIcon )
		icoRes = -1;

	result = lstrcmp( p1->tszTitle, p2->tszTitle );
	ttlRes = result;

	result = lstrcmp( p1->tszMessage, p2->tszMessage );
	msgRes = result;

	if ( !icoRes && !ttlRes && !msgRes )
		return 0;

	if ( favRes != 0 )
		return favRes;
	if ( icoRes != 0 )
		return icoRes;
	if ( ttlRes != 0 )
		return ttlRes;
	if ( msgRes != 0 )
		return msgRes;

	return 0;
}



void CSItemsList::loadItems( )
{
	unsigned int itemsCount = getWord( "ItemsCount", DEFAULT_ITEMS_COUNT );

	for ( unsigned int i = 1; i <= itemsCount; i++ )
	{
		StatusItem* item = new StatusItem( );
		char dbSetting[32];
		DBVARIANT dbv = { DBVT_TCHAR };
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dIcon", i );
		item->iIcon = getByte( dbSetting, DEFAULT_ITEM_ICON );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dTitle", i );
		getTString( dbSetting, &dbv );
		lstrcpy( item->tszTitle, dbv.ptszVal );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dMessage", i );
		getTString( dbSetting, &dbv );
		lstrcpy( item->tszMessage, dbv.ptszVal );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dFavourite", i );
		item->bFavourite = ( BOOL )getByte( dbSetting, DEFAULT_ITEM_IS_FAVOURITE );
		this->list->add( item );
	}
}


void CSItemsList::saveItems( )
{
	unsigned int i;
	unsigned int oldItemsCount = getWord( "ItemsCount", DEFAULT_ITEMS_COUNT );

	for ( i = 1; i <= this->list->getCount( ); i++ )
	{
		StatusItem* item = this->list->get( i - 1 );
		char dbSetting[32];
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dIcon", i );
		setByte( dbSetting, item->iIcon );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dTitle", i );
		setTString( dbSetting, item->tszTitle );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dMessage", i );
		setTString( dbSetting, item->tszMessage );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dFavourite", i );
		setByte( dbSetting, item->bFavourite );
	}

	setWord( "ItemsCount", this->list->getCount( ) );

	for ( ; i <= oldItemsCount; i++ )
	{
		char dbSetting[32];
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dIcon", i );
		deleteSetting( dbSetting );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dTitle", i );
		deleteSetting( dbSetting );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dMessage", i );
		deleteSetting( dbSetting );
		mir_snprintf( dbSetting, sizeof( dbSetting ), "Item%dFavourite", i );
		deleteSetting( dbSetting );
	}
}


// ====[ PROCEDURES ]=========================================================

INT_PTR CALLBACK CSWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	CSWindow* csw = ( CSWindow* )GetWindowLong( hwnd, GWL_USERDATA );

	switch ( message )
	{

	case WM_INITDIALOG:
		csw = ( CSWindow* )lparam;
		SetWindowLong( hwnd, GWL_USERDATA, lparam );
		csw->handle = hwnd;
		csw->initIcons( );
		csw->initButtons( );
		csw->listview = new CSListView( GetDlgItem( hwnd, IDC_CSLIST ), csw );
		csw->listview->initItems( csw->itemslist->list->getListHead( ) );
		csw->toggleEmptyListMessage( );
		csw->toggleSelection( );
		csw->loadWindowPosition( );
		SetWindowText( hwnd, TranslateT( PLUGIN_LONG_NAME ) );
		translateDialog( hwnd );
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wparam ) )
		{

		case IDC_MODIFY:
		case IDC_ADD:
			csw->addModifyDlg = new CSAMWindow( LOWORD( wparam ), csw );
			csw->addModifyDlg->exec( );
			if ( csw->addModifyDlg->bChanged )
			{
				if ( LOWORD( wparam ) == IDC_MODIFY )
					csw->itemslist->list->remove( csw->listview->getPositionInList( ) );

				csw->itemslist->list->add( csw->addModifyDlg->item );
				csw->bSomethingChanged = TRUE;
				csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
				csw->toggleEmptyListMessage( );
			}
			delete csw->addModifyDlg;
			break;

		case IDC_REMOVE:
			if ( getByte( "ConfirmDeletion", DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION ) )
				if ( MessageBox( hwnd, TranslateT( LPGEN( "Do you really want to delete selected item?" ) ), TranslateT( LPGEN( PLUGIN_LONG_NAME ) ), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION ) == IDNO )
					break;
			csw->itemslist->list->remove( csw->listview->getPositionInList( ) );
			csw->bSomethingChanged = TRUE;
			csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
			csw->toggleEmptyListMessage( );
			break;

		case IDC_FAVOURITE:
			{
				int selection = csw->listview->getPositionInList( );
				StatusItem* f = new StatusItem( *csw->itemslist->list->get( selection ) );
				f->bFavourite = ! f->bFavourite;
				csw->itemslist->list->remove( selection );
				csw->itemslist->list->add( f );
				csw->bSomethingChanged = TRUE;
				csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
				csw->toggleSelection( );
			}
			break;

		case IDC_UNDO:
			//csw->listview->reinitItems( csw->itemslist->list->top );
			break;

		case IDC_IMPORT:
			{
				int result = getByte( "DeleteAfterImport", DEFAULT_PLUGIN_DELETE_AFTER_IMPORT );
				if ( result == TRUE )
					result = IDYES;
				else
				{
					result = MessageBox( hwnd, TranslateT( "Do you want old database entries to be deleted after Import?" ),
					                     TranslateT( PLUGIN_LONG_NAME ), MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION );
					if ( result == IDCANCEL ) break;
				}

				CSList::ForAllProtocols( CSList::importCustomStatusUIStatusesFromAllProtos, ( void* )&result );
			}
			break;

		case IDC_FILTER:
			csw->toggleFilter( );
			csw->toggleSelection( );
			break;

		case IDC_FILTER_FIELD:
			if ( HIWORD( wparam ) == EN_CHANGE )
				csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
			break;

		case IDCLOSE:    // close and save, no custom status change
        case IDCANCEL:   // close and save, cancel custom status
        case IDOK:       // close and save, set selected custom status

			if ( LOWORD( wparam ) == IDOK && csw->toggleSelection( ) )
				cslist->setStatus( IDOK, csw->itemslist->list->get(
					csw->listview->getPositionInList( ) ) );
			if ( LOWORD( wparam ) == IDCANCEL )
				cslist->setStatus( IDCANCEL, NULL );
			if ( csw->bSomethingChanged )
				csw->itemslist->saveItems( );
			csw->saveWindowPosition( csw->handle );

			delete csw->listview;
			csw->deinitIcons( );
			cslist->closeList( hwnd );
			return TRUE;

		}
		return FALSE;

	case WM_NOTIFY:
		if ( wparam == IDC_CSLIST )
		{
			NMHDR* pnmh = ( NMHDR* )lparam;
			switch ( pnmh->code )
			{

			case NM_DBLCLK:
				PostMessage( hwnd, WM_COMMAND, IDOK, 0L );
				break;

			case LVN_ITEMCHANGED:
			case NM_CLICK:
				csw->toggleSelection( );
				break;

			}
		}
		return FALSE;

	case WM_CLOSE:
		PostMessage(hwnd, WM_COMMAND, IDCLOSE, 0L);
		return FALSE;

	case WM_CTLCOLORSTATIC:
		SetTextColor( ( HDC )wparam, RGB( 174, 174, 174 ) );
		if ( ( ( HWND )lparam == GetDlgItem( hwnd, IDC_NO_ITEMS ) ) ||
		     ( ( HWND )lparam == GetDlgItem( hwnd, IDC_ADD_SAMPLE ) ) )
			return ( BOOL )GetStockObject( WHITE_BRUSH );
		return FALSE;

	}

	return FALSE;
}


INT_PTR CALLBACK CSAMWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	CSAMWindow* csamw = ( CSAMWindow* )GetWindowLong( hwnd, GWL_USERDATA );

	switch ( message )
	{

		case WM_INITDIALOG:
			{
				csamw = ( CSAMWindow* )lparam;
				SetWindowLong( hwnd, GWL_USERDATA, lparam );
				csamw->handle = hwnd;
				EnableWindow( csamw->parent->handle, FALSE );
				csamw->hCombo = GetDlgItem( hwnd, IDC_COMBO );
				csamw->hTitle = GetDlgItem( hwnd, IDC_TITLE );
				csamw->hMessage = GetDlgItem( hwnd, IDC_MESSAGE );
				csamw->setCombo( );
				csamw->fillDialog( );
				translateDialog( hwnd );
			}
			break;

		case WM_COMMAND:
			switch ( LOWORD( wparam ) )
			{

			case IDC_COMBO:
				csamw->switchTitle( );
				break;

			case IDC_TITLE:
			case IDC_MESSAGE:
				csamw->checkFieldLimit( HIWORD( wparam ), LOWORD( wparam ) );
				break;

			case IDOK:
				csamw->checkItemValidity( );
			case IDCANCEL:
				EnableWindow( csamw->parent->handle, TRUE );
				EndDialog( hwnd, LOWORD( wparam ) );
				break;

			}
			break;

	}
	return FALSE;
}


INT_PTR CALLBACK CSOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	switch ( message )
	{

	case WM_INITDIALOG:
		{
			CheckDlgButton( hwnd, IDC_CONFIRM_DELETION,
							getByte( "ConfirmDeletion", DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION ) ?
							BST_CHECKED : BST_UNCHECKED );

			CheckDlgButton( hwnd, IDC_DELETE_AFTER_IMPORT,
							getByte( "DeleteAfterImport", DEFAULT_PLUGIN_DELETE_AFTER_IMPORT ) ?
							BST_CHECKED : BST_UNCHECKED );

			CheckDlgButton( hwnd, IDC_REMEMBER_POSITION,
			                getByte( "RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION ) ?
			                BST_CHECKED : BST_UNCHECKED );

			BYTE place = getByte( "Placement", DEFAULT_PLUGIN_MENU_PLACEMENT );

			CheckDlgButton( hwnd, IDC_PLACEMENT_MAIN,          ( place == MENU_PLACEMENT_MAIN ) );
			CheckDlgButton( hwnd, IDC_PLACEMENT_ICQ,           ( place == MENU_PLACEMENT_ICQ ) );
			CheckDlgButton( hwnd, IDC_PLACEMENT_GLOBAL_TOP,    ( place == MENU_PLACEMENT_GLOBAL_TOP ) );
			CheckDlgButton( hwnd, IDC_PLACEMENT_GLOBAL_BOTTOM, ( place == MENU_PLACEMENT_GLOBAL_BOTTOM ) );

			EnableWindow( GetDlgItem( hwnd, IDC_HIDE_CUSTOM_STATUS_UI ), ( place == MENU_PLACEMENT_ICQ ) );
			CheckDlgButton( hwnd, IDC_HIDE_CUSTOM_STATUS_UI, getByte( "HideCustomStatusUI",
			                DEFAULT_PLUGIN_HIDE_CUSTOM_STATUS_UI ) );

			// we need fixes made in 0.8.0.35 to allow placing item inside Global at the top
			if ( cslist->dwMirandaVersion < PLUGIN_MAKE_VERSION( 0, 8, 0, 35 ) )
				EnableWindow( GetDlgItem( hwnd, IDC_PLACEMENT_GLOBAL_TOP ), FALSE );

			unsigned int protosExtraEnabled = 0;

			CSList::ForAllProtocols( CSList::countPlusModProtos, ( void* )&protosExtraEnabled );

			if ( protosExtraEnabled ) // some protocol(s) allow(s) use of extra icons
			{
				EnableWindow( GetDlgItem( hwnd, IDC_ALLOW_EXTRA_ICONS ), TRUE );
				CheckDlgButton( hwnd, IDC_ALLOW_EXTRA_ICONS, getByte( "AllowExtraIcons",
				                DEFAULT_ALLOW_EXTRA_ICONS ) );
			}

			translateDialog( hwnd );
		}
		return TRUE;

	case WM_COMMAND:
		{
			SendMessage( GetParent( hwnd ), PSM_CHANGED, 0, 0 );
			EnableWindow( GetDlgItem( hwnd, IDC_HIDE_CUSTOM_STATUS_UI ), IsDlgButtonChecked( hwnd, IDC_PLACEMENT_ICQ ) );
		}
		return TRUE;

	case WM_NOTIFY:
		switch ( ( ( LPNMHDR )lparam )->code )
		{

		case PSN_APPLY:
			{
				setByte( "ConfirmDeletion", IsDlgButtonChecked( hwnd, IDC_CONFIRM_DELETION ) ? 1 : 0 );
				setByte( "DeleteAfterImport", IsDlgButtonChecked( hwnd, IDC_DELETE_AFTER_IMPORT ) ? 1 : 0 );

				setByte( "Placement", IsDlgButtonChecked( hwnd, IDC_PLACEMENT_MAIN ) ?
				                  1 : IsDlgButtonChecked( hwnd, IDC_PLACEMENT_ICQ ) ?
				                  2 : IsDlgButtonChecked( hwnd, IDC_PLACEMENT_GLOBAL_TOP ) ?
				                  3 : IsDlgButtonChecked( hwnd, IDC_PLACEMENT_GLOBAL_BOTTOM ) ? 4 : 0 );

				setByte( "HideCustomStatusUI", IsDlgButtonChecked( hwnd, IDC_HIDE_CUSTOM_STATUS_UI ) ? 1 : 0 );
				setByte( "AllowExtraIcons", IsDlgButtonChecked( hwnd, IDC_ALLOW_EXTRA_ICONS ) ? 1 : 0 );
				setByte( "RememberWindowPosition", IsDlgButtonChecked( hwnd, IDC_REMEMBER_POSITION ) ? 1 : 0 );

				cslist->rebuildMenuItems( );
			}
			break;

		}
		return TRUE;

	}
	return FALSE;
}


INT_PTR CALLBACK CSRNWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	switch ( message )
	{

	case WM_INITDIALOG:
		{
			TCHAR verPrint[64];
			mir_sntprintf( verPrint, SIZEOF( verPrint ),
			               TEXT( "%d.%d.%d.%d%s" ),
			               MAJOR_VERSION,
			               MINOR_VERSION,
			               RELEASE_VERSION,
			               BUILD_VERSION,
			               TEXT( PLUGIN_VERSION_POST ) );
			SetDlgItemText( hwnd, IDC_PLUGIN_VERSION, verPrint );

			// set bold font
			LOGFONT lf = {0};
			HFONT hfnt = ( HFONT )SendDlgItemMessage( hwnd, IDC_PLUGIN_NAME, WM_GETFONT, 0, 0 );
			GetObject( hfnt, sizeof( lf ), &lf );
			lf.lfWeight = FW_BOLD;
			SendDlgItemMessage( hwnd, IDC_PLUGIN_NAME, WM_SETFONT, ( WPARAM )CreateFontIndirect( &lf ), TRUE );

			translateDialog( hwnd );
		}
		break;

	case WM_COMMAND:
		switch ( LOWORD( wparam ) )
		{

		case IDC_DONATE:
			CallService( MS_UTILS_OPENURL, 1, ( LPARAM )PLUGIN_INFO_HOMEPAGE_DONATE );
			break;

		case IDOK:
		case IDCANCEL:
			setByte( "ShowReleaseNotes", IsDlgButtonChecked( hwnd, IDC_TOGGLE_SHOW ) ? 0 : 1 );

			// obsolete support
			BOOL obs = TRUE;
			if ( getByte( "ItemsV2Imported", 0 ) )
				obs = FALSE;
			DBVARIANT dbv = { DBVT_TCHAR };
			getTString( "listhistory", &dbv );
			if ( ! dbv.ptszVal )
				obs = FALSE;
			if ( obs )
			{
				cslist_import_v2_status_items( );
				setByte( "ItemsV2Imported", 1 );
				// deleteSetting( "listhistory" );
			}

			EndDialog( hwnd, LOWORD( wparam ) );
			break;

		}
		break;

	case WM_CTLCOLORSTATIC: // paint background of specific elements
		{
		if ( ((HWND)lparam == GetDlgItem(hwnd, IDC_WHITERECT)) ||
		     ((HWND)lparam == GetDlgItem(hwnd, IDC_PLUGIN_ICON)) ||
		     ((HWND)lparam == GetDlgItem(hwnd, IDC_PLUGIN_NAME)) ||
		     ((HWND)lparam == GetDlgItem(hwnd, IDC_PLUGIN_DESCRIPTION)) )
			return ( BOOL )GetStockObject( WHITE_BRUSH );
		}
	
		break;

	}
	return FALSE;
}


// ====[ HELPERS ]============================================================

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon )
{
	if ( ServiceExists( MS_SKIN2_GETICON ) ) {
		char szSettingName[64];
		mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", PLUGIN_SHORT_NAME, IcoLibName );
		return ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szSettingName );
	}
	else
		return ( HICON )LoadImage( cslist->handle, MAKEINTRESOURCE( NonIcoLibIcon ), IMAGE_ICON, 0, 0, 0 );
}


void translateDialog( HWND hwnd )
{
	LANGPACKTRANSLATEDIALOG lptd;

	lptd.cbSize = sizeof( lptd );
	lptd.flags = NULL;
	lptd.hwndDlg = hwnd;
	lptd.ignoreControls = NULL;
	CallService( MS_LANGPACK_TRANSLATEDIALOG, 0, ( LPARAM )&lptd );
}


/* ======================================================================== 78

                              Custom Status List
                              __________________

                                  by jarvis

// ======================================================================== */
