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
// File name      : $URL: https://miranda.svn.sourceforge.net/svnroot/miranda/trunk/miranda/protocols/IcqOscarJ/utilities.h $
// Revision       : $Revision: 8822 $
// Last change on : $Date: 2009-01-11 18:17:05 +0100 (Sun, 11 Jan 2009) $
// Last change by : $Author: jokusoftware $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#ifndef __UTILITIES_H
#define __UTILITIES_H

struct icq_ack_args
{
	HANDLE hContact;
	int    nAckType;
	int    nAckResult;
	HANDLE hSequence;
	LPARAM pszMessage;
};

struct icq_contacts_cache
{
	HANDLE hContact;
	DWORD dwUin;
	const char *szUid;
	DWORD flags;
};

typedef struct icq_check_contact {
	DWORD  dwUin;          // Contact UIN
	HANDLE hContact;       // Contact handle
	BOOL   bShowPopup;     // Whether show Popup or not
	int    iPopupEvent;    // Popup Event type
	BOOL   bHistoryEvent;  // History Event type
	BOOL   bLogToFile;     // Whether log event to file
	char*  pszMsg;         // Event message
	int    iICQEventType;  // ICQ Event type
	int    iDBEventFlag;   // DB Event flags

	BOOL   bNotTmpContact; // Is contact temporary?
	BOOL   bNotOnList;     // Is contact on list?
	BOOL   bOffline;       // Is contact offline?
	BOOL   bInvisibleFor;  // Are we invisible for this contact?
	int    iPSD;           // Value for calling SetUserStatus (-1 to disable)

	//icq_check_contact() {}

	//icq_check_contact(
	//    DWORD u, HANDLE h, BOOL p, int pe, BOOL he, BOOL ltf, char* m,
	//    int dbet, int dbef, BOOL ntc, BOOL nol, BOOL o, BOOL inf, int ps)
	//{
	//	dwUin = u;             hContact = h;
	//	bShowPopup = p;        iPopupEvent = pe;
	//	bHistoryEvent = he;    bLogToFile = ltf;
	//	pszMsg = m;            iDBEventType = dbet;
	//	iDBEventFlag = dbef;   bNotTmpContact = ntc;
	//	bNotOnList = nol;      bOffline = o;
	//	bInvisibleFor = inf;   iPSD = ps;
	//}
} CHECKCONTACT;


/*---------* Functions *---------------*/

void EnableDlgItem(HWND hwndDlg, UINT control, int state);
void icq_EnableMultipleControls(HWND hwndDlg, const UINT* controls, int cControls, int state);
void icq_ShowMultipleControls(HWND hwndDlg, const UINT* controls, int cControls, int state);
int IcqStatusToMiranda(WORD wStatus);
WORD MirandaStatusToIcq(int nStatus);
int MirandaStatusToSupported(int nMirandaStatus);
char *MirandaStatusToString(int);
char *MirandaStatusToStringUtf(int);

int AwayMsgTypeToStatus(int nMsgType);

void   SetGatewayIndex(HANDLE hConn, DWORD dwIndex);
DWORD  GetGatewayIndex(HANDLE hConn);
void   FreeGatewayIndex(HANDLE hConn);

char *NickFromHandle(HANDLE hContact);
char *NickFromHandleUtf(HANDLE hContact);
char *strUID(DWORD dwUIN, char *pszUID);

size_t __fastcall strlennull(const char *string);
int __fastcall strcmpnull(const char *str1, const char *str2);
int __fastcall stricmpnull(const char *str1, const char *str2);
char* __fastcall strstrnull(const char *str, const char *substr);
int null_snprintf(char *buffer, size_t count, const char *fmt, ...);
char* __fastcall null_strdup(const char *string);
size_t __fastcall null_strcut(char *string, size_t maxlen);

size_t __fastcall strlennull(const WCHAR *string);
int null_snprintf(WCHAR *buffer, size_t count, const WCHAR *fmt, ...);

void parseServerAddress(char *szServer, WORD* wPort);

char *DemangleXml(const char *string, int len);
char *MangleXml(const char *string, int len);
char *EliminateHtml(const char *string, int len);
char *ApplyEncoding(const char *string, const char *pszEncoding);

int RandRange(int nLow, int nHigh);

BOOL IsStringUIN(const char *pszString);

int GetGMTOffset(void);

BOOL validateStatusMessageRequest(HANDLE hContact, WORD byMessageType);

void __fastcall SAFE_FREE(void** p);
void* __fastcall SAFE_MALLOC(size_t size);
void* __fastcall SAFE_REALLOC(void* p, size_t size);

__inline static void SAFE_FREE(char** str) { SAFE_FREE((void**)str); }

DWORD ICQWaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds, int bWaitAlways = FALSE);

HANDLE NetLib_OpenConnection(HANDLE hUser, const char* szIdent, NETLIBOPENCONNECTION* nloc);
void NetLib_CloseConnection(HANDLE *hConnection, int bServerConn);
void NetLib_SafeCloseHandle(HANDLE *hConnection);

int __fastcall ICQTranslateDialog(HWND hwndDlg);
char* __fastcall ICQTranslate(const char *src);
char* __fastcall ICQTranslateUtf(const char *src);
char* __fastcall ICQTranslateUtfStatic(const char *src, char *buf, size_t bufsize);

WORD GetMyStatusFlags();

/* Unicode FS utility functions */

int IsValidRelativePath(const char *filename);
char *ExtractFileName(const char *fullname);
char *FileNameToUtf(const char *filename);

int FileStatUtf(const char *path, struct _stati64 *buffer);
int MakeDirUtf(const char *dir);
int OpenFileUtf(const char *filename, int oflag, int pmode);

/* Unicode UI utility functions */
WCHAR* GetWindowTextUcs(HWND hWnd);
void SetWindowTextUcs(HWND hWnd, WCHAR *text);
char *GetWindowTextUtf(HWND hWnd);
char *GetDlgItemTextUtf(HWND hwndDlg, int iItem);
void SetWindowTextUtf(HWND hWnd, const char *szText);
void SetDlgItemTextUtf(HWND hwndDlg, int iItem, const char *szText);

int ComboBoxAddStringUtf(HWND hCombo, const char *szString, DWORD data);
int ListBoxAddStringUtf(HWND hList, const char *szString);

int MessageBoxUtf(HWND hWnd, const char *szText, const char *szCaption, UINT uType);

#endif /* __UTILITIES_H */
