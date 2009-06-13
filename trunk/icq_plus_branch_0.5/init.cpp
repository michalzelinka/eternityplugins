// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
// Copyright © 2006-2009 BM, SSS, jarvis, S!N, persei and others
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
// File name      : $URL: https://miranda.svn.sourceforge.net/svnroot/miranda/trunk/miranda/protocols/IcqOscarJ/init.cpp $
// Revision       : $Revision: 8822 $
// Last change on : $Date: 2009-01-11 18:17:05 +0100 (Sun, 11 Jan 2009) $
// Last change by : $Author: jokusoftware $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"
#include "m_extraicons.h" // TODO: shouldn't this be in icqoscar.h? x)

HINSTANCE hInst;
PLUGINLINK* pluginLink;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
MD5_INTERFACE md5i;
LIST_INTERFACE li;

CLIST_INTERFACE* pcli;

BYTE gbUnicodeCore;
DWORD MIRANDA_VERSION;

HANDLE hStaticServices[1];
IcqIconHandle hStaticIcons[19];
HANDLE hStaticHooks[1];
HANDLE hExtraXStatus = NULL;
HANDLE hExtraDCIcon = NULL;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"IcqOscarJ "ICQ_PLUG_NAME" Protocol",
	PLUGIN_MAKE_VERSION(0,5,1,1),
	"Extended support for ICQ network. [Based on ICQOscarJ 0.5.1.1 SVN 10050]",
	"Joe Kucera, Bio, Martin Öberg, Richard Hughes, Jon Keating, sss, persei, jarvis, nullbie and others",
	"icq@miranda.im",
	"(C) 2000-2009 M.Öberg, R.Hughes, J.Keating, Bio, Angeli-Ka, G.Hazan, J.Kucera, sss, persei, jarvis, nullbie",
	"http://icq.miranda.im/",
	UNICODE_AWARE,
	0,   //doesn't replace anything built-in
	// ICQJ Plus Mod UUID
	#if defined( _UNICODE )
	{0xc78614bf, 0x878e, 0x43e4, { 0xa3, 0xf4, 0xa,  0xf9, 0x9a, 0xa5, 0x7a, 0x78 }} // {73A9615C-7D4E-4555-BADB-EE05DC928EFF}
	#else
	{0xc78614bf, 0x878e, 0x43e4, { 0xa3, 0xf4, 0xa,  0xf9, 0x9a, 0xa5, 0x7a, 0x87 }} // {89CF4C3D-7014-4658-A13B-46DB4968C73B}
	#endif
};

extern "C" PLUGININFOEX __declspec(dllexport) *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	// Only load for 0.8.0.33 or greater
	// We need the core stubs for PS_GETNAME and PS_GETSTATUS, ExtraIcons services
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 0, 33))
	{
		MessageBoxA( NULL, "ICQ plugin cannot be loaded. It requires Miranda IM 0.8.0.33 or later.", "ICQ Plugin",
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}

	MIRANDA_VERSION = mirandaVersion;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static PROTO_INTERFACE* icqProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	return new CIcqProto( pszProtoName, tszUserName );
}

static int icqProtoUninit( PROTO_INTERFACE* ppro )
{
	delete ( CIcqProto* )ppro;
	return 0;
}

static int OnModulesLoaded( WPARAM, LPARAM )
{
	hExtraXStatus = ExtraIcon_Register("xstatus", "ICQ XStatus");
	hExtraDCIcon = ExtraIcon_Register("dconnection", "ICQ Direct Connection");
	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLI( &li );
	mir_getMMI( &mmi );
	mir_getUTFI( &utfi );
	mir_getMD5I( &md5i );

	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)hInst); // >:P

	{ // Are we running under unicode Miranda core ?
		char szVer[MAX_PATH];

		CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
		_strlwr(szVer); // make sure it is lowercase
		gbUnicodeCore = (strstrnull(szVer, "unicode") != NULL);

		if (strstrnull(szVer, "alpha") != NULL)
		{ // Are we running under Alpha Core
			MIRANDA_VERSION |= 0x80000000;
		}
		else if (MIRANDA_VERSION >= 0x00050000 && strstrnull(szVer, "preview") == NULL)
		{ // for Final Releases of Miranda 0.5+ clear build number
			MIRANDA_VERSION &= 0xFFFFFF00;
		}
	}

	srand(time(NULL));
	_tzset();

	// Register the module
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize   = sizeof(pd);
	pd.szName   = ICQ_PROTOCOL_NAME;
	pd.type     = PROTOTYPE_PROTOCOL;
	pd.fnInit   = icqProtoInit;
	pd.fnUninit = icqProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// Initialize charset conversion routines
	InitI18N();

	// Register static services
	hStaticServices[0] = CreateServiceFunction(ICQ_DB_GETEVENTTEXT_MISSEDMESSAGE, icq_getEventTextMissedMessage);

	{ // Define global icons
		TCHAR lib[MAX_PATH];
		char szSectionName[MAX_PATH];
		char szProtocolsBuf[100], szNameBuf[100];

		null_snprintf(szSectionName, sizeof(szSectionName), "%s/%s", 
		  ICQTranslateUtfStatic(LPGEN("Protocols"), szProtocolsBuf, sizeof(szProtocolsBuf)), 
		  ICQTranslateUtfStatic(ICQ_PROTOCOL_NAME, szNameBuf, sizeof(szNameBuf)));

		GetModuleFileName(hInst, lib, MAX_PATH);
		hStaticIcons[ISI_AUTH_REQUEST] = IconLibDefine(LPGEN("Request authorization"), szSectionName, NULL, "req_auth", lib, -IDI_AUTH_ASK);
		hStaticIcons[ISI_AUTH_GRANT] = IconLibDefine(LPGEN("Grant authorization"), szSectionName, NULL, "grant_auth", lib, -IDI_AUTH_GRANT);
		hStaticIcons[ISI_AUTH_REVOKE] = IconLibDefine(LPGEN("Revoke authorization"), szSectionName, NULL, "revoke_auth", lib, -IDI_AUTH_REVOKE);
		hStaticIcons[ISI_ADD_TO_SERVLIST] = IconLibDefine(LPGEN("Add to server list"), szSectionName, NULL, "add_to_server", lib, -IDI_SERVLIST_ADD);
		//
		hStaticIcons[ISI_SEND_TZER] = IconLibDefine(LPGEN("Send tZer"), szSectionName, NULL, "send_tzer", lib, -IDI_TZER);
		hStaticIcons[ISI_ALWAYS_VISIBLE] = IconLibDefine(LPGEN("Always visible"), szSectionName, NULL, "always_visible", lib, -IDI_VISIBLE);
		hStaticIcons[ISI_NEVER_VISIBLE] = IconLibDefine(LPGEN("Never visible"), szSectionName, NULL, "never_visible", lib, -IDI_INVISIBLE);
		hStaticIcons[ISI_INCOGNITO_REQUEST] = IconLibDefine(LPGEN("Incognito Request"), szSectionName, NULL, "incognito_request", lib, -IDI_INCOGNITO);
		hStaticIcons[ISI_SCAN] = IconLibDefine(LPGEN("Scan"), szSectionName, NULL, "scan", lib, -IDI_SCAN);
		hStaticIcons[ISI_SCAN_START] = IconLibDefine(LPGEN("Start Scan"), szSectionName, NULL, "scan_start", lib, -IDI_SCAN_START);
		hStaticIcons[ISI_SCAN_STOP] = IconLibDefine(LPGEN("Stop Scan"), szSectionName, NULL, "scan_stop", lib, -IDI_SCAN_STOP);
		hStaticIcons[ISI_SCAN_PAUSE] = IconLibDefine(LPGEN("Pause Scan"), szSectionName, NULL, "scan_pause", lib, -IDI_SCAN_PAUSE);
		hStaticIcons[ISI_HIDDEN] = IconLibDefine(LPGEN("Hidden"), szSectionName, NULL, "hidden", lib, -IDI_HIDDEN);
		hStaticIcons[ISI_MANAGE_SERVER_LIST] = IconLibDefine(LPGEN("Server List"), szSectionName, NULL, "servlist", lib, -IDI_SERVLIST);
		hStaticIcons[ISI_ADVANCED_FEATURES] = IconLibDefine(LPGEN("Advanced Features"), szSectionName, NULL, "advanced_features", lib, -IDI_ADVANCED_FEATURES);
		hStaticIcons[ISI_DISABLED_ITEM] = IconLibDefine(LPGEN("Unchecked item"), szSectionName, NULL, "unchecked", lib, -IDI_UNCHECKED);
		hStaticIcons[ISI_ENABLED_ITEM] = IconLibDefine(LPGEN("Checked item"), szSectionName, NULL, "checked", lib, -IDI_CHECKED);
		hStaticIcons[ISI_DIRECT_CONNECT] = IconLibDefine(LPGEN("Direct Connection"), szSectionName, NULL, "dcico", lib, -IDI_DC);
		//
	}

	hStaticHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	int i;

	// Release static icon handles
	for (i = 0; i < SIZEOF(hStaticIcons); i++)
		IconLibRemove(&hStaticIcons[i]);

	// Release static event hooks
	for (i = 0; i < SIZEOF(hStaticHooks); i++)
		if (hStaticHooks[i])
			UnhookEvent(hStaticHooks[i]);

	// Destroy static service functions
	for (i = 0; i < SIZEOF(hStaticServices); i++)
		if (hStaticServices[i])
			DestroyServiceFunction(hStaticServices[i]);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnPrebuildContactMenu event

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

static void CListSetMenuItemIcon(HANDLE hMenuItem, HICON hIcon)
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;

	mi.hIcon = hIcon;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}


int CIcqProto::OnPreBuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	CListShowMenuItem(m_hContactMenuItems[ICMI_AUTH_REQUEST], getSettingByte((HANDLE)wParam, "Auth", 0));
	CListShowMenuItem(m_hContactMenuItems[ICMI_AUTH_GRANT], getSettingByte((HANDLE)wParam, "Grant", 0));
	CListShowMenuItem(m_hContactMenuItems[ICMI_AUTH_REVOKE], getSettingByte(NULL, "PrivacyItems", 0) && !getSettingByte((HANDLE)wParam, "Grant", 0));
	if (m_bSsiEnabled && !getSettingWord((HANDLE)wParam, DBSETTING_SERVLIST_ID, 0) && !getSettingWord((HANDLE)wParam, DBSETTING_SERVLIST_IGNORE, 0))
		CListShowMenuItem(m_hContactMenuItems[ICMI_ADD_TO_SERVLIST], 1);
	else
		CListShowMenuItem(m_hContactMenuItems[ICMI_ADD_TO_SERVLIST], 0);
	//
	CListShowMenuItem(m_hContactMenuItems[ICMI_SCAN], m_bASD);
	CListShowMenuItem(m_hContactMenuItems[ICMI_SEND_TZER], m_bTzersEnabled);
	CListShowMenuItem(m_hContactMenuItems[ICMI_INCOGNITO_REQUEST], m_bIncognitoRequest && !m_bIncognitoGlobal);
	//

	BYTE bXStatus = getContactXStatus((HANDLE)wParam);

	CListShowMenuItem(m_hContactMenuItems[ICMI_XSTATUS_DETAILS], (BYTE)(m_bHideXStatusUI ? 0 : bXStatus));
	if (bXStatus && !m_bHideXStatusUI)
		CListSetMenuItemIcon(m_hContactMenuItems[ICMI_XSTATUS_DETAILS], getXStatusIcon(bXStatus, LR_SHARED));

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// OnPrebuildContactMenu event

int CIcqProto::OnPreBuildStatusMenu(WPARAM wParam, LPARAM lParam)
{
	InitXStatusItems(TRUE);
	InitAdvancedFeaturesItems();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnReloadIcons event

int CIcqProto::OnReloadIcons(WPARAM wParam, LPARAM lParam)
{
	memset(bXStatusCListIconsValid, 0, sizeof(bXStatusCListIconsValid));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UpdateGlobalSettings event

void CIcqProto::UpdateGlobalSettings()
{
	if (m_hServerNetlibUser)
	{
		NETLIBUSERSETTINGS nlus = {0};

		nlus.cbSize = sizeof(NETLIBUSERSETTINGS);
		if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)m_hServerNetlibUser, (LPARAM)&nlus))
		{
			if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTP)
				m_bGatewayMode = 1;
			else
				m_bGatewayMode = 0;
		}
		else
			m_bGatewayMode = 0;
	}

	m_bSecureLogin = getSettingByte(NULL, "SecureLogin", DEFAULT_SECURE_LOGIN);
	m_bSecureConnection = getSettingByte(NULL, "SecureConnection", DEFAULT_SECURE_CONNECTION);
	m_bAimEnabled = getSettingByte(NULL, "AimEnabled", DEFAULT_AIM_ENABLED);
	m_bUtfEnabled = getSettingByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
	m_wAnsiCodepage = getSettingWord(NULL, "AnsiCodePage", DEFAULT_ANSI_CODEPAGE);
	m_bDCMsgEnabled = getSettingByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
	m_bTempVisListEnabled = getSettingByte(NULL, "TempVisListEnabled", DEFAULT_TEMPVIS_ENABLED);
	m_bSsiEnabled = getSettingByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED);
	m_bSsiSimpleGroups = FALSE; // TODO: enable, after server-list revolution is over
	m_bAvatarsEnabled = getSettingByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
	m_bXStatusEnabled = getSettingByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);

	// Plus: place here ALL account globals used f.e. for making decisions ///

	m_bIncognitoRequest = getSettingByte(NULL, "IncognitoRequest", DEFAULT_INCOGNITO_REQUEST);
	m_bIncognitoGlobal = getSettingByte(NULL, "IncognitoGlobal", DEFAULT_INCOGNITO_GLOBAL);
	m_bNoStatusReply = getSettingByte(NULL, "NoStatusReply", DEFAULT_NO_STATUS_REPLY);
	m_bTzersEnabled = getSettingByte(NULL, "TzersEnabled", DEFAULT_TZERS_ENABLED);
	m_bCustomProtoVersionEnabled = getSettingByte(NULL, "CustomProtoVersionEnabled", DEFAULT_CUSTOM_PROTO_VERSION_ENABLED);

	m_bASD = getSettingByte(NULL, "ASD", DEFAULT_ASD_ENABLED);
	m_bPSD = getSettingByte(NULL, "PSD", DEFAULT_PSD_ENABLED);
	m_bNoASDInInvisible = getSettingByte(NULL, "NoASDInInvisible", DEFAULT_NO_ASD_IN_INVISIBLE);
	m_bNoPSDForHidden = getSettingByte(NULL, "NoPSDForHidden", DEFAULT_NO_PSD_FOR_HIDDEN);
	m_bASDForOffline = getSettingByte(NULL, "ASDForOffline", DEFAULT_ASD_FOR_OFFLINE);
	m_bASDViaAwayMsg = getSettingByte(NULL, "ASDViaAwayMsg", DEFAULT_DETECT_VIA_STATUS_MESSAGE);
	m_bASDViaXtraz = getSettingByte(NULL, "ASDViaXtraz", DEFAULT_DETECT_VIA_XTRAZ);
	m_bASDViaURL = getSettingByte(NULL, "ASDViaURL", DEFAULT_DETECT_VIA_URL);
	m_bASDUnauthorized = getSettingByte(NULL, "ASDUnauthorized", DEFAULT_DETECT_UNAUTHORIZED);
	m_bASDViaAuth = getSettingByte(NULL, "ASDViaAuth", DEFAULT_DETECT_VIA_AUTH);
	m_wCheckSpeedDefault = getSettingWord(NULL, "_defSpeed", DEFAULT_STATUS_CHECK_SPEED);
	m_wCheckSpeed = m_wCheckSpeedDefault;

	m_bLogSelfRemoveFile = getSettingByte(NULL, "LogSelfRemoveFile", DEFAULT_SELFREMOVE_LOG_TO_FILE);
	m_bLogIgnoreCheckFile = getSettingByte(NULL, "LogIgnoreCheckFile", DEFAULT_IGNORECHECK_LOG_TO_FILE);
	m_bLogStatusCheckFile = getSettingByte(NULL, "LogStatusCheckFile", DEFAULT_STATUSCHECK_LOG_TO_FILE);
	m_bLogClientChangeFile = getSettingByte(NULL, "LogClientChangeFile", DEFAULT_CLIENTCHANGE_LOG_TO_FILE);
	m_bLogAuthFile = getSettingByte(NULL, "LogAuthFile", DEFAULT_AUTH_LOG_TO_FILE);
	m_bLogInfoRequestFile = getSettingByte(NULL, "LogRequestFile", DEFAULT_INFOREQUEST_LOG_TO_FILE);
	m_bLogASDFile = getSettingByte(NULL, "LogASDFile", DEFAULT_ASD_LOG_TO_FILE);
	m_bLogSelfRemoveHistory = getSettingByte(NULL, "LogSelfRemoveHistory", DEFAULT_SELFREMOVE_LOG_TO_HISTORY);
	m_bLogIgnoreCheckHistory = getSettingByte(NULL, "LogIgnoreCheckHistory", DEFAULT_IGNORECHECK_LOG_TO_HISTORY);
	m_bLogStatusCheckHistory = getSettingByte(NULL, "LogStatusCheckHistory", DEFAULT_STATUSCHECK_LOG_TO_HISTORY);
	m_bLogClientChangeHistory = getSettingByte(NULL, "LogClientChangeHistory", DEFAULT_CLIENTCHANGE_LOG_TO_HISTORY);
	m_bLogAuthHistory = getSettingByte(NULL, "LogAuthHistory", DEFAULT_AUTH_LOG_TO_HISTORY);
	m_bLogInfoRequestHistory = getSettingByte(NULL, "LogRequestHistory", DEFAULT_INFOREQUEST_LOG_TO_HISTORY);
	m_bLogASDHistory = getSettingByte(NULL, "LogASDHistory", DEFAULT_ASD_LOG_TO_HISTORY);
	m_bLogToContactsHistory = getSettingByte(NULL, "LogToContactsHistory", DEFAULT_LOG_TO_CONTACTS_HISTORY);

	m_bHideIdEnabled = getSettingByte(NULL, "HideRealID", DEFAULT_HIDE_REAL_ID);
	m_bCustomProtoVersionEnabled = getSettingByte(NULL, "ProtoVersionUseOwn", DEFAULT_PROTO_VERSION_USE_OWN);
	m_bCustomCapsEnabled = getSettingByte(NULL, "CustomCaps", DEFAULT_USE_CUSTOM_CAPS);

	m_bVisibility = getSettingByte(NULL, "SrvVisibility", DEFAULT_VISIBILITY_MODE);
	m_bWebAware = getSettingByte(NULL, "WebAware", DEFAULT_WEB_AWARE);

	//m_iIcqNewStatus = ID_STATUS_OFFLINE; Do NOT initiate!
	// Otherwise server looping will be broken -- each new login attemp sets globals once again

	m_bServerAutoChange = getSettingByte(NULL, "ServerAutoChange", DEFAULT_SERVER_AUTOCHANGE);
	m_bRTFEnabled = getSettingByte(NULL, "RTF", DEFAULT_RTF);

	m_bSecureIM = ServiceExists("SecureIM/IsContactSecured");

	// initiate custom capabilities list
	mir_getLI(&li);
	lstCustomCaps = li.List_Create(0, 1);
	lstCustomCaps->sortFunc = NULL;

	/* TODO + move to separate function to be usable in Options > Client ID, too
	{
		char tmp[MAXMODULELABELLENGTH];
		DBCONTACTENUMSETTINGS dbces;
		mir_snprintf(tmp, MAXMODULELABELLENGTH, "%sCaps", m_szModuleName);
		dbces.pfnEnumProc = CIcqProto::EnumCustomCapsProc;
		dbces.lParam = (LPARAM)tmp;
		dbces.szModule = tmp;
		CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
	}
	*/
}
