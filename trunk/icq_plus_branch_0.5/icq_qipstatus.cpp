// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Angeli-Ka, Joe Kucera
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
// File name      : $URL $
// Revision       : $Revision $
// Last change on : $Date $
// Last change by : $Author $
//
// DESCRIPTION:
//
//  Support for QIP Statuses
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"
/*
#include "m_cluiframes.h"
#include "m_folders.h"


int bQipStatusMenu = 0;

HANDLE hHookQipIconsRebuild = NULL;
HANDLE hHookQStatusBuild = NULL;
HANDLE hHookQipIconsApply = NULL;

extern HANDLE hIconFolder;
extern HANDLE hqipstatuschanged;
extern HANDLE hqipstatusiconchanged;
extern HANDLE hExtraqipstatus;

HANDLE hQIPStatusHandle[QIPSTATUS_COUNT];

WORD wQIPStatusCap[QIPSTATUS_COUNT + 1] = {
	ICQ_STATUSF_ONLINE,
	ICQ_STATUSF_DEPRESS,
	ICQ_STATUSF_EVIL,
	ICQ_STATUSF_HOME,
	ICQ_STATUSF_WORK,
	ICQ_STATUSF_LUNCH
};

const char* nameQIPStatus[QIPSTATUS_COUNT] = { 
	LPGEN("Depress"),
	LPGEN("Angry"),        
	LPGEN("At Home"),       
	LPGEN("At Work"), 
	LPGEN("Eating")
};


static HANDLE LoadQIPStatusIconLibrary(TCHAR *path, const TCHAR *sub)
{
	TCHAR* p = _tcsrchr(path, '\\');
	HANDLE hLib;

	_tcscpy(p, sub);
	_tcscat(p, _T("\\qipstatus_ICQ.dll"));
	if (hLib = LoadLibrary(path)) return hLib;
	_tcscpy(p, sub);
  _tcscat(p, _T("\\qipstatus_icons.dll"));
	if (hLib = LoadLibrary(path)) return hLib;
  _tcscpy(p, _T("\\"));
	return hLib;
}


static TCHAR *InitQIPStatusIconLibrary(TCHAR *buf, size_t buf_size)
{
	TCHAR path[2*MAX_PATH];

	// get miranda's exe path
	GetModuleFileName(NULL, path, MAX_PATH);

	HMODULE hQIPStatusIconsDLL = (HMODULE)LoadQIPStatusIconLibrary(path, _T("\\Icons"));
	if (!hQIPStatusIconsDLL) // TODO: add "Custom Folders" support
		hQIPStatusIconsDLL = (HMODULE)LoadQIPStatusIconLibrary(path, _T("\\Plugins"));

	if (hQIPStatusIconsDLL)
	{
		_tcsncpy(buf, path, buf_size);

    char ident[MAX_PATH];
		if (LoadStringA(hQIPStatusIconsDLL, IDS_IDENTIFY, ident, sizeof(ident)) == 0 || strcmpnull(ident, "# QIP Status Icons #"))
		{ // library is invalid
			*buf = 0;
		}
		FreeLibrary(hQIPStatusIconsDLL);
	}
	else
		*buf = 0;

	return buf;
}


HICON CIcqProto::getQIPStatusIcon(int bStatus, UINT flags)
{
	HICON icon = NULL;

	//if (bStatus > 0 && bStatus <= QIPSTATUS_COUNT) // TODO: Complete
		icon = hQIPStatusIcons[bStatus - 1]->GetIcon();

	if (flags & LR_SHARED || !icon)
		return icon;
	else
		return CopyIcon(icon);
}



// TODO
void UninitQipStatusEvents()
{
  if (hHookQStatusBuild)
    UnhookEvent(hHookQStatusBuild);

  if (hHookQipIconsRebuild)
    UnhookEvent(hHookQipIconsRebuild);

  if (hHookQipIconsApply)
    UnhookEvent(hHookQipIconsApply);
}


// TODO
static void __fastcall setQIPstatus(BYTE bStatus)
{ 
  WORD nStatus;

//  ICQWriteContactSettingWord(NULL, "ICQStatus", wQIPStatusCap[bStatus]);
  ICQWriteContactSettingWord(NULL, "QIPStatus", wQIPStatusCap[bStatus]);
  nStatus=(wQIPStatusCap[bStatus]==ICQ_STATUS_LUNCH)?ID_STATUS_AWAY:ID_STATUS_ONLINE;
  DBWriteContactSettingWord(NULL, "CList", "Status", nStatus);
  IcqSetStatus(nStatus,0);
}

// TODO
static int menuQIPstatus(WPARAM wParam,LPARAM lParam,LPARAM fParam)
{
  setQIPstatus((BYTE)fParam);
  return 0;
}

// TODO
char* QIPStatusToString(int QipStatus)
{
  int i;
  for (i = 0; i < SIZEOF(wQIPStatusCap); ++i)
    if (QipStatus == wQIPStatusCap[i])
      return (char*)(i?nameQIPStatus[i-1]:"");
  return "";
}


void CIcqProto::InitQIPStatusItems(BOOL bAllowStatus)
{
	CLISTMENUITEM mi;
	int i = 0, len = strlennull(m_szModuleName);
	char srvFce[MAX_PATH + 64];
	char szItem[MAX_PATH + 64];
	int bQIPStatusMenuBuilt = 0;

//	BYTE bQIPStatus = getContactQIPStatus(NULL); // Obsolete IMHO, not used here

	if (!m_bQIPStatusEnabled) return;

	if (bStatusMenu && !bAllowStatus) return;

	// TODO: Will we do it hide-able?
	// Custom Status UI is disabled, no need to continue items' init
	//if (m_bHideQIPStatusUI || m_bHideQIPStatusMenu) return;

	null_snprintf(szItem, sizeof(szItem), ICQTranslate("%s QIP Status"), m_szModuleName);
	mi.cbSize = sizeof(mi);
	mi.pszPopupName = szItem;
	mi.popupPosition= 500085000;
	mi.position = 2000040000;

	for(i = 0; i <= QIPSTATUS_COUNT; i++) 
	{
		null_snprintf(srvFce, sizeof(srvFce), "%s/menuQIPStatus%d", m_szModuleName, i);

		mi.position++;

		if (!i) 
			bQIPStatusMenuBuilt = ServiceExists(srvFce);

		if (!bQIPStatusMenuBuilt)
			CreateProtoServiceParam(srvFce+len, &CIcqProto::menuQIPStatus, i);

		mi.flags = (i ? CMIF_ICONFROMICOLIB : 0) | (bXStatus == i?CMIF_CHECKED:0);
		mi.icolibItem = i ? hQIPStatusIcons[i-1]->Handle() : NULL;
		mi.pszName = i ? (char*)nameQIPStatus[i-1] : (char *)LPGEN("None");
		mi.pszService = srvFce;
		mi.pszContactOwner = m_szModuleName;

		if (bStatusMenu)
			hQIPStatusItems[i] = (HANDLE)CallService(MS_CLIST_ADDSTATUSMENUITEM, (WPARAM)&m_hQIPStatusRoot, (LPARAM)&mi);
		else
			hQIPStatusItems[i] = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

		// CMIF_HIDDEN does not work for adding services
		//CListShowMenuItem(hQIPStatusItems[i], !(m_bHideQIPStatusUI || m_bHideQIPStatusMenu));
	}
}

void InitQipStatusItems()
{
  CLISTMENUITEM mi = {0};
  BYTE i = 0;
  char srvFce[MAX_PATH + 64], szItem[MAX_PATH + 64];
  int bQipStatusMenuBuilt = 0;
  int status;

  if (!gbQipStatusEnabled)
	  return;

    null_snprintf(szItem, sizeof(szItem), "%s", Translate("QIP Status"));
    mi.cbSize = sizeof(mi);
    mi.pszPopupName = szItem;
    mi.popupPosition= 500085000;
    mi.position = 2000050000;
    status = GetQipStatusID(NULL);

    for(i = 0; i < 6; i++)
    {
	null_snprintf(srvFce, sizeof(srvFce), "%s/menuQIPstatus%d", gpszICQProtoName, i);
    	mi.position++;

  	if (!i) 
		bQipStatusMenuBuilt = ServiceExists(srvFce);

	if (!bQipStatusMenuBuilt)
		CreateServiceFunctionParam(srvFce, menuQIPstatus,i);

	mi.flags = (i?CMIF_ICONFROMICOLIB:0) | ((status==i)?CMIF_CHECKED:0);
	mi.icolibItem = i ? hQIPStatusHandle[i-1] : NULL;

	mi.pszName = i ? (char*)nameQIPStatus[i-1] : LPGEN("None");
	mi.pszService = srvFce;
	mi.pszContactOwner = gpszICQProtoName;

	hQIPStatusItems[i] = (HANDLE)CallService(MS_CLIST_ADDSTATUSMENUITEM, (WPARAM)&hQIPStatusRoot, (LPARAM)&mi);
    }
}

// TODO
void InitQipStatusIcons()
{
  char szSection[MAX_PATH + 64];
  char str[MAX_PATH], prt[MAX_PATH];
  char lib[2*MAX_PATH] = {0};
  char* icon_lib;
  BYTE i;

  if (!gbQipStatusEnabled) return;
  icon_lib = InitQIPStatusIconLibrary(lib);
  null_snprintf(szSection, sizeof(szSection), ICQTranslateUtfStatic("Status Icons/%s/QIP Status", str, MAX_PATH), ICQTranslateUtfStatic(gpszICQProtoName, prt, MAX_PATH));
  for (i = 0; i < SIZEOF(hQIPStatusHandle); i++)
  {
    char szTemp[64];
    null_snprintf(szTemp, sizeof(szTemp), "QIPstatus%i", i);
    hQIPStatusHandle[i] = IconLibDefine(nameQIPStatus[i], szSection, szTemp, 0, icon_lib, -(IDI_ADD1+i), 0);
  }
  // initialize arrays for CList custom status icons
  memset(bQipStatusCListIconsValid,0,sizeof(bQipStatusCListIconsValid));
  memset(hQipStatusCListIcons,-1,sizeof(hQipStatusCListIcons));
}

// TODO
int GetQipStatusID(HANDLE hContact)
{
	int i = 0;
	switch (ICQGetContactSettingWord(hContact, "ICQStatus", 0))
	{
	case ICQ_STATUS_DEPRESS:
		i=1;
		break;
	case ICQ_STATUS_EVIL:
		i=2;
		break;
	case ICQ_STATUS_HOME:
		i=3;
		break;
	case ICQ_STATUS_WORK:
		i=4;
		break;
	case ICQ_STATUS_LUNCH:
		i=5;
		break;
	}
	return i;
}

// TODO
void ChangedIconsQipStatus()
{
  memset(bQipStatusCListIconsValid,0,sizeof(bQipStatusCListIconsValid));
}

*/