/* #############################################################################
####################################  HEADERS  #################################
############################################################################# */

#include <windows.h>
//#include <win2k.h> // only because of SIZEOF? x) O_o :D
#include <stdio.h>
#include <time.h>
#include <newpluginapi.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_file.h>
#include <m_system.h>
#include <m_clist.h>
#include <m_protosvc.h>
#include <m_protomod.h>
#include <m_clui.h>
#include <m_ignore.h>
#include <m_fingerprint.h>
#include <m_icolib.h>

#include <m_updater.h>
#include <m_smr.h>

#include "resource.h"
#include "version.h"

/* #############################################################################
##################################  DEFINITIONS  ###############################
############################################################################# */

#pragma optimize( "gsy", on )

#define _STATIC

#define MODULENAME        "ContactsMenuItemEx"

#define SIZEOF( x ) ( sizeof( x ) / sizeof( x[0] ) )

#define MIIM_STRING  0x00000040

#define MAX_PROTOS   32
#define MAX_GROUPS   32

#define DB_READ_SUCCESS   0
#define DB_READ_FAIL      -1

#define Index(ID_STATUS)  ID_STATUS - 40071
#define ID_STATUS_MIN     ID_STATUS_OFFLINE
#define ID_STATUS_MAX     ID_STATUS_OUTTOLUNCH

#define VF_AV        0x0001
#define VF_NV        0x0002
#define VF_HFL       0x0004
#define VF_IGN       0x0008
#define VF_PROTO     0x0010
#define VF_GRP       0x0020
#define VF_ADD       0x0040
#define VF_REQ       0x0080
#define VF_CID       0x0100
#define VF_CIDN      0x0200
#define VF_RECV      0x0400
#define VF_STAT      0x0800
#define VF_CIP       0x1000
#define VF_CMV       0x2000
#define VF_SLO       0x4000

#define MS_SETINVIS		"MenuEx/SetInvis"
#define MS_SETVIS		  "MenuEx/SetVis"
#define MS_HIDE			  "MenuEx/Hide"
#define MS_IGNORE		  "MenuEx/ShowIgnore"
#define MS_PROTO		  "MenuEx/ShowProto"
#define MS_GROUP		  "MenuEx/ShowGroup"
#define MS_ADDED		  "MenuEx/SendAdded"
#define MS_AUTHREQ		"MenuEx/SendAuthReq"
#define MS_COPYID		  "MenuEx/CopyID"
#define MS_RECVFILES	"MenuEx/RecvFiles"
#define MS_STATUSMSG	"MenuEx/CopyStatusMsg"
#define MS_COPYIP		  "MenuEx/CopyIP"
#define MS_COPYMIRVER	"MenuEx/CopyMirVer"
#define MS_LASTONLINE "ContactsEx/LastOnlineOpen"

#define MIT_AV        "Always visible"
#define MIT_NV        "Never visible"
#define MIT_HFL       "Hide from list"
#define MIT_IGN       "Ignore..."
#define MIT_PROTO     "Protocol..."
#define MIT_GRP       "Group..."
#define MIT_ADD       "Send 'You were added'"
#define MIT_REQ       "Request authorization"
#define MIT_CID       "Copy ID"
#define MIT_RECV      "Browse Received Files"
#define MIT_STAT      "Copy Status Message"
#define MIT_XSTAT     "X-Status Message"
#define MIT_CIP       "Copy IP"
#define MIT_CMV       "Copy MirVer"
#define MIT_SLO       "Last Online"

#define DEFAULT_LASTONLINE_FORMAT   "%d.%m.%Y %H:%M"

/* #############################################################################
######## GLOBALS ###############################################################
############################################################################# */

static const MUUID interfaces[] = {MIID_TESTPLUGIN, MIID_LAST};

PLUGINLINK *pluginLink;

WNDPROC mainProc;
HINSTANCE hinstance;
HMENU hpopupIgnore, hpopupProto, hpopupGroup;
HANDLE hmenuVis, hmenuOff, hmenuHide, hmenuIgnore, hmenuProto, hmenuGroup,
       hmenuAdded, hmenuAuthReq, hmenuCopyID, hmenuRecvFiles, hmenuStatusMsg,
       hmenuCopyIP, hmenuCopyMirVer, hmenuShowLastOnline;

// hook handles
HANDLE hContactSettingChanged, hHookContactStatusChanged, hContactStatusChanged;

HICON hIcon[7];
HWND hdummy;

const int default_flags = VF_AV | VF_NV | VF_HFL | VF_IGN | VF_PROTO | VF_GRP |
                          VF_ADD | VF_REQ | VF_CID | VF_CIDN | VF_RECV |
                          VF_STAT | VF_CIP | VF_CMV | VF_SLO;

char *szmainProto, *protodat[MAX_PROTOS], *groupdat[MAX_GROUPS];

static struct {
	char *module;
	char *name;
	char flag;
} status[] = 
{
	{ "CList", "StatusMsg", 1 },
	{ "ICQ", "XStatusMsg", 2 },
	{ "ICQ", "XStatusName", 2 },
	//{0}
};

static struct
{
  char*  szDescr;
  char*  szName;
  int    defIconID;
  HANDLE hIconLibItem;
} iconList[] =
{
  { LPGEN( MIT_AV ), "cmiex_av", IDI_AV },
  { LPGEN( MIT_NV ), "cmiex_nv", IDI_NV },
  { LPGEN( MIT_ADD ), "cmiex_add", IDI_ADD },
  { LPGEN( MIT_CID ), "cmiex_cid", IDI_CID },
  { LPGEN( MIT_CIP ), "cmiex_cip", IDI_CIP },
  { LPGEN( MIT_CMV ), "cmiex_cmv", IDI_CMV },
  { LPGEN( MIT_GRP ), "cmiex_grp", IDI_GRP },
  { LPGEN( MIT_HFL ), "cmiex_hfl", IDI_HFL },
  { LPGEN( MIT_IGN ), "cmiex_ign", IDI_IGN },
  { LPGEN( MIT_PROTO ), "cmiex_proto", IDI_PROTO },
  { LPGEN( MIT_RECV ), "cmiex_recv", IDI_RECV },
  { LPGEN( MIT_REQ ), "cmiex_req", IDI_REQ },
  { LPGEN( MIT_STAT ), "cmiex_stat", IDI_STAT },
  { LPGEN( MIT_XSTAT ), "cmiex_xstat", IDI_XSTAT },
  { LPGEN( MIT_SLO ), "cmiex_slo", IDI_SLO },
};

/* #############################################################################
######## MIRANDA STUFF #########################################################
############################################################################# */

int BuildMenu( WPARAM wparam, LPARAM lparam );
static int PluginInit( WPARAM wparam, LPARAM lparam );
__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion );
__declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion );
__declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void );
__declspec( dllexport ) int Load( PLUGINLINK *link );
__declspec( dllexport ) int Unload( void );
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved );
static int cmiex_ContactSettingChanged( WPARAM wParam, LPARAM lParam );

/* #############################################################################
######## VISIBLE ###############################################################
############################################################################# */

BOOL isVisSupport( HANDLE hContact );
int onSetVis( WPARAM wparam, LPARAM lparam );
void ModifyVisibleSet( CLISTMENUITEM *cli, int mode );

/* #############################################################################
######## INVISIBLE #############################################################
############################################################################# */

BOOL isInvSupport( HANDLE hContact );
int onSetInvis( WPARAM wparam, LPARAM lparam );
void ModifyInvisSet( CLISTMENUITEM *cli, int mode );

/* #############################################################################
######## HIDE FROM LIST ########################################################
############################################################################# */

int onHide( WPARAM wparam, LPARAM lparam );
void ModifyHidden( CLISTMENUITEM *cli, int mode );

/* #############################################################################
######## IGNORE ################################################################
############################################################################# */

int onShowIgnore( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## PROTOCOL ##############################################################
############################################################################# */

int onShowProto( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## GROUP #################################################################
############################################################################# */

int onShowGroup( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## YOU WERE ADDED ########################################################
############################################################################# */

int onSendAdded( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## AUTH REQUESTED ########################################################
############################################################################# */

int onSendAuthRequest( WPARAM wparam, LPARAM lparam );
BOOL CALLBACK AuthReqWndProc( HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## COPY ID ###############################################################
############################################################################# */

void GetID( HANDLE hContact, LPSTR szProto, LPSTR szID );
void ModifyCopyID( CLISTMENUITEM *cli, HANDLE hContact );
int onCopyID( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## BROWSE FILES ##########################################################
############################################################################# */

BOOL DirectoryExists( HANDLE hContact );
int onRecvFiles( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## COPY STATUS MSG #######################################################
############################################################################# */

BOOL StatusMsgExists( HANDLE hContact );
void ModifyStatusMsg( CLISTMENUITEM *cli, HANDLE hContact );
int onCopyStatusMsg( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## COPY IP ###############################################################
############################################################################# */

BOOL IPExists( HANDLE hContact );
void getIP( HANDLE hContact, LPSTR szProto, LPSTR szIP );
void ModifyCopyIP( CLISTMENUITEM *cli, HANDLE hContact );
int onCopyIP( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## COPY MIRVER ###########################################################
############################################################################# */

int onCopyMirVer( WPARAM wparam, LPARAM lparam );
LPSTR getMirVer( HANDLE hContact );
void ModifyCopyMirVer( CLISTMENUITEM *cli, HANDLE hContact );
BOOL MirVerExists( HANDLE hContact );

/* #############################################################################
######## SHOW LAST ONLINE ######################################################
############################################################################# */

void ModifyLastOnline( CLISTMENUITEM *cli, HANDLE hContact );

/* #############################################################################
######## SERVICE FUNCTIONS #####################################################
############################################################################# */

char *MIEXDBGetString( HANDLE hContact, const char *szModule, const char *szSetting );
void CopyToClipboard( HWND hwnd, LPSTR msg );
BOOL CALLBACK CatchMenuMsg( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
void ShowItem( CLISTMENUITEM *cli, HMENU hmenu );
void HideItem( CLISTMENUITEM *cli, HMENU hmenu );
void initIcoLibIcons( void );
HICON LoadIconEx( const char* name );

/* #############################################################################
######## OPTIONS SECTION #######################################################
############################################################################# */

BOOL CALLBACK OptionsProc( HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam );
int OptionsInit( WPARAM wparam, LPARAM lparam );

/* #############################################################################
######## RELEASE NOTES SECTION #################################################
############################################################################# */

INT_PTR CALLBACK RelNotesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
