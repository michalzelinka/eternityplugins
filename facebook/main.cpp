/*
Copyright © 2009 Michal Zelinka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include <list>
#include <string>
#include <iostream>
using namespace std;

PLUGINLINK *pluginLink;
MD5_INTERFACE md5i;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
LIST_INTERFACE li;
XML_API xi;

CLIST_INTERFACE* pcli;

HINSTANCE g_hInstance;

DWORD g_mirandaVersion;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Facebook Plugin",
	__VERSION_DWORD,
	"Provides basic support for Facebook protocol. [Built: "__DATE__" "__TIME__"]",
	"Michal Zelinka",
	"jarvis@jabber.cz",
	"© 2009 Michal Zelinka",
	"http://dev.miranda.im/~jarvis/",
	UNICODE_AWARE, //not transient
	0,             //doesn't replace anything built-in
	// {DDE897AD-E40A-4e2c-98D8-D20B575E96BC}
	{ 0xdde897ad, 0xe40a, 0x4e2c, { 0x98, 0xd8, 0xd2, 0xb, 0x57, 0x5e, 0x96, 0xbc } }

};

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int compare_protos(const FacebookProto *p1, const FacebookProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<FacebookProto> g_Instances(1, compare_protos);

DWORD WINAPI DllMain(HINSTANCE hInstance,DWORD,LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if(mirandaVersion < PLUGIN_MAKE_VERSION(0,8,0,29))
	{
		MessageBox(0,_T("The Facebook protocol plugin cannot be loaded. ")
			_T("It requires Miranda IM 0.8.0.29 or later."),_T("Miranda"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}

	g_mirandaVersion = mirandaVersion;
	/*unsigned long mv=_htonl(mirandaVersion);
	memcpy(&AIM_CAP_MIRANDA[8],&mv,sizeof(DWORD));
	memcpy(&AIM_CAP_MIRANDA[12],AIM_OSCAR_VERSION,sizeof(DWORD));*/
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char *proto_name,const TCHAR *username )
{
	FacebookProto *proto = new FacebookProto(proto_name,username);
	g_Instances.insert(proto);
	return proto;
}

static int protoUninit(PROTO_INTERFACE *proto)
{
	g_Instances.remove(static_cast<FacebookProto*>(proto));
	return 0;
}

int OnModulesLoaded(WPARAM,LPARAM)
{
	if ( ServiceExists( MS_UPDATE_REGISTER ) )
	{
		Update upd = {sizeof(upd)};
		char curr_version[30];

		upd.szComponentName = pluginInfo.shortName;
		upd.szUpdateURL = UPDATER_AUTOREGISTER;
		upd.szBetaVersionURL     = "http://dev.miranda.im/~jarvis/";
		upd.szBetaChangelogURL   = "http://dev.miranda.im/~jarvis/";
		upd.pbBetaVersionPrefix  = reinterpret_cast<BYTE*>("Facebook ");
		upd.cpbBetaVersionPrefix = strlen(reinterpret_cast<char*>(upd.pbBetaVersionPrefix));
		upd.szBetaUpdateURL      = "http://dev.miranda.im/~jarvis/";
		upd.pbVersion = reinterpret_cast<BYTE*>( CreateVersionStringPlugin(
			reinterpret_cast<PLUGININFO*>(&pluginInfo),curr_version) );
		upd.cpbVersion = strlen(reinterpret_cast<char*>(upd.pbVersion));
		CallService(MS_UPDATE_REGISTER,0,(LPARAM)&upd);
	}

	//// signature test
	//list< string >* a = new list< string >( );
	//a->push_back( "abc" );
	//a->push_back( "def" );
	//SignatureUtil::generateSignature( *(a), "secret" );

	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getMD5I(&md5i);
	mir_getUTFI(&utfi);
	mir_getLI(&li);
	mir_getXI(&xi);

	pcli = reinterpret_cast<CLIST_INTERFACE*>( CallService(
		MS_CLIST_RETRIEVE_INTERFACE,0,reinterpret_cast<LPARAM>(g_hInstance)) );

	PROTOCOLDESCRIPTOR pd = {sizeof(pd)};
	pd.szName = "Facebook";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE,0,reinterpret_cast<LPARAM>(&pd));

	HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);

	InitIcons();
	//InitContactMenus();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	//UninitContactMenus();
	return 0;
}

