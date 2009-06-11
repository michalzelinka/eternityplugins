// ############################ INCLUDES & DEFINITIONS #########################

#if defined( UNICODE ) && !defined( _UNICODE )
	#define _UNICODE
#elif !defined( UNICODE )
  #undef _UNICODE
#endif

#include <windows.h>
#include <win2k.h>
#include <windef.h>
#include <commctrl.h>
#include <string.h>
#include <tchar.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_database.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_options.h>
#include <m_toolbar.h>
#include <m_hotkeys.h>
#include <m_protocols.h>
//#include <stdio.h>

#include "resource.h"

#define MIID_STATUSLIST               { 0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 } }

#define CSLIST_RELNOTES               19 // actual eternity relnotes
#define CSLIST_TESTING                "1"
#define CSLIST_XTITLE_LIMIT           64 // limit of chars for x-status title
#define CSLIST_XMESSAGE_LIMIT         2048 // limit of chars for x-status message

#define CSLIST_MODULE_LONG_NAME       "Custom Status List"
#define CSLIST_MODULE_SHORT_NAME      "CSList"

// updater strings

#define CSLIST_UPD_VERURL           "http://dev.mirandaim.ru/~jarvis/"
#define CSLIST_UPD_UPDURL           "http://mirandapack.ic.cz/eternity_plugins/cslist.zip"
#define CSLIST_UPD_FLVERURL         "http://addons.miranda-im.org/details.php?action=viewfile&id=3483"
#define CSLIST_UPD_FLUPDURL         "http://addons.miranda-im.org/feed.php?dlfile=3483"
#define CSLIST_UPD_SZPREFIX         "Custom Status List</a> "

// services

#define MS_CLIST_ADDSTATUSMENUITEM    "CList/AddStatusMenuItem"
#define ME_CLIST_PREBUILDSTATUSMENU   "CList/PreBuildStatusMenu"
#define PS_ICQ_SETCUSTOMSTATUSEX      "ICQ/SetXStatusEx"
#define MS_CSLIST_SHOWLIST            "CSList/ShowList"

// status flags

#define CSSF_MASK_STATUS      0x0001  // status member valid for set/get
#define CSSF_MASK_NAME        0x0002  // pszName member valid for set/get
#define CSSF_MASK_MESSAGE     0x0004  // pszMessage member valid for set/get
#define CSSF_DISABLE_UI       0x0040  // disable default custom status UI, wParam = bEnable
#define CSSF_DEFAULT_NAME     0x0080  // only with CSSF_MASK_NAME and get API to get default custom status name (wParam = status)
#define CSSF_STATUSES_COUNT   0x0100  // returns number of custom statuses in wParam, only get API
#define CSSF_STR_SIZES        0x0200  // returns sizes of custom status name & message (wParam & lParam members) in chars
#define CSSF_UNICODE          0x1000  // strings are in Unicode


// ################################## STRUCTURES ###############################

// -------------------------- WORKING LIST ITEM STRUCTURE ----------------------

typedef struct {
  int  ItemIcon;
  TCHAR ItemTitle[CSLIST_XTITLE_LIMIT];
  TCHAR ItemMessage[CSLIST_XMESSAGE_LIMIT];
  BOOL ItemFavourite;
} CSLISTSTATUSITEM;


// -------------------------- ICQ CUSTOM STATUS STRUCTURE ----------------------

typedef struct {
  int cbSize;         // size of the structure
  int flags;          // combination of CSSF_*
  int *status;        // custom status id
  union {
    char *pszName;    // buffer for custom status name
    TCHAR *ptszName;
    WCHAR *pwszName;
  };
  union {
    char *pszMessage; // buffer for custom status message
    TCHAR *ptszMessage;
    WCHAR *pwszMessage;
  };
  WPARAM *wParam;     // extra params, see flags
  LPARAM *lParam;
} ICQ_CUSTOM_STATUS;


// ------------------------------ ICONS + BUTTONS ------------------------------

static struct {
  int idc;
  TCHAR *title;
  TCHAR *szDescr;
  char *iconIcoLib;
  int iconNoIcoLib;
  HANDLE hIcoLibItem;
} cslforms[] =
{
  { -1, L"Main Menu" , ( TCHAR * )"Main Icon", "csl_icon", IDI_CSLIST, NULL },
  { IDC_ADD, L"Add new item", ( TCHAR * )"Add", "csl_add", IDI_ADD, NULL },
	{ IDC_MODIFY, L"Modify selected item", ( TCHAR * )"Modify", "csl_modify", IDI_MODIFY, NULL },
	{ IDC_REMOVE, L"Delete selected item", ( TCHAR * )"Remove", "csl_remove", IDI_REMOVE, NULL },
  { IDC_IMPORT, L"Import statuses from database", ( TCHAR * )"Import", "csl_import", IDI_IMPORT, NULL },
  { IDC_WATCH, L"Whether or not watch custom status changes and add them into list", ( TCHAR * )"Watch", "csl_watch", IDI_WATCH, NULL },
  { IDC_FAV, L"Set/unset current item as favorite", ( TCHAR * )"Favorite", "csl_fav", IDI_FAV, NULL },
  { IDC_OPTS, L"Options...", ( TCHAR * )"Options", "csl_opts", IDI_OPTIONS, NULL },
  { IDC_NOCHNG, L"Close without changing custom status", ( TCHAR * )"No change", "csl_nochng", IDI_NOCHNG, NULL },
  { IDC_APPLY, L"Set custom status to selected one and close", ( TCHAR * )"Set", "csl_apply", IDI_APPLY, NULL },
  { IDC_EXIT, L"Clear custom status (reset to None) and close", ( TCHAR * )"Clear", "csl_clear", IDI_CLEAR, NULL },
};


// ----------------------- COMBOBOXEX XICONS STRUCTURE -------------------------

static struct
{
  int xnum;
	TCHAR *xname;
} xstatuses[] =
{
  {1, _T("Angry")},
  {2, _T("Taking a bath")},
  {3, _T("Tired")},
  {4, _T("Party")},
  {5, _T("Drinking beer")},
  {6, _T("Thinking")},
  {7, _T("Eating")},
  {8, _T("Watching TV")},
  {9, _T("Meeting")},
  {10, _T("Coffee")},
  {11, _T("Listening to music")},
  {12, _T("Business")},
  {13, _T("Shooting")},
  {14, _T("Having fun")},
  {15, _T("On the phone")},
  {16, _T("Gaming")},
  {17, _T("Studying")},
  {18, _T("Shopping")},
  {19, _T("Feeling sick")},
  {20, _T("Sleeping")},
  {21, _T("Surfing")},
  {22, _T("Browsing")},
  {23, _T("Working")},
  {24, _T("Typing")},
  {25, _T("Picnic")},
  {26, _T("Cooking")},
  {27, _T("Smoking")},
  {28, _T("I'm High")},
  {29, _T("On WC")},
  {30, _T("To be or not to be")},
  {31, _T("Watching pro7 on TV")},
  {32, _T("Love")},
  {33, _T("Searching")},
  {34, _T("Love")},       // I don't understand why this falls when 2 same named items appear O_o
  {35, _T("Journal")},    // edit: ..and now I REALLY don't understand why it's working now x))
  {36, _T("Sex")},
  {37, _T("Smoking")},    // -||-
};


// ---------------------- LIST SORTING OPTIONS STRUCTURE -----------------------

static struct
{
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
BYTE gbUnicodeCore;

TCHAR *rnthanks = L"induction - for his cool iconset :)\r\nfaith_healer - moral support :]\r\nCriS - project hosting @ http://dev.mirandaim.ru/ \r\nRobyer, kaye_styles, dEMoniZaToR, Drugwash, FREAK_THEMIGHTY - useful hints ;)\r\nplugin users, of course :) for their tolerance x) ;)\r\nMiranda IM Project Team - for their work on the best Instant Messenger I ever known :)";
TCHAR *rnchanges = L"";

int action = 0;
int AMResult = 0;
int ModifiedPos = -1;
int opened = 0;

static HWND hDlg = NULL;  // Main Dialog handler
static HWND hList = NULL;  // List View handler
HIMAGELIST hIml = NULL;
HWND hXCombo = NULL;
int hMainIcon = 0; // modern toolbar // TODO: is it needed?
LVCOLUMN LvCol = { 0 }; // Make Coluom struct for ListView
LVITEM LvItem = { 0 };  // ListView Item struct
COMBOBOXEXITEM CbItem = { 0 };
int iSelect = 0;
int flag = 0;
BOOL bChanged = 0;
CSLISTSTATUSITEM helpItem = { 0 };

// hooks
HANDLE hHookMenuBuild = NULL;
HANDLE hHookOnPluginsLoaded = NULL;
HANDLE hHookOnOptionsInit = NULL;
HANDLE hHookOnKatynkaIsLoggedIn = NULL;

// services
HANDLE hSvcShowList = NULL;

BOOL bStatusMenu = FALSE;

PLUGININFOEX pluginInfoEx = {
  sizeof( PLUGININFOEX ),
  CSLIST_MODULE_LONG_NAME,
  PLUGIN_MAKE_VERSION( 0, 0, 0, 19 ),
  "Offers list of your Custom Statuses. [test build #"CSLIST_TESTING" "__DATE__" "__TIME__"]",
  "jarvis [eThEreAL] .., HANAX",
  "mike.taussick@seznam.cz",
  "© 2007-2008 eternity crew .., © 2006-2007 HANAX Software",
  "http://dev.mirandaim.ru/~jarvis/",
  UNICODE_AWARE,  //not transient
  0,  //doesn't replace anything built-in
  // Generate your own unique id for your plugin.
  // Do not use this UUID!
  // Use uuidgen.exe to generate the uuuid
  {0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 }} //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
};

PLUGININFO pluginInfo = {
  sizeof( PLUGININFO ),
  CSLIST_MODULE_LONG_NAME,
  PLUGIN_MAKE_VERSION( 0, 0, 0, 19 ),
  "Offers list of your Custom Statuses. [test build #"CSLIST_TESTING" "__DATE__" "__TIME__"]",
  "jarvis [eThEreAL] .., HANAX",
  "mike.taussick@seznam.cz",
  "© 2007-2008 eternity crew .., © 2006-2007 HANAX Software",
  "http://dev.mirandaim.ru/~jarvis/",
  UNICODE_AWARE,  //not transient
  0
};

// ############################# INIT STUFF ####################################

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved );
static int PluginMenuCommand( WPARAM wParam, LPARAM lParam );

__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion );
__declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion );

static const MUUID interfaces[] = { MIID_TESTPLUGIN, MIID_STATUSLIST, MIID_LAST };
__declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void );

int __declspec( dllexport ) Load( PLUGINLINK *link );
int __declspec( dllexport ) Unload( void );
static int onPluginsLoaded( WPARAM wparam, LPARAM lparam );
int onOptionsInit( WPARAM wparam, LPARAM lparam );

// ############################### PROCEDURES ##################################

INT_PTR CALLBACK CSListProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK CSListAddModifyProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK CSListOptionsProc( HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam );
static int CALLBACK CSListSortProc( LPARAM, LPARAM, LPARAM );
INT_PTR CALLBACK RelNotesProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

// ############################# HELP FUNCTIONS ################################

int null_snprintf( char*, size_t, const char*, ... );
static int cslist_init_menu_item( WPARAM wParam, LPARAM lParam );

// ################## COMMANDS OF CSLIST #######################################

int cslist_add_item();
int cslist_modify_item();
int cslist_remove_item();
int cslist_AM_set_help_item( HWND hwndAMDlg );

// ################## SERVICES FOR CSLIST ######################################

void cslist_clear_selection();
void cslist_clear_help_item();
void cslist_sort_list();
void cslist_import_statuses_from_icq();
int cslist_KatysEasterEgg( WPARAM, LPARAM );

// ################## DB - LOAD AND SAVE #######################################

int cslist_initialize_list_content( HWND hwndDlg );
int cslist_parse_row( TCHAR *row );
int cslist_save_list_content( HWND hwndDlg );

// ######################### CREATING DIALOG ###################################

int cslist_set_status( HWND hwndDlg );
void cslist_init_icons( void );


// ######################### OTHER FUNCTIONS ###################################

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon );
//void DBDeleteContactSetting( char Module[64], char Setting[64] );

// ############################## EXTERNS ######################################

//extern int mir_free( const int & );
//extern int ImageList_AddIcon_IconLibLoaded( HIMAGELIST hIml, int iconId );

#pragma comment( lib, "comctl32.lib" )
