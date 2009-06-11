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
// File name      : $URL: https://miranda.svn.sourceforge.net/svnroot/miranda/trunk/miranda/protocols/IcqOscarJ/utilities.cpp $
// Revision       : $Revision: 8827 $
// Last change on : $Date: 2009-01-11 19:33:00 +0100 (Sun, 11 Jan 2009) $
// Last change by : $Author: ghazan $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

struct gateway_index
{
	HANDLE hConn;
	DWORD  dwIndex;
};

static int gatewayMutexRef = 0;
static CRITICAL_SECTION gatewayMutex;

static gateway_index *gateways = NULL;
static int gatewayCount = 0;

static DWORD *spammerList = NULL;
static int spammerListCount = 0;


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

	switch (nMirandaStatus) {
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

	switch (nMirandaStatus) {

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

char *MirandaStatusToString(int mirandaStatus)
{
	return (char*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, mirandaStatus, 0);
}

char *MirandaStatusToStringUtf(int mirandaStatus)
{ // return miranda status description in utf-8, use unicode service is possible
	return mtchar_to_utf8((TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, mirandaStatus, gbUnicodeCore ? GCMDF_UNICODE : 0));
}

char** CIcqProto::MirandaStatusToAwayMsg(int nStatus)
{
	switch (nStatus) {

  case ID_STATUS_ONLINE:
		return &m_modeMsgs.szOnline;

	case ID_STATUS_AWAY:
		return &m_modeMsgs.szAway;

	case ID_STATUS_NA:
		return &m_modeMsgs.szNa;

	case ID_STATUS_OCCUPIED:
		return &m_modeMsgs.szOccupied;

	case ID_STATUS_DND:
		return &m_modeMsgs.szDnd;

	case ID_STATUS_FREECHAT:
		return &m_modeMsgs.szFfc;

  case ID_STATUS_OFFLINE:
    return &m_modeMsgs.szOffline;

	default:
		return NULL;
	}
}

int AwayMsgTypeToStatus(int nMsgType)
{
	switch (nMsgType) {
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
	EnterCriticalSection(&gatewayMutex);

	for (int i = 0; i < gatewayCount; i++)
	{
		if (hConn == gateways[i].hConn)
		{
			gateways[i].dwIndex = dwIndex;
			LeaveCriticalSection(&gatewayMutex);
			return;
		}
	}

	gateways = (gateway_index *)SAFE_REALLOC(gateways, sizeof(gateway_index) * (gatewayCount + 1));
	gateways[gatewayCount].hConn = hConn;
	gateways[gatewayCount].dwIndex = dwIndex;
	gatewayCount++;

	LeaveCriticalSection(&gatewayMutex);
	return;
}

DWORD GetGatewayIndex(HANDLE hConn)
{
	int i;

	EnterCriticalSection(&gatewayMutex);

	for (i = 0; i < gatewayCount; i++)
	{
		if (hConn == gateways[i].hConn)
		{
			LeaveCriticalSection(&gatewayMutex);
			return gateways[i].dwIndex;
		}
	}

	LeaveCriticalSection(&gatewayMutex);
	return 1; // this is default
}

void FreeGatewayIndex(HANDLE hConn)
{
	EnterCriticalSection(&gatewayMutex);

	for (int i = 0; i < gatewayCount; i++)
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

	LeaveCriticalSection(&gatewayMutex);
}

void CIcqProto::AddToSpammerList(DWORD dwUIN)
{
	EnterCriticalSection(&cookieMutex);

	spammerList = (DWORD *)SAFE_REALLOC(spammerList, sizeof(DWORD) * (spammerListCount + 1));
	spammerList[spammerListCount] = dwUIN;
	spammerListCount++;

	LeaveCriticalSection(&cookieMutex);
}

BOOL CIcqProto::IsOnSpammerList(DWORD dwUIN)
{
	EnterCriticalSection(&cookieMutex);

	for (int i = 0; i < spammerListCount; i++)
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

void CIcqProto::AddToContactsCache(HANDLE hContact, DWORD dwUin, const char *szUid)
{
	if (!hContact || (!dwUin && !szUid))
		return;

#ifdef _DEBUG
  NetLog_Server("Adding contact to cache: %u%s%s", dwUin, dwUin ? "" : " - ", dwUin ? "" : szUid);
#endif

  icq_contacts_cache *cache_item = (icq_contacts_cache*)SAFE_MALLOC(sizeof(icq_contacts_cache));
  cache_item->hContact = hContact;
  cache_item->dwUin = dwUin;
  if (!dwUin)
    cache_item->szUid = null_strdup(szUid);

	EnterCriticalSection(&contactsCacheMutex);
  contactsCache.insert(cache_item);
	LeaveCriticalSection(&contactsCacheMutex);
}


void CIcqProto::InitContactsCache()
{
	InitializeCriticalSection(&contactsCacheMutex);
  if (!gatewayMutexRef++)
	  InitializeCriticalSection(&gatewayMutex);

	// build cache
	EnterCriticalSection(&contactsCacheMutex);

  HANDLE hContact = FindFirstContact();

	while (hContact)
	{
		DWORD dwUin;
    uid_str szUid;

    if (!getContactUid(hContact, &dwUin, &szUid))
      AddToContactsCache(hContact, dwUin, szUid);

		hContact = FindNextContact(hContact);
	}

	LeaveCriticalSection(&contactsCacheMutex);
}


void CIcqProto::UninitContactsCache(void)
{
  EnterCriticalSection(&contactsCacheMutex);
  // cleanup the cache
	for (int i = 0; i < contactsCache.getCount(); i++)
  {
    icq_contacts_cache *cache_item = contactsCache[i];

    SAFE_FREE((void**)&cache_item->szUid);
    SAFE_FREE((void**)&cache_item);
  }
  contactsCache.destroy();
  LeaveCriticalSection(&contactsCacheMutex);
	DeleteCriticalSection(&contactsCacheMutex);
  if (!--gatewayMutexRef)
	  DeleteCriticalSection(&gatewayMutex);
}


void CIcqProto::DeleteFromContactsCache(HANDLE hContact)
{
  EnterCriticalSection(&contactsCacheMutex);

	for (int i = 0; i < contactsCache.getCount(); i++)
	{
    icq_contacts_cache *cache_item = contactsCache[i];

		if (cache_item->hContact == hContact)
		{
#ifdef _DEBUG
			NetLog_Server("Removing contact from cache: %u%s%s, position: %u", cache_item->dwUin, cache_item->dwUin ? "" : " - ", cache_item->dwUin ? "" : cache_item->szUid, i);
#endif
      contactsCache.remove(i);
      // Release memory
      SAFE_FREE((void**)&cache_item->szUid);
      SAFE_FREE((void**)&cache_item);
			break;
    }
  }
  LeaveCriticalSection(&contactsCacheMutex);
}


HANDLE CIcqProto::HandleFromCacheByUid(DWORD dwUin, const char *szUid)
{
	HANDLE hContact = NULL;
  icq_contacts_cache cache_item = {NULL, dwUin, szUid};

	EnterCriticalSection(&contactsCacheMutex);
  // find in list
  int i = contactsCache.getIndex(&cache_item);
  if (i != -1)
		hContact = contactsCache[i]->hContact;
  LeaveCriticalSection(&contactsCacheMutex);

	return hContact;
}


HANDLE CIcqProto::HContactFromUIN(DWORD dwUin, int *Added)
{
	HANDLE hContact;

	if (Added) *Added = 0;

	hContact = HandleFromCacheByUid(dwUin, NULL);
	if (hContact) return hContact;

	hContact = FindFirstContact();
	while (hContact)
	{
		DWORD dwContactUin;

		dwContactUin = getContactUin(hContact);
		if (dwContactUin == dwUin)
		{
			AddToContactsCache(hContact, dwUin, NULL);
			return hContact;
		}

		hContact = FindNextContact(hContact);
	}

	//not present: add
	if (Added)
	{
		hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (!hContact)
		{
			NetLog_Server("Failed to create ICQ contact %u", dwUin);
			return INVALID_HANDLE_VALUE;
		}

		if (CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName) != 0)
		{
			// For some reason we failed to register the protocol to this contact
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
			NetLog_Server("Failed to register ICQ contact %u", dwUin);
			return INVALID_HANDLE_VALUE;
		}

		setSettingDword(hContact, UNIQUEIDSETTING, dwUin);

		if (!bIsSyncingCL)
		{
			DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			setContactHidden(hContact, 1);

			setSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

			icq_QueueUser(hContact);

			if (icqOnline())
			{
				icq_sendNewContact(dwUin, NULL);
			}
			if (getSettingByte(NULL, "KillSpambots", DEFAULT_KILLSPAM_ENABLED))
				icq_sendCheckSpamBot(hContact, dwUin, NULL);
		}
		AddToContactsCache(hContact, dwUin, NULL);
		*Added = 1;

		return hContact;
	}

	// not in list, check that uin do not belong to us
	if (getContactUin(NULL) == dwUin)
		return NULL;

	return INVALID_HANDLE_VALUE;
}


HANDLE CIcqProto::HContactFromUID(DWORD dwUin, const char *szUid, int *Added)
{
	HANDLE hContact;

	if (dwUin)
		return HContactFromUIN(dwUin, Added);

	if (Added) *Added = 0;

	if (!m_bAimEnabled) return INVALID_HANDLE_VALUE;

  hContact = HandleFromCacheByUid(dwUin, szUid);
	if (hContact) return hContact;

	hContact = FindFirstContact();
	while (hContact)
	{
    DWORD dwContactUin;
    uid_str szContactUid;

		if (!getContactUid(hContact, &dwContactUin, &szContactUid))
		{
			if (!dwContactUin && !stricmpnull(szContactUid, szUid))
			{
				if (strcmpnull(szContactUid, szUid))
				{ // fix case in SN
					setSettingString(hContact, UNIQUEIDSETTING, szUid);
				}
				return hContact;
			}
		}
		hContact = FindNextContact(hContact);
	}

	//not present: add
	if (Added)
	{
		hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName);

		setSettingString(hContact, UNIQUEIDSETTING, szUid);

		if (!bIsSyncingCL)
		{
			DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			setContactHidden(hContact, 1);

			setSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

			if (icqOnline())
			{
				icq_sendNewContact(0, szUid);
			}
			if (getSettingByte(NULL, "KillSpambots", DEFAULT_KILLSPAM_ENABLED))
				icq_sendCheckSpamBot(hContact, 0, szUid);
		}
		AddToContactsCache(hContact, 0, szUid);
		*Added = 1;

		return hContact;
	}

	return INVALID_HANDLE_VALUE;
}


HANDLE CIcqProto::HContactFromAuthEvent(HANDLE hEvent)
{
	DBEVENTINFO dbei;
	DWORD body[2];

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = sizeof(DWORD)*2;
	dbei.pBlob = (PBYTE)&body;

	if (CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbei))
		return INVALID_HANDLE_VALUE;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_HANDLE_VALUE;

	if (strcmpnull(dbei.szModule, m_szModuleName))
		return INVALID_HANDLE_VALUE;

	return (HANDLE)body[1]; // this is bad - needs new auth system
}

char *NickFromHandle(HANDLE hContact)
{
	if (hContact == INVALID_HANDLE_VALUE)
		return null_strdup(ICQTranslate("<invalid>"));

	return null_strdup((char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
}

char *NickFromHandleUtf(HANDLE hContact)
{
	if (hContact == INVALID_HANDLE_VALUE)
		return ICQTranslateUtf(LPGEN("<invalid>"));

	return mtchar_to_utf8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, gbUnicodeCore ? GCDNF_UNICODE : 0));
}

char *strUID(DWORD dwUIN, char *pszUID)
{
	if (dwUIN)
		_ltoa(dwUIN, pszUID, 10);

	return pszUID;
}

/* a strlen() that likes NULL */
size_t __fastcall strlennull(const char *string)
{
	if (string)
		return strlen(string);

	return 0;
}


/* a wcslen() that likes NULL */
size_t __fastcall strlennull(const WCHAR *string)
{
  if (string)
    return wcslen(string);

  return 0;
}


/* a strcmp() that likes NULL */
int __fastcall strcmpnull(const char *str1, const char *str2)
{
	if (str1 && str2)
		return strcmp(str1, str2);

  if (!str1 && !str2)
    return 0;

  return 1;
}

/* a stricmp() that likes NULL */
int __fastcall stricmpnull(const char *str1, const char *str2)
{
	if (str1 && str2)
		return _stricmp(str1, str2);

  if (!str1 && !str2)
    return 0;

	return 1;
}

char* __fastcall strstrnull(const char *str, const char *substr)
{
	if (str)
		return (char*)strstr(str, substr);

	return NULL;
}

int null_snprintf(char *buffer, size_t count, const char *fmt, ...)
{
	va_list va;
	int len;

	ZeroMemory(buffer, count);
	va_start(va, fmt);
	len = _vsnprintf(buffer, count-1, fmt, va);
	va_end(va);
	return len;
}

int null_snprintf(WCHAR *buffer, size_t count, const WCHAR *fmt, ...)
{
  va_list va;
  int len;

  ZeroMemory(buffer, count * sizeof(WCHAR));
  va_start(va, fmt);
  len = _vsnwprintf(buffer, count, fmt, va);
  va_end(va);
  return len;
}

char* __fastcall null_strdup(const char *string)
{
	if (string)
		return _strdup(string);

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
		if (!_strnicmp(string+i, "&gt;", 4))
		{
			*szChar = '>';
			szChar++;
			i += 3;
		}
		else if (!_strnicmp(string+i, "&lt;", 4))
		{
			*szChar = '<';
			szChar++;
			i += 3;
		}
		else if (!_strnicmp(string+i, "&amp;", 5))
		{
			*szChar = '&';
			szChar++;
			i += 4;
		}
		else if (!_strnicmp(string+i, "&quot;", 6))
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
		if (string[i]=='<' || string[i]=='>') l += 4; else if (string[i]=='&') l += 5; else if (string[i]=='"') l += 6; else l++;
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
    else if (string[i]=='"')
    {
      *(DWORD*)szChar = 'ouq&';
      szChar += 4;
      *(WORD*)szChar = ';t';
      szChar += 2;
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
			if ((i + 4 <= len) && (!_strnicmp(string + i, "<br>", 4) || !_strnicmp(string + i, "<br/>", 5)))
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
	SAFE_FREE((void**)&tmp);

	return res;
}

char *ApplyEncoding(const char *string, const char *pszEncoding)
{ // decode encoding to Utf-8
	if (string && pszEncoding)
	{ // we do only encodings known to icq5.1 // TODO: check if this is enough
		if (!_strnicmp(pszEncoding, "utf-8", 5))
		{ // it is utf-8 encoded
			return null_strdup(string);
		}
		if (!_strnicmp(pszEncoding, "unicode-2-0", 11))
		{ // it is UCS-2 encoded
			int wLen = strlennull((WCHAR*)string) + 1;
			WCHAR *szStr = (WCHAR*)_alloca(wLen*2);
			BYTE *tmp = (BYTE*)string;

			unpackWideString(&tmp, szStr, (WORD)(wLen*2));

			return make_utf8_string(szStr);
		}
		if (!_strnicmp(pszEncoding, "iso-8859-1", 10))
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

void CIcqProto::ResetSettingsOnListReload()
{
	// Reset a bunch of session specific settings
	setSettingWord(NULL, DBSETTING_SERVLIST_PRIVACY, 0);
	setSettingWord(NULL, DBSETTING_SERVLIST_METAINFO, 0);
	setSettingWord(NULL, DBSETTING_SERVLIST_AVATAR, 0);
	setSettingWord(NULL, DBSETTING_SERVLIST_PHOTO, 0);
	setSettingWord(NULL, "SrvRecordCount", 0);
	deleteSetting(NULL, DBSETTING_SERVLIST_UNHANDLED);

	HANDLE hContact = FindFirstContact();

	while (hContact)
	{
		// All these values will be restored during the serv-list receive
		setSettingWord(hContact, DBSETTING_SERVLIST_ID, 0);
		setSettingWord(hContact, DBSETTING_SERVLIST_GROUP, 0);
		setSettingWord(hContact, DBSETTING_SERVLIST_PERMIT, 0);
		setSettingWord(hContact, DBSETTING_SERVLIST_DENY, 0);
    deleteSetting(hContact, DBSETTING_SERVLIST_IGNORE);
		setSettingByte(hContact, "Auth", 0);
    deleteSetting(hContact, DBSETTING_SERVLIST_DATA);

		hContact = FindNextContact(hContact);
	}

	FlushSrvGroupsCache();
}

void CIcqProto::ResetSettingsOnConnect()
{
	// Reset a bunch of session specific settings
	setSettingByte(NULL, "SrvVisibility", 0);
	setSettingDword(NULL, "IdleTS", 0);

	HANDLE hContact = FindFirstContact();

	while (hContact)
	{
		setSettingDword(hContact, "LogonTS", 0);
		setSettingDword(hContact, "IdleTS", 0);
		setSettingDword(hContact, "TickTS", 0);
		setSettingByte(hContact, "TemporaryVisible", 0);

		// All these values will be restored during the login
		if (getContactStatus(hContact) != ID_STATUS_OFFLINE)
			setSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

		hContact = FindNextContact(hContact);
	}
}

void CIcqProto::ResetSettingsOnLoad()
{
	setSettingDword(NULL, "IdleTS", 0);
	setSettingDword(NULL, "LogonTS", 0);

	HANDLE hContact = FindFirstContact();

	while (hContact)
	{
		setSettingDword(hContact, "LogonTS", 0);
		setSettingDword(hContact, "IdleTS", 0);
		setSettingDword(hContact, "TickTS", 0);
		if (getContactStatus(hContact) != ID_STATUS_OFFLINE)
		{
			setSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

			deleteSetting(hContact, DBSETTING_XSTATUS_ID);
			deleteSetting(hContact, DBSETTING_XSTATUS_NAME);
			deleteSetting(hContact, DBSETTING_XSTATUS_MSG);
		}
		setSettingByte(hContact, "DCStatus", 0);

		hContact = FindNextContact(hContact);
	}
}

int RandRange(int nLow, int nHigh)
{
	return nLow + (int)((nHigh-nLow+1)*rand()/(RAND_MAX+1.0));
}


BOOL IsStringUIN(const char *pszString)
{
	int i;
	int nLen = strlennull(pszString);

	if (nLen > 0 && pszString[0] != '0')
	{
		for (i=0; i<nLen; i++)
			if ((pszString[i] < '0') || (pszString[i] > '9'))
				return FALSE;

		return TRUE;
	}

	return FALSE;
}


void __cdecl CIcqProto::ProtocolAckThread(icq_ack_args* pArguments)
{
	BroadcastAck(pArguments->hContact, pArguments->nAckType, pArguments->nAckResult, pArguments->hSequence, pArguments->pszMessage);

	if (pArguments->nAckResult == ACKRESULT_SUCCESS)
		NetLog_Server("Sent fake message ack");
	else if (pArguments->nAckResult == ACKRESULT_FAILED)
		NetLog_Server("Message delivery failed");

	SAFE_FREE((void**)(char **)&pArguments->pszMessage);
	SAFE_FREE((void**)&pArguments);
}

void CIcqProto::SendProtoAck(HANDLE hContact, DWORD dwCookie, int nAckResult, int nAckType, char* pszMessage)
{
	icq_ack_args* pArgs = (icq_ack_args*)SAFE_MALLOC(sizeof(icq_ack_args)); // This will be freed in the new thread
	pArgs->hContact = hContact;
	pArgs->hSequence = (HANDLE)dwCookie;
	pArgs->nAckResult = nAckResult;
	pArgs->nAckType = nAckType;
	pArgs->pszMessage = (LPARAM)null_strdup(pszMessage);
	
	ForkThread(( IcqThreadFunc )&CIcqProto::ProtocolAckThread, pArgs );
}

void CIcqProto::SetCurrentStatus(int nStatus)
{
	int nOldStatus = m_iStatus;

	m_iStatus = nStatus;
	BroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)nOldStatus, nStatus);
}


int CIcqProto::IsMetaInfoChanged(HANDLE hContact)
{
  DBVARIANT infoToken = {DBVT_DELETED};
  int res = 0;

  if (!getSetting(hContact, DBSETTING_METAINFO_TOKEN, &infoToken))
  { // contact does have info from directory, check if it is not outdated
    double dInfoTime = 0;
    double dInfoSaved = 0;

    if ((dInfoTime = getSettingDouble(hContact, DBSETTING_METAINFO_TIME, 0)) > 0)
    {
      if ((dInfoSaved = getSettingDouble(hContact, DBSETTING_METAINFO_SAVED, 0)) > 0)
      {
        if (dInfoSaved < dInfoTime)
          res = 2; // directory info outdated
      }
      else
        res = 1; // directory info not saved at all
    }

    ICQFreeVariant(&infoToken);
  }
  else
  { // it cannot be detected if user info was not changed, so use a generic threshold
    DBVARIANT infoSaved = {DBVT_DELETED};
    DWORD dwInfoTime = 0;

    if (!getSetting(hContact, DBSETTING_METAINFO_SAVED, &infoSaved))
    {
      if (infoSaved.type == DBVT_BLOB && infoSaved.cpbVal == 8)
      {
        double dwTime = *(double*)infoSaved.pbVal;

        dwInfoTime = (dwTime - 25567) * 86400;
      }
      else if (infoSaved.type == DBVT_DWORD)
        dwInfoTime = infoSaved.dVal;

      ICQFreeVariant(&infoSaved);

      if ((time(NULL) - dwInfoTime) > 14*3600*24)
      { 
        res = 3; // threshold exceeded
      }
    }
    else
      res = 4; // no timestamp found
  }

  return res;
}


void __cdecl CIcqProto::SetStatusNoteThread(void *pDelay)
{
  if (pDelay)
    SleepEx((DWORD)pDelay, TRUE);

  EnterCriticalSection(&cookieMutex);

  if (icqOnline() && (setStatusNoteText || setStatusMoodData))
  { // send status note change packets, write status note to database
    if (setStatusNoteText)
    { // change status note in directory
		  EnterCriticalSection(&ratesMutex);
		  if (m_rates)
		  { // rate management
			WORD wGroup = m_rates->getGroupFromSNAC(ICQ_EXTENSIONS_FAMILY, ICQ_META_CLI_REQUEST);

  			while (m_rates->getNextRateLevel(wGroup) < m_rates->getLimitLevel(wGroup, RML_LIMIT))
	  		{ // we are over rate, need to wait before sending
		  		int nDelay = m_rates->getDelayToLimitLevel(wGroup, RML_IDLE_10);

  				LeaveCriticalSection(&ratesMutex);
          LeaveCriticalSection(&cookieMutex);
#ifdef _DEBUG
		  		NetLog_Server("Rates: SetStatusNote delayed %dms", nDelay);
#endif
			  	SleepEx(nDelay, TRUE); // do not keep things locked during sleep
          EnterCriticalSection(&cookieMutex);
  				EnterCriticalSection(&ratesMutex);
	  			if (!m_rates) // we lost connection when we slept, go away
		  			break;
			  }
      }
      LeaveCriticalSection(&ratesMutex);

      BYTE *pBuffer = NULL;
      int cbBuffer = 0;

      ppackTLV(&pBuffer, &cbBuffer, 0x226, strlennull(setStatusNoteText), (BYTE*)setStatusNoteText);
      icq_changeUserDirectoryInfoServ(pBuffer, cbBuffer, DIRECTORYREQUEST_UPDATENOTE);

      SAFE_FREE((void**)&pBuffer);
    }

    if (setStatusNoteText || setStatusMoodData)
    { // change status note and mood in session data
      EnterCriticalSection(&ratesMutex);
      if (m_rates)
      { // rate management
        WORD wGroup = m_rates->getGroupFromSNAC(ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);

        while (m_rates->getNextRateLevel(wGroup) < m_rates->getLimitLevel(wGroup, RML_LIMIT))
        { // we are over rate, need to wait before sending
          int nDelay = m_rates->getDelayToLimitLevel(wGroup, RML_IDLE_10);

          LeaveCriticalSection(&ratesMutex);
          LeaveCriticalSection(&cookieMutex);
#ifdef _DEBUG
          NetLog_Server("Rates: SetStatusNote delayed %dms", nDelay);
#endif
          SleepEx(nDelay, TRUE); // do not keep things locked during sleep
          EnterCriticalSection(&cookieMutex);
          EnterCriticalSection(&ratesMutex);
          if (!m_rates) // we lost connection when we slept, go away
            break;
        }
      }
      LeaveCriticalSection(&ratesMutex);

      // check if the session data were not updated already
      char *szCurrentStatusNote = getSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, NULL);
      char *szCurrentStatusMood = NULL;
      DBVARIANT dbv = {DBVT_DELETED};

      if (!getSettingString(NULL, DBSETTING_STATUS_MOOD, &dbv))
        szCurrentStatusMood = dbv.pszVal;

      if (!setStatusNoteText && szCurrentStatusNote)
        setStatusNoteText = null_strdup(szCurrentStatusNote);
      if (!setStatusMoodData && szCurrentStatusMood)
        setStatusMoodData = null_strdup(szCurrentStatusMood);

      if (strcmpnull(szCurrentStatusNote, setStatusNoteText) || strcmpnull(szCurrentStatusMood, setStatusMoodData))
      {
        setSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, setStatusNoteText);
        setSettingString(NULL, DBSETTING_STATUS_MOOD, setStatusMoodData);

        WORD wStatusNoteLen = strlennull(setStatusNoteText);
        WORD wStatusMoodLen = strlennull(setStatusMoodData);
        icq_packet packet;
        WORD wDataLen = (wStatusNoteLen ? wStatusNoteLen + 4 : 0) + 4 + wStatusMoodLen + 4;

    		serverPacketInit(&packet, wDataLen + 14);
		    packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
        // Change only session data
		    packWord(&packet, 0x1D);              // TLV 1D
		    packWord(&packet, wDataLen);          // TLV length
        packWord(&packet, 0x02);              // Item Type
        if (wStatusNoteLen)
        {
          packWord(&packet, 0x400 | (WORD)(wStatusNoteLen + 4)); // Flags + Item Length
          packWord(&packet, wStatusNoteLen);  // Text Length
          packBuffer(&packet, (LPBYTE)setStatusNoteText, wStatusNoteLen);
          packWord(&packet, 0);               // Encoding not specified (utf-8 is default)
        }
        else
          packWord(&packet, 0);               // Flags + Item Length
  			packWord(&packet, 0x0E);              // Item Type
			  packWord(&packet, wStatusMoodLen);    // Flags + Item Length
        if (wStatusMoodLen)
			    packBuffer(&packet, (LPBYTE)setStatusMoodData, wStatusMoodLen); // Mood

        sendServPacket(&packet);
      }
      SAFE_FREE((void**)&szCurrentStatusNote);
      ICQFreeVariant(&dbv);
		}
  }
  SAFE_FREE((void**)&setStatusNoteText);
  SAFE_FREE((void**)&setStatusMoodData);

  LeaveCriticalSection(&cookieMutex);
}


int CIcqProto::SetStatusNote(const char *szStatusNote, DWORD dwDelay, int bForce)
{
  int bChanged = FALSE;

  // bForce is intended for login sequence - need to call this earlier than icqOnline()
  // the process is delayed and icqOnline() is ready when needed inside SetStatusNoteThread()
  if (!bForce && !icqOnline()) return bChanged;

  // reuse generic critical section (used for cookies list and object variables locks)
  EnterCriticalSection(&cookieMutex);

  if (!setStatusNoteText && !setStatusMoodData)
  { // check if the status note was changed and if yes, create thread to change it
    char *szCurrentStatusNote = getSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, NULL);

    if (strcmpnull(szCurrentStatusNote, szStatusNote))
    { // status note was changed
      // create thread to change status note on existing server connection
      setStatusNoteText = null_strdup(szStatusNote);

      if (dwDelay)
			ForkThread(&CIcqProto::SetStatusNoteThread, (void*)dwDelay);
      else // we cannot afford any delay, so do not run in separate thread
        SetStatusNoteThread(NULL);

	  bChanged = TRUE;
    }
    SAFE_FREE((void**)&szCurrentStatusNote);
  }
  else
  { // only alter status note object with new status note, keep the thread waiting for execution
    SAFE_FREE((void**)&setStatusNoteText);
    setStatusNoteText = null_strdup(szStatusNote);

	bChanged = TRUE;
  }
  LeaveCriticalSection(&cookieMutex);

  return bChanged;
}


int CIcqProto::SetStatusMood(const char *szMoodData, DWORD dwDelay)
{
  int bChanged = FALSE;

  if (!icqOnline()) return bChanged;

  // reuse generic critical section (used for cookies list and object variables locks)
  EnterCriticalSection(&cookieMutex);

  if (!setStatusNoteText && !setStatusMoodData)
  { // check if the status mood was changed and if yes, create thread to change it
    char *szCurrentStatusMood = NULL;
    DBVARIANT dbv = {DBVT_DELETED};

    if (!getSettingString(NULL, DBSETTING_STATUS_MOOD, &dbv))
      szCurrentStatusMood = dbv.pszVal;

    if (strcmpnull(szCurrentStatusMood, szMoodData))
    { // status mood was changed
      // create thread to change status mood on existing server connection
      setStatusMoodData = null_strdup(szMoodData);
      if (dwDelay)
        ForkThread(&CIcqProto::SetStatusNoteThread, (void*)dwDelay);
      else // we cannot afford any delay, so do not run in separate thread
        SetStatusNoteThread(NULL);

	  bChanged = TRUE;
    }
    ICQFreeVariant(&dbv);
  }
  else
  { // only alter status mood object with new status mood, keep the thread waiting for execution
    SAFE_FREE((void**)&setStatusMoodData);
    setStatusMoodData = null_strdup(szMoodData);

	bChanged = TRUE;
  }
  LeaveCriticalSection(&cookieMutex);

  return bChanged;
}


void CIcqProto::writeDbInfoSettingTLVStringUtf(HANDLE hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
  oscar_tlv *pTLV = chain->getTLV(wTlv, 1);

  if (pTLV && pTLV->wLen > 0)
  {
    char *str = (char*)_alloca(pTLV->wLen + 1); 

    memcpy(str, pTLV->pData, pTLV->wLen);
    str[pTLV->wLen] = '\0';
    setSettingStringUtf(hContact, szSetting, str);
  }
  else
    deleteSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVString(HANDLE hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
  oscar_tlv *pTLV = chain->getTLV(wTlv, 1);

  if (pTLV && pTLV->wLen > 0)
  {
    char *str = (char*)_alloca(pTLV->wLen + 1); 

    memcpy(str, pTLV->pData, pTLV->wLen);
    str[pTLV->wLen] = '\0';
    setSettingString(hContact, szSetting, str);
  }
  else
    deleteSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVWord(HANDLE hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
  int num = chain->getNumber(wTlv, 1);

  if (num > 0)
    setSettingWord(hContact, szSetting, num);
  else
    deleteSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVByte(HANDLE hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
  int num = chain->getNumber(wTlv, 1);

  if (num > 0)
    setSettingByte(hContact, szSetting, num);
  else
    deleteSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVDouble(HANDLE hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
  double num = chain->getDouble(wTlv, 1);

  if (num > 0)
    setSettingDouble(hContact, szSetting, num);
  else
    deleteSetting(hContact, szSetting);
}

void CIcqProto::writeDbInfoSettingTLVDate(HANDLE hContact, const char* szSettingYear, const char* szSettingMonth, const char* szSettingDay, oscar_tlv_chain* chain, WORD wTlv)
{
  double time = chain->getDouble(wTlv, 1);

  if (time > 0)
  { // date is stored as double with unit equal to a day, incrementing since 1/1/1900 0:00 GMT
    SYSTEMTIME sTime = {0};
    if (VariantTimeToSystemTime(time + 2, &sTime))
    {
      setSettingWord(hContact, szSettingYear, sTime.wYear);
      setSettingByte(hContact, szSettingMonth, (BYTE)sTime.wMonth);
      setSettingByte(hContact, szSettingDay, (BYTE)sTime.wDay);
    }
    else
    {
      deleteSetting(hContact, szSettingYear);
      deleteSetting(hContact, szSettingMonth);
      deleteSetting(hContact, szSettingDay);
    }
  }
  else
  {
    deleteSetting(hContact, szSettingYear);
    deleteSetting(hContact, szSettingMonth);
    deleteSetting(hContact, szSettingDay);
  }
}


void CIcqProto::writeDbInfoSettingTLVTimestamp(HANDLE hContact, const char* szSetting, oscar_tlv_chain* chain, WORD wTlv)
{
  double time = chain->getDouble(wTlv, 1); 

  if (time > 0)
  { // date is stored as double with unit equal to a day, incrementing since 1/1/1900 0:00 GMT
    setSettingDword(hContact, szSetting, (time - 25567) * 86400);
  }
  else
    deleteSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVBlob(HANDLE hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
  oscar_tlv *pTLV = chain->getTLV(wTlv, 1);

  if (pTLV && pTLV->wLen > 0)
    setSettingBlob(hContact, szSetting, pTLV->pData, pTLV->wLen);
  else
    deleteSetting(hContact, szSetting);
}


BOOL CIcqProto::writeDbInfoSettingString(HANDLE hContact, const char* szSetting, char** buf, WORD* pwLength)
{
	WORD wLen;

	if (*pwLength < 2)
		return FALSE;

	unpackLEWord((LPBYTE*)buf, &wLen);
	*pwLength -= 2;

	if (*pwLength < wLen)
		return FALSE;

	if ((wLen > 0) && (**buf) && ((*buf)[wLen-1]==0)) // Make sure we have a proper string
	{
		WORD wCp = getSettingWord(hContact, "InfoCodePage", getSettingWord(hContact, "InfoCP", CP_ACP));

		if (wCp != CP_ACP)
		{
			char *szUtf = ansi_to_utf8_codepage(*buf, wCp);

			if (szUtf)
			{
				setSettingStringUtf(hContact, szSetting, szUtf);
				SAFE_FREE((void**)&szUtf);
			}
			else
				setSettingString(hContact, szSetting, *buf);
		}
		else
			setSettingString(hContact, szSetting, *buf);
	}
	else
		deleteSetting(hContact, szSetting);

	*buf += wLen;
	*pwLength -= wLen;

	return TRUE;
}

BOOL CIcqProto::writeDbInfoSettingWord(HANDLE hContact, const char *szSetting, char **buf, WORD* pwLength)
{
	WORD wVal;


	if (*pwLength < 2)
		return FALSE;

	unpackLEWord((LPBYTE*)buf, &wVal);
	*pwLength -= 2;

	if (wVal != 0)
		setSettingWord(hContact, szSetting, wVal);
	else
		deleteSetting(hContact, szSetting);

	return TRUE;
}

BOOL CIcqProto::writeDbInfoSettingWordWithTable(HANDLE hContact, const char *szSetting, const FieldNamesItem *table, char **buf, WORD* pwLength)
{
	WORD wVal;
	char sbuf[MAX_PATH];
	char *text;

	if (*pwLength < 2)
		return FALSE;

	unpackLEWord((LPBYTE*)buf, &wVal);
	*pwLength -= 2;

	text = LookupFieldNameUtf(table, wVal, sbuf, MAX_PATH);
	if (text)
		setSettingStringUtf(hContact, szSetting, text);
	else
		deleteSetting(hContact, szSetting);

	return TRUE;
}

BOOL CIcqProto::writeDbInfoSettingByte(HANDLE hContact, const char *pszSetting, char **buf, WORD* pwLength)
{
	BYTE byVal;

	if (*pwLength < 1)
		return FALSE;

	unpackByte((LPBYTE*)buf, &byVal);
	*pwLength -= 1;

	if (byVal != 0)
		setSettingByte(hContact, pszSetting, byVal);
	else
		deleteSetting(hContact, pszSetting);

	return TRUE;
}

BOOL CIcqProto::writeDbInfoSettingByteWithTable(HANDLE hContact, const char *szSetting, const FieldNamesItem *table, char **buf, WORD* pwLength)
{
	BYTE byVal;
	char sbuf[MAX_PATH];
	char *text;

	if (*pwLength < 1)
		return FALSE;

	unpackByte((LPBYTE*)buf, &byVal);
	*pwLength -= 1;

	text = LookupFieldNameUtf(table, byVal, sbuf, MAX_PATH);
	if (text)
		setSettingStringUtf(hContact, szSetting, text);
	else
		deleteSetting(hContact, szSetting);

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


char* time2text(time_t time)
{
	tm *local = localtime(&time);

	if (local)
	{
		char *str = asctime(local);
		str[24] = '\0'; // remove new line
		return str;
	}
	else
		return "<invalid>";
}


BOOL CIcqProto::validateStatusMessageRequest(HANDLE hContact, WORD byMessageType)
{
	// Privacy control
	if (getSettingByte(NULL, "StatusMsgReplyCList", 0))
	{
		// Don't send statusmessage to unknown contacts
		if (hContact == INVALID_HANDLE_VALUE)
			return FALSE;

		// Don't send statusmessage to temporary contacts or hidden contacts
		if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) ||
			DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
			return FALSE;

		// Don't send statusmessage to invisible contacts
		if (getSettingByte(NULL, "StatusMsgReplyVisible", 0))
		{
			WORD wStatus = getContactStatus(hContact);
			if (wStatus == ID_STATUS_OFFLINE)
				return FALSE;
		}
	}

	// Don't send any status message reply, if set
	if (m_bNoStatusReply)
		return FALSE;

	// Dont send messages to people you are hiding from
	if (hContact != INVALID_HANDLE_VALUE &&
		getSettingWord(hContact, "ApparentMode", 0) == ID_STATUS_OFFLINE)
	{
		CHECKCONTACT chk = {0};
		chk.hContact = hContact;
		chk.iDBEventFlag = DBEF_READ;
		chk.bShowPopup = chk.bLogToFile = chk.bHistoryEvent = TRUE;
		chk.iPopupEvent = POPTYPE_VISIBILITY;
		chk.iICQEventType = ICQEVENTTYPE_CHECKSTATUS;
		chk.dwUin = getSettingDword(hContact, "UIN", 0);
		chk.pszMsg = "Contact which you're invisible for requests your status message";
		CheckContact(chk);
		return FALSE;
	}

	// Dont respond to request for other statuses than your current one
	if ((byMessageType == MTYPE_AUTOONLINE && m_iStatus != ID_STATUS_ONLINE) ||
		(byMessageType == MTYPE_AUTOAWAY && m_iStatus != ID_STATUS_AWAY) ||
		(byMessageType == MTYPE_AUTOBUSY && m_iStatus != ID_STATUS_OCCUPIED) ||
		(byMessageType == MTYPE_AUTONA   && m_iStatus != ID_STATUS_NA) ||
		(byMessageType == MTYPE_AUTODND  && m_iStatus != ID_STATUS_DND) ||
		(byMessageType == MTYPE_AUTOFFC  && m_iStatus != ID_STATUS_FREECHAT))
	{
		CHECKCONTACT chk = {0};
		chk.hContact = hContact;
		chk.iDBEventFlag = DBEF_READ;
		chk.bShowPopup = chk.bLogToFile = chk.bHistoryEvent = TRUE;
		chk.iPopupEvent = POPTYPE_INFOREQUEST;
		chk.iICQEventType = ICQEVENTTYPE_CHECKSTATUS;
		chk.dwUin = getSettingDword(hContact, "UIN", 0);
		chk.pszMsg = "Contact requests wrong status type or tries to detect you";
		CheckContact(chk);
		return FALSE;
	}

//	if (hContact != INVALID_HANDLE_VALUE && m_iStatus==ID_STATUS_INVISIBLE &&
//		getSettingWord(hContact, "ApparentMode", 0) != ID_STATUS_ONLINE)
	if (hContact != INVALID_HANDLE_VALUE && m_iStatus==ID_STATUS_INVISIBLE)
	{
		if (!getSettingByte(hContact, "TemporaryVisible", 0))
		{ // Allow request to temporary visible contacts
			return FALSE;
		}
	}
	{
		CHECKCONTACT chk = {0};
		chk.hContact = hContact;
		chk.dwUin = getSettingDword(hContact, "UIN", 0);
		chk.iPSD = -1;
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


DWORD ICQWaitForSingleObject(HANDLE hObject, DWORD dwMilliseconds, int bWaitAlways)
{
	DWORD dwResult;

	do { // will get WAIT_IO_COMPLETION for QueueUserAPC(), ignore it unless terminating
		dwResult = WaitForSingleObjectEx(hObject, dwMilliseconds, TRUE);
	} while (dwResult == WAIT_IO_COMPLETION && (bWaitAlways || !Miranda_Terminated()));

	return dwResult;
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

HANDLE CIcqProto::NetLib_BindPort(NETLIBNEWCONNECTIONPROC_V2 pFunc, void* lParam, WORD* pwPort, DWORD* pdwIntIP)
{
	NETLIBBIND nlb = {0};
	HANDLE hBoundPort;

	nlb.cbSize = sizeof(NETLIBBIND); 
	nlb.pfnNewConnectionV2 = pFunc;
	nlb.pExtra = lParam;
	SetLastError(ERROR_INVALID_PARAMETER); // this must be here - NetLib does not set any error :((
	hBoundPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hDirectNetlibUser, (LPARAM)&nlb);
	if (!hBoundPort && (GetLastError() == ERROR_INVALID_PARAMETER))
	{ // this ensures older Miranda also can bind a port for a dc - pre 0.6
		nlb.cbSize = NETLIBBIND_SIZEOF_V2;
		hBoundPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hDirectNetlibUser, (LPARAM)&nlb);
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

int CIcqProto::NetLog_Server(const char *fmt,...)
{
	va_list va;
	char szText[1024];

	va_start(va,fmt);
	mir_vsnprintf(szText,sizeof(szText),fmt,va);
	va_end(va);
	return CallService(MS_NETLIB_LOG,(WPARAM)m_hServerNetlibUser,(LPARAM)szText);
}

int CIcqProto::NetLog_Direct(const char *fmt,...)
{
	va_list va;
	char szText[1024];

	va_start(va,fmt);
	mir_vsnprintf(szText,sizeof(szText),fmt,va);
	va_end(va);
	return CallService(MS_NETLIB_LOG,(WPARAM)m_hDirectNetlibUser,(LPARAM)szText);
}

int CIcqProto::NetLog_Uni(BOOL bDC, const char *fmt,...)
{
	va_list va; 
	char szText[1024];
	HANDLE hNetlib;

	va_start(va,fmt);
	mir_vsnprintf(szText,sizeof(szText),fmt,va);
	va_end(va);

	if (bDC)
		hNetlib = m_hDirectNetlibUser;
	else
		hNetlib = m_hServerNetlibUser;

	return CallService(MS_NETLIB_LOG,(WPARAM)hNetlib,(LPARAM)szText);
}

int CIcqProto::BroadcastAck(HANDLE hContact,int type,int result,HANDLE hProcess,LPARAM lParam)
{
	ACKDATA ack={0};

	ack.cbSize = sizeof(ACKDATA);
	ack.szModule = m_szModuleName;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;
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

char* __fastcall ICQTranslate(const char *src)
{
	return (char*)CallService(MS_LANGPACK_TRANSLATESTRING,0,(LPARAM)src);
}

char* __fastcall ICQTranslateUtf(const char *src)
{ // this takes UTF-8 strings only!!!
	char *szRes = NULL;

	if (!strlennull(src))
	{ // for the case of empty strings
		return null_strdup(src);
	}

	{ // we can use unicode translate (0.5+)
		WCHAR* usrc = make_unicode_string(src);

		szRes = make_utf8_string(TranslateW(usrc));

		SAFE_FREE((void**)&usrc);
	}
	return szRes;
}

char* __fastcall ICQTranslateUtfStatic(const char *src, char *buf, size_t bufsize)
{ // this takes UTF-8 strings only!!!
	if (strlennull(src))
	{ // we can use unicode translate (0.5+)
		WCHAR *usrc = make_unicode_string(src);

		make_utf8_string_static(TranslateW(usrc), buf, bufsize);

		SAFE_FREE((void**)&usrc);
	}
	else
		buf[0] = '\0';

	return buf;
}

void CIcqProto::ForkThread( IcqThreadFunc pFunc, void* arg )
{
	CloseHandle(( HANDLE )mir_forkthreadowner(( pThreadFuncOwner )*( void** )&pFunc, this, arg, NULL ));
}

HANDLE CIcqProto::ForkThreadEx( IcqThreadFunc pFunc, void* arg, UINT* threadID )
{
	return ( HANDLE )mir_forkthreadowner(( pThreadFuncOwner )*( void** )&pFunc, this, arg, threadID );
}

char* CIcqProto::GetUserPassword(BOOL bAlways)
{
	if (m_szPassword[0] != '\0' && (m_bRememberPwd || bAlways))
		return m_szPassword;

	if (!getSettingStringStatic(NULL, "Password", m_szPassword, sizeof(m_szPassword)))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlennull(m_szPassword) + 1, (LPARAM)m_szPassword);

		if (!strlennull(m_szPassword)) return NULL;

		m_bRememberPwd = TRUE;

		return m_szPassword;
	}

	return NULL;
}

WORD CIcqProto::GetMyStatusFlags()
{
	WORD wFlags = 0;

	// Webaware setting bit flag
	if (getSettingByte(NULL, "WebAware", 0))
		wFlags = STATUS_WEBAWARE;

	// DC setting bit flag
	switch (getSettingByte(NULL, "DCType", 0))
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
	if (strstrnull(filename, "..\\") || strstrnull(filename, "../") ||
		strstrnull(filename, ":\\") || strstrnull(filename, ":/") ||
		filename[0] == '\\' || filename[0] == '/')
		return 0; // Contains malicious chars, Failure

	return 1; // Success
}

char *ExtractFileName(const char *fullname)
{
	char *szFileName;

	 // already is only filename
	if (((szFileName = strrchr((char*)fullname, '\\')) == NULL) && ((szFileName = strrchr((char*)fullname, '/')) == NULL))
		return (char*)fullname;

	return szFileName+1;  // skip backslash
}

char *FileNameToUtf(const char *filename)
{
	#if defined( _UNICODE )
		// reasonable only on NT systems
		HINSTANCE hKernel;
		DWORD (CALLBACK *RealGetLongPathName)(LPCWSTR, LPWSTR, DWORD);

		hKernel = GetModuleHandleA("KERNEL32");
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
	#endif

	return ansi_to_utf8(filename);
}

int FileStatUtf(const char *path, struct _stati64 *buffer)
{
	int wRes = -1;

	#if defined( _UNICODE )
		WCHAR* usPath = make_unicode_string(path);
		wRes = _wstati64(usPath, buffer);
		SAFE_FREE((void**)&usPath);
	#else
		int size = strlennull(path)+2;
		char* szAnsiPath = (char*)_alloca(size);
		if (utf8_decode_static(path, szAnsiPath, size))
			wRes = _stati64(szAnsiPath, buffer);
	#endif

	return wRes;
}

int MakeDirUtf(const char *dir)
{
	int wRes = -1;
	char *szLast;

	#if defined( _UNICODE )
		WCHAR* usDir = make_unicode_string(dir);
		// _wmkdir can created only one dir at once
		wRes = _wmkdir(usDir);
		// check if dir not already existed - return success if yes
		if (wRes == -1 && errno == 17 /* EEXIST */)
			wRes = 0;
		else if (wRes && errno == 2 /* ENOENT */)
		{ // failed, try one directory less first
			szLast = (char*)strrchr((char*)dir, '\\');
			if (!szLast) szLast = (char*)strrchr((char*)dir, '/');
			if (szLast)
			{
				char cOld = *szLast;

				*szLast = '\0';
				if (!MakeDirUtf(dir))
					wRes = _wmkdir(usDir);
				*szLast = cOld;
			}
		}
		SAFE_FREE((void**)&usDir);
	#else
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
				szLast = (char*)strrchr((char*)dir, '\\');
				if (!szLast) szLast = (char*)strrchr((char*)dir, '/');
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
	#endif

	return wRes;
}

int OpenFileUtf(const char *filename, int oflag, int pmode)
{
	int hFile = -1;

	#if defined( _UNICODE )
		WCHAR* usFile = make_unicode_string(filename);
		hFile = _wopen(usFile, oflag, pmode);
		SAFE_FREE((void**)&usFile);
	#else
		int size = strlennull(filename)+2;
		char* szAnsiFile = (char*)_alloca(size);

		if (utf8_decode_static(filename, szAnsiFile, size))
			hFile = _open(szAnsiFile, oflag, pmode); 
	#endif

	return hFile;
}

WCHAR *GetWindowTextUcs(HWND hWnd)
{
	WCHAR *utext;

	#if defined( _UNICODE )
		int nLen = GetWindowTextLengthW(hWnd);

		utext = (WCHAR*)SAFE_MALLOC((nLen+2)*sizeof(WCHAR));
		GetWindowTextW(hWnd, utext, nLen + 1);
	#else
		char *text;
		int wchars, nLen = GetWindowTextLengthA(hWnd);

		text = (char*)_alloca(nLen+2);
		GetWindowTextA(hWnd, text, nLen + 1);

		wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, text,
			strlennull(text), NULL, 0);

		utext = (WCHAR*)SAFE_MALLOC((wchars + 1)*sizeof(WCHAR));

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, text,
			strlennull(text), utext, wchars);
	#endif
	return utext;
}

void SetWindowTextUcs(HWND hWnd, WCHAR *text)
{
	#if defined( _UNICODE )
		SetWindowTextW(hWnd, text);
	#else
		char *tmp = (char*)SAFE_MALLOC(strlennull(text) + 1);
		WideCharToMultiByte(CP_ACP, 0, text, -1, tmp, strlennull(text)+1, NULL, NULL);
		SetWindowTextA(hWnd, tmp);
		SAFE_FREE((void**)&tmp);
	#endif
}

char *GetWindowTextUtf(HWND hWnd)
{
	TCHAR* szText;

	#if defined( _UNICODE )
		int nLen = GetWindowTextLengthW(hWnd);

		szText = (TCHAR*)_alloca((nLen+2)*sizeof(WCHAR));
		GetWindowTextW(hWnd, (WCHAR*)szText, nLen + 1);
	#else
		int nLen = GetWindowTextLengthA(hWnd);

		szText = (TCHAR*)_alloca(nLen+2);
		GetWindowTextA(hWnd, (char*)szText, nLen + 1);
	#endif

	return tchar_to_utf8(szText);
}

char *GetDlgItemTextUtf(HWND hwndDlg, int iItem)
{
	return GetWindowTextUtf(GetDlgItem(hwndDlg, iItem));
}

void SetWindowTextUtf(HWND hWnd, const char *szText)
{
	#if defined( _UNICODE )
		WCHAR* usText = make_unicode_string(szText);
		SetWindowTextW(hWnd, usText);
		SAFE_FREE((void**)&usText);
	#else
		int size = strlennull(szText)+2;
		char* szAnsi = (char*)_alloca(size);

		if (utf8_decode_static(szText, szAnsi, size))
			SetWindowTextA(hWnd, szAnsi);
	#endif
}

void SetDlgItemTextUtf(HWND hwndDlg, int iItem, const char *szText)
{
	SetWindowTextUtf(GetDlgItem(hwndDlg, iItem), szText);
}

static int ControlAddStringUtf(HWND ctrl, DWORD msg, const char *szString)
{
	char str[MAX_PATH];
	char *szItem = ICQTranslateUtfStatic(szString, str, MAX_PATH);
	int item = -1;

	#if defined( _UNICODE )
		WCHAR *wItem = make_unicode_string(szItem);
		item = SendMessage(ctrl, msg, 0, (LPARAM)wItem);
		SAFE_FREE((void**)&wItem);
	#else
		int size = strlennull(szItem) + 2;
		char *aItem = (char*)_alloca(size);

		if (utf8_decode_static(szItem, aItem, size))
			item = SendMessage(ctrl, msg, 0, (LPARAM)aItem);
	#endif

	return item;
}

int ComboBoxAddStringUtf(HWND hCombo, const char *szString, DWORD data)
{
	int item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
	SendMessage(hCombo, CB_SETITEMDATA, item, data);

	return item;
}

int ListBoxAddStringUtf(HWND hList, const char *szString)
{
	return ControlAddStringUtf(hList, LB_ADDSTRING, szString);
}

int MessageBoxUtf(HWND hWnd, const char *szText, const char *szCaption, UINT uType)
{
	int res;
	char str[1024];
	char cap[MAX_PATH];

	#if defined( _UNICODE )
		WCHAR *text = make_unicode_string(ICQTranslateUtfStatic(szText, str, 1024));
		WCHAR *caption = make_unicode_string(ICQTranslateUtfStatic(szCaption, cap, MAX_PATH));
		res = MessageBoxW(hWnd, text, caption, uType);
		SAFE_FREE((void**)&caption);
		SAFE_FREE((void**)&text);
	#else
		int size = strlennull(szText) + 2, size2 = strlennull(szCaption) + 2;
		char *text = (char*)_alloca(size);
		char *caption = (char*)_alloca(size2);

		utf8_decode_static(ICQTranslateUtfStatic(szText, str, 1024), text, size);
		utf8_decode_static(ICQTranslateUtfStatic(szCaption, cap, MAX_PATH), caption, size2);
		res = MessageBoxA(hWnd, text, caption, uType);
	#endif

	return res;
}

char* CIcqProto::ConvertMsgToUserSpecificAnsi(HANDLE hContact, const char* szMsg)
{ // this takes utf-8 encoded message
	WORD wCP = getSettingWord(hContact, "CodePage", m_wAnsiCodepage);
	char* szAnsi = NULL;

	if (wCP != CP_ACP) // convert to proper codepage
		if (!utf8_decode_codepage(szMsg, &szAnsi, wCP))
			return NULL;

	return szAnsi;
}

// just broadcast generic send error with dummy cookie and return that cookie
DWORD CIcqProto::ReportGenericSendError(HANDLE hContact, int nType, const char* szErrorMsg)
{ 
	DWORD dwCookie = GenerateCookie(0);
	SendProtoAck(hContact, dwCookie, ACKRESULT_FAILED, nType, ICQTranslate(szErrorMsg));
	return dwCookie;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::CreateProtoService(const char* szService, IcqServiceFunc serviceProc)
{
	char temp[MAX_PATH*2];

	null_snprintf(temp, sizeof(temp), "%s%s", m_szModuleName, szService);
	CreateServiceFunctionObj( temp, ( MIRANDASERVICEOBJ )*( void** )&serviceProc, this );
}

void CIcqProto::CreateProtoServiceParam(const char* szService, IcqServiceFuncParam serviceProc, LPARAM lParam)
{
	char temp[MAX_PATH*2];

	null_snprintf(temp, sizeof(temp), "%s%s", m_szModuleName, szService);
	CreateServiceFunctionObjParam( temp, ( MIRANDASERVICEOBJPARAM )*( void** )&serviceProc, this, lParam );
}


HANDLE CIcqProto::HookProtoEvent(const char* szEvent, IcqEventFunc pFunc)
{
  return ::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*(void**)&pFunc, this);
}


HANDLE CIcqProto::CreateProtoEvent(const char* szEvent)
{
	char str[MAX_PATH + 32];
	strcpy(str, m_szModuleName);
	strcat(str, szEvent);
	return CreateHookableEvent(str);
}

//////////////////////////////////////////////////////////////////////////////
// ICQ PLUS MOD SECTION //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

WORD CIcqProto::GetProtoVersion()
{
	if (m_bCustomProtoVersionEnabled)
		return getSettingWord(NULL, "ProtoVersion", ICQ_VERSION);
	else
	{
		int ver = 0;
		switch (getSettingWord(NULL, "ClientID", 0))
		{
		case 1:           // Unknown
			ver = 66;
			break;
		case 23:
		case 24:
		case 2:           // QiP
		case 3:           // ysm
		case 7:           // Trillian
		case 46:          // QiP Infium
			ver = 11;
			break;
		case 6:           // Jimm
			ver = 5;
			break;
		case 9:           // Kopete
		case 52:          // NanoICQ
			ver = 10;
			break;
		case 11:
			ver = 13;
			break;
		case 4:           // ICQ Lite
		case 5:           // &RQ
		case 12:          // ICQ 5.1 Rambler
		case 13:          // ICQ 5.1
		case 14:
		case 15:
		case 16:
		case 17:
		case 26:          // ICQ 6
		case 44:          // QNEXT
		case 45:          // pyICQ
			ver = 9;
			break;
		case 21:          // stICQ
			ver = 2;
			break;
		case 36:          // ICQ99   
			ver = 6;
			break;
		case 37:          // WebICQ
			ver = 7;
			break;
		case 35:          // GAIM
			ver = 0;
			break;           
		default :         // Miranda IM
			ver = ICQ_VERSION;
			break;
		}
		setSettingWord(NULL, "ProtoVersion", (WORD)ver);
		return ver;
	}
}

int CIcqProto::GetFromContactsCacheByID(int ID, HANDLE *hContact, DWORD *dwUin)
{
	DWORD uin;

	if (ID < 0 || ID >= contactsCache.getCount())
		return 0;

	EnterCriticalSection(&contactsCacheMutex);

	icq_contacts_cache* cc = contactsCache[ID];

	if (hContact)
		*hContact = cc->hContact;

	uin = cc->dwUin;

	if (dwUin)
		*dwUin = uin;

	LeaveCriticalSection(&contactsCacheMutex);

	return uin; // to be sure it is not 0
}

void CIcqProto::NickFromHandleStatic(HANDLE hContact, char *szNick, WORD wLen)
{
	if (!wLen || !szNick) return;

	if (hContact == INVALID_HANDLE_VALUE)
		lstrcpynA(szNick, "<invalid>", wLen);
	else
		lstrcpynA(szNick, (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0), wLen);
}

void CIcqProto::makeUserOffline(HANDLE hContact)
{

	WORD w;
	DWORD dw;

	if (getSettingWord(hContact, "Status", 0) != ID_STATUS_OFFLINE)
		setSettingWord(hContact, "Status", ID_STATUS_OFFLINE);

	if (dw = getSettingDword(hContact, "LogonTS", 0)) {
		setSettingDword(hContact, "LogonTS", 0);
		setSettingDword(hContact, "OldLogonTS", dw);
	}

	if (dw = getSettingDword(hContact, "IdleTS", 0)) {
		setSettingDword(hContact, "IdleTS", 0);
		setSettingDword(hContact, "OldIdleTS", dw);
	}

	if (dw = getSettingDword(hContact, "RealIP", 0)) {
		setSettingDword(hContact, "RealIP", 0);
		setSettingDword(hContact, "OldRealIP", dw);
	}

	if (dw = getSettingDword(hContact, "IP", 0)) {
		setSettingDword(hContact, "IP", 0);
		setSettingDword(hContact, "OldIP", dw);
	}

	if (w = getSettingWord(hContact, "UserPort", 0)) {
		setSettingWord(hContact, "UserPort", 0);
		setSettingWord(hContact, "OldUserPort", w);
	}

	if (w = getSettingWord(hContact, "Version", 0)) {
		setSettingWord(hContact, "Version", 0);
		setSettingWord(hContact, "OldVersion", w);
	}
}

int CIcqProto::CacheIDCount()
{
	return contactsCache.getCount();
}

int CIcqProto::IDFromContactsCacheByUin(DWORD dwUin)
{
	int i = -1;

	if (contactsCache.getCount() == 0)
		return i;

	EnterCriticalSection(&contactsCacheMutex);

	for (i = contactsCache.getCount() - 1; i >= 0; i--)
	{
		icq_contacts_cache* cc;
		cc = contactsCache[i];
		if (cc->dwUin == dwUin)
			break;
	}
	LeaveCriticalSection(&contactsCacheMutex);
	return i;
}

int CIcqProto::SetContactsCacheFlagsByID(int ID, DWORD flags)
{
	if (ID < 0 || ID >= contactsCache.getCount())
		return 0;

	icq_contacts_cache* cc = contactsCache[ID];
		cc->flags = flags;

	return 1;
}

int CIcqProto::GetContactsCacheByID(int ID, icq_contacts_cache *icc)
{
	if (!icc || ID < 0 || ID >= contactsCache.getCount())
		return 0;

	EnterCriticalSection(&contactsCacheMutex);

  icq_contacts_cache* cc = contactsCache[ID];
	icc->hContact = cc->hContact;
	icc->dwUin = cc->dwUin;
	icc->flags = cc->flags;

	LeaveCriticalSection(&contactsCacheMutex);

	return icc->dwUin; // to be sure it is not 0
}

BOOL CIcqProto::InvisibleForContact( DWORD dwUin, HANDLE hContact )
{/*
	WORD wApparent;
	if (hContact == NULL && !dwUin)
	  return FALSE;

	// Block all users from seeing you
	if ( m_bVisibility == 0x02 ) //we support AIM contacts which don't use UINs
	{
		NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
		return TRUE;
	}

	// Allow all users to see you
	if ( m_bVisibility == 0x01 )
		return FALSE;

	if ( !hContact ) {
		hContact = HContactFromUIN( dwUin, 0 );
		if ( !hContact )
		{
			NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
			return TRUE;
		}
	}

	if ( DBGetContactSettingByte( hContact, "CList", "NotOnList", 0 ) )
	{
		NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
		return TRUE;
	}

	// Allow only users in the buddy list to see you
	if ( m_bVisibility == 0x05 ) {
		if ( DBGetContactSettingByte( hContact, "CList", "Hidden", 0 ) )
		{
      NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
			return TRUE;
		}
		else
			return FALSE;
	}

	wApparent = getSettingWord( hContact, "ApparentMode", 0 );

	// Allow only users in the permit list to see you
	if ( m_bVisibility == 0x03 ) {
		if ( wApparent == ID_STATUS_ONLINE )
			return FALSE;
		else
		{
			NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
			return TRUE;
		}
	}

	// Block only users in the invisible list from seeing you
	if ( m_bVisibility == 0x04 ) {
		if ( wApparent == ID_STATUS_OFFLINE )
		{
			NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
			return TRUE;
		}
		else
			return FALSE;
	}

	// Invisible by default
	NetLog_Server( "We are invisible for %u, blocking all unsafe requests", dwUin );
	return TRUE;*/ return FALSE;
}

void CIcqProto::CheckContact(CHECKCONTACT chk)
{/* // TODO: Implement contact status checking
	BYTE Hidden = DBGetContactSettingByte( chk.hContact, "CList", "Hidden", 0 );
	BOOL Ignored = 0;
	BYTE InDb = CallService( MS_DB_CONTACT_IS, ( WPARAM )chk.hContact, 0 );
	BYTE NotOnList = DBGetContactSettingByte( chk.hContact, "CList", "NotOnList", 0 );
	BOOL Cancel = FALSE;
	BOOL showpopup = TRUE;
	char popmsg[512] = { 0 }, eventmsghistory[512] = { 0 }, eventmsgfile[512] = { 0 };
	if ( chk.hContact == NULL )
	  return;
	if ( DBGetContactSettingDword( chk.hContact, "Ignore", "Mask1", 0 ) == 0x0000007F )
	  Ignored = TRUE;
	if ( !InDb || NotOnList )
	{
		if ( !m_bPopUpsForNotOnList )
			showpopup = FALSE;
		if( m_bNoPSDForHidden )
			Cancel = TRUE;
	}
	if ( Hidden && !m_bPopUpsForHidden )
	  showpopup = FALSE;
	if ( Ignored && !m_bPopUpsForIgnored )
	  showpopup = FALSE;
	if ( !chk.popuptype )
		chk.popup = FALSE;
	if ( !chk.icqeventtype )
	{ // wrong call check - missing event types
		chk.historyevent=FALSE;
		chk.logtofile=FALSE;
	}
	if ( chk.dwUin )
	{
		if ( m_bTmpContacts == 1 )
		{
			int added;
			chk.hContact = HContactFromUIN( chk.dwUin, &added );
			if ( NotOnList )
			{
				{
					DBCONTACTWRITESETTING dbcws;
					dbcws.value.type = DBVT_TCHAR;
					dbcws.value.ptszVal = m_TmpGroupName;
					dbcws.szModule = "CList";
					dbcws.szSetting = "Group";
					CallService( MS_DB_CONTACT_WRITESETTING, ( WPARAM )chk.hContact, ( LPARAM )&dbcws );
				}	
				DBWriteContactSettingByte( chk.hContact, "CList", "Hidden" ,0 );
				DBWriteContactSettingByte( chk.hContact, "CList", "NotOnList", m_bAddTemp );
				setSettingByte( chk.hContact, "CheckSelfRemove", 1 ); // excluding from checkselfremove				
				setSettingByte( chk.hContact, "FoundedContact", 1 ); // mark found contacts
				ShowPopUpMsg( chk.hContact, NickFromHandleUtf( chk.hContact ), Translate( "Added to temporary group" ), LOG_NOTE ); // temporary solution
			}
		}
		if ( InDb )
		{
			if ( chk.msg )

			{
				if ( strlen( chk.msg ) > 1 && showpopup )
				{
					popmsg[0] = '\0';
					switch ( chk.popuptype )
					{
					case POPTYPE_SPAM:				
						// nothing here, used direct ShowPopupMsg call
						break;
					case POPTYPE_UNKNOWN:
						// nothing here, used direct ShowPopupMsg call
						break;
					case POPTYPE_VISIBILITY:
						if ( m_bVisPopUp )
							strcpy( popmsg, "... is checking your real status" );
						break;
					case LOG_NOTE:
					case LOG_WARNING:
					case LOG_ERROR:
					case LOG_FATAL:
						strcpy( popmsg, chk.msg );
						break;
					case POPTYPE_FOUND:
						if ( m_bFoundPopUp )
							strcpy( popmsg, chk.msg );
						break;
					case POPTYPE_SCAN:
						if ( m_bScanPopUp )
							strcpy(popmsg,chk.msg);
						break;
					case POPTYPE_CLIENTCHANGE:
						if ( m_bClientChangePopUp )
							strcpy( popmsg, chk.msg );
						break;
					case POPTYPE_INFOREQUEST:
						if ( m_bInfoRequestPopUp )
							strcpy( popmsg, chk.msg );
						break;
					case POPTYPE_IGNORECHECK:
						if ( m_bIgnoreCheckPop )
							strcpy( popmsg, chk.msg );
						break;
					case POPTYPE_SELFREMOVE:
						if ( m_bPopSelfRem )
							strcpy( popmsg, chk.msg );
						break;
					case POPTYPE_AUTH:
						if( m_bAuthPopUp )
							strcpy( popmsg, chk.msg );
						break;
					default:
						break;
					}
					if( chk.historyevent || chk.logtofile )
					{
						switch ( chk.icqeventtype )
						{
						case ICQEVENTTYPE_AUTH_DENIED:
							if ( m_bLogAuthHistory )
								strcpy( eventmsghistory, "Authorization request denied" );
							if ( m_bLogAuthFile )
								strcpy( eventmsgfile, "Authorization request denied" );
							break;
						case ICQEVENTTYPE_AUTH_GRANTED:
							if ( m_bLogAuthHistory )
								strcpy( eventmsghistory, "Authorization request granted" );
							if ( m_bLogAuthFile )
								strcpy( eventmsgfile, "Authorization request granted" );
							break;
						case ICQEVENTTYPE_CHECK_STATUS:
							if ( m_bLogStatusCheckHistory )
								strcpy( eventmsghistory, "checks your status" );
							if ( m_bLogStatusCheckFile )
								strcpy( eventmsgfile, "checks your status" );
							break;
						case ICQEVENTTYPE_WAS_FOUND:
							if ( m_bLogASDHistory )
								strcpy( eventmsghistory, "" );
							if ( m_bLogASDFile )
								strcpy( eventmsgfile, "" );
							break;
						case ICQEVENTTYPE_CLIENT_CHANGE:
							if ( m_bLogIgnoreCheckHistory )
								strcpy( eventmsghistory, chk.msg );
							if( m_bLogIgnoreCheckFile )
								strcpy( eventmsgfile, chk.msg );
							break;
						case ICQEVENTTYPE_EMAILEXPRESS:
							// nothing to do here at this time...
							break;
						case ICQEVENTTYPE_FUTURE_AUTH:
							// i do not know what can i do here, fix me...
							break;
						case ICQEVENTTYPE_IGNORECHECK_STATUS:
							if ( m_bLogIgnoreCheckHistory )
								strcpy( eventmsghistory, "checks your Ignore list" );
							if ( m_bLogIgnoreCheckFile )
								strcpy( eventmsgfile, "checks your Ignore list" );
							break;
						case ICQEVENTTYPE_SELF_REMOVE:
							if ( m_bLogSelfRemoveHistory )
								strcpy( eventmsghistory, "removed himself from your server-list!" );
							if ( m_bLogSelfRemoveFile )
								strcpy( eventmsgfile, "removed himself from your server-list!" );
							break;
						case ICQEVENTTYPE_SMS:
							// nothing to do here at this time...
							break;
						case ICQEVENTTYPE_WEBPAGER:
							// nothing to do here at this time...

							break;
						case ICQEVENTTYPE_YOU_ADDED:

							break;
						case ICQEVENTTYPE_AUTH_REQUESTED:
							break;
						default:
							break;
						}
						if ( popmsg && strlen( popmsg ) > 1 )
						{
							if ( !strstr( chk.msg, popmsg ) )
								strcat( popmsg, chk.msg );
							ShowPopUpMsg( chk.hContact, NickFromHandleUtf( chk.hContact ), popmsg, chk.popuptype );
						}
						if ( eventmsghistory && strlen( eventmsghistory ) > 1 )
						{
							if ( !strstr( chk.msg, eventmsghistory ) )
								strcat( eventmsghistory, chk.msg );
							HistoryLog( chk.hContact, chk.dwUin, eventmsghistory, chk.icqeventtype, chk.dbeventflag );
						}
						if ( eventmsgfile && strlen( eventmsgfile ) > 1 )
						{
							if ( !strstr( chk.msg, eventmsgfile ) )
								strcat( eventmsgfile, chk.msg );
							LogToFile( chk.hContact, chk.dwUin, eventmsgfile, chk.icqeventtype );
						}
					}
				}
			}
			if ( chk.PSD != -1 && !Cancel )
			{
				if ( !chk.PSD )
					chk.PSD = 21;
				if ( m_bASD && chk.PSD < 10 )
					icq_SetUserStatus( chk.dwUin, 0, chk.PSD, 0 );
				else if ( getSettingWord( chk.hContact, "Status", ID_STATUS_OFFLINE ) == ID_STATUS_OFFLINE && ( m_bPSD && chk.PSD > 20 ) ) // check for wrong PSD call
					icq_SetUserStatus( chk.dwUin, 0, chk.PSD, 0 );
			}
		}
	}
*/}

int CIcqProto::IncognitoAwayRequest(WPARAM wParam, LPARAM lParam)
{
	m_bIncognitoRequest = TRUE;

	CallService(MS_AWAYMSG_SHOWAWAYMSG, wParam, 0);
	return 0;
}

void CIcqProto::HistoryLog(HANDLE hContact, DWORD dwUin, char *data, int event_type, int flag)
{
	PBYTE pCurBlob;
	WORD wTextLen;
	char szText[MAX_PATH];
	DBEVENTINFO Event = {0};
	Event.cbSize = sizeof(Event);
	Event.szModule = m_szModuleName;
	Event.eventType = event_type;
	Event.flags = flag, DBEF_UTF;
	Event.timestamp = (DWORD)time(NULL);
	mir_snprintf(szText, sizeof(szText), "%s %s ( %s: %s, %s: (%u) )", Translate(data), Translate("by"), Translate("Nick"), (NickFromHandle(hContact)), Translate("UIN"), dwUin);
	wTextLen = strlen(szText);
	Event.cbBlob = sizeof(DWORD) + sizeof(HANDLE) + wTextLen + 1;
	pCurBlob = Event.pBlob = (PBYTE)_alloca(Event.cbBlob);
	memcpy(pCurBlob, &szText, wTextLen);
	pCurBlob += wTextLen;
	*(char*)pCurBlob = 0; pCurBlob++;
	memcpy(pCurBlob, &dwUin, sizeof(DWORD)); pCurBlob += sizeof(DWORD);
	memcpy(pCurBlob, &hContact, sizeof(HANDLE));
	if (m_bLogToContactsHistory)
		CallService(MS_DB_EVENT_ADD, (WPARAM)(HANDLE)hContact,(LPARAM)&Event);
	else
		CallService(MS_DB_EVENT_ADD, (WPARAM)(HANDLE)NULL, (LPARAM)&Event);
	//we need show popups for events from here
}

void CIcqProto::LogToFile(HANDLE hContact, DWORD dwUin, char *string, int event_type)
{
	char szTime[30];
	char content[MAX_PATH];
	char filename[1024];
	time_t now;
	FILE *f;
	DBVARIANT dbv = { 0 };

	getSettingString( NULL, "EventsLogFile", &dbv );
	if (dbv.pszVal && strlennull(dbv.pszVal) > 0)
		wsprintfA(filename, dbv.pszVal);
	else
		strcpy(filename, "EventsLog.txt");

	switch (event_type)
	{
	case ICQEVENTTYPE_AUTHGRANTED:
		if (!m_bLogAuthFile)
			return;
		mir_snprintf(content, sizeof(content), "%s %s", Translate("granted your authorization request "), Translate(string));
		break;
	case ICQEVENTTYPE_AUTHDENIED:
		if(!m_bLogAuthFile)
			return;
		mir_snprintf(content, sizeof(content), "%s %s", Translate("denied your authorization request "), Translate(string));
		break;
	case ICQEVENTTYPE_SELFREMOVE:
		if (!m_bLogSelfRemoveFile)
			return;
		mir_snprintf(content, sizeof(content), "%s %s", Translate("removed himself from your serverlist "), Translate(string));
		break;
	case ICQEVENTTYPE_FUTUREAUTH:
		if (!m_bLogAuthFile)
			return;
		mir_snprintf(content, sizeof(content), "%s %s", Translate("granted you future authorization "), Translate(string));
		break;
	case ICQEVENTTYPE_CLIENTCHANGE:
		if (!m_bLogClientChangeFile)
			return;
		utf8_decode_static(string, content, strlen(string));
		break;

	case ICQEVENTTYPE_CHECKSTATUS:
		if (!m_bLogStatusCheckFile)
			return;
		mir_snprintf(content, sizeof(content), "%s %s", Translate("checked your real status "), Translate(string));
		break;
	case ICQEVENTTYPE_WASFOUND:
		if (!m_bLogASDFile)
			return;
		mir_snprintf(content, sizeof(content), "%s", Translate(string));
		break;
	case ICQEVENTTYPE_IGNORECHECKSTATUS:
		if (!m_bLogIgnoreCheckFile)
			return;
		mir_snprintf(content, sizeof(content), "%s %s", Translate("checked his ignore state"), Translate(string));
		break;

	default:
		wsprintfA(content, "unknown eventype processed");
		return;
	}

	now = time(NULL);
	strftime(szTime, sizeof(szTime), "%a, %d %b %Y %H:%M:%S", localtime(&now));
                                         /* Sun, 00 Jan 0000 00:00:00 */
	f = fopen( filename, "a+" );
	if (f != NULL)
	{
		fprintf(f, "[%s] %s (%u) %s\n", szTime, NickFromHandle(hContact), dwUin, content); 
		fclose(f);
	}
}

void CIcqProto::SetDwFT(DWORD *dwFT, char* DbValue, DWORD DwValue)
{
	*dwFT = DwValue;
	setSettingDword(NULL, DbValue, DwValue);
}

void CIcqProto::SetTimeStamps(DWORD *dwFT1, DWORD *dwFT2, DWORD *dwFT3)
{
	switch (getSettingByte(NULL, "CurrentID", 0)) // dwFTs for current Client ID
	{
	case 3: // ysm
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFFAB);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 4: // ICQ lite
		SetDwFT(dwFT1, "dwFT1", 0x3AA773EE);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 5: // &RQ
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFF7F);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 7: // trillian
		SetDwFT(dwFT1, "dwFT1", 0x3B75AC09);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 8: // licq
		SetDwFT(dwFT1, "dwFT1", 0x2C0BA3DD);
		SetDwFT(dwFT2, "dwFT2", 0x7D800403);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 2: // qip
		SetDwFT(dwFT1, "dwFT1", 0x08000300);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 46: // QIP Infium
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
	case 1:  // unknown
	case 6:  // Jimm
	case 9:  // Kopete
	case 10: // icq for mac
	case 12: // rambler
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
	case 40: // uIM
	case 41: // TICQClient
	case 42: // IC@
	case 43: // PreludeICQ
	case 44: // QNEXT
	case 45: // pyICQ
	case 47: // JICQ
	case 49: // MIP
	case 50: // Trillian Astra
	case 52: // NanoICQ
	case 53: // IMadering
		SetDwFT(dwFT1, "dwFT1", 0x00000000);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 29:
		SetDwFT(dwFT1, "dwFT1", 0x44F523B0);
		SetDwFT(dwFT2, "dwFT2", 0x44F523A6);
		SetDwFT(dwFT3, "dwFT3", 0x44F523A6);
		break;
	case 30: // alicq
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFFBE);
		SetDwFT(dwFT2, "dwFT2", 0x00090800);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 31: // mICQ
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFF42);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 32: // StrICQ 0.4
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFF8F);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 33: // vICQ 0.43.0.0
		SetDwFT(dwFT1, "dwFT1", 0x04031980);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 34: // IM2
		SetDwFT(dwFT1, "dwFT1", 0x3FF19BEB);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x3FF19BEB);
		break;
	case 35: // GAIM
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFFFF);
		SetDwFT(dwFT2, "dwFT2", 0xFFFFFFFF);
		SetDwFT(dwFT3, "dwFT3", 0xFFFFFFFF);
		break;
	case 36: // ICQ99
		SetDwFT(dwFT1, "dwFT1", 0x3AA773EE);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 37: // WebICQ
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFFFF);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 38: // SmartICQ
		SetDwFT(dwFT1, "dwFT1", 0xDDDDEEFF);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 39: // IM+
		SetDwFT(dwFT1, "dwFT1", 0x494D2B00);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 48: // SpamBot
		SetDwFT(dwFT1, "dwFT1", 0xFFFFFFFF);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x3B7248ED);
		break;
	case 51: // R&Q
		SetDwFT(dwFT1, "dwFT1", 0xFFFFF666);
		SetDwFT(dwFT2, "dwFT2", 0x00000000);
		SetDwFT(dwFT3, "dwFT3", 0x00000000);
		break;
	case 11:
		SetDwFT(dwFT1, "dwFT1", gbUnicodeCore ? 0x7FFFFFFF : 0xFFFFFFFF);
		SetDwFT(dwFT2, "dwFT2", 0x06060600);
		SetDwFT(dwFT3, "dwFT3", m_bSecureIM ? 0x5AFEC0DE : 0x00000000);
		break;
	default : // Miranda
		SetDwFT(dwFT1, "dwFT1", gbUnicodeCore ? 0x7FFFFFFF : 0xFFFFFFFF);
		SetDwFT(dwFT2, "dwFT2", ICQ_PLUG_VERSION);
		SetDwFT(dwFT3, "dwFT3", m_bSecureIM ? 0x5AFEC0DE : 0x00000000);
		break;
	}
}
