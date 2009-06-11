#include <windows.h>
#include <commctrl.h>
#include "resource.h"
//#include "AggressiveOptimize.h"

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_icolib.h>
#include <m_langpack.h>
//#include <m_database.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_updater.h>
#include <m_utils.h>
#include <m_hotkeys.h>
#include <m_toptoolbar.h>
#include <m_toolbar.h>


#define MS_OPENFOLDER_OPEN                "openFolder/Open"

#define OPENFOLDER_VERSION                PLUGIN_MAKE_VERSION( 1, 1, 0, 0 )
#define OPENFOLDER_DESCRIPTION            "Adds a menu/toobar item which opens the main Miranda IM folder."
#define MIID_OPENFOLDER                   { 0x10896143, 0x7249, 0x4b36, { 0xa4, 0x8, 0x65, 0x1, 0xa6, 0xb6, 0x3, 0x5a } }                  
//#define OPENFOLDER_DB_MODULENAME          "openFolder"
#define OPENFOLDER_MODULE_NAME            "openfolder"

#define OPENFOLDER_TRANSLATE              TranslateT( "Open Folder" )

#define OPENFOLDER_UPD_VERURL             "http://dev.mirandaim.ru/~jarvis/"
#define OPENFOLDER_UPD_UPDURL             "http://mirandapack.ic.cz/eternity_plugins/openfolder.zip"
#define OPENFOLDER_UPD_SZPREFIX           "Open Miranda Folder</a> "

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
  "Open Miranda Folder",
  OPENFOLDER_VERSION,
  OPENFOLDER_DESCRIPTION,
  "jarvis, Kreisquadratur",
  "jarvis@jabber.cz, djui@kreisquadratur.de",
  "© 2008 jarvis, © 2004 Kreisquadratur",
  "http://addons.miranda-im.org/details.php?action=viewfile&id=1542",
  UNICODE_AWARE,    //not transient
  0    //doesn't replace anything built-in
};

PLUGININFOEX pluginInfoEx = {
  sizeof( PLUGININFOEX ),
  "Open Miranda Folder",
  OPENFOLDER_VERSION,
  OPENFOLDER_DESCRIPTION,
  "jarvis, Kreisquadratur",
  "jarvis@jabber.cz, djui@kreisquadratur.de",
  "© 2008 jarvis, © 2004 Kreisquadratur",
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
