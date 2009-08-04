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

// ====[ INCLUDES AND DEFINITIONS ]======================================== */

#ifndef __CSLIST_H
#define __CSLIST_H 1

#if defined UNICODE
#define WINVER          0x501
#define _WIN32_WINNT    0x501
#define _WIN32_IE	    0x600
#else
#define WINVER          0x400
#define _WIN32_WINNT    0x400
#define _WIN32_IE       0x400
#define LVIF_GROUPID    0
#endif
#define MIRANDA_VER   0x0500

#pragma warning( disable: 4996 )
#pragma comment( lib, "comctl32.lib" )

//#include <stdio.h>
#include <vector>
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_genmenu.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_icq.h>
#include <m_button.h>
#include <m_options.h>
#include <m_toolbar.h>
#include <m_hotkeys.h>
#include <m_updater.h>
#include <m_variables.h>

// ====[ BASIC DEFINITIONS ]==================================================

#include "resource.h"

#define PLUGIN_LONG_NAME       "Custom Status List"
#define PLUGIN_SHORT_NAME      "CSList"

#define PLUGIN_GUUID                  { 0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 } }
                                      //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}


// ====[ VERSIONING ]=========================================================

#define MAJOR_VERSION                 0
#define MINOR_VERSION                 1
#define RELEASE_VERSION               0
#define BUILD_VERSION                 4
#define ALPHA_BUILD_VERSION           "10"
#define RELEASE_CANDIDATE_VERSION     "4"

//#define ALPHA_BUILD
#ifndef ALPHA_BUILD
#define RELEASE_CANDIDATE
#ifndef RELEASE_CANDIDATE
#define FINAL_BUILD
#endif
#endif

#define PLUGIN_RELNOTES               ( MAJOR_VERSION * 1000 + MINOR_VERSION * 100 + RELEASE_VERSION * 10 + BUILD_VERSION )

#if defined( ALPHA_BUILD )
#define PLUGIN_VERSION_POST           " alpha"
#define PLUGIN_VER_PRINT              "[alpha build #"ALPHA_BUILD_VERSION" assembled "__DATE__" at "__TIME__"]\r\n"
#else
#if defined( RELEASE_CANDIDATE )
#define PLUGIN_VERSION_POST           " RC"
#define PLUGIN_VER_PRINT              "[Release Candidate #"RELEASE_CANDIDATE_VERSION" assembled "__DATE__"]\r\n"
#else
#define PLUGIN_VERSION_POST           ""
#define PLUGIN_VER_PRINT              ""
#endif
#endif

// ====[ PLUGIN INFO ]========================================================

#define PLUGIN_INFO_ABOUT             PLUGIN_VER_PRINT"Keep your ICQ Custom statuses in an integrated, " \
                                      "easy to use manager. This plugin offers simple management " \
									  "functions to keep your extra statuses on one place."
#define PLUGIN_INFO_AUTHOR            "jarvis"
#define PLUGIN_INFO_EMAIL             "jarvis@jabber.cz"
#define PLUGIN_INFO_COPYRIGHT         "© 2007-2009 jarvis, all acclaims go to HANAX"
#define PLUGIN_INFO_HOMEPAGE          "http://dev.miranda.im/~jarvis/"
// --
#define PLUGIN_INFO_HOMEPAGE_DONATE   PLUGIN_INFO_HOMEPAGE"#donations"


// ====[ LIMITS ]=============================================================

#define XSTATUS_TITLE_LIMIT           64   // limit of chars for x-status title
#define XSTATUS_MESSAGE_LIMIT         2048 // limit of chars for x-status message
#define XSTATUS_COUNT                 32
#define XSTATUS_COUNT_EXTENDED        37


// ====[ UPDATER STRINGS ]====================================================

#define PLUGIN_UPD_VERURL             "http://dev.mirandaim.ru/~jarvis/"
#define PLUGIN_UPD_UPDURL             "http://mirandapack.ic.cz/eternity_plugins/cslist.zip"
#define PLUGIN_UPD_FLVERURL           "http://addons.miranda-im.org/details.php?action=viewfile&id=3483"
#define PLUGIN_UPD_FLUPDURL           "http://addons.miranda-im.org/feed.php?dlfile=3483"
#define PLUGIN_UPD_SZPREFIX           "Custom Status List</a> "


// ====[ SERVICES ]===========================================================

#define MS_CSLIST_SHOWLIST                    "CSList/ShowList"
#define MS_PSEUDO_PROTO_PROTOACCSENUM         ( CSList::bAccountsSupported ? MS_PROTO_ENUMACCOUNTS : MS_PROTO_ENUMPROTOCOLS )


// ====[ MENU PLACEMENT IDs ]=================================================

#define MENU_PLACEMENT_MAIN                   1
#define MENU_PLACEMENT_ICQ                    2
#define MENU_PLACEMENT_GLOBAL_TOP             3
#define MENU_PLACEMENT_GLOBAL_BOTTOM          4
#define MENU_PLACEMENT_TRAY                   5 // obsolete, never used


// ====[ DEFAULT DB VALUES ]==================================================

#define DEFAULT_RELNOTES_VERSION                    0
#define DEFAULT_ICQ_XSTATUS_ENABLED                 1
#define DEFAULT_PLUGIN_SHOW_RELNOTES                1
#define DEFAULT_PLUGIN_SHOW_NONSTANDARD_XSTATUSES   1
#define DEFAULT_PLUGIN_HIDE_CUSTOM_STATUS_UI        0
#define DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION       0
#define DEFAULT_PLUGIN_DELETE_AFTER_IMPORT          0
#define DEFAULT_PLUGIN_MENU_PLACEMENT               MENU_PLACEMENT_MAIN
#define DEFAULT_ITEMS_COUNT                         0
#define DEFAULT_ITEM_ICON                           0
#define DEFAULT_ITEM_IS_FAVOURITE                   0
#define DEFAULT_ALLOW_EXTRA_ICONS                   0
#define DEFAULT_REMEMBER_WINDOW_POSITION            0


// ====[ FUNCTION MACROS ]====================================================

// set which row is selected (cannot be only focused, but fully selected - second param ;))
#define ListView_GetSelectedItemMacro( hwnd )   ListView_GetNextItem( hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED );
#ifndef ListView_SetSelectionMark
  #define ListView_SetSelectionMark( x, y )     0
#endif

#if defined( _UNICODE )
  #define tcstok( x, y )                    wcstok( x, y )
  #define tcprintf( a, b, c, d )            swprintf( a, b, c, d )
  #define tcslen( x )                       wcslen( x )
  #define tsscanf( a, b, c, d, e, f )       swscanf( a, b, &c, &d, &e, &f )
  #define ttoi( a )                         _wtoi( a )
  #define tcslwr( x )                       wcslwr( x )
#else
  #define tcstok( x, y )                    strtok( x, y )
  #define tcprintf( a, b, c, d )            sprintf( a, c, d )
  #define tcslen( x )                       strlen( x )
  #define tsscanf( a, b, c, d, e, f )       sscanf( a, b, c, d, e, f )
  #define ttoi( a )                         atoi( a )
  #define tcslwr( x )                       strlwr( x )
#endif
#define getByte( setting, error )           DBGetContactSettingByte( NULL, PLUGIN_SHORT_NAME, setting, error )
#define setByte( setting, value )           DBWriteContactSettingByte( NULL, PLUGIN_SHORT_NAME, setting, value )
#define getWord( setting, error )           DBGetContactSettingWord( NULL, PLUGIN_SHORT_NAME, setting, error )
#define setWord( setting, value )           DBWriteContactSettingWord( NULL, PLUGIN_SHORT_NAME, setting, value )
#define getDword( setting, error )          DBGetContactSettingDword( NULL, PLUGIN_SHORT_NAME, setting, error )
#define setDword( setting, value )          DBWriteContactSettingDword( NULL, PLUGIN_SHORT_NAME, setting, value )
#define getTString( setting, dest )         DBGetContactSettingTString( NULL, PLUGIN_SHORT_NAME, setting, dest )
#define setTString( setting, value )        DBWriteContactSettingTString( NULL, PLUGIN_SHORT_NAME, setting, value )
#define deleteSetting( setting )            DBDeleteContactSetting( NULL, PLUGIN_SHORT_NAME, setting )

// --
typedef void (__cdecl *pForAllProtosFunc)( char*, void* );


// ====[ STRUCTURES ]=========================================================

struct StatusItem { // list item structure
	int     iIcon;
	TCHAR   tszTitle[XSTATUS_TITLE_LIMIT];
	TCHAR   tszMessage[XSTATUS_MESSAGE_LIMIT];
	BOOL    bFavourite;

	StatusItem( )
	{
		this->iIcon = 0;
		lstrcpy( this->tszTitle, TEXT( "" ) );
		lstrcpy( this->tszMessage, TEXT( "" ) );
		this->bFavourite = FALSE;
	}

	StatusItem( const StatusItem& p )
	{
		this->iIcon = p.iIcon;
		lstrcpy( this->tszTitle, p.tszTitle );
		lstrcpy( this->tszMessage, p.tszMessage );
		this->bFavourite = p.bFavourite;
	}

	~StatusItem( )
	{
	}
};

static struct CSForm { // icons + buttons
	int     idc;
	TCHAR*  ptszTitle;
	TCHAR*  ptszDescr;
	char*   pszIconIcoLib;
	int     iconNoIcoLib;
	HANDLE  hIcoLibItem;

} forms[] = {

	{ -1, LPGENT( "Main Menu" ), LPGENT( "Main Icon" ), "icon", IDI_CSLIST, NULL },
//?	{ -1, LPGENT( "Import statuses from database" ), LPGENT( "Import" ), "import", IDI_IMPORT, NULL },
	{ IDC_ADD, LPGENT( "Add new item" ), LPGENT( "Add" ), "add", IDI_ADD, NULL },
	{ IDC_MODIFY, LPGENT( "Modify selected item" ), LPGENT( "Modify" ), "modify", IDI_MODIFY, NULL },
	{ IDC_REMOVE, LPGENT( "Delete selected item" ), LPGENT( "Remove" ), "remove", IDI_REMOVE, NULL },
	{ IDC_FAVOURITE, LPGENT( "Set/unset current item as favorite" ), LPGENT( "Favourite" ), "favourite", IDI_FAVOURITE, NULL },
	{ IDC_UNDO, LPGENT( "Undo changes" ), LPGENT( "Undo changes" ), "undo", IDI_UNDO, NULL },
	{ IDC_IMPORT, LPGENT( "Import statuses from database" ), LPGENT( "Import" ), "import", IDI_IMPORT, NULL },
	{ IDC_FILTER, LPGENT( "Filter list" ), LPGENT( "Filter" ), "filter", IDI_FILTER, NULL },
	{ IDCLOSE, LPGENT( "Close without changing custom status" ), LPGENT( "No change" ), "nochng", IDI_CLOSE, NULL },
	{ IDCANCEL, LPGENT( "Clear custom status (reset to None) and close" ), LPGENT( "Clear" ), "clear", IDI_UNSET, NULL },
	{ IDOK, LPGENT( "Set custom status to selected one and close" ), LPGENT( "Set" ), "apply", IDI_APPLY, NULL },
};

static struct CSXstatus { // combobox custom icons
	int iId;
	TCHAR* ptszTitle;

} xstatus_names[] = {

	{ 1,  LPGENT( "Angry" ) },
	{ 2,  LPGENT( "Taking a bath" ) },
	{ 3,  LPGENT( "Tired" ) },
	{ 4,  LPGENT( "Party" ) },
	{ 5,  LPGENT( "Drinking beer" ) },
	{ 6,  LPGENT( "Thinking" ) },
	{ 7,  LPGENT( "Eating" ) },
	{ 8,  LPGENT( "Watching TV" ) },
	{ 9,  LPGENT( "Meeting" ) },
	{ 10, LPGENT( "Coffee" ) },
	{ 11, LPGENT( "Listening to music" ) },
	{ 12, LPGENT( "Business" ) },
	{ 13, LPGENT( "Shooting" ) },
	{ 14, LPGENT( "Having fun" ) },
	{ 15, LPGENT( "On the phone" ) },
	{ 16, LPGENT( "Gaming" ) },
	{ 17, LPGENT( "Studying" ) },
	{ 18, LPGENT( "Shopping" ) },
	{ 19, LPGENT( "Feeling sick" ) },
	{ 20, LPGENT( "Sleeping" ) },
	{ 21, LPGENT( "Surfing" ) },
	{ 22, LPGENT( "Browsing" ) },
	{ 23, LPGENT( "Working" ) },
	{ 24, LPGENT( "Typing" ) },
	{ 25, LPGENT( "Picnic" ) },
	{ 26, LPGENT( "Cooking" ) },
	{ 27, LPGENT( "Smoking" ) },
	{ 28, LPGENT( "I'm High" ) },
	{ 29, LPGENT( "On WC" ) },
	{ 30, LPGENT( "To be or not to be" ) },
	{ 31, LPGENT( "Watching pro7 on TV" ) },
	{ 32, LPGENT( "Love" ) },
	{ 33, LPGENT( "Searching" ) },
	{ 34, LPGENT( "Love" ) },       // I don't understand why this falls when 2 same named items appear O_o
	{ 35, LPGENT( "Journal" ) },    // edit: ..and now I REALLY don't understand why it's working now x))
	{ 36, LPGENT( "Sex" ) },
	{ 37, LPGENT( "Smoking" ) },    // -||-
};


// ====[ MY BITCHY LIST IMPLEMENTATION x)) ]==================================
// TODO: Optimize it all x))

template< class T > struct ListItem
{
	T* item;
	ListItem* next;
	
	ListItem( )
	{
		this->item = NULL;
		this->next = NULL;
	}

	~ListItem( )
	{
		delete this->item;
	}
	
	ListItem( StatusItem* si )
	{
		this->item = si;
		this->next = NULL;
	}

	ListItem( const ListItem& p )
	{
		this->item = p.item;
		this->next = NULL;
	}
};


template< class T > struct List
{
private:
	ListItem< T >* items;
	unsigned int count;
	
public:
	typedef int ( *compareFunc )( const T* p1, const T* p2 );
	compareFunc compare;

	List( compareFunc compFnc )
	{
		this->items = NULL;
		this->count = 0;
		this->compare = compFnc;
	}

	~List( )
	{
		this->destroy( );
	}

	ListItem< T >* getListHead( )
	{
		return items;
	}

	unsigned int getCount( )
	{
		return count;
	}

	int add( T* csi )
	{
		int position = 0;
		ListItem< T >* item = new ListItem< T >( csi );
		if ( this->items == NULL )
			this->items = item;
		else
		{
			ListItem< T >* help = item;
			item->next = items;
			while ( help->next != NULL )
			{
				int cmp = compare( item->item, help->next->item );
			    if ( cmp == 1 )
					help = help->next;
				else if ( cmp == 0 )
				{
					delete item;
					return -1;
				}
				else
				    break;
				position++;
			}
			if ( help != item )
			{
				item->next = help->next;
				help->next = item;
			}
			else
			    items = item;
		}
		this->count++;
		return position;
	}

	void remove( const unsigned int item )
	{
		if ( item < 0 || item >= this->count )
			return;

		ListItem< T >* help = items;
		ListItem< T >* removed;
		if ( item == 0 )
		{
			items = items->next;
			removed = help;
		}
		else
		{
			for ( unsigned int i = 0; i < item - 1; i++ )
				help = help->next;
			removed = help->next;
			help->next = help->next->next;
		}
		delete removed;
		this->count--;
	}
	
	T* get( const unsigned int item )
	{
		ListItem< T >* help = items;
		for ( unsigned int i = 0; i < item; i++ )
			help = help->next;
		return help->item;
	}

	T* operator[]( const unsigned int item )
	{
		return get( item );
	}

	void destroy( )
	{
		while ( this->count > 0 )
			this->remove( 0 );
	}
};


// ====[ CLASSING -- FUNNY STUFF :) ]=========================================

struct CSList;
struct CSWindow;
struct CSAMWindow;
struct CSItemsList;
struct CSListView;

// --------

struct CSListView
{
	HWND        handle;
	CSWindow*   parent;

	CSListView( HWND, CSWindow* );

	void    addItem( StatusItem* item, int itemNumber );
	void    initItems( ListItem< StatusItem >* items );
	void    reinitItems( ListItem< StatusItem >* items );
	void    removeItems( );
	int     getPositionInList( );
	void    setFullFocusedSelection( int selection );
};


struct CSItemsList
{
	List< StatusItem >* list;

	static int compareItems( const StatusItem* p1, const StatusItem* p2 );
	void loadItems( );
	void saveItems( );

	CSItemsList( );
	~CSItemsList( );
};


struct CSAMWindow
{
	WORD        action;
	StatusItem* item;
	CSWindow*   parent;
	BOOL        bChanged;

	HWND        handle;
	HWND        hCombo;
	HWND        hTitle;
	HWND        hMessage;

	CSAMWindow( WORD action, CSWindow* parent );
	~CSAMWindow( );

	void    exec( );
	void    setCombo( );
	void    fillDialog( );
	void    switchTitle( );
	void    checkFieldLimit( WORD action, WORD item );
	void    checkItemValidity( );
};


struct CSWindow
{
	HWND            handle;
	BOOL            bExtraIcons;
	CSItemsList*    itemslist;
	CSListView*     listview;
	CSAMWindow*     addModifyDlg;
	HIMAGELIST      icons;
	BOOL            bSomethingChanged;
	TCHAR*          filterString;

	CSWindow( );
	~CSWindow( );

	void    initIcons( );
	void    deinitIcons( );
	void    initButtons( );
	static void __cdecl showWindow( void* arg );
	static void __cdecl closeWindow( void* arg );
	void    setForeground( );
	void    loadWindowPosition( );
	BOOL    toggleButtons( );
	void    toggleEmptyListMessage( );
	void    toggleFilter( );
	BOOL    itemPassedFilter( ListItem< StatusItem >* li );

	void __inline saveWindowPosition( HWND hwnd )
	{
		if ( getByte( "RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION ) == TRUE )
		{
			RECT rect = { 0 };
			GetWindowRect( hwnd, &rect );
			setWord( "PositionX", rect.left );
			setWord( "PositionY", rect.top );
		}
	}
};


struct CSList
{
	// global variables
	static HINSTANCE    handle;
	static DWORD        dwMirandaVersion;
	static BOOL         bUnicodeCore;
	static BOOL         bAccountsSupported;

	// class components
	CSWindow*   mainWindow;

	// events
	HANDLE      hHookOnPluginsLoaded;
	HANDLE      hHookOnOptionsInit;
	HANDLE      hHookOnDBSettingChanged;
	HANDLE      hHookOnStatusMenuBuild;

	// services
	HANDLE      hServiceShowList;

	// other handlers
	static HANDLE   hToolbarIcon;
	static HANDLE   hAnyMenuItem;

	CSList( );
	~CSList( );

	// event functions
	static int  postLoading( WPARAM, LPARAM );
	static int  initOptions( WPARAM, LPARAM );
	static int  respondDBChange( WPARAM, LPARAM );
	static int  createMenuItems( WPARAM, LPARAM );

	// service functions
	static INT_PTR __cdecl  showList( WPARAM, LPARAM );
	void    closeList( HWND );

	// protocols enumeration related functions
	static void ForAllProtocols( pForAllProtosFunc pFunc, void* arg );
	static void __cdecl addProtoStatusMenuItem( char* protoName, void* arg );
	static void __cdecl countICQProtos( char* protoName, void* arg );
	static void __cdecl countPlusModProtos( char* protoName, void* arg );
	static void __cdecl getFirstProto( char* protoName, void* arg );
	static void __cdecl setStatusOnAllProtos( char* protoName, void* arg );
	static void __cdecl toggleCustomStatusUIOnAllProtos( char* protoName, void* arg );
	static void __cdecl importCustomStatusUIStatusesFromAllProtos( char* protoName, void* arg );

	// other functions
	void    initIcoLib( );
	void    registerUpdater( );
	void    registerHotkeys( );
	void    registerToolbar( );
	void    rebuildMenuItems( );
	void    setStatus( WORD code, StatusItem* item );
	static void toggleCustomStatusUI( BOOL enabled );
	static void __cdecl showReleaseNotes( void* arg );
};

HINSTANCE CSList::handle = NULL;
DWORD CSList::dwMirandaVersion = 0x00000000;
BOOL CSList::bUnicodeCore = FALSE;
BOOL CSList::bAccountsSupported = FALSE;
HANDLE CSList::hToolbarIcon = NULL;
HANDLE CSList::hAnyMenuItem = NULL;


// ====[ GLOBALS ]============================================================

static const MUUID interfaces[] = { PLUGIN_GUUID, MIID_LAST };

PLUGINLINK*  pluginLink = NULL;
PLUGININFOEX pluginInfoEx = {
	sizeof( PLUGININFOEX ),
	PLUGIN_LONG_NAME,
	PLUGIN_MAKE_VERSION( MAJOR_VERSION,
	                     MINOR_VERSION,
	                     RELEASE_VERSION,
	                     BUILD_VERSION ),
	PLUGIN_INFO_ABOUT,
	PLUGIN_INFO_AUTHOR,
	PLUGIN_INFO_EMAIL,
	PLUGIN_INFO_COPYRIGHT,
	PLUGIN_INFO_HOMEPAGE,
	UNICODE_AWARE,  // Not transient
	0,  // Doesn't replace anything built-in
	// Generate your own unique id for your plugin.
	// Do not use this UUID!
	// Use uuidgen.exe to generate the uuuid
	PLUGIN_GUUID
};

PLUGININFO pluginInfo = {
	sizeof( PLUGININFO ),
	PLUGIN_LONG_NAME,
	PLUGIN_MAKE_VERSION( MAJOR_VERSION,
	                     MINOR_VERSION,
	                     RELEASE_VERSION,
	                     BUILD_VERSION ),
	PLUGIN_INFO_ABOUT,
	PLUGIN_INFO_AUTHOR,
	PLUGIN_INFO_EMAIL,
	PLUGIN_INFO_COPYRIGHT,
	PLUGIN_INFO_HOMEPAGE,
	UNICODE_AWARE,  // Not transient
	0,  // Doesn't replace anything built-in
};

CSList* cslist = NULL;


// ====[ INIT STUFF ]=========================================================

BOOL WINAPI DllMain( HINSTANCE, DWORD, LPVOID );

extern "C" __declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD );
extern "C" __declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD );
extern "C" __declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void );

extern "C" __declspec( dllexport ) int Load( PLUGINLINK* );
extern "C" __declspec( dllexport ) int Unload( void );


// ====[ THREAD FORK ]========================================================

void ForkThread( pThreadFunc pFunc, void* arg );


// ====[ PROCEDURES ]=========================================================

INT_PTR CALLBACK CSWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
INT_PTR CALLBACK CSAMWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
INT_PTR CALLBACK CSRNWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
INT_PTR CALLBACK CSOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );


// ====[ HELPERS ]============================================================

HICON LoadIconExEx( const char*, int );
void translateDialog( HWND );


void __fastcall SAFE_FREE(void** p)
{
	if (*p)
	{
		free(*p);
		*p = NULL;
	}
}


#endif /* __CSLIST_H */
