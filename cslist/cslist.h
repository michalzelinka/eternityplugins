// ############################ INCLUDES & DEFINITIONS #########################

#ifndef __CSLIST_H
#define __CSLIST_H 1

#define _WIN32_WINNT  0x501
#define MIRANDA_VER   0x403

#pragma warning( disable: 4996 )
#pragma comment( lib, "comctl32.lib" )

#if defined( UNICODE ) && !defined( _UNICODE )
	#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#if defined( _UNICODE )
  #include <tchar.h>
  #include <win2k.h>
#endif

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_protosvc.h>
#include <m_icq.h>
#include <m_database.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_variables.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_options.h>
#include <m_toolbar.h>
#include <m_hotkeys.h>
#include <m_protocols.h>
#include <m_genmenu.h>

// ----[ basic definitions ]----

#include "resource.h"

#define CSLIST_MODULE_LONG_NAME       "Custom Status List"
#define CSLIST_MODULE_SHORT_NAME      "CSList"

#define CSLIST_GUUID                  { 0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 } }
                                      //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}

// ----[ versioning ]----

#define CSLIST_MAJOR_VERSION          0
#define CSLIST_MINOR_VERSION          0
#define CSLIST_RELEASE_VERSION        0
#define CSLIST_BUILD_VERSION          20
#define CSLIST_TESTING_VERSION        1

#define CSLIST_MAJOR_VERSION_T        "0"
#define CSLIST_MINOR_VERSION_T        "0"
#define CSLIST_RELEASE_VERSION_T      "0"
#define CSLIST_BUILD_VERSION_T        "20"
#define CSLIST_TESTING_VERSION_T      "1"

#define FINAL_BUILD

#define CSLIST_RELNOTES               ( CSLIST_BUILD_VERSION * 100 + CSLIST_TESTING_VERSION )
                                      // actual eternity relnotes

#if defined( FINAL_BUILD )
#define CSLIST_VERSION                "0.0.0.20"
#else
#define CSLIST_VERSION                "0.0.0.20 (alpha)"
#endif

#if defined( FINAL_BUILD )
  #define CSLIST_VER_PRINT            ""
#elif defined( DEBUG )
  #define CSLIST_VER_PRINT            " [debug build]"
#else
  #define CSLIST_VER_PRINT            " [technology build #"CSLIST_TESTING_VERSION_T" "__DATE__"]"
#endif

// ----[ plugin info ]----

#define CSLIST_PLUGIN_INFO_ABOUT      "Keep your ICQ Custom statuses in an integrated, easy to use manager. " \
                                      "This plugin offers simple management functions to keep your extra " \
                                      "statuses on one place."CSLIST_VER_PRINT
#define CSLIST_PLUGIN_INFO_AUTHOR     "jarvis [#eThEreAL] .., HANAX"
#define CSLIST_PLUGIN_INFO_EMAIL      "jarvis@jabber.cz"
#define CSLIST_PLUGIN_INFO_COPYRIGHT  "© 2007-2008 eternity crew .., © 2006-2008 HANAX Software"
#define CSLIST_PLUGIN_INFO_HOMEPAGE   "http://dev.mirandaim.ru/~jarvis/"

// ----[ limits ]----

#define CSLIST_XTITLE_LIMIT           64   // limit of chars for x-status title
#define CSLIST_XMESSAGE_LIMIT         2048 // limit of chars for x-status message

// ----[ strings (all which should be translated in LANGPACKs) ]----

#define CSLIST_STRING_CUSTOM_STATUS_LIST      "Custom Status List"
#define CSLIST_STRING_SHOW_CSLIST             "Show Custom Status List"
#define CSLIST_STRING_CUSTOM_STATUS           "Custom status"
#define CSLIST_STRING_TITLE                   "Title"
#define CSLIST_STRING_MESSAGE                 "Message"
#define CSLIST_STRING_CANCEL                  "Cancel"
#define CSLIST_STRING_ICON                    "Icon"
#define CSLIST_STRING_FIELD_STRING_LIMIT      "This field doesn't accept string longer than %d characters. The string will be truncated."
#define CSLIST_STRING_WARNING                 "Warning"
#define CSLIST_STRING_INPORT_DELETE_CONFIRM   "Do you want to delete old DB entries after Import?"
#define CSLIST_STRING_ITEM_DELETE_CONFIRM     "Do you really want to delete selected item?"
#define CSLIST_STRING_MAIN_MENU               "Main Menu"
#define CSLIST_STRING_MAIN_ICON               "Main Icon"
#define CSLIST_STRING_ADD_NEW_ITEM            "Add new item"
#define CSLIST_STRING_ADD                     "Add"
#define CSLIST_STRING_MODIFY_ITEM             "Modify selected item"
#define CSLIST_STRING_MODIFY                  "Modify"
#define CSLIST_STRING_REMOVE_ITEM             "Delete selected item"
#define CSLIST_STRING_REMOVE                  "Remove"
#define CSLIST_STRING_IMPORT_FROM_DB          "Import statuses from database" 
#define CSLIST_STRING_IMPORT                  "Import"
#define CSLIST_STRING_GLOBAL_STATUS_SET       "Set the desired status globally on all ICQ accounts" 
#define CSLIST_STRING_GLOBAL                  "Global"
#define CSLIST_STRING_SET_UNSET_FAVOURITE     "Set/unset current item as favorite" 
#define CSLIST_STRING_FAVOURITE               "Favorite"
#define CSLIST_STRING_OPTIONS                 "Options"
#define CSLIST_STRING_REFRESH                 "Refresh"
#define CSLIST_STRING_UNDO                    "Undo changes"
#define CSLIST_STRING_CLOSE_WITHOUT_CHANGE    "Close without changing custom status"
#define CSLIST_STRING_NO_CHANGE               "No change"
#define CSLIST_STRING_SET_AND_CLOSE           "Set custom status to selected one and close" 
#define CSLIST_STRING_SET                     "Set"
#define CSLIST_STRING_CLEAR_AND_CLOSE         "Clear custom status (reset to None) and close"
#define CSLIST_STRING_CLEAR                   "Clear"

// ----[ updater strings ]----

#define CSLIST_UPD_VERURL             "http://dev.mirandaim.ru/~jarvis/"
#define CSLIST_UPD_UPDURL             "http://mirandapack.ic.cz/eternity_plugins/cslist.zip"
#define CSLIST_UPD_FLVERURL           "http://addons.miranda-im.org/details.php?action=viewfile&id=3483"
#define CSLIST_UPD_FLUPDURL           "http://addons.miranda-im.org/feed.php?dlfile=3483"
#define CSLIST_UPD_SZPREFIX           "Custom Status List</a> "

// ----[ services ]----

#define MS_CSLIST_SHOWLIST						  "CSList/ShowList"
#define MS_PSEUDO_PROTO_PROTOACCSENUM   ( gbAccountsSupported ? MS_PROTO_ENUMACCOUNTS : MS_PROTO_ENUMPROTOCOLS )

// ----[ menu placement IDs ]----

#define CSLIST_MENU_PLACEMENT_DEFAULT         3
#define CSLIST_MENU_PLACEMENT_GLOBAL          1
#define CSLIST_MENU_PLACEMENT_GLOBAL_TOP      11
#define CSLIST_MENU_PLACEMENT_GLOBAL_BOTTOM   12
#define CSLIST_MENU_PLACEMENT_ICQ             2
#define CSLIST_MENU_PLACEMENT_MAIN            3
#define CSLIST_MENU_PLACEMENT_TRAY            4

// ----[ default DB values ]----

#define ICQ_XSTATUS_ENABLED_DEFAULT           1
#define CSLIST_SHOW_RELNOTES_DEFAULT          1
#define CSLIST_SHOW_NONSTANDARD_XSTATUSES_DEFAULT     1
#define CSLIST_HIDE_CSUI_DEFAULT              0
#define CSLIST_ITEMS_DELETION_DEFAULT         0
#define CSLIST_IMPORT_DELETION_DEFAULT        0

// ----[ function macros ]----

#if defined( _UNICODE )
  #define tcstok( x, y )            wcstok( x, y )
  #define tcprintf( a, b, c, d )    swprintf( a, b, c, d )
  #define tcslen( x )               wcslen( x )
  #define tsscanf( a, b, c, d, e, f ) \
                                    swscanf( a, b, &c, &d, &e, &f )
  #define ttoi( a )                 _wtoi( a )
#else
  #define tcstok( x, y )            strtok( x, y )
  #define tcprintf( a, b, c, d )    sprintf( a, c, d )
  #define tcslen( x )               strlen( x )
  #define tsscanf( a, b, c, d, e, f ) \
                                    sscanf( a, b, c, d, e, f )
  #define ttoi( a )                 atoi( a )
#endif
#define DBReadByte( b, c, d )       DBGetContactSettingByte( NULL, b, c, d )
#define DBWriteByte( b, c, d )      DBWriteContactSettingByte( NULL, b, c, d )
#define DBReadWord( b, c, d )       DBGetContactSettingWord( NULL, b, c, d )
#define DBWriteWord( b, c, d )      DBWriteContactSettingWord( NULL, b, c, d )
#define DBReadDword( b, c, d )      DBGetContactSettingDword( NULL, b, c, d )
#define DBWriteDword( b, c, d )     DBWriteContactSettingDword( NULL, b, c, d )
#define DBReadTString( b, c, d )    DBGetContactSettingTString( NULL, b, c, d )
#define DBWriteTString( b, c, d )   DBWriteContactSettingTString( NULL, b, c, d )
#define DBDelete( b, c )            DBDeleteContactSetting( NULL, b, c )

#ifndef WIN2K_H__
#define SIZEOF( x )                 ( sizeof( x ) / sizeof( x[0] ) )
#define IsWinVerXPPlus( )           ( LOBYTE( LOWORD( GetVersion( ) ) ) >= 5 && LOWORD( GetVersion( ) ) != 5 )
#endif


// ################################## STRUCTURES ###############################

typedef struct { // list item structure
  int   ItemIcon;
  TCHAR ItemTitle[CSLIST_XTITLE_LIMIT];
  TCHAR ItemMessage[CSLIST_XMESSAGE_LIMIT];
  BOOL  ItemFavourite;
  // struct CSLISTSTATUSITEM* next; // future: pointer to next item in backup pseudo-list
} CSLISTSTATUSITEM;

static struct { // icons + buttons
  int    idc;
  TCHAR* ptszTitle;
  TCHAR* ptszDescr;
  char*  pszIconIcoLib;
  int    iconNoIcoLib;
  HANDLE hIcoLibItem;
} cslforms[] =
{
  { -1, LPGENT( CSLIST_STRING_MAIN_MENU ), LPGENT( CSLIST_STRING_MAIN_ICON ), "icon", IDI_CSLIST, NULL },
  { IDC_ADD, LPGENT( CSLIST_STRING_ADD_NEW_ITEM ), LPGENT( CSLIST_STRING_ADD ), "add", IDI_ADD, NULL },
	{ IDC_MODIFY, LPGENT( CSLIST_STRING_MODIFY_ITEM ), LPGENT( CSLIST_STRING_MODIFY ), "modify", IDI_MODIFY, NULL },
	{ IDC_REMOVE, LPGENT( CSLIST_STRING_REMOVE_ITEM ), LPGENT( CSLIST_STRING_REMOVE ), "remove", IDI_REMOVE, NULL },
//  { IDC_FAV, LPGENT( CSLIST_STRING_SET_UNSET_FAVOURITE ), LPGENT( CSLIST_STRING_FAVOURITE ), "favourite", IDI_FAV, NULL },
  { IDC_IMPORT, LPGENT( CSLIST_STRING_IMPORT_FROM_DB ), LPGENT( CSLIST_STRING_IMPORT ), "import", IDI_IMPORT, NULL },
  { IDC_UNDO, LPGENT( CSLIST_STRING_UNDO ), LPGENT( CSLIST_STRING_UNDO ), "undo", IDI_UNDO, NULL },
//  { IDC_GLOBAL, LPGENT( CSLIST_STRING_GLOBAL_STATUS_SET ), LPGENT( CSLIST_STRING_GLOBAL ), "global", IDI_GLOBAL, NULL },
  { IDC_NOCHNG, LPGENT( CSLIST_STRING_CLOSE_WITHOUT_CHANGE ), LPGENT( CSLIST_STRING_NO_CHANGE ), "nochng", IDI_NOCHNG, NULL },
  { IDC_APPLY, LPGENT( CSLIST_STRING_SET_AND_CLOSE ), LPGENT( CSLIST_STRING_SET ), "apply", IDI_APPLY, NULL },
  { IDC_EXIT, LPGENT( CSLIST_STRING_CLEAR_AND_CLOSE ), LPGENT( CSLIST_STRING_CLEAR ), "clear", IDI_CLEAR, NULL },
};

static struct { // combobox custom icons
  int xnum;
	TCHAR *xname;
} xstatuses[] =
{
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

static struct { // list sorting structure
  int columnIndex;
  HWND listView;
  BOOL isOrderDesc;
} sorting = {
  0, NULL, FALSE
};

// ################################## GLOBALS ##################################

HINSTANCE hInst = NULL;
PLUGINLINK *pluginLink = NULL;

DWORD gMirandaVersion = 0x00000000;
BYTE gbUnicodeCore = 0;
BOOL gbAccountsSupported = FALSE;

int action = 0;
int AMResult = 0;
int ModifiedPos = -1;

static HWND hDlg = NULL;  // Main Dialog handler
static HWND hList = NULL;  // List View handler
static HANDLE hMainMenuItem = NULL;
HIMAGELIST hIml = NULL;
HWND hXCombo = NULL;
int hMainIcon = 0; // modern toolbar // TODO: is it needed?
LVCOLUMN LvCol = { 0 }; // Make Coluom struct for ListView
LVITEM LvItem = { 0 };  // ListView Item struct
COMBOBOXEXITEM CbItem = { 0 };
int iSelect = 0;
int flag = 0;
BOOL bChanged = 0;
BOOL gbSetStatusGlobally = FALSE;
CSLISTSTATUSITEM helpItem = { 0 };

// hooks
HANDLE hHookMenuBuild = NULL;
HANDLE hHookOnPluginsLoaded = NULL;
HANDLE hHookOnOptionsInit = NULL;
HANDLE hHookOnDBSettingChanged = NULL;

// services
HANDLE hSvcShowList = NULL;

BOOL bStatusMenu = FALSE;

static const MUUID interfaces[] = { MIID_TESTPLUGIN, CSLIST_GUUID, MIID_LAST };

PLUGININFOEX pluginInfoEx = {
  sizeof( PLUGININFOEX ),
  CSLIST_MODULE_LONG_NAME,
  PLUGIN_MAKE_VERSION( CSLIST_MAJOR_VERSION,
                       CSLIST_MINOR_VERSION,
                       CSLIST_RELEASE_VERSION,
                       CSLIST_BUILD_VERSION
  ),
  CSLIST_PLUGIN_INFO_ABOUT,
  CSLIST_PLUGIN_INFO_AUTHOR,
  CSLIST_PLUGIN_INFO_EMAIL,
  CSLIST_PLUGIN_INFO_COPYRIGHT,
  CSLIST_PLUGIN_INFO_HOMEPAGE,
  UNICODE_AWARE,  //not transient
  0,  //doesn't replace anything built-in
  // Generate your own unique id for your plugin.
  // Do not use this UUID!
  // Use uuidgen.exe to generate the uuuid
  CSLIST_GUUID
};

PLUGININFO pluginInfo = {
  sizeof( PLUGININFO ),
  CSLIST_MODULE_LONG_NAME,
  PLUGIN_MAKE_VERSION( CSLIST_MAJOR_VERSION,
                       CSLIST_MINOR_VERSION,
                       CSLIST_RELEASE_VERSION,
                       CSLIST_BUILD_VERSION
  ),
  CSLIST_PLUGIN_INFO_ABOUT,
  CSLIST_PLUGIN_INFO_AUTHOR,
  CSLIST_PLUGIN_INFO_EMAIL,
  CSLIST_PLUGIN_INFO_COPYRIGHT,
  CSLIST_PLUGIN_INFO_HOMEPAGE,
  UNICODE_AWARE,  //not transient
  0,  //doesn't replace anything built-in
};

// ############################# INIT STUFF ####################################

BOOL WINAPI DllMain( HINSTANCE, DWORD, LPVOID );
static int PluginMenuCommand( WPARAM, LPARAM );

__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD );
__declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD );
__declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void );

int __declspec( dllexport ) Load( PLUGINLINK* );
int __declspec( dllexport ) Unload( void );
static int onPluginsLoaded( WPARAM, LPARAM );
int onOptionsInit( WPARAM, LPARAM );

// ############################### PROCEDURES ##################################

INT_PTR CALLBACK CSListProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK CSListAddModifyProc( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK CSListOptionsProc( HWND, UINT, WPARAM, LPARAM );
static int CALLBACK CSListSortProc( LPARAM, LPARAM, LPARAM );
INT_PTR CALLBACK RelNotesProc( HWND, UINT, WPARAM, LPARAM );

// ############################# HELP FUNCTIONS ################################

static int cslist_init_menu_items( WPARAM, LPARAM );

// ################## COMMANDS OF CSLIST #######################################

int cslist_add_item();
int cslist_modify_item();
int cslist_remove_item();
int cslist_AM_set_help_item( HWND );

// ################## SERVICES FOR CSLIST ######################################

void cslist_clear_selection();
void cslist_clear_help_item();
void cslist_reset_status_menu();
void cslist_sort_list();
void cslist_import_statuses_from_icq();
int cslist_setting_changed( WPARAM, LPARAM );

// ################## DB - LOAD AND SAVE #######################################

int cslist_initialize_list_content( HWND );
int cslist_parse_row( TCHAR* );
int cslist_save_list_content( HWND );

// ######################### CREATING DIALOG ###################################

int cslist_set_status( HWND, WORD );
void cslist_init_icons( void );

// ######################### OTHER FUNCTIONS ###################################

HICON LoadIconExEx( const char*, int );
void cslist_translate_dialog( HWND );

#endif /* __CSLIST_H */
