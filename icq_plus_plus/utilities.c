// eternity modified file
// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006,2007 Joe Kucera
// Copyright © 2006,2007,2008 [sss], chaos.persei, [sin], Faith Healer, Thief, Angeli-Ka, nullbie
// Copyright © 2007,2008 jarvis
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
// File name      : $HeadURL: https://icqjplusmod.googlecode.com/svn/trunk/utilities.c $
// Revision       : $Revision: 51 $
// Last change on : $Date: 2007-08-30 23:46:51 +0300 (Ð§Ñ‚, 30 Ð°Ð²Ð³ 2007) $
// Last change by : $Author: sss123next $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"



typedef struct gateway_index_s
{
  HANDLE hConn;
  DWORD  dwIndex;
} gateway_index;

extern CRITICAL_SECTION cookieMutex; 

static gateway_index *gateways = NULL;
static int gatewayCount = 0;

static DWORD *spammerList = NULL;
static int spammerListCount = 0;


static icq_contacts_cache *contacts_cache = NULL;
static int cacheCount = 0;
static int cacheListSize = 0;
static CRITICAL_SECTION cacheMutex;

extern BOOL bIsSyncingCL;


void EnableDlgItem(HWND hwndDlg, UINT control, int state)
{
  EnableWindow(GetDlgItem(hwndDlg, control), state);
}



void icq_EnableMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
  int i;

  for (i = 0; i < cControls; i++)
    EnableDlgItem(hwndDlg, controls[i], state);
}



void icq_ShowMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
  int i;

  for(i = 0; i < cControls; i++)
    ShowWindow(GetDlgItem(hwndDlg, controls[i]), state);
}


// Maps the ICQ status flag (as seen in the status change SNACS) and returns
// a Miranda style status.
int IcqStatusToMiranda(WORD nIcqStatus)
{
  int nMirandaStatus;

  // :NOTE: The order in which the flags are compared are important!
  // I dont like this method but it works.

  if (nIcqStatus & ICQ_STATUSF_INVISIBLE)
    nMirandaStatus = ID_STATUS_INVISIBLE;
  else
    if (nIcqStatus & ICQ_STATUSF_DND)
      nMirandaStatus = ID_STATUS_DND;
  else
    if (nIcqStatus & ICQ_STATUSF_OCCUPIED)
      nMirandaStatus = ID_STATUS_OCCUPIED;
  else
    if (nIcqStatus & ICQ_STATUSF_NA)
      nMirandaStatus = ID_STATUS_NA;
  else
    if (nIcqStatus & ICQ_STATUSF_AWAY)
      nMirandaStatus = ID_STATUS_AWAY;
  else
    if (nIcqStatus & ICQ_STATUSF_FFC)
      nMirandaStatus = ID_STATUS_FREECHAT;
  else
    // Can be discussed, but I think 'online' is the most generic ICQ status
    nMirandaStatus = ID_STATUS_ONLINE;

  return nMirandaStatus;
}



WORD MirandaStatusToIcq(int nMirandaStatus)
{
  WORD nIcqStatus;


  switch (nMirandaStatus)
  {

  case ID_STATUS_ONLINE:
    nIcqStatus = ICQ_STATUS_ONLINE;
    break;

  case ID_STATUS_AWAY:
    nIcqStatus = ICQ_STATUS_AWAY;
    break;

  case ID_STATUS_OUTTOLUNCH:
  case ID_STATUS_NA:
    nIcqStatus = ICQ_STATUS_NA;
    break;

  case ID_STATUS_ONTHEPHONE:
  case ID_STATUS_OCCUPIED:
    nIcqStatus = ICQ_STATUS_OCCUPIED;
    break;

  case ID_STATUS_DND:
    nIcqStatus = ICQ_STATUS_DND;
    break;

  case ID_STATUS_INVISIBLE:
    nIcqStatus = ICQ_STATUS_INVISIBLE;
    break;

  case ID_STATUS_FREECHAT:
    nIcqStatus = ICQ_STATUS_FFC;
    break;

  case ID_STATUS_OFFLINE:
    // Oscar doesnt have anything that maps to this status. This should never happen.
    _ASSERTE(nMirandaStatus != ID_STATUS_OFFLINE);
    nIcqStatus = 0;
    break;

  default:
    // Online seems to be a good default.
    // Since it cant be offline, it must be a new type of online status.
    nIcqStatus = ICQ_STATUS_ONLINE;
    break;
  }

  return nIcqStatus;
}



int MirandaStatusToSupported(int nMirandaStatus)
{
  int nSupportedStatus;

  switch (nMirandaStatus)
  {

    // These status mode does not need any mapping
  case ID_STATUS_ONLINE:
  case ID_STATUS_AWAY:
  case ID_STATUS_NA:
  case ID_STATUS_OCCUPIED:
  case ID_STATUS_DND:
  case ID_STATUS_INVISIBLE:
  case ID_STATUS_FREECHAT:
  case ID_STATUS_OFFLINE:
    nSupportedStatus = nMirandaStatus;
    break;

    // This mode is not support and must be mapped to something else
  case ID_STATUS_OUTTOLUNCH:
    nSupportedStatus = ID_STATUS_NA;
    break;

    // This mode is not support and must be mapped to something else
  case ID_STATUS_ONTHEPHONE:
    nSupportedStatus = ID_STATUS_OCCUPIED;
    break;

    // This is not supposed to happen.
  default:
    _ASSERTE(0);
    // Online seems to be a good default.
    nSupportedStatus = ID_STATUS_ONLINE;
    break;
  }

  return nSupportedStatus;
}



char* MirandaStatusToString(int mirandaStatus)
{
  return (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, mirandaStatus, 0);
}



char* MirandaStatusToStringUtf(int mirandaStatus)
{ // return miranda status description in utf-8, use unicode service is possible
  return mtchar_to_utf8((TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, mirandaStatus, gbUnicodeCore ? GCMDF_UNICODE : 0));
}



char**MirandaStatusToAwayMsg(int nStatus)
{
  switch (nStatus)
  {

  case ID_STATUS_ONLINE:
    return &modeMsgs.szOnline;
    break;

  case ID_STATUS_AWAY:
    return &modeMsgs.szAway;
    break;

  case ID_STATUS_NA:
    return &modeMsgs.szNa;

  case ID_STATUS_OCCUPIED:
    return &modeMsgs.szOccupied;

  case ID_STATUS_DND:
    return &modeMsgs.szDnd;

  case ID_STATUS_FREECHAT:
    return &modeMsgs.szFfc;

  default:
    return NULL;
  }
}



int AwayMsgTypeToStatus(int nMsgType)
{
  switch (nMsgType)
  {
    case MTYPE_AUTOONLINE:
		return ID_STATUS_ONLINE;

    case MTYPE_AUTOAWAY:
      return ID_STATUS_AWAY;

    case MTYPE_AUTOBUSY:
      return ID_STATUS_OCCUPIED;

    case MTYPE_AUTONA:
      return ID_STATUS_NA;

    case MTYPE_AUTODND:
      return ID_STATUS_DND;

    case MTYPE_AUTOFFC:
      return ID_STATUS_FREECHAT;

    default:
      return ID_STATUS_OFFLINE;
  }
}



void SetGatewayIndex(HANDLE hConn, DWORD dwIndex)
{
  int i;

  EnterCriticalSection(&cookieMutex);

  for (i = 0; i < gatewayCount; i++)
  {
    if (hConn == gateways[i].hConn)
    {
      gateways[i].dwIndex = dwIndex;

      LeaveCriticalSection(&cookieMutex);

      return;
    }
  }

  gateways = (gateway_index *)SAFE_REALLOC(gateways, sizeof(gateway_index) * (gatewayCount + 1));
  gateways[gatewayCount].hConn = hConn;
  gateways[gatewayCount].dwIndex = dwIndex;
  gatewayCount++;

  LeaveCriticalSection(&cookieMutex);

  return;
}



DWORD GetGatewayIndex(HANDLE hConn)
{
  int i;

  EnterCriticalSection(&cookieMutex);

  for (i = 0; i < gatewayCount; i++)
  {
    if (hConn == gateways[i].hConn)
    {
      LeaveCriticalSection(&cookieMutex);

      return gateways[i].dwIndex;
    }
  }

  LeaveCriticalSection(&cookieMutex);

  return 1; // this is default
}



void FreeGatewayIndex(HANDLE hConn)
{
  int i;


  EnterCriticalSection(&cookieMutex);

  for (i = 0; i < gatewayCount; i++)
  {
    if (hConn == gateways[i].hConn)
    {
      gatewayCount--;
      memmove(&gateways[i], &gateways[i+1], sizeof(gateway_index) * (gatewayCount - i));
      gateways = (gateway_index*)SAFE_REALLOC(gateways, sizeof(gateway_index) * gatewayCount);

      // Gateway found, exit loop
      break;
    }
  }

  LeaveCriticalSection(&cookieMutex);
}



void AddToSpammerList(DWORD dwUIN)
{
  EnterCriticalSection(&cookieMutex);

  spammerList = (DWORD *)SAFE_REALLOC(spammerList, sizeof(DWORD) * (spammerListCount + 1));
  spammerList[spammerListCount] = dwUIN;
  spammerListCount++;

  LeaveCriticalSection(&cookieMutex);
}



BOOL IsOnSpammerList(DWORD dwUIN)
{
  int i;

  EnterCriticalSection(&cookieMutex);

  for (i = 0; i < spammerListCount; i++)
  {
    if (dwUIN == spammerList[i])
    {
      LeaveCriticalSection(&cookieMutex);

      return TRUE;
    }
  }

  LeaveCriticalSection(&cookieMutex);

  return FALSE;
}



// ICQ contacts cache
static void AddToCache(HANDLE hContact, DWORD dwUin)
{
  int i = 0;

  if (!hContact || !dwUin)
    return;

  EnterCriticalSection(&cacheMutex);

  if (cacheCount + 1 >= cacheListSize)
  {
    cacheListSize += 100;
    contacts_cache = (icq_contacts_cache *)SAFE_REALLOC(contacts_cache, sizeof(icq_contacts_cache) * cacheListSize);
  }

#ifdef _DEBUG
  Netlib_Logf(ghServerNetlibUser, "Adding contact to cache: %u, position: %u", dwUin, cacheCount);
#endif

  contacts_cache[cacheCount].hContact = hContact;
  contacts_cache[cacheCount].dwUin = dwUin;

  cacheCount++;

  LeaveCriticalSection(&cacheMutex);
}

int GetFromCacheByID(int ID, HANDLE *hContact, DWORD *dwUin)
{

    DWORD uin;

	if (ID < 0 || ID >= cacheCount)
		return 0;

	EnterCriticalSection(&cacheMutex);

	if (hContact)
		*hContact = contacts_cache[ID].hContact;

	uin = contacts_cache[ID].dwUin;

	if (dwUin)
		*dwUin = uin;

	LeaveCriticalSection(&cacheMutex);

	return uin; // to be sure it is not 0

}


void InitCache(void)
{
  HANDLE hContact;

  InitializeCriticalSection(&cacheMutex);
  cacheCount = 0;
  cacheListSize = 0;
  contacts_cache = NULL;

  // build cache
  EnterCriticalSection(&cacheMutex);

  hContact = ICQFindFirstContact();

  while (hContact)
  {
    DWORD dwUin;

    dwUin = ICQGetContactSettingUIN(hContact);
    if (dwUin) AddToCache(hContact, dwUin);

    hContact = ICQFindNextContact(hContact);
  }

  LeaveCriticalSection(&cacheMutex);
}



void UninitCache(void)
{
  SAFE_FREE(&contacts_cache);

  DeleteCriticalSection(&cacheMutex);
}



void DeleteFromCache(HANDLE hContact)
{
  int i;

  if (cacheCount == 0)
    return;

  EnterCriticalSection(&cacheMutex);

  for (i = cacheCount-1; i >= 0; i--)
    if (contacts_cache[i].hContact == hContact)
    {
      cacheCount--;

#ifdef _DEBUG
      Netlib_Logf(ghServerNetlibUser, "Removing contact from cache: %u, position: %u", contacts_cache[i].dwUin, i);
#endif
      // move last contact to deleted position
      if (i < cacheCount)
        memcpy(&contacts_cache[i], &contacts_cache[cacheCount], sizeof(icq_contacts_cache));

      // clear last contact position
      ZeroMemory(&contacts_cache[cacheCount], sizeof(icq_contacts_cache));

      break;
    }

  LeaveCriticalSection(&cacheMutex);
}



static HANDLE HandleFromCacheByUin(DWORD dwUin)
{
  int i;
  HANDLE hContact = NULL;

  if (cacheCount == 0)
    return hContact;

  EnterCriticalSection(&cacheMutex);

  for (i = cacheCount-1; i >= 0; i--)
    if (contacts_cache[i].dwUin == dwUin)
    {
      hContact = contacts_cache[i].hContact;
      break;
    }

  LeaveCriticalSection(&cacheMutex);

  return hContact;
}



HANDLE HContactFromUIN(DWORD uin, int *Added)
{
  HANDLE hContact;

  if (Added) *Added = 0;

  hContact = HandleFromCacheByUin(uin);
  if (hContact) return hContact;

  hContact = ICQFindFirstContact();
  while (hContact != NULL)
  {
    DWORD dwUin;

    dwUin = ICQGetContactSettingUIN(hContact);
    if (dwUin == uin)
    {
      AddToCache(hContact, dwUin);
      return hContact;
    }

    hContact = ICQFindNextContact(hContact);
  }

  //not present: add
  if (Added)
  {
    hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
    if (!hContact)
    {
      NetLog_Server("Failed to create ICQ contact %u", uin);
      return INVALID_HANDLE_VALUE;
    }

    if (CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)gpszICQProtoName) != 0)
    {
      // For some reason we failed to register the protocol to this contact
      CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
      NetLog_Server("Failed to register ICQ contact %u", uin);
      return INVALID_HANDLE_VALUE;
    }

    ICQWriteContactSettingDword(hContact, UNIQUEIDSETTING, uin);

    if (!bIsSyncingCL)
    {
      DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
      SetContactHidden(hContact, 1);

      ICQWriteContactSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

      icq_QueueUser(hContact);

      if (icqOnline)
      {
        icq_sendNewContact(uin, NULL);
		// else need to wait for CList/NotOnList to be deleted
		//icq_GetUserStatus(hContact, 3);
      }
      if (ICQGetContactSettingByte(NULL, "KillSpambots", DEFAULT_KILLSPAM_ENABLED))
        icq_sendGetLocationInfo(hContact, uin, NULL);
    }
    AddToCache(hContact, uin);
    *Added = 1;

    return hContact;
  }

  // not in list, check that uin do not belong to us
  if (ICQGetContactSettingUIN(NULL) == uin)
    return NULL;

  return INVALID_HANDLE_VALUE;
}



HANDLE HContactFromUID(DWORD dwUIN, char* pszUID, int *Added)
{
  HANDLE hContact;
  DWORD dwUin;
  uid_str szUid;

  if (dwUIN)
    return HContactFromUIN(dwUIN, Added);

  if (Added) *Added = 0;

  if (!gbAimEnabled) return INVALID_HANDLE_VALUE;

  hContact = ICQFindFirstContact();
  while (hContact != NULL)
  {
    if (!ICQGetContactSettingUID(hContact, &dwUin, &szUid))
    {
      if (!dwUin && !stricmp(szUid, pszUID))
      {
        if (strcmpnull(szUid, pszUID))
        { // fix case in SN
          ICQWriteContactSettingString(hContact, UNIQUEIDSETTING, pszUID);
        }
        return hContact;
      }
    }
    hContact = ICQFindNextContact(hContact);
  }

  //not present: add
  if (Added)
  {
    hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
    CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)gpszICQProtoName);

    ICQWriteContactSettingString(hContact, UNIQUEIDSETTING, pszUID);

    if (!bIsSyncingCL)
    {
      DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
      SetContactHidden(hContact, 1);

      ICQWriteContactSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

      if (icqOnline)
      {
        icq_sendNewContact(0, pszUID);
      }
      if (ICQGetContactSettingByte(NULL, "KillSpambots", DEFAULT_KILLSPAM_ENABLED))
        icq_sendGetLocationInfo(hContact, 0, pszUID);
    }
    *Added = 1;

    return hContact;
  }

  return INVALID_HANDLE_VALUE;
}



char *NickFromHandle(HANDLE hContact)
{
  if (hContact == INVALID_HANDLE_VALUE)
    return null_strdup(ICQTranslate("<invalid>"));

  return null_strdup((char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
}

void NickFromHandleStatic(HANDLE hContact, char *szNick, WORD wLen)
{

    if (!wLen || !szNick) return;

	if (hContact == INVALID_HANDLE_VALUE)
		lstrcpyn(szNick, "<invalid>", wLen);
	else
		lstrcpyn(szNick, (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0), wLen);

}


char *NickFromHandleUtf(HANDLE hContact)
{
  if (hContact == INVALID_HANDLE_VALUE)
    return ICQTranslateUtf("<invalid>");

  return mtchar_to_utf8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, gbUnicodeCore ? GCDNF_UNICODE : 0));
}



char *strUID(DWORD dwUIN, char *pszUID)
{
  if (dwUIN)
    ltoa(dwUIN, pszUID, 10);

  return pszUID;
}



void SetContactHidden(HANDLE hContact, BYTE bHidden)
{
  DBWriteContactSettingByte(hContact, "CList", "Hidden", bHidden);

  if (!bHidden) // clear zero setting
    DBDeleteContactSetting(hContact, "CList", "Hidden");
}



/* a strlennull() that likes NULL */
size_t __fastcall strlennull(const char *string)
{
  if (string)
    return strlen(string);

  return 0;
}


/* a strcmp() that likes NULL */
int __fastcall strcmpnull(const char *str1, const char *str2)
{
  if (str1 && str2)
    return strcmp(str1, str2);

  return 1;
}



int null_snprintf(char *buffer, size_t count, const char* fmt, ...)
{
  va_list va;
  int len;

  ZeroMemory(buffer, count);
  va_start(va, fmt);
  len = _vsnprintf(buffer, count-1, fmt, va);
  va_end(va);
  return len;
}



char* __fastcall null_strdup(const char *string)
{
  if (string)
    return strdup(string);

  return NULL;
}



size_t __fastcall null_strcut(char *string, size_t maxlen)
{ // limit the string to max length (null & utf-8 strings ready)
  size_t len = strlennull(string);

  if (len < maxlen) 
    return len;

  len = maxlen;

  if (UTF8_IsValid(string)) // handle utf-8 string
  { // find the first byte of possible multi-byte character
    while ((string[len] & 0xc0) == 0x80) len--;
  }
  // simply cut the string
  string[len] = '\0';

  return len;
}



void parseServerAddress(char* szServer, WORD* wPort)
{
  int i = 0;

  while (szServer[i] && szServer[i] != ':') i++;
  if (szServer[i] == ':')
  { // port included
    *wPort = atoi(&szServer[i + 1]);
  } // otherwise do not change port

  szServer[i] = '\0';
}



char *DemangleXml(const char *string, int len)
{
  char *szWork = (char*)SAFE_MALLOC(len+1), *szChar = szWork;
  int i;

  for (i=0; i<len; i++)
  {
    if (!strnicmp(string+i, "&gt;", 4))
    {
      *szChar = '>';
      szChar++;
      i += 3;
    }
    else if (!strnicmp(string+i, "&lt;", 4))
    {
      *szChar = '<';
      szChar++;
      i += 3;
    }
    else if (!strnicmp(string+i, "&amp;", 5))
    {
      *szChar = '&';
      szChar++;
      i += 4;
    }
    else if (!strnicmp(string+i, "&quot;", 6))
    {
      *szChar = '"';
      szChar++;
      i += 5;
    }
    else
    {
      *szChar = string[i];
      szChar++;
    }
  }
  *szChar = '\0';

  return szWork;
}



char *MangleXml(const char *string, int len)
{
  int i, l = 1;
  char *szWork, *szChar;

  for (i = 0; i<len; i++)
  {
    if (string[i]=='<' || string[i]=='>') l += 4; else if (string[i]=='&') l += 5; else l++;
  }
  szChar = szWork = (char*)SAFE_MALLOC(l + 1);
  for (i = 0; i<len; i++)
  {
    if (string[i]=='<')
    {
      *(DWORD*)szChar = ';tl&';
      szChar += 4;
    }
    else if (string[i]=='>')
    {
      *(DWORD*)szChar = ';tg&';
      szChar += 4;
    }
    else if (string[i]=='&')
    {
      *(DWORD*)szChar = 'pma&';
      szChar += 4;
      *szChar = ';';
      szChar++;
    }
    else
    {
      *szChar = string[i];
      szChar++;
    }
  }
  *szChar = '\0';

  return szWork;
}



char *EliminateHtml(const char *string, int len)
{
  char *tmp = (char*)SAFE_MALLOC(len + 1);
  int i,j;
  BOOL tag = FALSE;
  char *res;

  for (i=0,j=0;i<len;i++)
  {
    if (!tag && string[i] == '<')
    {
      if ((i + 4 <= len) && (!strnicmp(string + i, "<br>", 4) || !strnicmp(string + i, "<br/>", 5)))
      { // insert newline
        tmp[j] = '\r';
        j++;
        tmp[j] = '\n';
        j++;
      }
      tag = TRUE;
    }
    else if (tag && string[i] == '>')
    {
      tag = FALSE;
    }
    else if (!tag)
    {
      tmp[j] = string[i];
      j++;
    }
    tmp[j] = '\0';
  }
  SAFE_FREE((void**)&string);
  res = DemangleXml(tmp, strlennull(tmp));
  SAFE_FREE(&tmp);

  return res;
}

void makeUserOffline(HANDLE hContact)
{

	WORD w;
	DWORD dw;

	if (DBGetContactSettingWord(hContact, gpszICQProtoName, "Status", 0) != ID_STATUS_OFFLINE)
		DBWriteContactSettingWord(hContact, gpszICQProtoName, "Status", ID_STATUS_OFFLINE);

	if (dw=DBGetContactSettingDword(hContact, gpszICQProtoName, "LogonTS", 0)) {
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "LogonTS", 0);
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "OldLogonTS", dw);
	}

	if (dw=DBGetContactSettingDword(hContact, gpszICQProtoName, "IdleTS", 0)) {
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "IdleTS", 0);
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "OldIdleTS", dw);
	}

	if (dw=DBGetContactSettingDword(hContact, gpszICQProtoName, "RealIP", 0)) {
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "RealIP", 0);
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "OldRealIP", dw);
	}

	if (dw=DBGetContactSettingDword(hContact, gpszICQProtoName, "IP", 0)) {
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "IP", 0);
		DBWriteContactSettingDword(hContact, gpszICQProtoName, "OldIP", dw);
	}

	if (w=DBGetContactSettingWord(hContact, gpszICQProtoName, "UserPort", 0)) {
		DBWriteContactSettingWord(hContact, gpszICQProtoName, "UserPort", 0);
		DBWriteContactSettingWord(hContact, gpszICQProtoName, "OldUserPort", w);
	}

	if (w=DBGetContactSettingWord(hContact, gpszICQProtoName, "Version", 0)) {
		DBWriteContactSettingWord(hContact, gpszICQProtoName, "Version", 0);
		DBWriteContactSettingWord(hContact, gpszICQProtoName, "OldVersion", w);
	}


	// if (DBGetContactSettingDword(hContact, gpszICQProtoName, "TickTS", 0))
	//	DBWriteContactSettingDword(hContact, gpszICQProtoName, "TickTS", 0);

	//DBWriteContactSettingDword(hContact, gpszICQProtoName, "DirectCookie", 0);
}

char* ApplyEncoding(const char *string, const char* pszEncoding)
{ // decode encoding to Utf-8
  if (string && pszEncoding)
  { // we do only encodings known to icq5.1 // TODO: check if this is enough
    if (!strnicmp(pszEncoding, "utf-8", 5))
    { // it is utf-8 encoded
      return null_strdup(string);
    }
    else if (!strnicmp(pszEncoding, "unicode-2-0", 11))
    { // it is UCS-2 encoded
      int wLen = wcslen((WCHAR*)string) + 1;
      WCHAR *szStr = (WCHAR*)_alloca(wLen*2);
      char *tmp = (char*)string;

      unpackWideString(&tmp, szStr, (WORD)(wLen*2));

      return make_utf8_string(szStr);
    }
    else if (!strnicmp(pszEncoding, "iso-8859-1", 10))
    { // we use "Latin I" instead - it does the job
      return ansi_to_utf8_codepage(string, 1252);
    }
  }
  if (string)
  { // consider it CP_ACP
    return ansi_to_utf8(string);
  }

  return NULL;
}



void ResetSettingsOnListReload()
{
  HANDLE hContact;

  // Reset a bunch of session specific settings
  ICQWriteContactSettingWord(NULL, "SrvVisibilityID", 0);
  ICQWriteContactSettingWord(NULL, "SrvAvatarID", 0);
  ICQWriteContactSettingWord(NULL, "SrvPhotoID", 0);
  ICQWriteContactSettingWord(NULL, "SrvRecordCount", 0);

  hContact = ICQFindFirstContact();

  while (hContact)
  {
    // All these values will be restored during the serv-list receive
    ICQWriteContactSettingWord(hContact, "ServerId", 0);
    ICQWriteContactSettingWord(hContact, "SrvGroupId", 0);
    ICQWriteContactSettingWord(hContact, "SrvPermitId", 0);
    ICQWriteContactSettingWord(hContact, "SrvDenyId", 0);
    ICQWriteContactSettingByte(hContact, "Auth", 0);

    hContact = ICQFindNextContact(hContact);
  }

  FlushSrvGroupsCache();
}



void ResetSettingsOnConnect()
{
  HANDLE hContact;

  // Reset a bunch of session specific settings
  ICQWriteContactSettingByte(NULL, "SrvVisibility", 0);
  ICQWriteContactSettingDword(NULL, "IdleTS", 0);

  hContact = ICQFindFirstContact();

  while (hContact)
  {
    ICQWriteContactSettingDword(hContact, "LogonTS", 0);
    ICQWriteContactSettingDword(hContact, "IdleTS", 0);
    ICQWriteContactSettingDword(hContact, "TickTS", 0);
    ICQWriteContactSettingByte(hContact, "TemporaryVisible", 0);

    // All these values will be restored during the login
    if (ICQGetContactStatus(hContact) != ID_STATUS_OFFLINE)
      ICQWriteContactSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

    hContact = ICQFindNextContact(hContact);
  }
}



void ResetSettingsOnLoad()
{
  HANDLE hContact;

  ICQWriteContactSettingDword(NULL, "IdleTS", 0);
  ICQWriteContactSettingDword(NULL, "LogonTS", 0);

  hContact = ICQFindFirstContact();

  while (hContact)
  {
    ICQWriteContactSettingDword(hContact, "LogonTS", 0);
    ICQWriteContactSettingDword(hContact, "IdleTS", 0);
    ICQWriteContactSettingDword(hContact, "TickTS", 0);
    if (ICQGetContactStatus(hContact) != ID_STATUS_OFFLINE)
    {
      ICQWriteContactSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

      ICQDeleteContactSetting(hContact, DBSETTING_XSTATUSID);
      ICQDeleteContactSetting(hContact, DBSETTING_XSTATUSNAME);
      ICQDeleteContactSetting(hContact, DBSETTING_XSTATUSMSG);
    }
    ICQWriteContactSettingByte(hContact, "DCStatus", 0);

    hContact = ICQFindNextContact(hContact);
  }
}



int RandRange(int nLow, int nHigh)
{
  return nLow + (int)((nHigh-nLow+1)*rand()/(RAND_MAX+1.0));
}



BOOL IsStringUIN(char* pszString)
{
  int i;
  int nLen = strlennull(pszString);


  if (nLen > 0 && pszString[0] != '0')
  {
    for (i=0; i<nLen; i++)
    {
      if ((pszString[i] < '0') || (pszString[i] > '9'))
        return FALSE;
    }

    return TRUE;
  }

  return FALSE;
}



static DWORD __stdcall icq_ProtocolAckThread(icq_ack_args* pArguments)
{
  ICQBroadcastAck(pArguments->hContact, pArguments->nAckType, pArguments->nAckResult, pArguments->hSequence, pArguments->pszMessage);

  if (pArguments->nAckResult == ACKRESULT_SUCCESS)
    NetLog_Server("Sent fake message ack");
  else if (pArguments->nAckResult == ACKRESULT_FAILED)
    NetLog_Server("Message delivery failed");

  SAFE_FREE((char **)&pArguments->pszMessage);
  SAFE_FREE(&pArguments);

  return 0;
}



void icq_SendProtoAck(HANDLE hContact, DWORD dwCookie, int nAckResult, int nAckType, char* pszMessage)
{
  icq_ack_args* pArgs;


  pArgs = (icq_ack_args*)SAFE_MALLOC(sizeof(icq_ack_args)); // This will be freed in the new thread

  pArgs->hContact = hContact;
  pArgs->hSequence = (HANDLE)dwCookie;
  pArgs->nAckResult = nAckResult;
  pArgs->nAckType = nAckType;
  pArgs->pszMessage = (LPARAM)null_strdup(pszMessage);

  ICQCreateThread(icq_ProtocolAckThread, pArgs);
}



void SetCurrentStatus(int nStatus)
{
  int nOldStatus = gnCurrentStatus;

  gnCurrentStatus = nStatus;
  ICQBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)nOldStatus, nStatus);
}



BOOL writeDbInfoSettingString(HANDLE hContact, const char* szSetting, char** buf, WORD* pwLength)
{
  WORD wLen;


  if (*pwLength < 2)
    return FALSE;

  unpackLEWord(buf, &wLen);
  *pwLength -= 2;

  if (*pwLength < wLen)
    return FALSE;

  if ((wLen > 0) && (**buf) && ((*buf)[wLen-1]==0)) // Make sure we have a proper string
  {
    WORD wCp = ICQGetContactSettingWord(hContact, "InfoCodePage", ICQGetContactSettingWord(hContact, "InfoCP", CP_ACP));

    if (wCp != CP_ACP)
    {
      char *szUtf = ansi_to_utf8_codepage(*buf, wCp);

      if (szUtf)
      {
        ICQWriteContactSettingUtf(hContact, szSetting, szUtf);
        SAFE_FREE(&szUtf);
      }
      else
        ICQWriteContactSettingString(hContact, szSetting, *buf);
    }
    else
      ICQWriteContactSettingString(hContact, szSetting, *buf);
  }
  else
    ICQDeleteContactSetting(hContact, szSetting);

  *buf += wLen;
  *pwLength -= wLen;

  return TRUE;
}



BOOL writeDbInfoSettingWord(HANDLE hContact, const char *szSetting, char **buf, WORD* pwLength)
{
  WORD wVal;


  if (*pwLength < 2)
    return FALSE;

  unpackLEWord(buf, &wVal);
  *pwLength -= 2;

  if (wVal != 0)
    ICQWriteContactSettingWord(hContact, szSetting, wVal);
  else
    ICQDeleteContactSetting(hContact, szSetting);

  return TRUE;
}



BOOL writeDbInfoSettingWordWithTable(HANDLE hContact, const char *szSetting, struct fieldnames_t *table, char **buf, WORD* pwLength)
{
  WORD wVal;
  char sbuf[MAX_PATH];
  char *text;

  if (*pwLength < 2)
    return FALSE;

  unpackLEWord(buf, &wVal);
  *pwLength -= 2;

  text = LookupFieldNameUtf(table, wVal, sbuf, MAX_PATH);
  if (text)
    ICQWriteContactSettingUtf(hContact, szSetting, text);
  else
    ICQDeleteContactSetting(hContact, szSetting);

  return TRUE;
}



BOOL writeDbInfoSettingByte(HANDLE hContact, const char *pszSetting, char **buf, WORD* pwLength)
{
  BYTE byVal;

  if (*pwLength < 1)
    return FALSE;

  unpackByte(buf, &byVal);
  *pwLength -= 1;

  if (byVal != 0)
    ICQWriteContactSettingByte(hContact, pszSetting, byVal);
  else
    ICQDeleteContactSetting(hContact, pszSetting);

  return TRUE;
}



BOOL writeDbInfoSettingByteWithTable(HANDLE hContact, const char *szSetting, struct fieldnames_t *table, char **buf, WORD* pwLength)
{
  BYTE byVal;
  char sbuf[MAX_PATH];
  char *text;

  if (*pwLength < 1)
    return FALSE;

  unpackByte(buf, &byVal);
  *pwLength -= 1;

  text = LookupFieldNameUtf(table, byVal, sbuf, MAX_PATH);
  if (text)
    ICQWriteContactSettingUtf(hContact, szSetting, text);
  else
    ICQDeleteContactSetting(hContact, szSetting);

  return TRUE;
}



// Returns the current GMT offset in seconds
int GetGMTOffset(void)
{
  TIME_ZONE_INFORMATION tzinfo;
  DWORD dwResult;
  int nOffset = 0;


  dwResult = GetTimeZoneInformation(&tzinfo);

  switch(dwResult)
  {

  case TIME_ZONE_ID_STANDARD:
    nOffset = -(tzinfo.Bias + tzinfo.StandardBias) * 60;
    break;

  case TIME_ZONE_ID_DAYLIGHT:
    nOffset = -(tzinfo.Bias + tzinfo.DaylightBias) * 60;
    break;

  case TIME_ZONE_ID_UNKNOWN:
  case TIME_ZONE_ID_INVALID:
  default:
    nOffset = 0;
    break;

  }

  return nOffset;
}



BOOL validateStatusMessageRequest(HANDLE hContact, WORD byMessageType)
{
	DWORD uin = DBGetContactSettingDword(hContact,gpszICQProtoName,"UIN",0);
  // Privacy control
  if (ICQGetContactSettingByte(NULL, "StatusMsgReplyCList", 0))
  {
    // Don't send statusmessage to unknown contacts
    if (hContact == INVALID_HANDLE_VALUE)
      return FALSE;

    // Don't send statusmessage to temporary contacts or hidden contacts
    if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) ||
      DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
      return FALSE;

    // Don't send statusmessage to invisible contacts
    if (ICQGetContactSettingByte(NULL, "StatusMsgReplyVisible", 0))
    {
      WORD wStatus = ICQGetContactStatus(hContact);
      if (wStatus == ID_STATUS_OFFLINE)
        return FALSE;
    }
  }

  //Don't send any status message reply, if on
  if(bNoStatusReply)
	  return FALSE;

  // Dont send messages to people you are hiding from
  if (hContact != INVALID_HANDLE_VALUE &&
	  ICQGetContactSettingWord(hContact, "ApparentMode", 0) == ID_STATUS_OFFLINE)
  {
	  CHECKCONTACT chk = {0};
	  chk.hContact = hContact;
	  chk.dbeventflag=DBEF_READ;
	  chk.popup=chk.logtofile=chk.historyevent=TRUE;
	  chk.popuptype=POPTYPE_VIS;
	  chk.icqeventtype=ICQEVENTTYPE_CHECK_STATUS;
	  chk.dwUin = uin;
	  //chk.msg = "(contact which you invisible for request status message)";
	  chk.msg = "contact which you are invisible for requests your status message";
	  CheckContact(chk);
	  return FALSE;
  }

  // Dont respond to request for other statuses than your current one
  if ((byMessageType == MTYPE_AUTOAWAY && gnCurrentStatus != ID_STATUS_AWAY) ||
    (byMessageType == MTYPE_AUTOBUSY && gnCurrentStatus != ID_STATUS_OCCUPIED) ||
    (byMessageType == MTYPE_AUTONA   && gnCurrentStatus != ID_STATUS_NA) ||
    (byMessageType == MTYPE_AUTODND  && gnCurrentStatus != ID_STATUS_DND) ||
    (byMessageType == MTYPE_AUTOFFC  && gnCurrentStatus != ID_STATUS_FREECHAT))
  {
	  CHECKCONTACT chk = {0};
	  chk.dwUin=uin;
	  chk.hContact=hContact;
	  chk.dbeventflag=DBEF_READ;
	  chk.popup=chk.logtofile=chk.historyevent=TRUE;
	  chk.icqeventtype=ICQEVENTTYPE_CHECK_STATUS;
	  chk.popuptype=POPTYPE_INFO_REQUEST;
	  //chk.msg="(request wrong status type)";
	  chk.msg="requested wrong status type - is trying to find you";
	  CheckContact(chk);
    return FALSE;
  }

  if (hContact != INVALID_HANDLE_VALUE && gnCurrentStatus==ID_STATUS_INVISIBLE)
  {
    if (!ICQGetContactSettingByte(hContact, "TemporaryVisible", 0))
    { // Allow request to temporary visible contacts
      return FALSE;
    }
  }

  {
	  CHECKCONTACT chk = {0};
	  chk.dwUin = uin;
	  chk.hContact=hContact;
	  chk.PSD = -1;
	  CheckContact(chk);
  }
  // All OK!
  return TRUE;
}



void __fastcall SAFE_FREE(void** p)
{
  if (*p)
  {
    free(*p);
    *p = NULL;
  }
}



void* __fastcall SAFE_MALLOC(size_t size)
{
  void* p = NULL;

  if (size)
  {
    p = malloc(size);

    if (p)
      ZeroMemory(p, size);
  }
  return p;
}



void* __fastcall SAFE_REALLOC(void* p, size_t size)
{
  if (p)
  {
    return realloc(p, size);
  }
  else
    return SAFE_MALLOC(size);
}



HANDLE NetLib_OpenConnection(HANDLE hUser, const char* szIdent, NETLIBOPENCONNECTION* nloc)
{
  HANDLE hConnection;

  Netlib_Logf(hUser, "%sConnecting to %s:%u", szIdent?szIdent:"", nloc->szHost, nloc->wPort);

  nloc->cbSize = sizeof(NETLIBOPENCONNECTION);
  nloc->flags |= NLOCF_V2;

  hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hUser, (LPARAM)nloc);
  if (!hConnection && (GetLastError() == 87))
  { // this ensures, an old Miranda will be able to connect also
    nloc->cbSize = NETLIBOPENCONNECTION_V1_SIZE;
    hConnection = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hUser, (LPARAM)nloc);
  }
  return hConnection;
}



HANDLE NetLib_BindPort(NETLIBNEWCONNECTIONPROC_V2 pFunc, void* lParam, WORD* pwPort, DWORD* pdwIntIP)
{
  NETLIBBIND nlb = {0};
  HANDLE hBoundPort;

  nlb.cbSize = sizeof(NETLIBBIND); 
  nlb.pfnNewConnectionV2 = pFunc;
  nlb.pExtra = lParam;
  SetLastError(ERROR_INVALID_PARAMETER); // this must be here - NetLib does not set any error :((
  hBoundPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)ghDirectNetlibUser, (LPARAM)&nlb);
  if (!hBoundPort && (GetLastError() == ERROR_INVALID_PARAMETER))
  { // this ensures older Miranda also can bind a port for a dc - pre 0.6
    nlb.cbSize = NETLIBBIND_SIZEOF_V2;
    hBoundPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)ghDirectNetlibUser, (LPARAM)&nlb);
  }
  if (pwPort) *pwPort = nlb.wPort;
  if (pdwIntIP) *pdwIntIP = nlb.dwInternalIP;

  return hBoundPort;
}



void NetLib_CloseConnection(HANDLE *hConnection, int bServerConn)
{
  if (*hConnection)
  {
    int sck = CallService(MS_NETLIB_GETSOCKET, (WPARAM)*hConnection, (LPARAM)0);

    if (sck!=INVALID_SOCKET) shutdown(sck, 2); // close gracefully

    NetLib_SafeCloseHandle(hConnection);

    if (bServerConn)
      FreeGatewayIndex(*hConnection);
  }
}



void NetLib_SafeCloseHandle(HANDLE *hConnection)
{
  if (*hConnection)
  {
    Netlib_CloseHandle(*hConnection);
    *hConnection = NULL;
  }
}



int NetLog_Server(const char *fmt,...)
{
  va_list va;
  char szText[1024];

  va_start(va,fmt);
  mir_vsnprintf(szText,sizeof(szText),fmt,va);
  va_end(va);
  return CallService(MS_NETLIB_LOG,(WPARAM)ghServerNetlibUser,(LPARAM)szText);
}



int NetLog_Direct(const char *fmt,...)
{
  va_list va;
  char szText[1024];

  va_start(va,fmt);
  mir_vsnprintf(szText,sizeof(szText),fmt,va);
  va_end(va);
  return CallService(MS_NETLIB_LOG,(WPARAM)ghDirectNetlibUser,(LPARAM)szText);
}



int NetLog_Uni(BOOL bDC, const char *fmt,...)
{
  va_list va; 
  char szText[1024];
  HANDLE hNetlib;

  va_start(va,fmt);
  mir_vsnprintf(szText,sizeof(szText),fmt,va);
  va_end(va);

  if (bDC)
    hNetlib = ghDirectNetlibUser;
  else
    hNetlib = ghServerNetlibUser;

  return CallService(MS_NETLIB_LOG,(WPARAM)hNetlib,(LPARAM)szText);
}



int ICQBroadcastAck(HANDLE hContact,int type,int result,HANDLE hProcess,LPARAM lParam)
{
  ACKDATA ack={0};

  ack.cbSize=sizeof(ACKDATA);
  ack.szModule=gpszICQProtoName;
  ack.hContact=hContact;
  ack.type=type;
  ack.result=result;
  ack.hProcess=hProcess;
  ack.lParam=lParam;
  return CallService(MS_PROTO_BROADCASTACK,0,(LPARAM)&ack);
}



int __fastcall ICQTranslateDialog(HWND hwndDlg)
{
  LANGPACKTRANSLATEDIALOG lptd;

  lptd.cbSize=sizeof(lptd);
  lptd.flags=0;
  lptd.hwndDlg=hwndDlg;
  lptd.ignoreControls=NULL;
  return CallService(MS_LANGPACK_TRANSLATEDIALOG,0,(LPARAM)&lptd);
}



char* __fastcall ICQTranslate(const char* src)
{
  return (char*)CallService(MS_LANGPACK_TRANSLATESTRING,0,(LPARAM)src);
}



char* __fastcall ICQTranslateUtf(const char* src)
{ // this takes UTF-8 strings only!!!
  char* szRes = NULL;

  if (!strlennull(src))
  { // for the case of empty strings
    return null_strdup(src);
  }

  { // we can use unicode translate (0.5+)
    WCHAR* usrc = make_unicode_string(src);

    szRes = make_utf8_string(TranslateW(usrc));

    SAFE_FREE(&usrc);
  }
  return szRes;
}



char* __fastcall ICQTranslateUtfStatic(const char* src, char* buf, size_t bufsize)
{ // this takes UTF-8 strings only!!!
  if (strlennull(src))
  { // we can use unicode translate (0.5+)
    WCHAR* usrc = make_unicode_string(src);

    make_utf8_string_static(TranslateW(usrc), buf, bufsize);

    SAFE_FREE(&usrc);
  }
  else
    buf[0] = '\0';

  return buf;
}



HANDLE ICQCreateThreadEx(pThreadFuncEx AFunc, void* arg, DWORD* pThreadID)
{
  FORK_THREADEX_PARAMS params;
  DWORD dwThreadId;
  HANDLE hThread;

  params.pFunc      = AFunc;
  params.arg        = arg;
  params.iStackSize = 0;
  params.threadID   = &dwThreadId;
  hThread = (HANDLE)CallService(MS_SYSTEM_FORK_THREAD_EX, 0, (LPARAM)&params);
  if (pThreadID)
    *pThreadID = dwThreadId;

  return hThread;
}



void ICQCreateThread(pThreadFuncEx AFunc, void* arg)
{
  HANDLE hThread = ICQCreateThreadEx(AFunc, arg, NULL);

  CloseHandle(hThread);
}



char* GetUserPassword(BOOL bAlways)
{
  if (gpszPassword[0] != '\0' && (gbRememberPwd || bAlways))
    return gpszPassword;

  if (!ICQGetContactStaticString(NULL, "Password", gpszPassword, sizeof(gpszPassword)))
  {
    CallService(MS_DB_CRYPT_DECODESTRING, strlennull(gpszPassword) + 1, (LPARAM)gpszPassword);

    if (!strlennull(gpszPassword)) return NULL;

    gbRememberPwd = TRUE;

    return gpszPassword;
  }

  return NULL;
}



WORD GetMyStatusFlags()
{
  WORD wFlags = 0;

  // Webaware setting bit flag
  if (ICQGetContactSettingByte(NULL, "WebAware", 0))
    wFlags = STATUS_WEBAWARE;

  // DC setting bit flag
  switch (ICQGetContactSettingByte(NULL, "DCType", 0))
  {
    case 0:
      break;

    case 1:
      wFlags = wFlags | STATUS_DCCONT;
      break;

    case 2:
      wFlags = wFlags | STATUS_DCAUTH;
      break;

    default:
      wFlags = wFlags | STATUS_DCDISABLED;
      break;
  }
  return wFlags;
}



int IsValidRelativePath(const char *filename)
{
  if (strstr(filename, "..\\") || strstr(filename, "../") ||
      strstr(filename, ":\\") || strstr(filename, ":/") ||
      filename[0] == '\\' || filename[0] == '/')
    return 0; // Contains malicious chars, Failure

  return 1; // Success
}



char* ExtractFileName(const char *fullname)
{
  char* szFileName;

  if (((szFileName = strrchr(fullname, '\\')) == NULL) && ((szFileName = strrchr(fullname, '/')) == NULL))
  { // already is only filename
    return (char*)fullname;
  }
  szFileName++; // skip backslash

  return szFileName;
}



char* FileNameToUtf(const char *filename)
{
  if (gbUnicodeAPI)
  { // reasonable only on NT systems
    HINSTANCE hKernel;
    DWORD (CALLBACK *RealGetLongPathName)(LPCWSTR, LPWSTR, DWORD);

    hKernel = GetModuleHandle("KERNEL32");
    *(FARPROC *)&RealGetLongPathName = GetProcAddress(hKernel, "GetLongPathNameW");

    if (RealGetLongPathName)
    { // the function is available (it is not on old NT systems)
      WCHAR *unicode, *usFileName = NULL;
      int wchars;

      wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, filename,
        strlennull(filename), NULL, 0);

      unicode = (WCHAR*)_alloca((wchars + 1) * sizeof(WCHAR));
      unicode[wchars] = 0;

      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, filename,
        strlennull(filename), unicode, wchars);

      wchars = RealGetLongPathName(unicode, usFileName, 0);
      usFileName = (WCHAR*)_alloca((wchars + 1) * sizeof(WCHAR));
      RealGetLongPathName(unicode, usFileName, wchars);

      return make_utf8_string(usFileName);
    }
    else
      return ansi_to_utf8(filename);
  }
  else
    return ansi_to_utf8(filename);
}



int FileStatUtf(const char *path, struct _stati64 *buffer)
{
  int wRes = -1;

  if (gbUnicodeAPI)
  {
    WCHAR* usPath = make_unicode_string(path);

    wRes = _wstati64(usPath, buffer);
    SAFE_FREE(&usPath);
  }
  else
  {
    int size = strlennull(path)+2;
    char* szAnsiPath = (char*)_alloca(size);

    if (utf8_decode_static(path, szAnsiPath, size))
      wRes = _stati64(szAnsiPath, buffer);
  }
  return wRes;
}



int MakeDirUtf(const char *dir)
{
  int wRes = -1;
  char *szLast;

  if (gbUnicodeAPI)
  {
    WCHAR* usDir = make_unicode_string(dir);
    // _wmkdir can created only one dir at once
    wRes = _wmkdir(usDir);
    // check if dir not already existed - return success if yes
    if (wRes == -1 && errno == 17 /* EEXIST */)
      wRes = 0;
    else if (wRes && errno == 2 /* ENOENT */)
    { // failed, try one directory less first
      szLast = strrchr(dir, '\\');
      if (!szLast) szLast = strrchr(dir, '/');
      if (szLast)
      {
        char cOld = *szLast;

        *szLast = '\0';
        if (!MakeDirUtf(dir))
          wRes = _wmkdir(usDir);
        *szLast = cOld;
      }
    }
    SAFE_FREE(&usDir);
  }
  else
  {
    int size = strlennull(dir)+2;
    char* szAnsiDir = (char*)_alloca(size);

    if (utf8_decode_static(dir, szAnsiDir, size))
    { // _mkdir can create only one dir at once
      wRes = _mkdir(szAnsiDir);
      // check if dir not already existed - return success if yes
      if (wRes == -1 && errno == 17 /* EEXIST */)
        wRes = 0;
      else if (wRes && errno == 2 /* ENOENT */)
      { // failed, try one directory less first
        szLast = strrchr(dir, '\\');
        if (!szLast) szLast = strrchr(dir, '/');
        if (szLast)
        {
          char cOld = *szLast;

          *szLast = '\0';
          if (!MakeDirUtf(dir))
            wRes = _mkdir(szAnsiDir);
          *szLast = cOld;
        }
      }
    }
  }
  return wRes;
}



int OpenFileUtf(const char *filename, int oflag, int pmode)
{
  int hFile = -1;

  if (gbUnicodeAPI)
  {
    WCHAR* usFile = make_unicode_string(filename);

    hFile = _wopen(usFile, oflag, pmode);
    SAFE_FREE(&usFile);
  }
  else
  {
    int size = strlennull(filename)+2;
    char* szAnsiFile = (char*)_alloca(size);

    if (utf8_decode_static(filename, szAnsiFile, size))
      hFile = _open(szAnsiFile, oflag, pmode); 
  }
  return hFile;
}



WCHAR *GetWindowTextUcs(HWND hWnd)
{
  WCHAR *utext;

  if (gbUnicodeAPI)
  {
    int nLen = GetWindowTextLengthW(hWnd);

    utext = (WCHAR*)SAFE_MALLOC((nLen+2)*sizeof(WCHAR));
    GetWindowTextW(hWnd, utext, nLen + 1);
  }
  else
  {
    char *text;
    int wchars, nLen = GetWindowTextLengthA(hWnd);

    text = (char*)_alloca(nLen+2);
    GetWindowTextA(hWnd, text, nLen + 1);

    wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, text,
      strlennull(text), NULL, 0);

    utext = (WCHAR*)SAFE_MALLOC((wchars + 1)*sizeof(WCHAR));

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, text,
      strlennull(text), utext, wchars);
  }
  return utext;
}



void SetWindowTextUcs(HWND hWnd, WCHAR *text)
{
  if (gbUnicodeAPI)
  {
    SetWindowTextW(hWnd, text);
  }
  else
  {
    char *tmp = (char*)SAFE_MALLOC(wcslen(text) + 1);

    WideCharToMultiByte(CP_ACP, 0, text, -1, tmp, wcslen(text)+1, NULL, NULL);
    SetWindowTextA(hWnd, tmp);
    SAFE_FREE(&tmp);
  }
}



char* GetWindowTextUtf(HWND hWnd)
{
  TCHAR* szText;
  
  if (gbUnicodeAPI)
  {
    int nLen = GetWindowTextLengthW(hWnd);

    szText = (TCHAR*)_alloca((nLen+2)*sizeof(WCHAR));
    GetWindowTextW(hWnd, (WCHAR*)szText, nLen + 1);
  }
  else
  {
    int nLen = GetWindowTextLengthA(hWnd);

    szText = (TCHAR*)_alloca(nLen+2);
    GetWindowTextA(hWnd, (char*)szText, nLen + 1);
  }
  return tchar_to_utf8(szText);
}



char* GetDlgItemTextUtf(HWND hwndDlg, int iItem)
{
  return GetWindowTextUtf(GetDlgItem(hwndDlg, iItem));
}



void SetWindowTextUtf(HWND hWnd, const char* szText)
{
  if (gbUnicodeAPI)
  {
    WCHAR* usText = make_unicode_string(szText);

    SetWindowTextW(hWnd, usText);
    SAFE_FREE(&usText);
  }
  else
  {
    int size = strlennull(szText)+2;
    char* szAnsi = (char*)_alloca(size);

    if (utf8_decode_static(szText, szAnsi, size))
      SetWindowTextA(hWnd, szAnsi);
  }
}



void SetDlgItemTextUtf(HWND hwndDlg, int iItem, const char* szText)
{
  SetWindowTextUtf(GetDlgItem(hwndDlg, iItem), szText);
}



LONG SetWindowLongUtf(HWND hWnd, int nIndex, LONG dwNewLong)
{
  if (gbUnicodeAPI)
    return SetWindowLongW(hWnd, nIndex, dwNewLong);
  else
    return SetWindowLongA(hWnd, nIndex, dwNewLong);
}



LRESULT CallWindowProcUtf(WNDPROC OldProc, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (gbUnicodeAPI)
    return CallWindowProcW(OldProc,hWnd,msg,wParam,lParam);
  else
    return CallWindowProcA(OldProc,hWnd,msg,wParam,lParam);
}



static int ControlAddStringUtf(HWND ctrl, DWORD msg, const char* szString)
{
  char str[MAX_PATH];
  char *szItem = ICQTranslateUtfStatic(szString, str, MAX_PATH);
  int item = -1;

  if (gbUnicodeAPI)
  {
    WCHAR *wItem = make_unicode_string(szItem);

    item = SendMessageW(ctrl, msg, 0, (LPARAM)wItem);
    SAFE_FREE(&wItem);
  }
  else
  {
    int size = strlennull(szItem) + 2;
    char *aItem = (char*)_alloca(size);

    if (utf8_decode_static(szItem, aItem, size))
      item = SendMessageA(ctrl, msg, 0, (LPARAM)aItem);
  }
  return item;
}



int ComboBoxAddStringUtf(HWND hCombo, const char* szString, DWORD data)
{
  int item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
  SendMessage(hCombo, CB_SETITEMDATA, item, data);

  return item;
}



int ListBoxAddStringUtf(HWND hList, const char* szString)
{
  return ControlAddStringUtf(hList, LB_ADDSTRING, szString);
}



HWND DialogBoxUtf(BOOL bModal, HINSTANCE hInstance, const char* szTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{ // Unicode pump ready dialog box
  if (gbUnicodeAPI)
  {
    if (bModal)
      return (HANDLE)DialogBoxParamW(hInstance, (LPCWSTR)szTemplate, hWndParent, lpDialogFunc, dwInitParam);
    else
      return CreateDialogParamW(hInstance, (LPCWSTR)szTemplate, hWndParent, lpDialogFunc, dwInitParam);
  }
  else
  {
    if (bModal)
      return (HANDLE)DialogBoxParamA(hInstance, szTemplate, hWndParent, lpDialogFunc, dwInitParam);
    else
      return CreateDialogParamA(hInstance, szTemplate, hWndParent, lpDialogFunc, dwInitParam);
  }
}



HWND CreateDialogUtf(HINSTANCE hInstance, const char* lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
  if (gbUnicodeAPI)
    return CreateDialogW(hInstance, (LPCWSTR)lpTemplate, hWndParent, lpDialogFunc);
  else
    return CreateDialogA(hInstance, lpTemplate, hWndParent, lpDialogFunc);
}



int MessageBoxUtf(HWND hWnd, const char* szText, const char* szCaption, UINT uType)
{
  int res;
  char str[1024];
  char cap[MAX_PATH];

  if (gbUnicodeAPI)
  {
    WCHAR *text = make_unicode_string(ICQTranslateUtfStatic(szText, str, 1024));
    WCHAR *caption = make_unicode_string(ICQTranslateUtfStatic(szCaption, cap, MAX_PATH));
    res = MessageBoxW(hWnd, text, caption, uType);
    SAFE_FREE(&caption);
    SAFE_FREE(&text);
  }
  else
  {
    int size = strlennull(szText) + 2, size2 = strlennull(szCaption) + 2;
    char *text = (char*)_alloca(size);
    char *caption = (char*)_alloca(size2);

    utf8_decode_static(ICQTranslateUtfStatic(szText, str, 1024), text, size);
    utf8_decode_static(ICQTranslateUtfStatic(szCaption, cap, MAX_PATH), caption, size2);
    res = MessageBoxA(hWnd, text, caption, uType);
  }
  return res;
}


int CacheIDCount(void)
{

	return cacheCount;

}

int IDFromCacheByUin(DWORD dwUin)
{

	int i = -1;

	if (cacheCount == 0)
		return i;

	EnterCriticalSection(&cacheMutex);

	for (i = cacheCount-1; i >= 0; i--)
		if (contacts_cache[i].dwUin == dwUin)
			break;

	LeaveCriticalSection(&cacheMutex);

	return i;

}

int SetCacheFlagsByID(int ID, DWORD flags)
{

	if (ID < 0 || ID >= cacheCount)
		return 0;

	contacts_cache[ID].flags = flags;

	return 1;

}

int GetCacheByID(int ID, icq_contacts_cache *icc)
{

	if (!icc || ID < 0 || ID >= cacheCount)
		return 0;

	EnterCriticalSection(&cacheMutex);

	icc->hContact = contacts_cache[ID].hContact;
	icc->dwUin = contacts_cache[ID].dwUin;
	icc->flags = contacts_cache[ID].flags;

	LeaveCriticalSection(&cacheMutex);

	return icc->dwUin; // to be sure it is not 0

}

/*BOOL invis_for(DWORD dwUin, HANDLE hContact)  //this function checking are you invisible for hContact or dwUin, need optimization...
{
	WORD wApparent = DBGetContactSettingWord(hContact,gpszICQProtoName,"ApparentMode",0);

	if(bVisibility==0x02) 
		return TRUE;
	if (dwUin && !hContact)
		hContact=HContactFromUIN(dwUin,0);
	if (!hContact)
	{
		return TRUE;
	}
	if(DBGetContactSettingByte(hContact, "CList", "NotOnList", 0)&&bVisibility!=0x01||bVisibility!=0x04)
		return TRUE;
	if((wApparent==ID_STATUS_OFFLINE||gnCurrentStatus == ID_STATUS_INVISIBLE&&bVisibility!=0x04&&wApparent!=ID_STATUS_ONLINE)&&bVisibility!=0x01)
		return TRUE;
	return FALSE;
} */



BOOL invis_for(DWORD dwUin, HANDLE hContact)  //this function checking are you invisible for hContact or dwUin, need optimization... (modified by Bio)
{
	WORD wApparent;

	// Block all users from seeing you
//	if ( bVisibility==0x02 || !dwUin )
	if ( bVisibility==0x02) //we support AIM which do not use UIN's
	{
		NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
		return TRUE;
	}

	// Allow all users to see you
	if ( bVisibility == 0x01 )
		return FALSE;

	if ( !hContact ) {
		hContact=HContactFromUIN(dwUin,0);
		if (!hContact)
		{
			NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
			return TRUE;
		}
	}

	if ( DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) )
	{
		NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
		return TRUE;
	}

	// Allow only users in the buddy list to see you
	if ( bVisibility == 0x05 ) {
		if ( DBGetContactSettingByte(hContact, "CList", "Hidden", 0) )
		{
			NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
			return TRUE;
		}
		else
			return FALSE;
	}

	wApparent = DBGetContactSettingWord(hContact,gpszICQProtoName,"ApparentMode",0);

	// Allow only users in the permit list to see you
	if ( bVisibility == 0x03 ) {
		if ( wApparent == ID_STATUS_ONLINE )
			return FALSE;
		else
		{
			NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
			return TRUE;
		}
	}

	// Block only users in the invisible list from seeing you
	if ( bVisibility == 0x04 ) {
		if ( wApparent == ID_STATUS_OFFLINE )
		{
			NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
			return TRUE;
		}
		else
			return FALSE;
	}

	// invisible by default
	NetLog_Server("we invisible for %u, blocking all unsafe requests", dwUin);
	return TRUE;
}

void CheckContact(CHECKCONTACT chk) //dwUin necessary, function for show popups, write events, log to file, call PSD, parsing temporary contacts, NEED TO BE FINISHED
{
	BYTE Hidden = DBGetContactSettingByte(chk.hContact, "CList", "Hidden", 0);
	BYTE InDb = CallService(MS_DB_CONTACT_IS, (WPARAM)chk.hContact, 0);
	BYTE NotOnList = DBGetContactSettingByte(chk.hContact,"CList","NotOnList",0);
	BOOL Cancel = FALSE;
	BOOL showpopup = TRUE;
	char popmsg[512] = {0}, eventmsghistory[512] = {0}, eventmsgfile[512] = {0};
	if(!InDb||NotOnList||Hidden)
	{
		if(!bPopUpForNotOnList)
			showpopup = FALSE;
		if(bNoPSDForHidden)
			Cancel = TRUE;
	}
	if(!chk.popuptype)                  //
		chk.popup=FALSE;				//
	if(!chk.icqeventtype)				//
	{									//wrong call check ... (missing event types)
		chk.historyevent=FALSE;			//
		chk.logtofile=FALSE;			//
	}									//
	if(chk.dwUin) 		
	{
		if (bTmpContacts == 1)
		{
			int added;
			chk.hContact = HContactFromUIN(chk.dwUin, &added);
			if(NotOnList)
			{
				{
					DBCONTACTWRITESETTING dbcws;
					dbcws.value.type = DBVT_UTF8;
					dbcws.value.pszVal = TmpGroupName;
					dbcws.szModule = "CList";
					dbcws.szSetting = "Group";
					CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)chk.hContact, (LPARAM)&dbcws);    
				}	
				DBWriteContactSettingByte(chk.hContact,"CList","Hidden",0);
				DBWriteContactSettingByte(chk.hContact,"CList","NotOnList",bAddTemp);
				DBWriteContactSettingByte(chk.hContact, gpszICQProtoName, "CheckSelfRemove", 1);//excluding from checkselfremove				
				//				DBWriteContactSettingByte(hContact, gpszICQProtoName, "FoundedContact", 1); //mark founded contacts
			}
		}
		if(InDb)
		{
			if(chk.msg)
			{
				if(strlen(chk.msg) > 1 && showpopup)
				{
					popmsg[0] = '\0';
					switch(chk.popuptype)
					{
					case POPTYPE_SPAM:				
						//nothing here, used direct ShowPopupMsg call
						break;
					case POPTYPE_UNKNOWN:
						//nothing here, used direct ShowPopupMsg call
						break;
					case POPTYPE_VIS:
						if(bVisPopUp)
						{
							strcpy(popmsg,"... is checking your real status ");						
						}
						break;
					case LOG_NOTE:
					case LOG_WARNING:
					case LOG_ERROR:
					case LOG_FATAL:
						strcpy(popmsg,chk.msg);
						break;
					case POPTYPE_FOUND:
						if(bFoundPopUp)
						{
							strcpy(popmsg,chk.msg);
						}
						break;
					case POPTYPE_SCAN:
						if(bScanPopUp)
						{
							strcpy(popmsg,chk.msg);
						}
						break;
					case POPTYPE_CLIENT_CHANGE:
						if(bClientChangePopUp)
						{
							strcpy(popmsg,chk.msg);
						}
						break;
					case POPTYPE_INFO_REQUEST:
						if(bInfoRequestPopUp)
							strcpy(popmsg,chk.msg);
						break;
					case POPTYPE_IGNORE_CHECK:
						if(bIgnoreCheckPop)
							strcpy(popmsg,chk.msg);
						break;
					case POPTYPE_SELFREMOVE:
						if(bPopSelfRem)
							strcpy(popmsg,chk.msg);
						break;
					case POPTYPE_AUTH:
						if(bAuthPopUp)
							strcpy(popmsg,chk.msg);
						break;
					default:
						break;
					}
					if(chk.historyevent||chk.logtofile)
					{
						switch(chk.icqeventtype)
						{
						case ICQEVENTTYPE_AUTH_DENIED:
							if(bLogAuthHistory)
								strcpy(eventmsghistory,"was denied your authorization request");
							if(bLogAuthFile)
								strcpy(eventmsgfile,"was denied your authorization request");
							break;
						case ICQEVENTTYPE_AUTH_GRANTED:
							if(bLogAuthHistory)
								strcpy(eventmsghistory,"Authorization request granted");
							if(bLogAuthFile)
								strcpy(eventmsgfile,"Authorization request granted");
							break;
						case ICQEVENTTYPE_CHECK_STATUS:
							if(bLogStatusCheckHistory)
								strcpy(eventmsghistory,"check your status ");
							if(bLogStatusCheckFile)
								strcpy(eventmsgfile,"check your status ");
							break;	
						case ICQEVENTTYPE_WAS_FOUND:
							if(bLogASDHistory)
								strcpy(eventmsghistory,"");
							if(bLogASDFile)
								strcpy(eventmsgfile,"");
							break;
						case ICQEVENTTYPE_CLIENT_CHANGE:
							if(bLogIgnoreCheckHistory)
								strcpy(eventmsghistory,chk.msg);
							if(bLogIgnoreCheckFile)
								strcpy(eventmsgfile,chk.msg);
							break;
						case ICQEVENTTYPE_EMAILEXPRESS:
							//nothing to do here at this time ...
							break;
						case ICQEVENTTYPE_FUTURE_AUTH:
							//i do not know what can i do here, fix me....
							break;
						case ICQEVENTTYPE_IGNORECHECK_STATUS:
							if(bLogIgnoreCheckHistory)
								strcpy(eventmsghistory,"check your ignore list");
							if(bLogIgnoreCheckFile)
								strcpy(eventmsgfile,"check your ignore list");
							break;
						case ICQEVENTTYPE_SELF_REMOVE:
							if(bLogSelfRemoveHistory)
								strcpy(eventmsghistory,"removed himself from your Serverlist!");
							if(bLogSelfRemoveFile)
								strcpy(eventmsgfile,"removed himself from your Serverlist!");
							break;
						case ICQEVENTTYPE_SMS:
							//nothing to do here at this time ...
							break;
						case ICQEVENTTYPE_WEBPAGER:
							//nothing to do here at this time ...
							break;
						case ICQEVENTTYPE_YOU_ADDED:
							break;
						case ICQEVENTTYPE_AUTH_REQUESTED:
							break;
						default:
							break;
						}
						if(popmsg && strlen(popmsg) > 1)
						{
							if(!strstr(chk.msg,popmsg))
								strcat(popmsg, chk.msg);
							ShowPopUpMsg(chk.hContact, chk.dwUin, NickFromHandleUtf(chk.hContact), popmsg, chk.popuptype);
						}
						if(eventmsghistory && strlen(eventmsghistory) > 1)
						{
							if(!strstr(chk.msg,eventmsghistory))
								strcat(eventmsghistory, chk.msg);
							HistoryLog(chk.hContact, chk.dwUin, eventmsghistory, chk.icqeventtype, chk.dbeventflag);
						}
						if(eventmsgfile && strlen(eventmsgfile) > 1)
						{
							if(!strstr(chk.msg,eventmsgfile))
								strcat(eventmsgfile, chk.msg);
							LogToFile(chk.hContact, chk.dwUin, eventmsgfile, chk.icqeventtype);
						}
					}
				}
			}
			if(chk.PSD != -1 && !Cancel)
			{
				if(!chk.PSD)
					chk.PSD=21;
				if(gbASD&&chk.PSD<10)
					icq_SetUserStatus(chk.dwUin, 0, chk.PSD, 0);
				else if(ICQGetContactSettingWord(chk.hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE && (bPSD&&chk.PSD>20)) //check for wrong PSD call
					icq_SetUserStatus(chk.dwUin, 0, chk.PSD, 0);
			}
		}
	}
}
int IncognitoAwayRequest(WPARAM wParam, LPARAM lParam)
{
	bIncognitoRequest = TRUE;

	CallService(MS_AWAYMSG_SHOWAWAYMSG, wParam, 0);
	return 0;
}



void HistoryLog(HANDLE hContact, DWORD dwUin, char *data, int event_type, int flag)
{
	extern BOOL bHcontactHistory;
	PBYTE pCurBlob;
	WORD wTextLen;
	char szText[MAX_PATH];
	DBEVENTINFO Event = {0};
	Event.cbSize=sizeof(Event);
	Event.szModule=gpszICQProtoName;
	Event.eventType=event_type;
	Event.flags=flag,DBEF_UTF;
	Event.timestamp=(DWORD)time(NULL);
	mir_snprintf(szText, sizeof(szText), "%s %s ( %s: %s, %s: (%u) )", Translate(data), Translate("by"), Translate("Nick"), (NickFromHandle(hContact)), Translate("UIN"), dwUin);
	wTextLen = strlen(szText);
	Event.cbBlob = sizeof(DWORD)+sizeof(HANDLE)+wTextLen+1;
	pCurBlob = Event.pBlob = (PBYTE)_alloca(Event.cbBlob);
	memcpy(pCurBlob,&szText,wTextLen); pCurBlob+=wTextLen;
	*(char *)pCurBlob = 0; pCurBlob++;
	memcpy(pCurBlob,&dwUin,sizeof(DWORD)); pCurBlob+=sizeof(DWORD);
	memcpy(pCurBlob,&hContact,sizeof(HANDLE));
	if(bHcontactHistory)
		CallService(MS_DB_EVENT_ADD,(WPARAM)(HANDLE)hContact,(LPARAM)&Event);
	else
		CallService(MS_DB_EVENT_ADD,(WPARAM)(HANDLE)NULL,(LPARAM)&Event);
	//we need show popups for events from here
}


void LogToFile(HANDLE hContact, DWORD dwUin, char *string, int event_type)
{

	char szTime[30];
	char content[MAX_PATH];
	char filename[1024];
	time_t now;
	FILE *f;
 
	wsprintf(filename, UniGetContactSettingUtf(NULL, gpszICQProtoName, "EventsLog", "EventsLog.txt"));
	

    switch(event_type)
	{
		case ICQEVENTTYPE_AUTH_GRANTED:
			if(!bLogAuthFile)
				return;
			mir_snprintf(content, sizeof(content), "%s %s", Translate("granted your authorization request "), Translate(string));
			break;
		case ICQEVENTTYPE_AUTH_DENIED:
			if(!bLogAuthFile)
				return;
			mir_snprintf(content, sizeof(content), "%s %s", Translate("denied your authorization request "), Translate(string));
			break;
		case ICQEVENTTYPE_SELF_REMOVE:
			if(!bLogSelfRemoveFile)
				return;
			mir_snprintf(content, sizeof(content), "%s %s", Translate("removed himself from your serverlist "), Translate(string));
			break;
		case ICQEVENTTYPE_FUTURE_AUTH:
			if(!bLogAuthFile)
				return;
			mir_snprintf(content, sizeof(content), "%s %s", Translate("granted you future authorization "), Translate(string));
			break;
		case ICQEVENTTYPE_CLIENT_CHANGE:
			if(!bLogClientChangeFile)
				return;
			utf8_decode_static(string, content, strlen(string));
			break;

		case ICQEVENTTYPE_CHECK_STATUS:
			if(!bLogStatusCheckFile)
				return;
			mir_snprintf(content, sizeof(content), "%s %s", Translate("checked your real status "), Translate(string));
			break;
		case ICQEVENTTYPE_WAS_FOUND:
			if(!bLogASDFile)
				return;
			mir_snprintf(content, sizeof(content), "%s", Translate(string));
			break;
		case ICQEVENTTYPE_IGNORECHECK_STATUS:
			if(!bLogIgnoreCheckFile)
				return;
			mir_snprintf(content, sizeof(content), "%s %s", Translate("checked his ignore state"), Translate(string));
			break;

		default:
            wsprintf(content, "unknown eventype processed");
				return;
	}


	now = time(NULL);
	strftime(szTime, sizeof(szTime), "%a, %d %b %Y %H:%M:%S", localtime(&now));
                                         /* Sun, 00 Jan 0000 00:00:00 */
    f = fopen( filename, "a+" );
	if( f != NULL )
	{
		fprintf( f, "[%s] %s (%u) %s\n", szTime, NickFromHandle(hContact), dwUin, content); 
		fclose(f);
	}
}

WORD GetProtoVersion()
{
	int ver = 0;
	if (gbVerEnabled)
	{
		ver = (DBGetContactSettingWord(NULL,gpszICQProtoName,"setVersion", 0));
	}
	else
	{
		switch (DBGetContactSettingWord(NULL,gpszICQProtoName,"CurrentID",0)) //client change version
		{
		case 1:						//unknown
			ver = 66;
			break;
		case 23:
		case 24:
		case 2:						//qip
		case 3:						//ysm
		case 7:						//trillian
		case 46:                   //qip infium
			ver = 11;
			break;
		case 6:						//Jimm
			ver = 5;
			break;
		case 9:						//Kopete
		case 52:                  //NanoICQ
			ver = 10;
			break;
		case 11:
			ver = 13;
			break;
		case 4:						//icq lite
		case 5:						//&RQ
		case 12:					//rambler
		case 13:						//icq 5.1
		case 14:
		case 15:
		case 16:
		case 17:
		case 26:         //icq6
		case 44:        //QNEXT
		case 45:         //pyICQ
			ver = 9;
			break;
		case 21:                 //stICQ
			ver = 2;
			break;
		case 36:       //ICQ99   
			ver = 6;
			break;
		case 37:               //WebICQ
			ver = 7;
			break;
		case 35:          //GAIM
			ver = 0;
			break;           
		default :						//miranda
			ver = 8;
			break;
		}
	}
	DBWriteContactSettingWord(NULL,gpszICQProtoName,"setVersion",(WORD)ver);
	return ver;
}

static void SetDwFT(DWORD *dwFT, char* DbValue, DWORD DwValue) 
{ 
  *dwFT=DwValue; 
  ICQWriteContactSettingDword(NULL, DbValue, DwValue); 
}

void SetTimeStamps(DWORD *dwFT1, DWORD *dwFT2, DWORD *dwFT3)
{
	switch (DBGetContactSettingWord(NULL, gpszICQProtoName, "CurrentID", 0))  //client change dwFT
	{
	case 3:											//ysm
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFFAB);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 4:											//ICQ lite
 	  SetDwFT(dwFT1, "dwFT1", 0x3AA773EE);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 5:													//&RQ
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFF7F);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 7:										//trillian
 	  SetDwFT(dwFT1, "dwFT1", 0x3B75AC09);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 8:													//licq
 	  SetDwFT(dwFT1, "dwFT1", 0x2C0BA3DD);
 	  SetDwFT(dwFT2, "dwFT2", 0x7D800403);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 2:											//qip
 	  SetDwFT(dwFT1, "dwFT1", 0x08000300);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 46:       //QIP Infium
 	  SetDwFT(dwFT1, "dwFT1", 0x0000232C);
 	  SetDwFT(dwFT2, "dwFT2", 0x0000000B);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 18:
	case 19:
 	  SetDwFT(dwFT1, "dwFT1", 0x3AA773EE);
 	  SetDwFT(dwFT2, "dwFT2", 0x3AA66380);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 21:
    SetDwFT(dwFT1, "dwFT1", 0x3BA8DBAF);
 	  SetDwFT(dwFT2, "dwFT2", 0x3BEB5373);
    SetDwFT(dwFT3, "dwFT3", 0x3BEB5262);
		break;
	case 22:
 	  SetDwFT(dwFT1, "dwFT1", 0x3B4C4C0C);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
    SetDwFT(dwFT3, "dwFT3", 0x3B7248ED);
		break;
	case 1:											//unknown
	case 6:											//Jimm
	case 9:											//Kopete
	case 10:										////icq for mac
	case 12:										//rambler
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 20:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 40:           //uIM
	case 41:           //TICQClient
	case 42:           //IC@
	case 43:          //PreludeICQ
	case 44:         //QNEXT
	case 45:         //pyICQ
	case 47:       //JICQ
	case 49:       //MIP
	case 50:     //Trillian Astra
	case 52:     //NanoICQ
	case 53:		//IMadering
 	  SetDwFT(dwFT1, "dwFT1", 0x00000000);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 29:
 	  SetDwFT(dwFT1, "dwFT1", 0x44F523B0);
 	  SetDwFT(dwFT2, "dwFT2", 0x44F523A6);
 	  SetDwFT(dwFT3, "dwFT3", 0x44F523A6);
		break;
	case 30:           //alicq
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFFBE);
 	  SetDwFT(dwFT2, "dwFT2", 0x00090800);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 31:           //mICQ
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFF42);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 32:            //StrICQ 0.4
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFF8F);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 33:            //vICQ 0.43.0.0
 	  SetDwFT(dwFT1, "dwFT1", 0x04031980);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 34:            //IM2
 	  SetDwFT(dwFT1, "dwFT1", 0x3FF19BEB);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x3FF19BEB);
		break;
	case 35:           //GAIM
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFFFF);
 	  SetDwFT(dwFT2, "dwFT2", 0xFFFFFFFF);
 	  SetDwFT(dwFT3, "dwFT3", 0xFFFFFFFF);
		break;
	case 36:          //ICQ99
 	  SetDwFT(dwFT1, "dwFT1", 0x3AA773EE);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 37:          //WebICQ
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFFFF);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 38:          //SmartICQ
 	  SetDwFT(dwFT1, "dwFT1", 0xDDDDEEFF);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 39:           //IM+
 	  SetDwFT(dwFT1, "dwFT1", 0x494D2B00);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 48:  //SpamBot
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFFFFF);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x3B7248ED);
		break;
	case 51:           //R&Q
 	  SetDwFT(dwFT1, "dwFT1", 0xFFFFF666);
 	  SetDwFT(dwFT2, "dwFT2", 0x00000000);
 	  SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 11:
    SetDwFT(dwFT1, "dwFT1", gbUnicodeCore ? 0x7FFFFFFF : 0xFFFFFFFF);
 	  SetDwFT(dwFT2, "dwFT2", 0x06060600);
		/*/ eternity : commented, Unicode is already set in dwFT1
    if(gbHideIdEnabled)
      SetDwFT(dwFT3, "dwFT3", gbUnicodeCore ? 0x80000000 : 0x00000000);
		else
			SetDwFT(dwFT3, "dwFT3", gbSecureIM ? 0x5AFEC0DE : 0x00000000);
	  */
    SetDwFT(dwFT3, "dwFT3", gbSecureIM ? 0x5AFEC0DE : 0x00000000);
	  // eternity END	  
		break;
	default :								//miranda
    SetDwFT(dwFT1, "dwFT1", gbUnicodeCore ? 0x7FFFFFFF : 0xFFFFFFFF);
 	  SetDwFT(dwFT2, "dwFT2", ICQ_THISPLUG_VERSION);
    // note by jarvis : SecureIM flag is already send in MirVer packet (xxxxxxxxxxxxHERE)
    // I wonder if it is needed here whereas otherwise fake MIM IDs are not sending Unicode flag
    // I think here in timestamps Unicode flag should be prior
    // also, someone with SecureIM would like secure communication more than faking his ID, where safecode is included :)
    // maybe what we could use:
    // SetDwFT(dwFT3, "dwFT3", gbSecureIM ? 0x5AFEC0DE : gbUnicodeCore ? 0x80000000 : 0x00000000);
    // or better, if safecode wouldn't be so important, then only first expression without "if" clause
		/*if(gbHideIdEnabled)
      SetDwFT(dwFT3, "dwFT3", gbUnicodeCore ? 0x80000000 : 0x00000000);
		else*/
 	  SetDwFT(dwFT3, "dwFT3", gbSecureIM ? 0x5AFEC0DE : 0x00000000);
		break;
	}
}
