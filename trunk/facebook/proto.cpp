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

#include "proto.h"

FacebookProto::FacebookProto(const char* proto_name,const TCHAR* username)
{
	m_szProtoName  = mir_strdup( proto_name );
	m_szModuleName = mir_strdup( proto_name );
	m_tszUserName  = mir_tstrdup( username );

	CreateProtoService(m_szModuleName,PS_CREATEACCMGRUI,
		&FacebookProto::SvcCreateAccMgrUI,this);

	HookProtoEvent(ME_OPT_INITIALISE, &FacebookProto::OnOptionsInit, this);

    NETLIBUSER nlu = {0};
    nlu.cbSize = sizeof(nlu);
    nlu.szSettingsModule = m_szModuleName;
    nlu.szDescriptiveName = "Facebook accounts server connection";
    nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS;

    m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	CreateProtoService("Facebook","/Test",
		&FacebookProto::Test,this);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = 0;
	mi.hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_FACEBOOK), IMAGE_ICON, 0, 0, 0);
	mi.pszContactOwner = NULL;
	mi.position = 1000000000;
	mi.pszName = Translate("Test Facebook");
	mi.pszService = "Facebook/Test";

	CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
}

FacebookProto::~FacebookProto( )
{
}

HICON FacebookProto::GetIcon(int index)
{
	if(LOWORD(index) == PLI_PROTOCOL)
	{
		HICON ico = (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)"Facebook_facebook");
		return CopyIcon(ico);
	}
	else
		return 0;
}

int FacebookProto::SvcCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (int)CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_FACEBOOKACCOUNT), 
		 (HWND)lParam, FBAccountProc, (LPARAM)this );
}

int FacebookProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {sizeof(odp)};
	odp.position    = 271828;
	odp.hInstance   = g_hInstance;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab     = LPGENT("Basic");
    odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc  = FBOptionsProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	if(ServiceExists(MS_POPUP_ADDPOPUPT))
	{
		odp.ptszTab     = LPGENT("Popups");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_POPUPS);
		odp.pfnDlgProc  = FBPopupsProc;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

	return 0;
}




