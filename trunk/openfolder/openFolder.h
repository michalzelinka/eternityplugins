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

#ifndef __OPENFOLDER_H
#define __OPENFOLDER_H 1

#define MIRANDA_VER 0x0400

#pragma warning( disable: 4996 )

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "resource.h"

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_updater.h>
#include <m_utils.h>
#include <m_hotkeys.h>
#include <m_toptoolbar.h>
#include <m_toolbar.h>
#include <m_openfolder.h>


#define OPENFOLDER_VERSION                PLUGIN_MAKE_VERSION( 1, 1, 1, 0 )
#define OPENFOLDER_DESCRIPTION            "Adds a menu/toobar item which opens the main Miranda IM folder."
#define MIID_OPENFOLDER                   { 0x10896143, 0x7249, 0x4b36, { 0xa4, 0x8, 0x65, 0x1, 0xa6, 0xb6, 0x3, 0x5a } }                  
#define OPENFOLDER_MODULE_NAME            "OpenFolder"

#define OPENFOLDER_TEXT                   "Open Miranda Folder"

#define OPENFOLDER_UPD_VERURL             "http://dev.mirandaim.ru/~jarvis/"
#define OPENFOLDER_UPD_UPDURL             "http://mirandapack.ic.cz/eternity_plugins/openfolder.zip"
#define OPENFOLDER_UPD_SZPREFIX           "Open Miranda Folder</a> "

#ifdef UNICODE
#define tstrrchr wcsrchr
#else
#define tstrrchr strrchr
#endif

static const MUUID interfaces[] = { MIID_TESTPLUGIN, MIID_OPENFOLDER, MIID_LAST };

HINSTANCE hInst;
PLUGINLINK *pluginLink;
DWORD gMirandaVersion;

struct {
	HANDLE hEventModulesLoaded;
	HANDLE hServiceOpenFolder;
	HANDLE hButtonTopToolbar, hButtonModernToolBar, hItemMainMenu;
	HANDLE hIconOpenFolder;
	HANDLE hHotkey;
} handles = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

PLUGININFO pluginInfo = {
	sizeof( PLUGININFO ),
	OPENFOLDER_TEXT,
	OPENFOLDER_VERSION,
	OPENFOLDER_DESCRIPTION,
	"jarvis, Kreisquadratur",
	"jarvis@jabber.cz, djui@kreisquadratur.de",
	"© 2008-9 jarvis, © 2004 Kreisquadratur",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=1542",
	UNICODE_AWARE,    //not transient
	0    //doesn't replace anything built-in
};

PLUGININFOEX pluginInfoEx = {
	sizeof( PLUGININFOEX ),
	OPENFOLDER_TEXT,
	OPENFOLDER_VERSION,
	OPENFOLDER_DESCRIPTION,
	"jarvis, Kreisquadratur",
	"jarvis@jabber.cz, djui@kreisquadratur.de",
	"© 2008-9 jarvis, © 2004 Kreisquadratur",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=1542",
	UNICODE_AWARE,    //not transient
	0,    //doesn't replace anything built-in
	MIID_OPENFOLDER // {10896143-7249-4b36-A408-6501A6B6035A}
};

BOOL WINAPI DllMain( HINSTANCE, DWORD, LPVOID );

__declspec( dllexport ) PLUGININFO*   MirandaPluginInfo( DWORD );
__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD );
__declspec( dllexport ) int           Load( PLUGINLINK *link );
__declspec( dllexport ) int           Unload( void );
                        int           ModulesLoaded( WPARAM, LPARAM );

                        int           MenuCommand_OpenFolder(  WPARAM, LPARAM );

                        HICON         LoadIconExEx( const char*, int );

#endif /* __OPENFOLDER_H */
