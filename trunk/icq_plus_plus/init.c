// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006,2007 Joe Kucera
// Copyright © 2006,2007 [sss], chaos.persei, [sin], Faith Healer, Theif, nullbie
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//
// File name      : $Source$
// Revision       : $Revision: 62 $
// Last change on : $Date: 2007-10-16 14:56:05 +0300 (Ð’Ñ‚, 16 Ð¾ÐºÑ‚ 2007) $
// Last change by : $Author: chaos.persei $
//
// DESCRIPTION:
//
//  Miranda init file - exported functions of plugin
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"
#include "isee.h"

#include "m_updater.h"


PLUGINLINK* pluginLink;
struct MM_INTERFACE mmi;
struct MD5_INTERFACE md5i;
HANDLE hHookUserInfoInit = NULL;
HANDLE hHookOptionInit = NULL;
HANDLE hHookUserMenu = NULL;
HANDLE hHookIdleEvent = NULL;
HANDLE hHookIconsChanged = NULL;
static HANDLE hUserMenuAddServ = NULL;
static HANDLE hUserMenuAuth = NULL;
static HANDLE hUserMenuGrant = NULL;
static HANDLE hUserMenuRevoke = NULL;
static HANDLE hUserMenuXStatus = NULL;
HANDLE hIconProtocol = NULL;
static HANDLE hIconMenuAuth = NULL;
static HANDLE hIconMenuGrant = NULL;
static HANDLE hIconMenuRevoke = NULL;
static HANDLE hIconMenuAddServ = NULL;
static HANDLE hUserMenuSetVis = NULL;
static HANDLE hUserMenuSetInvis = NULL;
static HANDLE hUserMenuIncognito = NULL;
static HANDLE hUserMenuSendtZer = NULL;
HANDLE hPopUpsList = NULL;
HANDLE hUserMenuStatus = NULL;

extern HANDLE hServerConn;
CRITICAL_SECTION localSeqMutex;
CRITICAL_SECTION connectionHandleMutex;
HANDLE hsmsgrequest;
HANDLE hxstatuschanged;
HANDLE hxstatusiconchanged;
//popup settings
BOOL bPopUpService = FALSE;
BOOL bPopUpsEnabled = FALSE;
BOOL bSpamPopUp = FALSE;
BOOL bUnknownPopUp = FALSE;
BOOL bFoundPopUp = TRUE;
BOOL bScanPopUp = TRUE;
BOOL bClientChangePopUp = FALSE;
BOOL bVisPopUp = TRUE;
BOOL bPopUpForNotOnList = FALSE;
BOOL bPopSelfRem = TRUE;
BOOL bIgnoreCheckPop = TRUE;
BOOL bInfoRequestPopUp = FALSE;
BOOL bAuthPopUp = FALSE;
BOOL bUinPopup = FALSE;
//visibility variable
BYTE bVisibility;
BOOL bIncognitoRequest = FALSE;
BOOL bIncognitoGlobal = FALSE;
BOOL bShowAuth = FALSE;
//BOOL bStealthRequest = FALSE;
BOOL bPSD = TRUE;
BOOL bExcludePSD = FALSE;
BOOL bNoASD = TRUE;
BOOL bNoPSDForHidden = TRUE;
BOOL bNoStatusReply = FALSE;
//xstatus settings
BOOL bXstatusIconShow = TRUE;
//loging options
BOOL bLogSelfRemoveFile = FALSE;
BOOL bLogIgnoreCheckFile = FALSE;
BOOL bLogStatusCheckFile = FALSE;
BOOL bLogClientChangeFile = FALSE;
BOOL bLogAuthFile = FALSE;
BOOL bLogInfoRequestFile = FALSE; 
BOOL bLogSelfRemoveHistory = FALSE;
BOOL bLogIgnoreCheckHistory = FALSE;
BOOL bLogStatusCheckHistory = FALSE;
BOOL bLogClientChangeHistory = FALSE;
BOOL bLogAuthHistory = FALSE;
BOOL bLogInfoRequestHistory = FALSE;
BOOL bHcontactHistory = FALSE;
//other settings
BYTE bTmpContacts = 0;
char* TmpGroupName = 0;
BOOL bTmpGroupCreated = FALSE;
BOOL bAddTemp = FALSE;
BOOL bServerAutoChange = TRUE;
WORD wClass;

extern int bHideXStatusUI;

/* eternity mod :: version */
PLUGININFOEX pluginInfo = {
  sizeof(PLUGININFOEX),
  "IcqOscarJ " ICQ_THISMODNAME " Mod Protocol",
  PLUGIN_MAKE_VERSION(0,3,88,33),
  "Support for ICQ network, enhanced. [based on ICQJ Plus 0.3.8.105 testing build #93 rc 2 + IcqOscarJ 0.3.10.6 svn 6639]",
  "J.Kucera, Bio, M.berg, R.Hughes, Jon Keating, BM, S7, [sss], chaos.persei, Faith, jarvis, ghazan, baloo, nullbie etc",
  "mike.taussick@seznam.cz, sss123next@list.ru, chaos.persei@gmail.com, sin@miranda-me.org, jokusoftware@miranda-im.org",
  "(C) 2000-2007 M. berg, R.Hughes, J.Keating, Bio, J.Kucera, Angeli-Ka, Faith Healer, chaos.persei, Se7ven, BM, [sss], [sin], jarvis, nullbie and others",
  "http://dev.mirandaim.ru/jarvis/, http://dev.mirandaim.ru/~sss/",
  0,
  0,   //doesn't replace anything built-in :: eternity mod uuid
  { 0xdfbe27d6, 0x32de, 0x47ac, { 0xb1, 0x45, 0xcd, 0xf5, 0x46, 0xa9, 0x4a, 0x58 } }
};

/* version info
0.3.88.33 - svn merge #6639 - icq 6 moods, fixes
          - icq plus svn merge - fixes, optimalizations
0.3.88.32 - svn merge (string handling changes - 0.7pr8/0.8a1 required)
          - send tZer icon :]
0.3.88.31 - only numbers possible to enter into First Run dialog asking for UIN/pass in UIN field :)
          - "Release Notes" moved to PluginsLoaded hooked method, not blocking Miranda to complete load x)
          - svn merge (coming with ClientChangeNotify-like option in Popups :))
          - tZers sending support by LubomirR
0.3.88.30 - svn merge, now works with Miranda IM 0.8 only
          - "Release Notes" dialog :)
          - - Finally solved sending Miranda capabilities problems (SecureIM flag, 0.8 flag)
0.3.88.29 - new environment for choosing desired ICQ Mod you can be identified as :]
0.3.88.28 - svn merge, back in work again x)
0.3.88.25 - svn merge + official svn merge (changes in translations, ..)
0.3.88.24 - svn merge
          - attemp to fix not delivering messages to newly added custom caps (MIM mods)
0.3.88.23 - svn merge
0.3.88.22 - updated infos - Updater adresses, author e-mail and site
          - svn merge (official solving of the OTF CID detection)
0.3.88.21 - svn merge
          - fix for bug in svn 840 with OTF Client ID detection (solved with Shurik's assistency :))
0.3.88.20 - svn merge
0.3.88.18 - official icq svn merge - better UTF8 handling
0.3.88.16 - in Client ID options, added field for own capability (need more work - own caps list)
0.3.88.12 - official icq svn merge - not to use ICQ6 signature x) because of not working FTs (reported to Joe@Whale)
0.3.88.10 - svn merge
          - official icq svn merge - new Filetransfers
0.3.88.8  - fix to use right MirandaM or Capability when Show/Hide real ID to Mods users
0.3.88.7  - fix for cabability using
0.3.88.6  - changed Mod's identification, now it uses its own capability :)
          * 8803xxxx is now identified as old eternity mod
          * new ID uses regular 80xx system
          - changed Mod name to "eternity/PlusPlus++"
0.3.88.5  - svn merge
          - Server list replaced with the solution from ICQ Plus
0.3.88.3  - improvements in Mod cabability (better handling with original & mod ICQ plugins)
0.3.88.2  - fixed terrible interpretation of Mod's cabability x)
0.3.88.1  - changed versioning
          * leading 0 is virtual, send capability is 8803xxxx - like BM's 8103xxxx
          * needed bacause of strange politics in ICQ S7&SSS/Plus versioning - infinite 105 builds x)
0.0.8.105 - svn merge
          - improvements in Server list
          - changed Mod name to "eternity mod"
0.0.8.104 - based on S7&SSS 0.3.8.104
          - added Extended ComboBox instead of String for server address (try for multi-servers list)
          - mod named "S7&SSS extended eternity"
*/

struct LIST_INTERFACE listInterface;
SortedList *lstCustomCaps;


static char UnicodeAware[32];


static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam);
static int OnSystemPreShutdown(WPARAM wParam,LPARAM lParam);
static int icq_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
static int IconLibIconsChanged(WPARAM wParam, LPARAM lParam);


static BOOL bInited = FALSE;

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
  if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 44)) 
  {
    MessageBox( NULL, "ICQ plugin cannot be loaded. It requires Miranda IM 0.7 PR 8/0.8 alpha build #1 or later.", "ICQ Plugin", 
      MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
    return NULL;
  }
  else
  {
      // Are we running under Unicode Windows version ?
    gbUnicodeAPI = (GetVersion() & 0x80000000) == 0;
	  if (gbUnicodeAPI)
	  {
		  pluginInfo.flags = 1;
	  }
	  MIRANDA_VERSION = mirandaVersion;
    bInited = TRUE;
  }
  
  return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
  hInst = hinstDLL;

  return TRUE;
}

static HANDLE ICQCreateServiceFunction(const char* szService, MIRANDASERVICE serviceProc)
{
  char str[MAX_PATH + 32];
  strcpy(str, gpszICQProtoName);
  strcat(str, szService);
  return CreateServiceFunction(str, serviceProc);
}

static HANDLE ICQCreateHookableEvent(const char* szEvent)
{
  char str[MAX_PATH + 32];
  strcpy(str, gpszICQProtoName);
  strcat(str, szEvent);
  return CreateHookableEvent(str);
}


static int EnumCustomCapsProc(const char *szSetting,LPARAM lParam)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	dbcgs.szModule = (char *)lParam;
	dbcgs.szSetting = szSetting;
	dbcgs.pValue = &dbv;
	CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&dbcgs);
	if (dbv.type == DBVT_BLOB)
	{
		ICQ_CUSTOMCAP icqCustomCap;
		icqCustomCap.cbSize = sizeof(icqCustomCap);
		strncpy(icqCustomCap.name, szSetting, 64);
		memcpy(icqCustomCap.caps, dbv.pbVal, min(0x10, dbv.cpbVal));
		CallProtoService(gpszICQProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
	} else
	if (dbv.type == DBVT_ASCIIZ)
	{
		ICQ_CUSTOMCAP icqCustomCap;
		icqCustomCap.cbSize = sizeof(icqCustomCap);
		strncpy(icqCustomCap.name, szSetting, 64);
		strncpy(icqCustomCap.caps, dbv.pszVal, 0x10);
		CallProtoService(gpszICQProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
	}
	CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
	return 0;
}

void RegEventType(int EventType, char* EventDescription)
{
	DBEVENTTYPEDESCR evt;
	evt.cbSize=sizeof(evt);
	evt.module=gpszICQProtoName;
	evt.eventType=EventType;
	evt.descr=EventDescription;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&evt);
}

// eternity : release notes procedure

INT_PTR CALLBACK RelNotesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          DBWriteContactSettingWord(NULL, gpszICQProtoName, "ModRelNotes", ICQ_RELNOTES);
          EndDialog(hwndDlg, LOWORD(wParam));
          break;
      }
    }
    break;
  }
  return FALSE;
}

// eternity : release notes procedure END

int __declspec(dllexport) Load(PLUGINLINK *link)
{
  PROTOCOLDESCRIPTOR pd = {0};

  pluginLink = link;
  mir_getMMI( &mmi );
  mir_getMD5I( &md5i );
    
  ghServerNetlibUser = NULL;

  // Are we running under Unicode Windows version ?
  gbUnicodeAPI = (GetVersion() & 0x80000000) == 0;
  { // Are we running under unicode Miranda core ?
    char szVer[MAX_PATH];

    CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
    _strlwr(szVer); // make sure it is lowercase
    gbUnicodeCore = (strstr(szVer, "unicode") != NULL);

    if (strstr(szVer, "alpha") != NULL)
    { // Are we running under Alpha Core
      MIRANDA_VERSION |= 0x80000000;
    }
    else if (MIRANDA_VERSION >= 0x00050000 && strstr(szVer, "preview") == NULL)
    { // for Final Releases of Miranda 0.5+ clear build number
      MIRANDA_VERSION &= 0xFFFFFF00;
    }
  }

  srand(time(NULL));
  _tzset();

  // Get module name from DLL file name
  {
    char* str1;
    char str2[MAX_PATH];
    int nProtoNameLen;

    GetModuleFileName(hInst, str2, MAX_PATH);
    str1 = strrchr(str2, '\\');
    nProtoNameLen = strlennull(str1);
    if (str1 != NULL && (nProtoNameLen > 5))
    {
      strncpy(gpszICQProtoName, str1+1, nProtoNameLen-5);
      gpszICQProtoName[nProtoNameLen-4] = 0;
    }
    CharUpper(gpszICQProtoName);
  }

  ZeroMemory(gpszPassword, sizeof(gpszPassword));

  HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
  HookEvent(ME_SYSTEM_PRESHUTDOWN, OnSystemPreShutdown);

  InitializeCriticalSection(&connectionHandleMutex);
  InitializeCriticalSection(&localSeqMutex);
  InitializeCriticalSection(&modeMsgsMutex);

  // Initialize core modules
  InitDB();       // DB interface
  InitCookies();  // cookie utils
  InitCache();    // contacts cache
  InitReguin();   // reguin module

  DBWriteContactSettingDword(NULL, gpszICQProtoName, "SrvLastUpdate", 0);
  DBWriteContactSettingWord(NULL, gpszICQProtoName, "SrvRecordCount", 0);

  InitRates();    // rate management

  // Register the module
  pd.cbSize = sizeof(pd);
  pd.szName = gpszICQProtoName;
  pd.type   = PROTOTYPE_PROTOCOL;
  CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

  // Initialize status message struct
  ZeroMemory(&modeMsgs, sizeof(icq_mode_messages));

  // Initialize temporary DB settings
  ICQCreateResidentSetting("Status"); // NOTE: XStatus cannot be temporary
  ICQCreateResidentSetting("TemporaryVisible");
  ICQCreateResidentSetting("TickTS");
  ICQCreateResidentSetting("IdleTS");
  ICQCreateResidentSetting("LogonTS");
  ICQCreateResidentSetting("CapBuf");
  ICQCreateResidentSetting("DCStatus");
  ICQCreateResidentSetting("IsContactChecked");

  // Reset a bunch of session specific settings
  ResetSettingsOnLoad();


  // Setup services
  ICQCreateServiceFunction(PS_GETCAPS, IcqGetCaps);
  ICQCreateServiceFunction(PS_GETNAME, IcqGetName);
  ICQCreateServiceFunction(PS_LOADICON, IcqLoadIcon);
  ICQCreateServiceFunction(PS_SETSTATUS, IcqSetStatus);
  ICQCreateServiceFunction(PS_GETSTATUS, IcqGetStatus);
  ICQCreateServiceFunction(PS_SETAWAYMSG, IcqSetAwayMsg);
  ICQCreateServiceFunction(PS_AUTHALLOW, IcqAuthAllow);
  ICQCreateServiceFunction(PS_AUTHDENY, IcqAuthDeny);
  ICQCreateServiceFunction(PS_BASICSEARCH, IcqBasicSearch);
  ICQCreateServiceFunction(PS_SEARCHBYEMAIL, IcqSearchByEmail);
  ICQCreateServiceFunction(MS_ICQ_SEARCHBYDETAILS, IcqSearchByDetails);
  ICQCreateServiceFunction(PS_SEARCHBYNAME, IcqSearchByDetails);
  ICQCreateServiceFunction(PS_CREATEADVSEARCHUI, IcqCreateAdvSearchUI);
  ICQCreateServiceFunction(PS_SEARCHBYADVANCED, IcqSearchByAdvanced);
  ICQCreateServiceFunction(MS_ICQ_SENDSMS, IcqSendSms);
  ICQCreateServiceFunction(PS_ADDTOLIST, IcqAddToList);
  ICQCreateServiceFunction(PS_ADDTOLISTBYEVENT, IcqAddToListByEvent);
  ICQCreateServiceFunction(PS_FILERESUME, IcqFileResume);
  ICQCreateServiceFunction(PS_SET_NICKNAME, IcqSetNickName);
  ICQCreateServiceFunction(PSS_GETINFO, IcqGetInfo);
  ICQCreateServiceFunction(PSS_MESSAGE, IcqSendMessage);
  ICQCreateServiceFunction(PSS_URL, IcqSendUrl);
  ICQCreateServiceFunction(PSS_CONTACTS, IcqSendContacts);
  ICQCreateServiceFunction(PSS_SETAPPARENTMODE, IcqSetApparentMode);
  ICQCreateServiceFunction(PSS_GETAWAYMSG, IcqGetAwayMsg);
  ICQCreateServiceFunction(PSS_FILEALLOW, IcqFileAllow);
  ICQCreateServiceFunction(PSS_FILEDENY, IcqFileDeny);
  ICQCreateServiceFunction(PSS_FILECANCEL, IcqFileCancel);
  ICQCreateServiceFunction(PSS_FILE, IcqSendFile);
  ICQCreateServiceFunction(PSR_AWAYMSG, IcqRecvAwayMsg);
  ICQCreateServiceFunction(PSR_FILE, IcqRecvFile);
  ICQCreateServiceFunction(PSR_MESSAGE, IcqRecvMessage);
  ICQCreateServiceFunction(PSR_CONTACTS, IcqRecvContacts);
  ICQCreateServiceFunction(PSR_AUTH, IcqRecvAuth);
  ICQCreateServiceFunction(PSS_AUTHREQUEST, IcqSendAuthRequest);
  ICQCreateServiceFunction(PSS_ADDED, IcqSendYouWereAdded);
  ICQCreateServiceFunction(PSS_USERISTYPING, IcqSendUserIsTyping);
  // Session password API
  ICQCreateServiceFunction(PS_ICQ_SETPASSWORD, IcqSetPassword);
  // ChangeInfo API
  ICQCreateServiceFunction(PS_CHANGEINFOEX, IcqChangeInfoEx);
  // Avatar API
  ICQCreateServiceFunction(PS_GETAVATARINFO, IcqGetAvatarInfo);
  ICQCreateServiceFunction(PS_GETAVATARCAPS, IcqGetAvatarCaps);
  ICQCreateServiceFunction(PS_GETMYAVATAR, IcqGetMyAvatar);
  ICQCreateServiceFunction(PS_SETMYAVATAR, IcqSetMyAvatar);
  // Custom Status API
  ICQCreateServiceFunction(PS_ICQ_SETCUSTOMSTATUS, IcqSetXStatus); // obsolete (remove in next version)
  ICQCreateServiceFunction(PS_ICQ_GETCUSTOMSTATUS, IcqGetXStatus); // obsolete
  ICQCreateServiceFunction(PS_ICQ_SETCUSTOMSTATUSEX, IcqSetXStatusEx);
  ICQCreateServiceFunction(PS_ICQ_GETCUSTOMSTATUSEX, IcqGetXStatusEx);
  ICQCreateServiceFunction(PS_ICQ_GETCUSTOMSTATUSICON, IcqGetXStatusIcon);
  ICQCreateServiceFunction(PS_ICQ_REQUESTCUSTOMSTATUS, IcqRequestXStatusDetails);
  ICQCreateServiceFunction(PS_ICQ_GETADVANCEDSTATUSICON, IcqRequestAdvStatusIconIdx);
  // Custom caps
  ICQCreateServiceFunction(PS_ICQ_ADDCAPABILITY, IcqAddCapability);
  ICQCreateServiceFunction(PS_ICQ_CHECKCAPABILITY, IcqCheckCapability);
	


  hsmsgrequest = ICQCreateHookableEvent(ME_ICQ_STATUSMSGREQ);
  hxstatuschanged = ICQCreateHookableEvent(ME_ICQ_CUSTOMSTATUS_CHANGED);
  hxstatusiconchanged = ICQCreateHookableEvent(ME_ICQ_CUSTOMSTATUS_EXTRAICON_CHANGED);

  InitDirectConns();
  InitOscarFileTransfer();
  InitServerLists();
  icq_InitInfoUpdate();
  RegEventType(ICQEVENTTYPE_IGNORECHECK_STATUS, "Check ICQ Ignore State");
  RegEventType(ICQEVENTTYPE_CHECK_STATUS, "Check ICQ Status");
  RegEventType(ICQEVENTTYPE_CLIENT_CHANGE, "Chenge ICQ Client");
  RegEventType(ICQEVENTTYPE_SELF_REMOVE, "ICQ Contact SelfRemove");
  RegEventType(ICQEVENTTYPE_AUTH_DENIED, "ICQ Auth Denied");
  RegEventType(ICQEVENTTYPE_AUTH_GRANTED, "ICQ Auth Granted");
  RegEventType(ICQEVENTTYPE_AUTH_REQUESTED, "ICQ Auth Requested");
  RegEventType(ICQEVENTTYPE_YOU_ADDED, "ICQ You Added");
  

  icq_InitISee();
  // Initialize charset conversion routines
  InitI18N();

  UpdateGlobalSettings();

  gnCurrentStatus = ID_STATUS_OFFLINE;

  ICQCreateServiceFunction(MS_ICQ_ADDSERVCONTACT, IcqAddServerContact);

  ICQCreateServiceFunction(MS_REQ_AUTH, icq_RequestAuthorization);
  ICQCreateServiceFunction(MS_GRANT_AUTH, IcqGrantAuthorization);
  ICQCreateServiceFunction(MS_REVOKE_AUTH, IcqRevokeAuthorization);
  ICQCreateServiceFunction(MS_SETINVIS, IcqSetInvis);
  ICQCreateServiceFunction(MS_SETVIS, IcqSetVis);
  ICQCreateServiceFunction(MS_INCOGNITO_REQUEST, IncognitoAwayRequest);
  ICQCreateServiceFunction(MS_SEND_TZER, IcqSendtZer);

  ICQCreateServiceFunction(MS_XSTATUS_SHOWDETAILS, IcqShowXStatusDetails);


  hHookIconsChanged = IconLibHookIconsChanged(IconLibIconsChanged);

  InitXStatusIcons();

  // This must be here - the events are called too early, WTF?
  InitXStatusEvents();



  //Custom caps
  mir_getLI(&listInterface);
  lstCustomCaps = listInterface.List_Create(0,1);
  lstCustomCaps->sortFunc = NULL;

  {
  	char tmp[MAXMODULELABELLENGTH];
    DBCONTACTENUMSETTINGS dbces;
	  mir_snprintf(tmp, MAXMODULELABELLENGTH, "%sCaps", gpszICQProtoName);
	  dbces.pfnEnumProc = EnumCustomCapsProc;
	  dbces.lParam = (LPARAM)tmp;
	  dbces.szModule = tmp;
	  CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
  }

  
  
  ICQCreateThread(icq_XStatusUpdaterThread, NULL);

/*
  {
    ICQ_CUSTOMCAP icqCustomCap;
	IcqBuildMirandaCap(&icqCustomCap, "Miranda ICQ Capability Test", NULL, "TestTest");
    CallProtoService(gpszICQProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
  }
*/
  return 0;

}



int __declspec(dllexport) Unload(void)
{
  if (gbXStatusEnabled) gbXStatusEnabled = 10; // block clist changing

  UninitXStatusEvents();

  if (hServerConn)
  {
    icq_sendCloseConnection();

    icq_serverDisconnect(TRUE);
  }

  UninitServerLists();
  UninitOscarFileTransfer();
  UninitDirectConns();
  icq_InfoUpdateCleanup();
  icq_ISeeCleanup();

  NetLib_SafeCloseHandle(&ghDirectNetlibUser, FALSE);
  NetLib_SafeCloseHandle(&ghServerNetlibUser, FALSE);
  UninitRates();
  UninitCookies();
  UninitCache();
  DeleteCriticalSection(&modeMsgsMutex);
  DeleteCriticalSection(&localSeqMutex);
  DeleteCriticalSection(&connectionHandleMutex);
  SAFE_FREE(&modeMsgs.szOnline);
  SAFE_FREE(&modeMsgs.szAway);
  SAFE_FREE(&modeMsgs.szNa);
  SAFE_FREE(&modeMsgs.szOccupied);
  SAFE_FREE(&modeMsgs.szDnd);
  SAFE_FREE(&modeMsgs.szFfc);

  if (hHookIconsChanged)
    UnhookEvent(hHookIconsChanged);

  if (hHookUserInfoInit)
    UnhookEvent(hHookUserInfoInit);

  if (hHookOptionInit)
    UnhookEvent(hHookOptionInit);

  if (hsmsgrequest)
    DestroyHookableEvent(hsmsgrequest);

  if (hxstatuschanged)
    DestroyHookableEvent(hxstatuschanged);

  if (hxstatusiconchanged)
    DestroyHookableEvent(hxstatusiconchanged);

  if (hHookUserMenu)
    UnhookEvent(hHookUserMenu);

  if (hHookIdleEvent)
    UnhookEvent(hHookIdleEvent);

  bVisibility = ICQGetContactSettingByte(NULL, "SrvVisibility", 0);

  return 0;
}






static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam)
{
  NETLIBUSER nlu = {0};
  char pszP2PName[MAX_PATH+3];
  char pszGroupsName[MAX_PATH+10];
  char pszSrvGroupsName[MAX_PATH+10];
  char pszCapsName[MAX_PATH+10];
  char szBuffer[MAX_PATH+64];
  char* modules[6] = {0,0,0,0,0,0};

  if (ServiceExists(MS_POPUP_ADDPOPUPEX))
	  bPopUpService = TRUE;
  if (ICQGetContactSettingByte(NULL,"SpamPopUpEnabled",1)==1)
	  bSpamPopUp = TRUE;
  if (ICQGetContactSettingByte(NULL,"UnknownPopUpEnabled",1)==1)
	  bUnknownPopUp = TRUE;
  if (ICQGetContactSettingByte(NULL,"FoundPopUpEnabled",1)==0)
	  bFoundPopUp = FALSE;
  if (ICQGetContactSettingByte(NULL,"ScanPopUpEnabled",1)==0)
	  bScanPopUp = FALSE;
  if (ICQGetContactSettingByte(NULL,"VisPopUpEnabled",1)==0)
	  bVisPopUp = FALSE;
  if (ICQGetContactSettingByte(NULL,"ClientChangePopup",0)==1)
	  bClientChangePopUp = TRUE;
  if (ICQGetContactSettingByte(NULL,"PopUpForNotOnList",0)==1)
	  bPopUpForNotOnList = TRUE;
  if (ICQGetContactSettingByte(NULL, "PopupsEnabled", 1)==1)
	  bPopUpsEnabled = TRUE;
  if (ICQGetContactSettingByte(NULL, "XStatusIconShow", 1)==0)
	  bXstatusIconShow = FALSE;
  if(ICQGetContactSettingByte(NULL, "IgnoreCheckPop", 1) == 0)
	  bIgnoreCheckPop = FALSE;
  if(ICQGetContactSettingByte(NULL, "PopSelfRem", 1) == 0)
	  bPopSelfRem = FALSE;
  if(ICQGetContactSettingByte(NULL, "InfoRequestPopUp", 0) == 1)
	  bInfoRequestPopUp = TRUE;
  if(ICQGetContactSettingByte(NULL, "AuthPopUp", 0) == 1)
	  bAuthPopUp = TRUE;
  if(ICQGetContactSettingByte(NULL, "PSD", 1) == 0)
	  bPSD = FALSE;
  if(ICQGetContactSettingByte(NULL, "NoASD", 1) == 0)
	  bNoASD = FALSE;
  if(ICQGetContactSettingByte(NULL, "LogSelfRemoveFile", 0) == 1)
	  bLogSelfRemoveFile = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogIgnoreCheckFile", 0) == 1)
	  bLogIgnoreCheckFile = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogStatusCheckFile", 0) == 1)
	  bLogStatusCheckFile = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogClientChangeFile", 0) == 1)
	  bLogClientChangeFile = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogAuthFile", 0) == 1)
	  bLogAuthFile = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogRequestFile", 0) == 1)
	  bLogInfoRequestFile = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogSelfRemoveHistory", 0) == 1)
	  bLogSelfRemoveHistory = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogIgnoreCheckHistory", 0) == 1)
	  bLogIgnoreCheckHistory = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogStatusCheckHistory", 0) == 1)
	  bLogStatusCheckHistory = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogClientChangeHistory", 0) == 1)
	  bLogClientChangeHistory = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogAuthHistory", 0) == 1)
	  bLogAuthHistory = TRUE;
  if(ICQGetContactSettingByte(NULL, "LogRequestHistory", 0) == 1)
	  bLogInfoRequestHistory = TRUE;
  if(ICQGetContactSettingByte(NULL,"LogToHcontact",0)==1)
	  bHcontactHistory = TRUE;
  if(ICQGetContactSettingByte(NULL, "TempContacts", 0) == 1)
	  bTmpContacts = 1;
  if(ICQGetContactSettingByte(NULL, "GroupCreated", 0) == 1)
	  bTmpGroupCreated = TRUE;
  TmpGroupName = UniGetContactSettingUtf(NULL,gpszICQProtoName,"TmpContactsGroup", Translate("General"));
  if(ICQGetContactSettingByte(NULL, "AddTemp", 0) == 1)
	  bAddTemp = TRUE;
  if(ICQGetContactSettingByte(NULL,"NoStatusReply", 0))
	  bNoStatusReply = TRUE;
  if(ICQGetContactSettingByte(NULL,"ServerAutoChange", 1)==0)
	  bServerAutoChange = FALSE;
  if(ICQGetContactSettingByte(NULL, "IncognitoGlobal", 0) == 1)
	  bIncognitoGlobal = TRUE;
  if(ICQGetContactSettingByte(NULL, "ShowAuth", 0) == 1)
	  bShowAuth = TRUE;
  if(ICQGetContactSettingByte(NULL, "UinPopup", 0) == 1)
	  bUinPopup = TRUE;
  if(ICQGetContactSettingByte(NULL, "NoPSDForHidden", 1)==0)
	  bNoPSDForHidden = FALSE;
//  if(ICQGetContactSettingByte(NULL, "StealthRequest", 0) == 1)
//	  bStealthRequest = TRUE;
	 

 /* if(ServiceExists(MS_ASSOCMGR_ADDNEWFILETYPE))  //need to be finishaed
  {
	  {
		  FILETYPEDESC ftd;
		  ftd.cbSize=sizeof(ftd);
		  ftd.pwszDescription=TranslateW("ICQ link file");
		  ftd.pszDescription=Translate("ICQ link file");
		  ftd.ptszDescription=TranslateT("ICQ link file");
		  ftd.pszFileExt=".icq";
		  ftd.pszMimeType="application/x-icq";
		  ftd.pszService=0;
		  ftd.flags=FTDF_DEFAULTDISABLED;
		  CallService(MS_ASSOCMGR_ADDNEWFILETYPE,0,(LPARAM)&ftd);
	  } 
	  {
		  URLTYPEDESC utd;
		  utd.cbSize=sizeof(utd);
		  utd.pszDescription=Translate("ICQ Link");
		  utd.ptszDescription=TranslateT("ICQ Link");
		  utd.pwszDescription=TranslateW("ICQ Link");
		  utd.flags=UTDF_DEFAULTDISABLED;
		  utd.pszService=0;
		  utd.pszProtoPrefix="http:";
		  CallService(MS_ASSOCMGR_ADDNEWURLTYPE,0,(LPARAM)&utd);
	  }
  }*/

  if(ServiceExists(MS_UPDATE_REGISTER)) 
  { // eternity :: updated informations for Updater plugin
	  Update upd = {0};
	  char szCurrentVersion[30];
	  //char pszBuild[10];
	  int i=0;
	  BOOL testing = DBGetContactSettingByte(NULL, "Updater", "UseBetaIcqOscarJ (" ICQ_THISMODNAME " Mod) Protocol", 0);

	  // get build number
	  //char *p=strstr(pluginInfo.shortName, "#");
	  //if(p) p++;
	  //while (p && (*p>='0')&&(*p<='9'))
	  //{
		//  pszBuild[i++]=*p;
		//  p++;
	  //}
	  //pszBuild[i]=0;

	  upd.cbSize = sizeof(upd);
	  upd.szComponentName = "IcqOscarJ (" ICQ_THISMODNAME " Mod) Protocol";
	  upd.pbVersion = (BYTE *)CreateVersionStringPluginEx((PLUGININFOEX *)&pluginInfo, szCurrentVersion);
	  upd.cpbVersion = strlen(( char *)upd.pbVersion);
	  upd.szBetaUpdateURL = "http://mirandapack.ic.cz/eternity_plugins/icq.zip";
	  upd.szBetaVersionURL = "http://dev.mirandaim.ru/jarvis/";
	  upd.szBetaChangelogURL = "http://dev.mirandaim.ru/jarvis/";
	  upd.pbBetaVersionPrefix = (BYTE *)"ICQ eternity/PlusPlus++ Mod</a> ";
	  /* rev 853
	  upd.szBetaUpdateURL = "http://dev.mirandaim.ru/~sss/get.php?file=icq_test.zip";
	  upd.szBetaVersionURL = "http://dev.mirandaim.ru/~sss/get.php?file=icq_test.txt";
	  upd.pbBetaVersionPrefix = (BYTE *)"icq test build ";
    */
	  upd.cpbBetaVersionPrefix = strlen(( char *)upd.pbBetaVersionPrefix);

	  upd.szUpdateURL = upd.szBetaUpdateURL;
	  upd.szVersionURL = upd.szBetaVersionURL;
	  upd.pbVersionPrefix = upd.pbBetaVersionPrefix;
	  upd.cpbVersionPrefix = strlen(( char *)upd.pbVersionPrefix);

	  CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
  } //thx sje


  strcpy(pszP2PName, gpszICQProtoName);
  strcat(pszP2PName, "P2P");

  strcpy(pszGroupsName, gpszICQProtoName);
  strcat(pszGroupsName, "Groups");
  strcpy(pszSrvGroupsName, gpszICQProtoName);
  strcat(pszSrvGroupsName, "SrvGroups");
  strcpy(pszCapsName, gpszICQProtoName);
  strcat(pszCapsName, "Caps");
  modules[0] = gpszICQProtoName;
  modules[1] = pszP2PName;
  modules[2] = pszGroupsName;
  modules[3] = pszSrvGroupsName;
  modules[4] = pszCapsName;
  CallService("DBEditorpp/RegisterModule",(WPARAM)modules,(LPARAM)5);


  null_snprintf(szBuffer, sizeof szBuffer, ICQTranslate("%s server connection"), gpszICQProtoName);
  nlu.cbSize = sizeof(nlu);
  nlu.flags = NUF_OUTGOING | NUF_HTTPGATEWAY;
  nlu.szDescriptiveName = szBuffer;
  nlu.szSettingsModule = gpszICQProtoName;
  nlu.szHttpGatewayHello = "http://http.proxy.icq.com/hello";
  nlu.szHttpGatewayUserAgent = "Mozilla/4.08 [en] (WinNT; U ;Nav)";
  nlu.pfnHttpGatewayInit = icq_httpGatewayInit;
  nlu.pfnHttpGatewayBegin = icq_httpGatewayBegin;
  nlu.pfnHttpGatewayWrapSend = icq_httpGatewayWrapSend;
  nlu.pfnHttpGatewayUnwrapRecv = icq_httpGatewayUnwrapRecv;

  ghServerNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

  null_snprintf(szBuffer, sizeof szBuffer, ICQTranslate("%s client-to-client connections"), gpszICQProtoName);
  nlu.flags = NUF_OUTGOING | NUF_INCOMING;
  nlu.szDescriptiveName = szBuffer;
  nlu.szSettingsModule = pszP2PName;
  nlu.minIncomingPorts = 1;
  ghDirectNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

  hHookOptionInit = HookEvent(ME_OPT_INITIALISE, IcqOptInit);
  hHookUserInfoInit = HookEvent(ME_USERINFO_INITIALISE, OnDetailsInit);
  hHookUserMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, icq_PrebuildContactMenu);
  hHookIdleEvent = HookEvent(ME_IDLE_CHANGED, IcqIdleChanged);

  hPopUpsList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);

  icq_FirstRunCheck();

  InitAvatars();

  // Init extra optional modules
  //InitPopUps();
  InitIconLib();

  hHookIconsChanged = IconLibHookIconsChanged(IconLibIconsChanged);
  // Initialize IconLib icons
  InitXStatusIcons();
  InitXStatusEvents();
  InitXStatusItems(FALSE);

  InitDCEvents();

  {	
     // userinfo
	ICQWriteContactSettingUtf(NULL, "MirVer", Translate("you need to be connected..."));
	DBWriteContactSettingDword(NULL, gpszICQProtoName, "IP", 0);
  if(!DBGetContactSettingByte(NULL, gpszICQProtoName, "ConstRealIP", 0))
	DBWriteContactSettingDword(NULL, gpszICQProtoName, "RealIP", 0);
  DBWriteContactSettingWord(NULL, gpszICQProtoName, "UserPort", 0);
  DBWriteContactSettingDword(NULL, gpszICQProtoName, "LogonTS", 0);
  DBWriteContactSettingDword(NULL, gpszICQProtoName, "IdleTS", 0);
	DBWriteContactSettingDword(NULL, gpszICQProtoName, "Status", ID_STATUS_OFFLINE);
  }
  {
    CLISTMENUITEM mi;
    char pszServiceName[MAX_PATH+30];
  
  	strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_REQ_AUTH);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = 1000030000;
  	mi.hIcon = IconLibGetIcon("req_auth");
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Request authorization");
  	mi.pszService = pszServiceName;
  	hUserMenuAuth = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
  	IconLibReleaseIcon("req_auth");
  
  	strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_GRANT_AUTH);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = 1000029999;
  	mi.hIcon = IconLibGetIcon("grant_auth");
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Grant authorization");
  	mi.pszService = pszServiceName;
  	hUserMenuGrant = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
  	IconLibReleaseIcon("grant_auth");
  	
  	strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_REVOKE_AUTH);
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = 1000029998;
  	mi.hIcon = IconLibGetIcon("revoke_auth");
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Revoke authorization");
  	mi.pszService = pszServiceName;
  	hUserMenuRevoke = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
  	IconLibReleaseIcon("revoke_auth");
  
  	strcpy(pszServiceName, gpszICQProtoName);
    strcat(pszServiceName, MS_ICQ_ADDSERVCONTACT);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
    mi.position = -2049999999;
    mi.hIcon = IconLibGetIcon("add_to_server");
  	mi.pszContactOwner = gpszICQProtoName;
    mi.pszName = LPGEN("Add to server list");
  	mi.pszService = pszServiceName;
    hUserMenuAddServ = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
    IconLibReleaseIcon("add_to_server");
    strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_XSTATUS_SHOWDETAILS);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = -2000004999;
  	mi.hIcon = NULL; // dynamically updated
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Show custom status details");
  	mi.pszService = pszServiceName;
  	mi.flags=CMIF_NOTOFFLINE;
  	hUserMenuXStatus = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
  	
  
  	strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_SETVIS);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = 2106000000;
  	mi.flags = 0;
  	mi.hIcon = IconLibGetIcon("set_vis");
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Always visible");
  	mi.pszService = pszServiceName;
  	hUserMenuSetVis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
  	IconLibReleaseIcon("set_vis");
  
  	strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_SETINVIS);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = 2107000000;
  	mi.flags = 0;
  	mi.hIcon = IconLibGetIcon("set_invis");
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Always invisible");
  	mi.pszService = pszServiceName;
  	hUserMenuSetInvis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
  	IconLibReleaseIcon("set_invis");
  
  
  	strcpy(pszServiceName, gpszICQProtoName);
  	strcat(pszServiceName, MS_INCOGNITO_REQUEST);
  
  	ZeroMemory(&mi, sizeof(mi));
  	mi.cbSize = sizeof(mi);
  	mi.position = -2000005000;
  	mi.flags = 0;
  	mi.hIcon = IconLibGetIcon("incognito");
  	mi.pszContactOwner = gpszICQProtoName;
  	mi.pszName = LPGEN("Incognito Away-Request");
  	mi.pszService = pszServiceName;
  	hUserMenuIncognito=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
  	IconLibReleaseIcon("incognito");
  	
	  strcpy(pszServiceName, gpszICQProtoName);
	  strcat(pszServiceName, MS_SEND_TZER);

	  ZeroMemory(&mi, sizeof(mi));
	  mi.cbSize = sizeof(mi);
	  mi.position = 2108000000;
	  mi.flags = 0;
	  mi.hIcon = IconLibGetIcon("send_tzer");
	  mi.pszContactOwner = gpszICQProtoName;
	  mi.pszName = LPGEN("Send tZer");
	  mi.pszService = pszServiceName;
	  hUserMenuSendtZer=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
	  IconLibReleaseIcon("send_tzer");

    // eternity : release notes dialog
    {
      int relnotes;
      relnotes = DBGetContactSettingWord(NULL, gpszICQProtoName, "ModRelNotes", 0);
      if (relnotes != ICQ_RELNOTES)
        DialogBox(hInst, MAKEINTRESOURCE(IDD_RELNOTES), NULL, (DLGPROC)RelNotesProc);
    }
    // eternity : release notes dialog END
  }
  return 0;
}



static int OnSystemPreShutdown(WPARAM wParam,LPARAM lParam)
{ // all threads should be terminated here
  if (hServerConn)
  {
    icq_sendCloseConnection();

    icq_serverDisconnect(TRUE);
  }

  icq_InfoUpdateCleanup();

  CheckSelfRemoveShutdown();

  return 0;
}



void CListShowMenuItem(HANDLE hMenuItem, BYTE bShow)
{
  CLISTMENUITEM mi = {0};

  mi.cbSize = sizeof(mi);
  if (bShow)
    mi.flags = CMIM_FLAGS;
  else
    mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

  CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}



void CListSetMenuItemIcon(HANDLE hMenuItem, HICON hIcon)
{
  CLISTMENUITEM mi = {0};

  mi.cbSize = sizeof(mi);
  mi.flags = CMIM_FLAGS | CMIM_ICON;

  mi.hIcon = hIcon;
  CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}



static int icq_PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
  BYTE bXStatus;
  CLISTMENUITEM cli={0};

  CListShowMenuItem(hUserMenuAuth, bShowAuth?1:ICQGetContactSettingByte((HANDLE)wParam, "Auth", 0) && icqOnline);
  CListShowMenuItem(hUserMenuGrant, bShowAuth?1:ICQGetContactSettingByte((HANDLE)wParam, "Grant", 0) && icqOnline);
  CListShowMenuItem(hUserMenuRevoke, icqOnline);
  CListShowMenuItem(hUserMenuSetVis, icqOnline);
  CListShowMenuItem(hUserMenuSetInvis, icqOnline);
  CListShowMenuItem(hUserMenuStatus, icqOnline);
  CListShowMenuItem(hUserMenuIncognito, (/*bStealthRequest && */!bIncognitoGlobal &&
	  (ICQGetContactSettingWord((HANDLE)wParam, "Status", 0)!= ID_STATUS_OFFLINE) &&
	  (ICQGetContactSettingWord((HANDLE)wParam, "Status", 0)!= ID_STATUS_INVISIBLE)  && icqOnline)); //now away messages for online supported

  CListShowMenuItem(hUserMenuSendtZer, icqOnline);
  cli.cbSize=sizeof(CLISTMENUITEM);
  cli.flags=CMIM_FLAGS;
  cli.hIcon=NULL;
  cli.pszContactOwner=NULL;

  switch(ICQGetContactSettingWord((HANDLE)wParam, "ApparentMode",0))
  {
	case ID_STATUS_ONLINE:
		cli.flags|=CMIF_CHECKED;
		CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hUserMenuSetVis,(LPARAM)&cli);
		break;

	case ID_STATUS_OFFLINE:
		cli.flags|=CMIF_CHECKED;
		CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hUserMenuSetInvis,(LPARAM)&cli);
		break;

	default:
		break;
  }


  if (gbSsiEnabled && !ICQGetContactSettingWord((HANDLE)wParam, "ServerId", 0) && !ICQGetContactSettingWord((HANDLE)wParam, "SrvIgnoreId", 0))
    CListShowMenuItem(hUserMenuAddServ, 1);
  else
    CListShowMenuItem(hUserMenuAddServ, 0);
  bXStatus = ICQGetContactSettingByte((HANDLE)wParam, DBSETTING_XSTATUSID, 0);
  CListShowMenuItem(hUserMenuXStatus, (BYTE)(bHideXStatusUI ? 0 : bXStatus));
  if (bXStatus && !bHideXStatusUI)
  {
    CListSetMenuItemIcon(hUserMenuXStatus, GetXStatusIcon(bXStatus, LR_SHARED));
  }

  return 0;
}



static int IconLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
  CListSetMenuItemIcon(hUserMenuAuth, IconLibGetIcon("req_auth"));
  IconLibReleaseIcon("req_auth");
  CListSetMenuItemIcon(hUserMenuGrant, IconLibGetIcon("grant_auth"));
  IconLibReleaseIcon("grant_auth");
  CListSetMenuItemIcon(hUserMenuRevoke, IconLibGetIcon("revoke_auth"));
  IconLibReleaseIcon("revoke_auth");
  CListSetMenuItemIcon(hUserMenuAddServ, IconLibGetIcon("add_to_server"));
  IconLibReleaseIcon("add_to_server");
  CListSetMenuItemIcon(hUserMenuIncognito, IconLibGetIcon("incognito_request"));
  IconLibReleaseIcon("incognito_request");
  CListSetMenuItemIcon(hUserMenuSendtZer, IconLibGetIcon("send_tzer"));
  IconLibReleaseIcon("send_tzer");

  ChangedIconsXStatus();

  return 0;
}



void UpdateGlobalSettings()
{
  if (ghServerNetlibUser)
  {
    NETLIBUSERSETTINGS nlus = {0};

    nlus.cbSize = sizeof(NETLIBUSERSETTINGS);
    if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)ghServerNetlibUser, (LPARAM)&nlus))
    {
      if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTP)
        gbGatewayMode = 1;
      else
        gbGatewayMode = 0;
    }
    else
      gbGatewayMode = 0;
  }

  gbSecureLogin = ICQGetContactSettingByte(NULL, "SecureLogin", DEFAULT_SECURE_LOGIN);
  gbAimEnabled = ICQGetContactSettingByte(NULL, "AimEnabled", DEFAULT_AIM_ENABLED);
  gbUtfEnabled = ICQGetContactSettingByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
  gwAnsiCodepage = ICQGetContactSettingWord(NULL, "AnsiCodePage", DEFAULT_ANSI_CODEPAGE);
  gbDCMsgEnabled = ICQGetContactSettingByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
  gbTempVisListEnabled = ICQGetContactSettingByte(NULL, "TempVisListEnabled", DEFAULT_TEMPVIS_ENABLED);
  gbSsiEnabled = ICQGetContactSettingByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED);
  gbAvatarsEnabled = ICQGetContactSettingByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
  gbXStatusEnabled = ICQGetContactSettingByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);
  gbCustomCapEnabled = ICQGetContactSettingByte(NULL, "customcap", 1);
  gbHideIdEnabled = ICQGetContactSettingByte(NULL, "Hide ID", 1);
  gbRTFEnabled = ICQGetContactSettingByte(NULL, "RTF", 0);
  gbVerEnabled = ICQGetContactSettingByte(NULL, "CurrentVer", 0);
  gbTzerEnabled = ICQGetContactSettingByte(NULL, "tZer", 0);
}
