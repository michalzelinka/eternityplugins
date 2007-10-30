// ############################ INCLUDES & DEFINITIONS #########################

#include <windows.h>
#include <win2k.h>
#include <windef.h>
#include <commctrl.h>
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

#include "resource.h"

#define MIID_STATUSLIST               { 0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 } }

#define CSLIST_RELNOTES               14 // actual eternity relnotes
#define CSLIST_XTITLE_LIMIT           64 // limit of chars for x-status title
#define CSLIST_XMESSAGE_LIMIT         2048 // limit of chars for x-status message

#define CSLIST_MODULE_LONG_NAME       "Custom Status List"
#define CSLIST_MODULE_SHORT_NAME      "CSList"

#define MS_CLIST_ADDSTATUSMENUITEM    "CList/AddStatusMenuItem"
#define ME_CLIST_PREBUILDSTATUSMENU   "CList/PreBuildStatusMenu"
#define PS_ICQ_SETCUSTOMSTATUSEX      "ICQ/SetXStatusEx"
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
  char ItemTitle[CSLIST_XTITLE_LIMIT];
  char ItemMessage[CSLIST_XMESSAGE_LIMIT];
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


// -------------- ICONIZED BUTTONS FILLING STRUCTURE ---------------------------

static struct
{
    char*  szDescr;
    char*  szName;
    int    defIconID;
    HANDLE hIconLibItem;
} iconList[] =
{
  { LPGEN( "Custom Status List Main Icon" ), "csl_icon", IDI_CSLIST },
  { LPGEN( "Add new status" ), "csl_add", IDI_ADD },
  { LPGEN( "Modify selected status" ), "csl_modify", IDI_MODIFY },
  { LPGEN( "Delete selected status" ), "csl_delete", IDI_DELETE },
  { LPGEN( "Set selected and close" ), "csl_apply", IDI_APPLY },
  { LPGEN( "Set None and close" ), "csl_exit", IDI_EXIT },
};

static struct
{
  int idc;
	TCHAR *title;
	char *icon;
  int iconNoIcoLib;
} buttons[] =
{
  { IDC_ADD, _T( "Add new item" ), "csl_add", IDI_ADD },
	{ IDC_MODIFY, _T( "Modify selected item" ), "csl_modify", IDI_MODIFY },
	{ IDC_DELETE, _T( "Delete selected item" ), "csl_delete", IDI_DELETE },
  { IDC_IMPORT, _T( "Import statuses from database" ), "csl_import", NULL },
  { IDC_WATCH, _T( "Watch custom status changes and add them to the list" ), "csl_watch", NULL },
  { IDC_APPLY, _T( "Set custom status to selected one and close" ), "csl_apply", IDI_APPLY },
  { IDC_EXIT, _T( "Clear custom status (reset to None) and close" ), "csl_exit", IDI_EXIT },
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
  {35, _T("Journal")},    // edit: I REALLY don't understand why it's working now x))
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

HINSTANCE hInst;
PLUGINLINK *pluginLink;

char *rnthanks = "induction - for his cool iconset :)\r\nfaith_healer - moral support :]\r\nCriS - project hosting @ http://dev.mirandaim.ru/ \r\nRobyer, kaye_styles, dEMoniZaToR, Drugwash, FREAK_THEMIGHTY - useful hints ;)\r\nplugin users, of course :) for their tolerance x) ;)\r\nMiranda IM Project Team - for their work on the best Instant Messenger I ever known :)";

int action = 0;
int AMResult = 0;
int ModifiedPos = -1;

static HWND hList = NULL;  // List View identifier
HIMAGELIST hIml;
HWND hXCombo = NULL;
LVCOLUMN LvCol; // Make Coluom struct for ListView
LVITEM LvItem;  // ListView Item struct
COMBOBOXEXITEM CbItem;
int iSelect = 0;
int flag = 0;
BOOL bChanged = 0;
CSLISTSTATUSITEM helpItem;
HANDLE hHookMenuBuild = NULL;
BOOL bStatusMenu = FALSE;

PLUGININFOEX pluginInfoEx = {
  sizeof( PLUGININFOEX ),
  CSLIST_MODULE_LONG_NAME,
  PLUGIN_MAKE_VERSION( 0,0,0,15 ),
  "Offers list of your Custom Statuses.",
  "jarvis [eThEreAL] .., HANAX",
  "mike.taussick@seznam.cz",
  "© 2007 eternity crew .., © 2006 HANAX Software",
  "http://dev.mirandaim.ru/jarvis/",
  0,  //not transient
  0,  //doesn't replace anything built-in
  // Generate your own unique id for your plugin.
  // Do not use this UUID!
  // Use uuidgen.exe to generate the uuuid
  {0x8b86253, 0xec6e, 0x4d09, { 0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8 }} //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
};

PLUGININFO pluginInfo = {
  sizeof(PLUGININFO),
  CSLIST_MODULE_LONG_NAME,
  PLUGIN_MAKE_VERSION(0,0,0,15),
  "Offers list of your Custom Statuses.",
  "jarvis [eThEreAL] .., HANAX",
  "mike.taussick@seznam.cz",
  "© 2007 eternity crew .., © 2006 HANAX Software",
  "http://dev.mirandaim.ru/jarvis/",
  0,  //not transient
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
static int CALLBACK CSListSortProc( LPARAM lparam1, LPARAM lparam2, LPARAM lparamSort );
INT_PTR CALLBACK RelNotesProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

// ############################# HELP FUNCTIONS ################################

int null_snprintf( char*, size_t, const char*, ... );
static int CListMW_BuildStatusItems( WPARAM wParam, LPARAM lParam );
void InitMenuItem( BOOL bAllowStatus, int menuItemPlacement );

// ################## COMMANDS OF CSLIST #######################################

int cslist_add_item();
int cslist_remove_item();
int cslist_AM_set_help_item( HWND hwndAMDlg );

// ################## SERVICES FOR CSLIST ######################################

void cslist_clear_selection();
void cslist_clear_help_item();

// ################## DB - LOAD AND SAVE #######################################

int cslist_initialize_list_content( HWND hwndDlg );
int cslist_parse_row( char *row );
int cslist_save_list_content( HWND hwndDlg );

// ######################### CREATING DIALOG ###################################

int cslist_set_status( HWND hwndDlg );
void cslist_init_icons( void );


// ######################### OTHER FUNCTIONS ###################################

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon );

// ############################## EXTERNS ######################################

//extern int mir_free( const int & );
//extern int ImageList_AddIcon_IconLibLoaded( HIMAGELIST hIml, int iconId );

#pragma comment( lib, "comctl32.lib" )
