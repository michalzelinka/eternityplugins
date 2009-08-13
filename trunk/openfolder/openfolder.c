/* ======================================================================== 78

                                 Open Folder
                                 ___________

  Open Folder plugin for Miranda IM (www.miranda-im.org)
  Follower of openFolder by Kreisquadratur
  Copyright © 2004 Kreisquad
  Copyright © 2008-2009 jarvis

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

  Revision       : $Rev$
  Last change on : $Date$
  Last change by : $Author$

  ============================================================================  


  DESCRIPTION:

  Adds a menu/toobar item which opens the main Miranda IM folder.

// ======================================================================== */

#include "openfolder.h"

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	hInst = hinstDLL;
	return TRUE;
}

__declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion )
{
	gMirandaVersion = mirandaVersion;
	return &pluginInfo;
}

__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion )
{
	gMirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}

__declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void ) {
	return interfaces;
}

int __declspec( dllexport ) Load( PLUGINLINK *link )
{
	pluginLink = link;
	handles.hEventModulesLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, ModulesLoaded );

	return 0;
}

int __declspec( dllexport ) Unload( void )
{
	if ( handles.hEventModulesLoaded )
		UnhookEvent( handles.hEventModulesLoaded );
	if ( handles.hServiceOpenFolder )
		DestroyServiceFunction( handles.hServiceOpenFolder );
	CloseHandle( handles.hButtonModernToolBar );
	CloseHandle( handles.hButtonTopToolbar );
	CloseHandle( handles.hHotkey );
	DestroyIcon( ( HICON )handles.hIconOpenFolder );
	CloseHandle( handles.hItemMainMenu );

	return 0;
}

int ModulesLoaded( WPARAM wParam, LPARAM lParam )
{
	handles.hServiceOpenFolder = CreateServiceFunction( MS_OPENFOLDER_OPEN, MenuCommand_OpenFolder );

	// icolib (0.7+)
	if ( ServiceExists( MS_SKIN2_ADDICON ) )
	{
		SKINICONDESC sid = { 0 };
		TCHAR szFile[MAX_PATH];
		char szSettingName[64];

		GetModuleFileName( hInst, szFile, MAX_PATH );
		sid.cbSize = sizeof( SKINICONDESC );
		sid.ptszDefaultFile = szFile;
		sid.flags = SIDF_ALL_TCHAR;
		sid.cx = sid.cy = 16;
		sid.ptszSection = TranslateT( OPENFOLDER_TEXT );
		mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", OPENFOLDER_MODULE_NAME, "open" );
		sid.pszName = szSettingName;
		sid.ptszDescription = TranslateT( OPENFOLDER_TEXT );
		sid.iDefaultIndex = -IDI_FOLDER;
		handles.hIconOpenFolder = ( HANDLE )CallService( MS_SKIN2_ADDICON, 0, ( LPARAM )&sid );
	}

	// icon handle is loaded universally (icolib for 0.7+, from resource for < 0.7)
	if ( !handles.hIconOpenFolder )
		handles.hIconOpenFolder = ( HANDLE )LoadIconExEx( "open", IDI_FOLDER );
  
	// hotkeys service (0.8+)
	if ( ServiceExists( MS_HOTKEY_REGISTER ) )
	{
		HOTKEYDESC hotkey = { 0 };
		hotkey.cbSize = sizeof( hotkey );
		hotkey.pszName = Translate( OPENFOLDER_TEXT );
		hotkey.pszDescription = Translate( OPENFOLDER_TEXT );
		hotkey.pszSection = "Main";
		hotkey.pszService = MS_OPENFOLDER_OPEN;
		hotkey.DefHotKey = MAKEWORD( 'O', HOTKEYF_SHIFT | HOTKEYF_ALT );
		handles.hHotkey = ( HANDLE )CallService( MS_HOTKEY_REGISTER, 0, ( LPARAM )&hotkey );
	}

	// toptoolbar (if plugin is installed)
	if ( ServiceExists( MS_TTB_ADDBUTTON ) )
	{
		TTBButton ttbb = { 0 };

		ZeroMemory( &ttbb, sizeof( ttbb ) );
		ttbb.cbSize = sizeof( ttbb );
		ttbb.hbBitmapUp = 
		ttbb.hbBitmapDown = LoadBitmap( hInst, MAKEINTRESOURCE( IDB_FOLDER ) );
		ttbb.pszServiceUp =
		ttbb.pszServiceDown = MS_OPENFOLDER_OPEN;
		ttbb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_DRAWBORDER;
		ttbb.name = Translate( OPENFOLDER_TEXT );

		handles.hButtonTopToolbar = ( HANDLE )CallService( MS_TTB_ADDBUTTON, ( WPARAM )&ttbb, 0 );
	}

	// modern toolbar (modern 0.7+)
	if ( ServiceExists( MS_TB_ADDBUTTON ) )
	{
		TBButton button = { 0 };

		button.cbSize = sizeof( button );
		button.pszButtonID = MS_OPENFOLDER_OPEN;
		button.pszTooltipUp = button.pszTooltipDn = button.pszButtonName = Translate( OPENFOLDER_TEXT );
		button.pszServiceName = MS_OPENFOLDER_OPEN;
		button.defPos = 190;
		button.tbbFlags = TBBF_SHOWTOOLTIP | TBBF_VISIBLE;
		button.hPrimaryIconHandle = handles.hIconOpenFolder;
		button.hSecondaryIconHandle = button.hPrimaryIconHandle;
		CallService( MS_TB_ADDBUTTON, 0, ( LPARAM )&button );
	}

	// main menu item (0.1+)
	if ( ServiceExists( MS_CLIST_ADDMAINMENUITEM ) )
	{
		CLISTMENUITEM mi;

		ZeroMemory( &mi, sizeof( mi ) );
		mi.cbSize = sizeof( mi );
		mi.position = 0x7FFFFFFF;
		mi.flags = CMIF_TCHAR;
		mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOLDER));
		mi.hIcon = LoadIconExEx( "open", IDI_FOLDER );
		mi.ptszName = TranslateT( OPENFOLDER_TEXT );
		mi.pszService = MS_OPENFOLDER_OPEN;

		handles.hItemMainMenu = ( HANDLE )CallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
	}

	// updater
	if ( ServiceExists( MS_UPDATE_REGISTER ) )
	{
		static Update upd = { 0 };

		static char szCurrentVersion[30];
		static char *szVersionUrl = OPENFOLDER_UPD_VERURL;
		static char *szUpdateUrl = OPENFOLDER_UPD_UPDURL;
		static char *szPrefix = OPENFOLDER_UPD_SZPREFIX;

		upd.cbSize = sizeof( upd );
		upd.szComponentName = pluginInfo.shortName;
		if ( gMirandaVersion > PLUGIN_MAKE_VERSION( 0, 7, 0, 0 ) )
			upd.pbVersion = ( BYTE * )CreateVersionStringPluginEx( ( PLUGININFOEX * )&pluginInfoEx, szCurrentVersion );
		else // updater support for < 0.7 cores
			upd.pbVersion = ( BYTE * )CreateVersionStringPlugin( ( PLUGININFO * )&pluginInfo, szCurrentVersion );
		upd.cpbVersion = strlen( ( char * )upd.pbVersion );
		upd.pbVersionPrefix = ( BYTE * )OPENFOLDER_UPD_SZPREFIX;
		upd.cpbVersionPrefix = strlen( ( char * )upd.pbVersionPrefix );

		upd.szBetaUpdateURL = szUpdateUrl;
		upd.szBetaVersionURL = szVersionUrl;
		upd.pbVersion = szCurrentVersion;
		upd.cpbVersion = lstrlenA( szCurrentVersion );
		upd.pbBetaVersionPrefix = ( BYTE * )szPrefix;
		upd.cpbBetaVersionPrefix = strlen( ( char * )upd.pbBetaVersionPrefix );

		CallService( MS_UPDATE_REGISTER, 0, ( LPARAM )&upd );
	}

	return 0;
}

int MenuCommand_OpenFolder( WPARAM wParam, LPARAM lParam )
{
	// idea taken from "miranda\src\modules\utils\path.c"
	TCHAR tszMirandaPath[MAX_PATH];
	TCHAR* p = 0;

	GetModuleFileName( GetModuleHandle( NULL ), tszMirandaPath, sizeof( tszMirandaPath ) );
	p = tstrrchr( tszMirandaPath, TEXT( '\\' ) );
	if ( p && p + 1 )
		*( p + 1 ) = 0;
  
	if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		ShellExecute( 0, TEXT( "explore" ), tszMirandaPath, 0, 0, SW_SHOWNORMAL );
	else
		ShellExecute( 0, TEXT( "open"    ), tszMirandaPath, 0, 0, SW_SHOWNORMAL );
    
	if ( handles.hButtonTopToolbar )
		CallService( MS_TTB_SETBUTTONSTATE, ( WPARAM )handles.hButtonTopToolbar, TTBST_RELEASED );
  
	return 0;
}

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon )
{
	if ( ServiceExists( MS_SKIN2_GETICON ) ) {
		char szSettingName[64];
		mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", OPENFOLDER_MODULE_NAME, IcoLibName );
		return ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szSettingName );
	}
	else
		return ( HICON )LoadImage( hInst, MAKEINTRESOURCE( NonIcoLibIcon ), IMAGE_ICON, 0, 0, 0 );
}

/* ======================================================================== 78

                                 Open Folder
                                 ___________

                                  by jarvis

// ======================================================================== */
