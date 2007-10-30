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
// Revision       : $Revision: 51 $
// Last change on : $Date: 2007-08-30 23:46:51 +0300 (Ð§Ñ‚, 30 Ð°Ð²Ð³ 2007) $
// Last change by : $Author: sss123next $
//
// DESCRIPTION:
//
//  Support for IcoLib plug-in
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"
#include "m_icolib.h"


void InitIconLib()
{
  char str[MAX_PATH], proto[MAX_PATH], lib[MAX_PATH];
    
	ICQTranslateUtfStatic(gpszICQProtoName, proto, MAX_PATH);

  GetModuleFileName(hInst, lib, MAX_PATH);

	IconLibDefine(ICQTranslateUtfStatic("Protocol icon", str, MAX_PATH), proto, "proto", NULL, lib, -IDI_ICQ);
  IconLibDefine(ICQTranslateUtfStatic("Request authorization", str, MAX_PATH), proto, "req_auth", NULL, lib, -IDI_AUTH);
  IconLibDefine(ICQTranslateUtfStatic("Grant authorization", str, MAX_PATH), proto, "grant_auth", NULL, lib, -IDI_GRANT);
  IconLibDefine(ICQTranslateUtfStatic("Revoke authorization", str, MAX_PATH), proto, "revoke_auth", NULL, lib, -IDI_REMOVE);
  IconLibDefine(ICQTranslateUtfStatic("Add to server list", str, MAX_PATH), proto, "add_to_server", NULL, lib, -IDI_SERVLIST_ADD);
  IconLibDefine(ICQTranslateUtfStatic("Send tZer", str, MAX_PATH), proto, "send_tzer", NULL, lib, -IDI_SEND_TZER);
	IconLibDefine(ICQTranslateUtfStatic("Always visible", str, MAX_PATH), proto, "set_vis", NULL, lib, -IDI_SET_VIS);
	IconLibDefine(ICQTranslateUtfStatic("Never visible", str, MAX_PATH), proto, "set_invis", NULL, lib, -IDI_SET_INVIS);
	IconLibDefine(ICQTranslateUtfStatic("Incognito Request", str, MAX_PATH), proto, "incognito", NULL, lib, -IDI_INCOGNITO_R);
	IconLibDefine(ICQTranslateUtfStatic("Scan", str, MAX_PATH), proto, "scan", NULL, lib, -IDI_SCAN);
	IconLibDefine(ICQTranslateUtfStatic("Scan start", str, MAX_PATH), proto, "start", NULL, lib, -IDI_START);
	IconLibDefine(ICQTranslateUtfStatic("Scan stop", str, MAX_PATH), proto, "stop", NULL, lib, -IDI_STOP);
	IconLibDefine(ICQTranslateUtfStatic("Scan pause", str, MAX_PATH), proto, "pause", NULL, lib, -IDI_PAUSE);
	IconLibDefine(ICQTranslateUtfStatic("Hidden", str, MAX_PATH), proto, "was_found", NULL, lib, -IDI_HIDDEN);
	IconLibDefine(ICQTranslateUtfStatic("Manage Server List", str, MAX_PATH), proto, "servlist", NULL, lib, -IDI_LIST);
	IconLibDefine(ICQTranslateUtfStatic("Advanced Features Menu", str, MAX_PATH), proto, "privacy", NULL, lib, -IDI_PRIVACY);
	IconLibDefine(ICQTranslateUtfStatic("Disable menu item", str, MAX_PATH), proto, "dot", NULL, lib, -IDI_DOT);
	IconLibDefine(ICQTranslateUtfStatic("Enable menu item", str, MAX_PATH), proto, "check", NULL, lib, -IDI_CHECK);

  // nothing
}



HANDLE IconLibDefine(const char* desc, const char* section, const char* ident, HICON icon, const char* def_file, int def_idx)
{
  SKINICONDESC sid = {0};
  char szTemp[MAX_PATH + 128];
  HANDLE hIcon;

  sid.cbSize = SKINICONDESC_SIZE;
  sid.pwszSection = make_unicode_string(section);
  sid.pwszDescription = make_unicode_string(ICQTranslateUtfStatic(desc, szTemp, MAX_PATH));
  sid.flags = SIDF_UNICODE;

  null_snprintf(szTemp, sizeof(szTemp), "%s_%s", gpszICQProtoName, ident);
  sid.pszName = szTemp;
  sid.pszDefaultFile = (char*)def_file;
  sid.iDefaultIndex = def_idx;
  sid.hDefaultIcon = icon;
  sid.cx = sid.cy = 16;

  hIcon = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

  SAFE_FREE(&sid.pwszSection);
  SAFE_FREE(&sid.pwszDescription);

  return hIcon;
}



HICON IconLibGetIcon(const char* ident)
{
  char szTemp[MAX_PATH + 128];

  null_snprintf(szTemp, sizeof(szTemp), "%s_%s", gpszICQProtoName, ident);

  return (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szTemp);
}



void IconLibReleaseIcon(const char* ident)
{
  char szTemp[MAX_PATH + 128];

  null_snprintf(szTemp, sizeof(szTemp), "%s_%s", gpszICQProtoName, ident);
  CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)szTemp);
}



HANDLE IconLibHookIconsChanged(MIRANDAHOOK hook)
{
  return HookEvent(ME_SKIN2_ICONSCHANGED, hook);
}
