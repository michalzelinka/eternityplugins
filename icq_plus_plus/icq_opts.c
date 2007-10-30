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
// Revision       : $Revision: 64 $
// Last change on : $Date: 2007-10-16 17:39:11 +0300 (Ð’Ñ‚, 16 Ð¾ÐºÑ‚ 2007) $
// Last change by : $Author: chaos.persei $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#include <win2k.h>
#include <uxtheme.h>

static BOOL CALLBACK DlgProcIcqOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqContactsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqFeaturesOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqFeatures2Opts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqPrivacyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqEventLogOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqClientIDOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqNewUINOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK DlgProcIcqPopupOpts2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern BOOL bXstatusIconShow;

extern void ShowSrvListDialog(HWND hwndCaller);
extern void setUserInfo();

static const char* szLogLevelDescr[] = {"Display all problems", "Display problems causing possible loss of data", "Display explanations for disconnection", "Display problems requiring user intervention", "Do not display problems"};

static BOOL (WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

static HWND hwndRegUin = NULL;
static HWND hwndPopUpsOpts = NULL;


int IcqOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQ);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Account");
	odp.groupPosition = 910000000;
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcIcqOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQCONTACTS);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Contacts");
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcIcqContactsOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);


	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQFEATURES);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Main Features");
	odp.groupPosition = 910000000;
	odp.flags=ODPF_BOLDGROUPS | ODPF_EXPERTONLY;
	odp.pfnDlgProc = DlgProcIcqFeaturesOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);


	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQFEATURES2);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Advanced Features");
	odp.groupPosition = 910000000;
	odp.flags=ODPF_BOLDGROUPS | ODPF_EXPERTONLY;
	odp.pfnDlgProc = DlgProcIcqFeatures2Opts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);


	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQPRIVACY);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Privacy");
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcIcqPrivacyOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);


	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQ_EVENTS);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Log");
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcIcqEventLogOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);


	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQ_CLIENT_ID);
	odp.pszTitle = gpszICQProtoName;
	odp.pszGroup = LPGEN("Network");
	odp.pszTab = LPGEN("Client ID");
	odp.flags=ODPF_BOLDGROUPS | ODPF_EXPERTONLY;
	odp.pfnDlgProc = DlgProcIcqClientIDOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	if (bPopUpService)
	{
		ZeroMemory(&odp,sizeof(odp));
		odp.cbSize = sizeof(odp);
		odp.hInstance = hInst;
		odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_POPUPS);
		odp.pszTitle = gpszICQProtoName;
		odp.pszGroup = LPGEN("PopUps");
		odp.flags=ODPF_BOLDGROUPS;
		odp.pfnDlgProc = DlgProcIcqPopupOpts;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

	return 0;
}


static void LoadDBCheckState(HWND hwndDlg, int idCtrl, const char* szSetting, BYTE bDef)
{
  CheckDlgButton(hwndDlg, idCtrl, ICQGetContactSettingByte(NULL, szSetting, bDef));
}



static void StoreDBCheckState(HWND hwndDlg, int idCtrl, const char* szSetting)
{
  ICQWriteContactSettingByte(NULL, szSetting, (BYTE)IsDlgButtonChecked(hwndDlg, idCtrl));
}



static void OptDlgChanged(HWND hwndDlg)
{
  SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}


// standalone option pages

static BOOL CALLBACK DlgProcIcqOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  char DBModule[64], buf[64];
  WORD SrvCount;
  int i, iIndex;
  switch (msg)
  {
  case WM_INITDIALOG:
    {
      char pszPwd[16];
      char szServer[MAX_PATH];

      ICQTranslateDialog(hwndDlg);

      SetDlgItemInt(hwndDlg, IDC_ICQNUM, ICQGetContactSettingUIN(NULL), FALSE);

      if (!ICQGetContactStaticString(NULL, "Password", pszPwd, sizeof(pszPwd)))
      {
        CallService(MS_DB_CRYPT_DECODESTRING, strlennull(pszPwd) + 1, (LPARAM)pszPwd);

        //bit of a security hole here, since it's easy to extract a password from an edit box
        SetDlgItemText(hwndDlg, IDC_PASSWORD, pszPwd);
      }
      
	  SendDlgItemMessage(hwndDlg, IDC_ICQSERVER, CB_RESETCONTENT, 0, 0);
	  mir_snprintf(DBModule, 64, "%sSrvs", gpszICQProtoName);
	  SrvCount = DBGetContactSettingWord(0, DBModule, "SrvCount", 0);
	 
	  for(i = 1; i <= SrvCount; i++){
		mir_snprintf(buf, 64, "server%luhost", i);
		SendDlgItemMessage(hwndDlg, IDC_ICQSERVER, CB_ADDSTRING, 0, (LPARAM)UniGetContactSettingUtf(NULL, DBModule, buf, 0));
	  }

      if (!ICQGetContactStaticString(NULL, "OscarServer", szServer, MAX_PATH))
      {
		SetDlgItemText(hwndDlg, IDC_ICQSERVER, szServer);
      }
      else
      {
        SetDlgItemText(hwndDlg, IDC_ICQSERVER, DEFAULT_SERVER_HOST);
      }
      
      SetDlgItemInt(hwndDlg, IDC_ICQPORT, ICQGetContactSettingWord(NULL, "OscarPort", DEFAULT_SERVER_PORT), FALSE);
      LoadDBCheckState(hwndDlg, IDC_KEEPALIVE, "KeepAlive", 1);
      LoadDBCheckState(hwndDlg, IDC_SECURE, "SecureLogin", DEFAULT_SECURE_LOGIN);
      SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_SETRANGE, FALSE, MAKELONG(0, 4));
      SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_SETPOS, TRUE, 4-ICQGetContactSettingByte(NULL, "ShowLogLevel", LOG_WARNING));
      SetDlgItemTextUtf(hwndDlg, IDC_LEVELDESCR, ICQTranslateUtfStatic(szLogLevelDescr[4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_GETPOS, 0, 0)], szServer, MAX_PATH));
      ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_HIDE);
      LoadDBCheckState(hwndDlg, IDC_NOERRMULTI, "IgnoreMultiErrorBox", 0);
	  LoadDBCheckState(hwndDlg, IDC_AUTOCHANGE, "ServerAutoChange", 1);
      
      return TRUE;
    }
    
  case WM_HSCROLL:
    {
      char str[MAX_PATH];

      SetDlgItemTextUtf(hwndDlg, IDC_LEVELDESCR, ICQTranslateUtfStatic(szLogLevelDescr[4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL,TBM_GETPOS, 0, 0)], str, MAX_PATH));
      OptDlgChanged(hwndDlg);
    }
    break;
    
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDC_REGUIN:
		  ShowRegUinDialog();
		  return TRUE;

      case IDC_SERVERBOX:
		  ShowSrvListDialog(hwndDlg);
		  return TRUE;

	  case IDC_ICQSERVER:
		  switch(HIWORD(wParam))
		  {
		   case CBN_SELCHANGE:
			 
			 iIndex = (int)SendDlgItemMessage(hwndDlg, IDC_ICQSERVER, CB_GETCURSEL, 0, 0);
			 if(iIndex < 0) return FALSE;
			 mir_snprintf(DBModule, 64, "%sSrvs", gpszICQProtoName);
			 mir_snprintf(buf, 64, "server%luport", iIndex + 1);
			 DBWriteContactSettingWord(0, DBModule, "CurrServ", (WORD)(iIndex + 1));
			 SetDlgItemInt(hwndDlg, IDC_ICQPORT, DBGetContactSettingWord(0, DBModule, buf, DEFAULT_SERVER_PORT), 0);
			 OptDlgChanged(hwndDlg);
			break;
		  }
		  break;
      }
      
      if (icqOnline && LOWORD(wParam) != IDC_NOERRMULTI)
      {
        char szClass[80];
        
        
        GetClassName((HWND)lParam, szClass, sizeof(szClass));
        
        if (stricmp(szClass, "EDIT") || HIWORD(wParam) == EN_CHANGE)
          ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_SHOW);
      }
      
      if ((LOWORD(wParam)==IDC_ICQNUM || LOWORD(wParam)==IDC_PASSWORD || LOWORD(wParam)==IDC_ICQSERVER || LOWORD(wParam)==IDC_ICQPORT) &&
        (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
      {
        return 0;
      }
      
      OptDlgChanged(hwndDlg);
      break;
    }
    
  case WM_NOTIFY:
    {
      switch (((LPNMHDR)lParam)->code)
      {
        
      case PSN_APPLY:
        {
          char str[128];
          
          ICQWriteContactSettingDword(NULL, UNIQUEIDSETTING, (DWORD)GetDlgItemInt(hwndDlg, IDC_ICQNUM, NULL, FALSE));
          GetDlgItemText(hwndDlg, IDC_PASSWORD, str, sizeof(gpszPassword));
          if (strlennull(str))
          {
            strcpy(gpszPassword, str);
            gbRememberPwd = TRUE;
          }
          else
          {
            gbRememberPwd = ICQGetContactSettingByte(NULL, "RememberPass", 0);
          }
          CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(gpszPassword), (LPARAM)str);
          ICQWriteContactSettingString(NULL, "Password", str);
          GetDlgItemText(hwndDlg,IDC_ICQSERVER, str, sizeof(str));
          ICQWriteContactSettingString(NULL, "OscarServer", str);
          ICQWriteContactSettingWord(NULL, "OscarPort", (WORD)GetDlgItemInt(hwndDlg, IDC_ICQPORT, NULL, FALSE));
          StoreDBCheckState(hwndDlg, IDC_KEEPALIVE, "KeepAlive");
          StoreDBCheckState(hwndDlg, IDC_SECURE, "SecureLogin");
          StoreDBCheckState(hwndDlg, IDC_NOERRMULTI, "IgnoreMultiErrorBox");
		      StoreDBCheckState(hwndDlg, IDC_AUTOCHANGE, "ServerAutoChange");
		      if(ICQGetContactSettingByte(NULL,"ServerAutoChange",1)==0)
			      bServerAutoChange = FALSE;
          ICQWriteContactSettingByte(NULL, "ShowLogLevel", (BYTE)(4-SendDlgItemMessage(hwndDlg, IDC_LOGLEVEL, TBM_GETPOS, 0, 0)));

          return TRUE;
        }
      }
    }
    break;
  }

  return FALSE;
}



static const UINT icqPrivacyControls[]={IDC_DCALLOW_ANY, IDC_DCALLOW_CLIST, IDC_DCALLOW_AUTH, IDC_ADD_ANY, IDC_ADD_AUTH, IDC_WEBAWARE, IDC_PUBLISHPRIMARY, IDC_STATIC_DC1, IDC_STATIC_DC2, IDC_STATIC_CLIST};
static BOOL CALLBACK DlgProcIcqPrivacyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    
  case WM_INITDIALOG:
    {
      int nDcType;
      int nAddAuth;
	  BYTE bData;

      nDcType = ICQGetContactSettingByte(NULL, "DCType", 0);
      nAddAuth = ICQGetContactSettingByte(NULL, "Auth", 1);
      
      ICQTranslateDialog(hwndDlg);
      if (!icqOnline)
      {
        icq_EnableMultipleControls(hwndDlg, icqPrivacyControls, sizeof(icqPrivacyControls)/sizeof(icqPrivacyControls[0]), FALSE);
        ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_NOTONLINE), SW_SHOW);
      }
      else 
      {
        ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_NOTONLINE), SW_HIDE);
      }
      CheckDlgButton(hwndDlg, IDC_DCALLOW_ANY, (nDcType == 0));
      CheckDlgButton(hwndDlg, IDC_DCALLOW_CLIST, (nDcType == 1));
      CheckDlgButton(hwndDlg, IDC_DCALLOW_AUTH, (nDcType == 2));
      CheckDlgButton(hwndDlg, IDC_ADD_ANY, (nAddAuth == 0));
      CheckDlgButton(hwndDlg, IDC_ADD_AUTH, (nAddAuth == 1));
      LoadDBCheckState(hwndDlg, IDC_WEBAWARE, "WebAware", 0);
      LoadDBCheckState(hwndDlg, IDC_PUBLISHPRIMARY, "PublishPrimaryEmail", 0);
      LoadDBCheckState(hwndDlg, IDC_NOSTATUSREPLY, "NoStatusReply", 0);
      LoadDBCheckState(hwndDlg, IDC_STATUSMSG_CLIST, "StatusMsgReplyCList", 0);
      LoadDBCheckState(hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible", 0);
      if (!ICQGetContactSettingByte(NULL, "StatusMsgReplyCList", 0))
        EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
	  bData = DBGetContactSettingByte(NULL,gpszICQProtoName,"NoStatusReply", 0);
	  CheckDlgButton(hwndDlg, IDC_NOSTATUSREPLY, bData);

      return TRUE;
    }
    
  case WM_COMMAND:
    switch (LOWORD(wParam))  
    {
    case IDC_DCALLOW_ANY:
    case IDC_DCALLOW_CLIST:
    case IDC_DCALLOW_AUTH:
    case IDC_ADD_ANY:
    case IDC_ADD_AUTH:
    case IDC_WEBAWARE:
    case IDC_PUBLISHPRIMARY:
	case IDC_NOSTATUSREPLY:
	  if (IsDlgButtonChecked(hwndDlg, IDC_NOSTATUSREPLY)) 
           icq_sendSetAimAwayMsgServ("");//set aim message to nothing
    case IDC_STATUSMSG_VISIBLE:
      if ((HWND)lParam != GetFocus())  return 0;
      break;
    case IDC_STATUSMSG_CLIST:
      if (IsDlgButtonChecked(hwndDlg, IDC_STATUSMSG_CLIST)) 
      {
        EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, TRUE);
        LoadDBCheckState(hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible", 0);
      }
      else 
      {
        EnableDlgItem(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
        CheckDlgButton(hwndDlg, IDC_STATUSMSG_VISIBLE, FALSE);
      }
      break;
    default:
      return 0;
    }
    OptDlgChanged(hwndDlg);
    break;

  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code) 
    {
      case PSN_APPLY:

        StoreDBCheckState(hwndDlg, IDC_WEBAWARE, "WebAware");
        StoreDBCheckState(hwndDlg, IDC_PUBLISHPRIMARY, "PublishPrimaryEmail");
        StoreDBCheckState(hwndDlg, IDC_NOSTATUSREPLY, "NoStatusReply");
        StoreDBCheckState(hwndDlg, IDC_STATUSMSG_CLIST, "StatusMsgReplyCList");
        StoreDBCheckState(hwndDlg, IDC_STATUSMSG_VISIBLE, "StatusMsgReplyVisible");
        if (IsDlgButtonChecked(hwndDlg, IDC_DCALLOW_AUTH))
          ICQWriteContactSettingByte(NULL, "DCType", 2);
        else if (IsDlgButtonChecked(hwndDlg, IDC_DCALLOW_CLIST))
          ICQWriteContactSettingByte(NULL, "DCType", 1);
        else 
          ICQWriteContactSettingByte(NULL, "DCType", 0);
        StoreDBCheckState(hwndDlg, IDC_ADD_AUTH, "Auth");

		if(ICQGetContactSettingByte(NULL, "NoStatusReply", 0))
			bNoStatusReply = TRUE;
		else 
			bNoStatusReply = FALSE;

        if (icqOnline)
        {
          PBYTE buf=NULL;
          int buflen=0;

          ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail", (BYTE)!ICQGetContactSettingByte(NULL, "PublishPrimaryEmail", 0), TLV_EMAIL);
          ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail0", 0, TLV_EMAIL);
          ppackTLVLNTSBytefromDB(&buf, &buflen, "e-mail1", 0, TLV_EMAIL);

          ppackTLVByte(&buf, &buflen, (BYTE)!ICQGetContactSettingByte(NULL, "Auth", 1), TLV_AUTH, 1);

          ppackTLVByte(&buf, &buflen, (BYTE)ICQGetContactSettingByte(NULL, "WebAware", 0), TLV_WEBAWARE, 1);

          icq_changeUserDetailsServ(META_SET_FULLINFO_REQ, buf, (WORD)buflen);

          SAFE_FREE(&buf);

          // Send a status packet to notify the server about the webaware setting
          {
            WORD wStatus;

            wStatus = MirandaStatusToIcq(gnCurrentStatus);

            if (gnCurrentStatus == ID_STATUS_INVISIBLE) 
            {
              if (gbSsiEnabled)
                updateServVisibilityCode(3);
              icq_setstatus(wStatus);
              // Tell who is on our visible list
              icq_sendEntireVisInvisList(0);
            }
            else
            {
              icq_setstatus(wStatus);
              if (gbSsiEnabled)
                updateServVisibilityCode(4);
              // Tell who is on our invisible list
              icq_sendEntireVisInvisList(1);
            }
          }
        }
        return TRUE;
      }
      break;
  }
  
  return FALSE;  
}


static const UINT icqPopupsControls[] = {IDC_POPUPS_LOG_ENABLED,IDC_PREVIEW,IDC_USEWINCOLORS,IDC_USESYSICONS,IDC_POPUP_LOG0_TIMEOUT,IDC_POPUP_LOG1_TIMEOUT,IDC_POPUP_LOG2_TIMEOUT,IDC_POPUP_LOG3_TIMEOUT,IDC_POPUP_LOG4_TIMEOUT,IDC_POPUP_LOG5_TIMEOUT,IDC_POPUP_SPAM_TIMEOUT,IDC_POPUP_UNKNOWN_TIMEOUT,IDC_POPUP_LOG6_TIMEOUT,IDC_POPUP_FOR_NOTONLIST,IDC_UINPOPUP};
static const UINT icqPopupColorControls[] = {
	IDC_POPUP_LOG0_TEXTCOLOR,IDC_POPUP_LOG1_TEXTCOLOR,IDC_POPUP_LOG2_TEXTCOLOR,IDC_POPUP_LOG3_TEXTCOLOR,IDC_POPUP_LOG4_TEXTCOLOR,IDC_POPUP_LOG5_TEXTCOLOR,IDC_POPUP_SPAM_TEXTCOLOR,IDC_POPUP_UNKNOWN_TEXTCOLOR,IDC_POPUP_LOG6_TEXTCOLOR,IDC_POPUP_CLIENT_CHANGE_TEXTCOLOR,IDC_POPUP_REMOVE_HIMSELF_TEXTCOLOR,IDC_POPUP_INFO_REQUEST_TEXTCOLOR,IDC_POPUP_IGNORECHECK_TEXTCOLOR,IDC_POPUP_AUTH_TEXTCOLOR,
	IDC_POPUP_LOG0_BACKCOLOR,IDC_POPUP_LOG1_BACKCOLOR,IDC_POPUP_LOG2_BACKCOLOR,IDC_POPUP_LOG3_BACKCOLOR,IDC_POPUP_LOG4_BACKCOLOR,IDC_POPUP_LOG5_BACKCOLOR,IDC_POPUP_SPAM_BACKCOLOR,IDC_POPUP_UNKNOWN_BACKCOLOR,IDC_POPUP_LOG6_BACKCOLOR,IDC_POPUP_CLIENT_CHANGE_BACKCOLOR,IDC_POPUP_REMOVE_HIMSELF_BACKCOLOR,IDC_POPUP_INFO_REQUEST_BACKCOLOR,IDC_POPUP_IGNORECHECK_BACKCOLOR,IDC_POPUP_AUTH_BACKCOLOR
};
void ShowPopUpsOpts(void);
static BOOL CALLBACK DlgProcIcqPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  BYTE bEnabled;
  switch (msg)
  {
  case WM_INITDIALOG:
    ICQTranslateDialog(hwndDlg);
	LoadDBCheckState(hwndDlg, IDC_POPUP_FOR_NOTONLIST, "PopUpForNotOnList", 0);
	LoadDBCheckState(hwndDlg, IDC_UINPOPUP, "UinPopup", 0);
    CheckDlgButton(hwndDlg, IDC_POPUPS_LOG_ENABLED, ICQGetContactSettingByte(NULL,"PopupsLogEnabled",1));
    bEnabled = ICQGetContactSettingByte(NULL,"PopupsWinColors",0);
    CheckDlgButton(hwndDlg, IDC_USEWINCOLORS, bEnabled);
    CheckDlgButton(hwndDlg, IDC_USESYSICONS, ICQGetContactSettingByte(NULL,"PopupsSysIcons",0));
    bEnabled = ICQGetContactSettingByte(NULL,"PopupsEnabled",1);
    CheckDlgButton(hwndDlg, IDC_POPUPS_ENABLED, bEnabled);
    icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);
    return TRUE;
  case WM_COMMAND:
	switch (LOWORD(wParam))
    {
    case IDC_PREVIEW:
      {
		  if (ICQGetContactSettingByte(NULL,"PopupsLogEnabled",1))
		  {
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Note",    LOG_NOTE);
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Warning", LOG_WARNING);
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Error",   LOG_ERROR);
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Fatal",   LOG_FATAL);		  
		  }
		  if(bFoundPopUp)
			  ShowPopUpMsg(NULL, 0, "Contact", "Sample ...was found!",   POPTYPE_FOUND);
		  if(bScanPopUp)
		  {   gbScan = TRUE;
			  ShowPopUpMsg(NULL, 0, "Entire List Check", "Sample Users Status Scan Started",   POPTYPE_SCAN);

			  gbScan = FALSE;
		      ShowPopUpMsg(NULL, 0, "Active Status Scan", "Sample Users Status Scan Complete",   POPTYPE_SCAN);
		  }
		  if(bSpamPopUp)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Spambot Found", POPTYPE_SPAM);
		  if(bUnknownPopUp)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Unknown Found", POPTYPE_UNKNOWN);
		  if(bVisPopUp)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Visibility event", POPTYPE_VIS);
		  if(bClientChangePopUp)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample ClientChange event", POPTYPE_CLIENT_CHANGE);
		  if(bInfoRequestPopUp)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample InfoRequest event", POPTYPE_INFO_REQUEST);
		  if(bIgnoreCheckPop)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Check Your Ignore State", POPTYPE_IGNORE_CHECK);
		  if(bPopSelfRem)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Removed Himself", POPTYPE_SELFREMOVE);
		  if(bAuthPopUp)
			  ShowPopUpMsg(NULL, 0, "Popup Title", "Sample Auth PopUp", POPTYPE_AUTH);
      }
      return FALSE;

    case IDC_POPUPS_ENABLED:
      bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
      icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);
      break;
    case IDC_USEWINCOLORS:
      bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
      icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
      break;
	case IDC_POPUPS_OPTIONS:
	  ShowPopUpsOpts();
	return TRUE;
    }
    SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
    break;
  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code)
    {
    case PSN_APPLY:
      ICQWriteContactSettingByte(NULL,"PopupsEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED));
      ICQWriteContactSettingByte(NULL,"PopupsLogEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_LOG_ENABLED));
	  ICQWriteContactSettingByte(NULL,"PopupsWinColors",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
      ICQWriteContactSettingByte(NULL,"PopupsSysIcons",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USESYSICONS));
	  StoreDBCheckState(hwndDlg, IDC_POPUP_FOR_NOTONLIST , "PopUpForNotOnList");
	  if(ICQGetContactSettingByte(NULL, "PopUpForNotOnList", 0) == 1)
		  bPopUpForNotOnList = TRUE;
	  else
		  bPopUpForNotOnList = FALSE;
	  StoreDBCheckState(hwndDlg, IDC_UINPOPUP , "UinPopup");
	  if(ICQGetContactSettingByte(NULL, "UinPopup", 0) == 1)
		  bUinPopup = TRUE;
	  else
		  bUinPopup = FALSE;
      return TRUE;
    }
    break;
  }
  return FALSE;
}




static BOOL CALLBACK DlgProcIcqPopupOpts2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);
		LoadDBCheckState(hwndDlg, IDC_SPAM_POPUP_ENABLE, "SpamPopUpEnabled", 0);
		LoadDBCheckState(hwndDlg, IDC_UNKNOWN_POPUP_ENABLE, "UnknownPopUpEnabled", 0);
		LoadDBCheckState(hwndDlg, IDC_WAS_FOUND_POPUP_ENABLE, "FoundPopUpEnabled", 1);
		LoadDBCheckState(hwndDlg, IDC_SCAN_POPUP_ENABLE, "ScanPopUpEnabled", 1);
		LoadDBCheckState(hwndDlg, IDC_VIS_POPUP_ENABLE, "VisPopUpEnabled", 1);
		LoadDBCheckState(hwndDlg, IDC_CLIENT_CHANGE_POPUP, "ClientChangePopup", 0);
		LoadDBCheckState(hwndDlg, IDC_POPSELFREM, "PopSelfRem", 1);
		LoadDBCheckState(hwndDlg, IDC_IGNCHECKPOP, "IgnoreCheckPop", 1);
		LoadDBCheckState(hwndDlg, IDC_INFO_REQUEST_POPUP, "InfoRequestPopUp", 0);
		LoadDBCheckState(hwndDlg, IDC_POPAUTH, "AuthPopUp", 0);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG0_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups0TextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG0_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups0BackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG0_TIMEOUT, ICQGetContactSettingDword(NULL,"Popups0Timeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG1_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups1TextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG1_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups1BackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG1_TIMEOUT, ICQGetContactSettingDword(NULL,"Popups1Timeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG2_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups2TextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG2_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups2BackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG2_TIMEOUT, ICQGetContactSettingDword(NULL,"Popups2Timeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG3_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups3TextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG3_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"Popups3BackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG3_TIMEOUT, ICQGetContactSettingDword(NULL,"Popups3Timeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG4_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsFoundTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG4_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsFoundBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG4_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsFoundTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG5_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsFinishedTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG5_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsFinishedBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG5_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsFinishedTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SPAM_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsSpamTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SPAM_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsSpamBackColor",RGB(0,0,255)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_SPAM_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsSpamTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_UNKNOWN_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsUnknownTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_UNKNOWN_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsUnknownBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_UNKNOWN_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsUnknownTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG6_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsVisTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG6_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsVisBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG6_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsVisTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_CLIENT_CHANGE_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsClientChangeTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_CLIENT_CHANGE_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsClientChangeBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_CLIENT_CHANGE_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsClientChangeTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_INFO_REQUEST_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsInfoRequestTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_INFO_REQUEST_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsInfoRequestBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_INFO_REQUEST_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsInfoRequestTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_IGNORECHECK_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsIgnoreCheckTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_IGNORECHECK_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsIgnoreCheckBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_IGNORECHECK_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsIgnoreCheckTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_REMOVE_HIMSELF_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsRemoveHimselfTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_REMOVE_HIMSELF_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsRemoveHimselfBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_REMOVE_HIMSELF_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsRemoveHimselfTimeout",0),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_AUTH_TEXTCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsAuthTextColor",RGB(255,255,255)));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_AUTH_BACKCOLOR, CPM_SETCOLOUR, 0, ICQGetContactSettingDword(NULL,"PopupsAuthBackColor",RGB(0,0,0)));
		SetDlgItemInt(hwndDlg, IDC_POPUP_AUTH_TIMEOUT, ICQGetContactSettingDword(NULL,"PopupsAuthTimeout",0),FALSE);
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), ICQGetContactSettingByte(NULL, "PopupsWinColors", 0)-1);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_OK:
			ICQWriteContactSettingDword(NULL,"Popups0TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG0_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups0BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG0_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups0Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG0_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"Popups1TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG1_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups1BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG1_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups1Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG1_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"Popups2TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG2_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups2BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG2_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups2Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG2_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"Popups3TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG3_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups3BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG3_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"Popups3Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG3_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsFoundTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG4_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsFoundBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG4_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsFoundTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG4_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsFinishedTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG5_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsFinishedBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG5_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsFinishedTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG5_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsSpamTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SPAM_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsSpamBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SPAM_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsSpamTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_SPAM_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsUnknownTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_UNKNOWN_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsUnknownBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_UNKNOWN_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsUnknownTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_UNKNOWN_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsVisTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG6_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsVisBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG6_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsVisTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG6_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsClientChangeTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_CLIENT_CHANGE_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsClientChangeBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_CLIENT_CHANGE_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsClientChangeTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_CLIENT_CHANGE_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsInfoRequestTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_INFO_REQUEST_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsInfoRequestBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_INFO_REQUEST_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsInfoRequestTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_INFO_REQUEST_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsIgnoreCheckTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_IGNORECHECK_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsIgnoreCheckBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_IGNORECHECK_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsIgnoreCheckTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_IGNORECHECK_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsRemoveHimselfTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_REMOVE_HIMSELF_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsRemoveHimselfBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_REMOVE_HIMSELF_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsRemoveHimselfTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_REMOVE_HIMSELF_TIMEOUT, NULL, FALSE));
			ICQWriteContactSettingDword(NULL,"PopupsAuthTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_AUTH_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsAuthBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_AUTH_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ICQWriteContactSettingDword(NULL,"PopupsAuthTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_AUTH_TIMEOUT, NULL, FALSE));
			StoreDBCheckState(hwndDlg, IDC_SPAM_POPUP_ENABLE , "SpamPopUpEnabled");
			if(ICQGetContactSettingByte(NULL, "SpamPopUpEnabled", 0) == 1)
				bSpamPopUp = TRUE;
			else
				bSpamPopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_UNKNOWN_POPUP_ENABLE , "UnknownPopUpEnabled");
			if(ICQGetContactSettingByte(NULL, "UnknownPopUpEnabled", 0) == 1)
				bUnknownPopUp = TRUE;
			else
				bUnknownPopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_WAS_FOUND_POPUP_ENABLE , "FoundPopUpEnabled");
			if(ICQGetContactSettingByte(NULL, "FoundPopUpEnabled", 1) == 1)
				bFoundPopUp = TRUE;
			else
				bFoundPopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_SCAN_POPUP_ENABLE , "ScanPopUpEnabled");
			if(ICQGetContactSettingByte(NULL, "ScanPopUpEnabled", 1) == 1)
				bScanPopUp = TRUE;
			else
				bScanPopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_VIS_POPUP_ENABLE , "VisPopUpEnabled");
			if(ICQGetContactSettingByte(NULL, "VisPopUpEnabled", 1) == 1)
				bVisPopUp = TRUE;
			else
				bVisPopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_CLIENT_CHANGE_POPUP , "ClientChangePopup");
			if(ICQGetContactSettingByte(NULL, "ClientChangePopup", 0) == 1)
				bClientChangePopUp = TRUE;
			else
				bClientChangePopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_IGNCHECKPOP , "IgnoreCheckPop");
			if(ICQGetContactSettingByte(NULL, "IgnoreCheckPop", 1) == 1)
				bIgnoreCheckPop = TRUE;
			else
				bIgnoreCheckPop = FALSE;
			StoreDBCheckState(hwndDlg, IDC_POPSELFREM , "PopSelfRem");
			if(ICQGetContactSettingByte(NULL, "PopSelfRem", 1) == 1)
				bPopSelfRem = TRUE;
			else
				bPopSelfRem = FALSE;
			StoreDBCheckState(hwndDlg, IDC_INFO_REQUEST_POPUP , "InfoRequestPopUp");
			if(ICQGetContactSettingByte(NULL, "InfoRequestPopUp", 0) == 1)
				bInfoRequestPopUp = TRUE;
			else
				bInfoRequestPopUp = FALSE;
			StoreDBCheckState(hwndDlg, IDC_POPAUTH , "AuthPopUp");
			if(ICQGetContactSettingByte(NULL, "AuthPopUp", 0) == 1)
				bAuthPopUp = TRUE;
			else
				bAuthPopUp = FALSE;
			DestroyWindow(hwndDlg);
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		  break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	case WM_DESTROY:
		hwndPopUpsOpts = NULL;
		break;
	}
	return FALSE;
}




static HWND hCpCombo;

struct CPTABLE {
  WORD cpId;
  char *cpName;
};


struct CPTABLE cpTable[] = {
  {  874,  "Thai" },
  {  932,  "Japanese" },
  {  936,  "Simplified Chinese" },
  {  949,  "Korean" },
  {  950,  "Traditional Chinese" },
  {  1250,  "Central European" },
  {  1251,  "Cyrillic" },
  {  1252,  "Latin I" },
  {  1253,  "Greek" },
  {  1254,  "Turkish" },
  {  1255,  "Hebrew" },
  {  1256,  "Arabic" },
  {  1257,  "Baltic" },
  {  1258,  "Vietnamese" },
  {  1361,  "Korean (Johab)" },
  {   -1, NULL}
};

static BOOL CALLBACK FillCpCombo(LPCSTR str)
{
  int i;
  UINT cp;

  cp = atoi(str);
  for (i=0; cpTable[i].cpName != NULL && cpTable[i].cpId!=cp; i++);
  if (cpTable[i].cpName != NULL) 
  {
    ComboBoxAddStringUtf(hCpCombo, cpTable[i].cpName, cpTable[i].cpId);
  }
  return TRUE;
}


static const UINT icqUnicodeControls[] = {IDC_UTFALL,IDC_UTFSTATIC,IDC_UTFCODEPAGE};
static const UINT icqDCMsgControls[] = {IDC_DCPASSIVE};
static const UINT icqDCIconsControls[] = {IDC_DC_ICON_POS,IDC_DC_ICON_POS_TEXT};
static const UINT icqXStatusControls[] = {IDC_XSTNONSTD,IDC_XSTUPDATERATE,IDC_XSTUPDATE,IDC_XSTATUSAUTO,IDC_XSTATUSRESET,IDC_FORCEXSTATUS,IDC_XSTATUS_ICON_POS,IDC_XSTATUS_SHOW,IDC_REPLACEXTEXT,IDC_ICON_POS_TEXT};
static const UINT icqAimControls[] = {IDC_AIMENABLE};
static BOOL CALLBACK DlgProcIcqFeaturesOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
	  {
		  BYTE bData;
		  int sCodePage;
		  int i;

		  ICQTranslateDialog(hwndDlg);
		  bData = ICQGetContactSettingByte(NULL, "UtfEnabled", DEFAULT_UTF_ENABLED);
		  CheckDlgButton(hwndDlg, IDC_UTFENABLE, bData?TRUE:FALSE);
		  CheckDlgButton(hwndDlg, IDC_UTFALL, bData==2?TRUE:FALSE);
		  icq_EnableMultipleControls(hwndDlg, icqUnicodeControls, sizeof(icqUnicodeControls)/sizeof(icqUnicodeControls[0]), bData?TRUE:FALSE);
		  LoadDBCheckState(hwndDlg, IDC_XSTUPDATE, "UpdateXStatus", 1);
		  SetDlgItemInt(hwndDlg, IDC_XSTUPDATERATE, ICQGetContactSettingDword(NULL, "XStatusUpdatePeriod", 15), 0);
	      EnableDlgItem(hwndDlg, IDC_XSTUPDATERATE, IsDlgButtonChecked(hwndDlg, IDC_XSTUPDATE));
		  LoadDBCheckState(hwndDlg, IDC_TEMPVISIBLE, "TempVisListEnabled",DEFAULT_TEMPVIS_ENABLED);
		  LoadDBCheckState(hwndDlg, IDC_SLOWSEND, "SlowSend", DEFAULT_SLOWSEND);
		  LoadDBCheckState(hwndDlg, IDC_ONLYSERVERACKS, "OnlyServerAcks", DEFAULT_ONLYSERVERACKS);
		  bData = ICQGetContactSettingByte(NULL, "DirectMessaging", DEFAULT_DCMSG_ENABLED);
		  LoadDBCheckState(hwndDlg, IDC_DCICON, "ShowDCIcon", 1);
		  CheckDlgButton(hwndDlg, IDC_DCENABLE, bData?TRUE:FALSE);
		  CheckDlgButton(hwndDlg, IDC_DCPASSIVE, bData==1?TRUE:FALSE);
		  icq_EnableMultipleControls(hwndDlg, icqDCMsgControls, sizeof(icqDCMsgControls)/sizeof(icqDCMsgControls[0]), bData?TRUE:FALSE);
		  bData = ICQGetContactSettingByte(NULL, "XStatusEnabled", DEFAULT_XSTATUS_ENABLED);
		  CheckDlgButton(hwndDlg, IDC_XSTATUSENABLE, bData);
		  icq_EnableMultipleControls(hwndDlg, icqXStatusControls, sizeof(icqXStatusControls)/sizeof(icqXStatusControls[0]), bData);
		  LoadDBCheckState(hwndDlg, IDC_XSTATUSAUTO, "XStatusAuto", DEFAULT_XSTATUS_AUTO);
		  LoadDBCheckState(hwndDlg, IDC_XSTATUSRESET, "XStatusReset", DEFAULT_XSTATUS_RESET);
		  LoadDBCheckState(hwndDlg, IDC_XSTATUS_SHOW, "XStatusIconShow", 1);
		  LoadDBCheckState(hwndDlg, IDC_FORCEXSTATUS, "ForceXstatus", 0);
		  LoadDBCheckState(hwndDlg, IDC_KILLSPAMBOTS, "KillSpambots", DEFAULT_KILLSPAM_ENABLED);
		  LoadDBCheckState(hwndDlg, IDC_KILLUNKNOWN, "KillUnknown", 0);
		  LoadDBCheckState(hwndDlg, IDC_AIMENABLE, "AimEnabled", DEFAULT_AIM_ENABLED);
		  LoadDBCheckState(hwndDlg, IDC_RTF, "RTF", 0);
		  LoadDBCheckState(hwndDlg, IDC_REPLACEXTEXT, "ShowMyXText", 1);
		  icq_EnableMultipleControls(hwndDlg, icqAimControls, sizeof(icqAimControls)/sizeof(icqAimControls[0]), icqOnline?FALSE:TRUE);
		  {
			  char* CIdComboBox[] =
			  {
				  "Email","Protocol","SMS","Advanced 1","Advanced 2","Web","Client","VisMode","Advanced 3","Advanced 4"
			  };
			  int i;
			  for (i=0; i<sizeof(CIdComboBox)/sizeof(CIdComboBox[0]); i++)
			  {
				  ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_DC_ICON_POS), CIdComboBox[i],i);
				  ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_XSTATUS_ICON_POS), CIdComboBox[i],i);
			  }
		  }
		  SendDlgItemMessage(hwndDlg, IDC_DC_ICON_POS, CB_SETCURSEL, (DBGetContactSettingWord(NULL,gpszICQProtoName,"dc_icon_pos", 5))-1, 0);
		  SendDlgItemMessage(hwndDlg, IDC_XSTATUS_ICON_POS, CB_SETCURSEL, (DBGetContactSettingWord(NULL,gpszICQProtoName,"xstatus_icon_pos", 4))-1, 0);
		  hCpCombo = GetDlgItem(hwndDlg, IDC_UTFCODEPAGE);
		  sCodePage = ICQGetContactSettingWord(NULL, "AnsiCodePage", CP_ACP);
		  ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_UTFCODEPAGE), "System default codepage", 0);
		  EnumSystemCodePagesA(FillCpCombo, CP_INSTALLED);
		  if(sCodePage == 0)
			  SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_SETCURSEL, (WPARAM)0, 0);
		  else 
		  {
			  for (i = 0; i < SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETCOUNT, 0, 0); i++) 
			  {
				  if (SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETITEMDATA, (WPARAM)i, 0) == sCodePage)
				  {
					  SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_SETCURSEL, (WPARAM)i, 0);
					  break;
				  }
			  }
		  }
		  LoadDBCheckState(hwndDlg, IDC_XSTNONSTD, "NonStandartXstatus", 1);
		  return TRUE;
	  }
  case WM_COMMAND:
	  switch (LOWORD(wParam))
	  {
	  case IDC_UTFENABLE:
		  icq_EnableMultipleControls(hwndDlg, icqUnicodeControls, sizeof(icqUnicodeControls)/sizeof(icqUnicodeControls[0]), IsDlgButtonChecked(hwndDlg, IDC_UTFENABLE));
		  break;
	  case IDC_DCENABLE:
		  icq_EnableMultipleControls(hwndDlg, icqDCMsgControls, sizeof(icqDCMsgControls)/sizeof(icqDCMsgControls[0]), IsDlgButtonChecked(hwndDlg, IDC_DCENABLE));
		  break;
	  case IDC_XSTATUSENABLE:
		  icq_EnableMultipleControls(hwndDlg, icqXStatusControls, sizeof(icqXStatusControls)/sizeof(icqXStatusControls[0]), IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE));
		  break;
	  case IDC_DCICON:
		  icq_EnableMultipleControls(hwndDlg, icqDCIconsControls, sizeof(icqDCIconsControls)/sizeof(icqDCIconsControls[0]), IsDlgButtonChecked(hwndDlg, IDC_DCICON));
		  break;
	  case IDC_XSTUPDATE:
		  EnableDlgItem(hwndDlg, IDC_XSTUPDATERATE, IsDlgButtonChecked(hwndDlg, IDC_XSTUPDATE));
	  }
	  OptDlgChanged(hwndDlg);
	  break;
  case WM_NOTIFY:
	  switch (((LPNMHDR)lParam)->code)
	  {
	  case PSN_APPLY:
		  {
			  int i = SendDlgItemMessage(hwndDlg, IDC_DC_ICON_POS, CB_GETCURSEL, 0, 0)+1;
			  DBWriteContactSettingWord(NULL, gpszICQProtoName, "dc_icon_pos", (WORD)i);
		  }
		  {
			  int i = SendDlgItemMessage(hwndDlg, IDC_XSTATUS_ICON_POS, CB_GETCURSEL, 0, 0)+1;
			  DBWriteContactSettingWord(NULL, gpszICQProtoName, "xstatus_icon_pos", (WORD)i);
		  }
		  if( DBGetContactSettingByte(NULL,gpszICQProtoName,"RTF", 0) != (BYTE)IsDlgButtonChecked(hwndDlg,IDC_RTF))
		  {
			  MessageBox(0,Translate("To enable RTF text reciving you must reconnect your Miranda after option is enabled"),Translate("Warning"),MB_OK);
			  ICQWriteContactSettingByte(NULL,"RTF",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_RTF));
		  }
		  if (IsDlgButtonChecked(hwndDlg, IDC_UTFENABLE))
			  gbUtfEnabled = IsDlgButtonChecked(hwndDlg, IDC_UTFALL)?2:1;
		  else
			  gbUtfEnabled = 0;
		  {
			  int i = SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETCURSEL, 0, 0);
			  gwAnsiCodepage = (WORD)SendDlgItemMessage(hwndDlg, IDC_UTFCODEPAGE, CB_GETITEMDATA, (WPARAM)i, 0);
			  ICQWriteContactSettingWord(NULL, "AnsiCodePage", gwAnsiCodepage);
		  }
		  ICQWriteContactSettingByte(NULL, "UtfEnabled", gbUtfEnabled);
		  gbTempVisListEnabled = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TEMPVISIBLE);
		  ICQWriteContactSettingByte(NULL, "TempVisListEnabled", gbTempVisListEnabled);
		  StoreDBCheckState(hwndDlg, IDC_SLOWSEND, "SlowSend");
		  StoreDBCheckState(hwndDlg, IDC_ONLYSERVERACKS, "OnlyServerAcks");
		  if (IsDlgButtonChecked(hwndDlg, IDC_DCENABLE))
			  gbDCMsgEnabled = IsDlgButtonChecked(hwndDlg, IDC_DCPASSIVE)?1:2;
		  else
			  gbDCMsgEnabled = 0;
		  ICQWriteContactSettingByte(NULL, "DirectMessaging", gbDCMsgEnabled);
		  gbXStatusEnabled = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_XSTATUSENABLE);
		  ICQWriteContactSettingByte(NULL, "XStatusEnabled", gbXStatusEnabled);
		  StoreDBCheckState(hwndDlg, IDC_DCICON, "ShowDCIcon");
		  StoreDBCheckState(hwndDlg, IDC_XSTATUSAUTO, "XStatusAuto");
		  StoreDBCheckState(hwndDlg, IDC_XSTATUSRESET, "XStatusReset");
		  StoreDBCheckState(hwndDlg, IDC_FORCEXSTATUS , "ForceXstatus");
		  StoreDBCheckState(hwndDlg, IDC_XSTATUS_SHOW, "XStatusIconShow");
		  if (ICQGetContactSettingByte(NULL, "XStatusIconShow", 1)==0)
			  bXstatusIconShow = FALSE;
		  else 
			  bXstatusIconShow = TRUE;
		  StoreDBCheckState(hwndDlg, IDC_KILLSPAMBOTS , "KillSpambots");
		  StoreDBCheckState(hwndDlg, IDC_KILLUNKNOWN , "KillUnknown");
		  StoreDBCheckState(hwndDlg, IDC_AIMENABLE, "AimEnabled");
		  StoreDBCheckState(hwndDlg, IDC_RTF, "RTF");
		  StoreDBCheckState(hwndDlg, IDC_REPLACEXTEXT, "ShowMyXText");
		  {
			  DWORD dwRate;
			  StoreDBCheckState(hwndDlg, IDC_XSTUPDATE, "UpdateXStatus");
			  dwRate = GetDlgItemInt(hwndDlg, IDC_XSTUPDATERATE, 0, 0);
			  if(dwRate < 1 || dwRate > 60)
				  dwRate = 15;
			  SetDlgItemInt(hwndDlg, IDC_XSTUPDATERATE, dwRate, 0);
			  ICQWriteContactSettingDword(NULL, "XStatusUpdatePeriod", dwRate);
		  }
		  StoreDBCheckState(hwndDlg, IDC_XSTNONSTD, "NonStandartXstatus");
		  return TRUE;
	  }
	  break;
  }
  return FALSE;
}
static const UINT icqASDControls[] = {IDC_NOASD, IDC_CHECKITEM, IDC_ASDSTARTUP};
static const UINT icqPSDControls[] = {IDC_NOPSD_FOR_HIDDEN};
static BOOL CALLBACK DlgProcIcqFeatures2Opts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	extern BOOL bShowAuth;
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			BYTE bData;

			ICQTranslateDialog(hwndDlg);
//			LoadDBCheckState(hwndDlg, IDC_STEALTHRQST, "StealthRequest", 0);
			LoadDBCheckState(hwndDlg, IDC_PSD, "PSD", 0);
            LoadDBCheckState(hwndDlg, IDC_NOASD, "NoASD", 0);
			LoadDBCheckState(hwndDlg, IDC_SHOW_AUTH, "ShowAuth", 0);
//			LoadDBCheckState(hwndDlg, IDC_INV4INV, "Inv4Inv", DEFAULT_INV4INV_DISABLED);
//			LoadDBCheckState(hwndDlg, IDC_LOGSELFREM, "LogSelfRem", 0);
//			LoadDBCheckState(hwndDlg, IDC_IGNCHECKLOG, "IgnoreCheckLog", 0);
			LoadDBCheckState(hwndDlg, IDC_ASDSTARTUP, "ASDStartup", 0);
			LoadDBCheckState(hwndDlg, IDC_NOPSD_FOR_HIDDEN, "NoPSDForHidden", 1);
			bData = DBGetContactSettingByte(NULL,gpszICQProtoName,"ASD", 0);
			CheckDlgButton(hwndDlg, IDC_ASD, bData);
			icq_EnableMultipleControls(hwndDlg, icqASDControls, sizeof(icqASDControls)/sizeof(icqASDControls[0]), bData?TRUE:FALSE);
//			bData = DBGetContactSettingByte(NULL,gpszICQProtoName,"Inv4Inv", DEFAULT_INV4INV_DISABLED);
//		    CheckDlgButton(hwndDlg, IDC_INV4INV, bData);
			bData = DBGetContactSettingByte(NULL,gpszICQProtoName,"ASDStartup", 0);
		    CheckDlgButton(hwndDlg, IDC_ASDSTARTUP, bData);
			LoadDBCheckState(hwndDlg, IDC_TZER, "tZer", 0);
//            icq_EnableMultipleControls(hwndDlg, icqIncognitoControls, sizeof(icqIncognitoControls)/sizeof(icqIncognitoControls[0]), bStealthRequest?TRUE:FALSE);
	        CheckDlgButton(hwndDlg, IDC_INCUSER, (bIncognitoGlobal == 0));
	        CheckDlgButton(hwndDlg, IDC_INCGLOBAL, (bIncognitoGlobal == 1));
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		  case IDC_ASD:
			  icq_EnableMultipleControls(hwndDlg, icqASDControls, sizeof(icqASDControls)/sizeof(icqASDControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ASD));
			  break;
		  case IDC_PSD:
			  icq_EnableMultipleControls(hwndDlg, icqPSDControls, sizeof(icqPSDControls)/sizeof(icqPSDControls[0]), IsDlgButtonChecked(hwndDlg, IDC_PSD));
			  break;
//		  case IDC_STEALTHRQST:
//		  icq_EnableMultipleControls(hwndDlg, icqIncognitoControls, sizeof(icqIncognitoControls)/sizeof(icqIncognitoControls[0]), IsDlgButtonChecked(hwndDlg, IDC_STEALTHRQST));
		  
		}
		OptDlgChanged(hwndDlg);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			if( DBGetContactSettingByte(NULL,gpszICQProtoName,"ASD", 0) != (BYTE)IsDlgButtonChecked(hwndDlg,IDC_ASD))
		    {
			  MessageBox(0,Translate("To enable Active Status Discovery you must restart your Miranda after option is enabled"),Translate("Warning"),MB_OK);
			  ICQWriteContactSettingByte(NULL,"ASD",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ASD));
		    }
/*			if( DBGetContactSettingByte(NULL,gpszICQProtoName,"Inv4Inv", 0) != (BYTE)IsDlgButtonChecked(hwndDlg,IDC_INV4INV))
		    {
			  MessageBox(0,"To enable/disable Inv4Inv you must restart your Miranda","Warning",MB_OK);
			  ICQWriteContactSettingByte(NULL,"Inv4Inv",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_INV4INV));
		    }*/
			if( DBGetContactSettingByte(NULL,gpszICQProtoName,"ASDStartup", 0) != (BYTE)IsDlgButtonChecked(hwndDlg,IDC_ASDSTARTUP))
		    {
			  MessageBox(0,Translate("To enable/disable Entire list check you must restart your Miranda"),Translate("Warning"),MB_OK);
			  ICQWriteContactSettingByte(NULL,"ASDStartup",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ASDSTARTUP));
		    }
//		    StoreDBCheckState(hwndDlg, IDC_STEALTHRQST , "StealthRequest");
//			if(ICQGetContactSettingByte(NULL, "StealthRequest", 0) == 1)
//				bStealthRequest = TRUE;
//			else 
//				bStealthRequest = FALSE;
			StoreDBCheckState(hwndDlg, IDC_SHOW_AUTH, "ShowAuth");
			if(ICQGetContactSettingByte(NULL, "ShowAuth", 0) == 1)
				bShowAuth = TRUE;
			else
				bShowAuth = FALSE;
			StoreDBCheckState(hwndDlg, IDC_PSD , "PSD");
			if(ICQGetContactSettingByte(NULL, "PSD", 1) == 1)
				bPSD = TRUE;
			else 
				bPSD = FALSE;
			StoreDBCheckState(hwndDlg, IDC_NOASD , "NoASD");
			if(ICQGetContactSettingByte(NULL, "NoASD", 1) == 1)
				bNoASD = TRUE;
			else 
				bNoASD = FALSE;
			StoreDBCheckState(hwndDlg, IDC_NOPSD_FOR_HIDDEN, "NoPSDForHidden");
			if(ICQGetContactSettingByte(NULL, "NoPSDForHidden", 1)==1)
				bNoPSDForHidden = TRUE;
			else
				bNoPSDForHidden = FALSE;
//			StoreDBCheckState(hwndDlg, IDC_INV4INV , "Inv4Inv");
			StoreDBCheckState(hwndDlg, IDC_INCGLOBAL , "IncognitoGlobal");
			if(ICQGetContactSettingByte(NULL, "IncognitoGlobal", 0) == 1)
				bIncognitoGlobal = TRUE;
			else 
				bIncognitoGlobal = FALSE;
			StoreDBCheckState(hwndDlg, IDC_NOASD , "NoASD");
			StoreDBCheckState(hwndDlg, IDC_ASDSTARTUP , "ASDStartup");
			StoreDBCheckState(hwndDlg, IDC_TZER, "tZer");
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static BOOL CALLBACK DlgProcIcqEventLogOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	extern BOOL bHcontactHistory;
	switch (msg)
	{
	case WM_INITDIALOG:
		    ICQTranslateDialog(hwndDlg);
		
			LoadDBCheckState(hwndDlg, IDC_LOG_REMOVE_FILE, "LogSelfRemoveFile", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_IGNORECHECK_FILE, "LogIgnoreCheckFile", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_CHECKSTATUS_FILE, "LogStatusCheckFile", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_CLIENTCHANGE_FILE, "LogClientChangeFile", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_AUTH_FILE, "LogAuthFile", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_REQUEST_FILE, "LogRequestFile", 0);

			LoadDBCheckState(hwndDlg, IDC_LOG_REMOVE_HISTORY, "LogSelfRemoveHistory", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_IGNORECHECK_HISTORY, "LogIgnoreCheckHistory", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_CHECKSTATUS_HISTORY, "LogStatusCheckHistory", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_CLIENTCHANGE_HISTORY, "LogClientChangeHistory", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_AUTH_HISTORY, "LogAuthHistory", 0);
			LoadDBCheckState(hwndDlg, IDC_LOG_REQUEST_HISTORY, "LogRequestHistory", 0);

			LoadDBCheckState(hwndDlg, IDC_LOG_TO_HCONTACT_HISTORY, "LogToHcontact", 0);


		    SetDlgItemText(hwndDlg, IDC_FILEPATH, UniGetContactSettingUtf(NULL, gpszICQProtoName, "EventsLog", "EventsLog.txt"));

		    SendMessage(GetDlgItem(hwndDlg, IDC_BROWSE), BUTTONSETASFLATBTN, 0, 0);
		    SendMessage(GetDlgItem(hwndDlg, IDC_BROWSE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_FILE));

			return TRUE;
		
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BROWSE:
		{	TCHAR str[MAX_PATH+2];
			OPENFILENAME ofn={0};
			TCHAR filter[512],*pfilter;

			GetWindowText(GetWindow((HWND)lParam,GW_HWNDPREV),str,SIZEOF(str));
			ofn.lStructSize=CDSIZEOF_STRUCT(OPENFILENAME,lpTemplateName);
			ofn.hwndOwner=hwndDlg;
			ofn.Flags=OFN_HIDEREADONLY;
			if (LOWORD(wParam)==IDC_BROWSE) {
				ofn.lpstrTitle=TranslateT("Select where log file will be created");
			} else {
				ofn.Flags|=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
				ofn.lpstrTitle=TranslateT("Select program to be run");
			}					
			_tcscpy(filter,TranslateT("All Files"));
			_tcscat(filter,_T(" (*)"));
			pfilter=filter+lstrlen(filter)+1;
			_tcscpy(pfilter,_T("*"));
			pfilter=pfilter+lstrlen(pfilter)+1;
			*pfilter='\0';
			ofn.lpstrFilter=filter;
			ofn.lpstrFile=str;
			ofn.nMaxFile=SIZEOF(str)-2;
			ofn.nMaxFileTitle=MAX_PATH;
			if (LOWORD(wParam)==IDC_BROWSE) {
				if(!GetSaveFileName(&ofn)) return 1;
			} else {
				if(!GetOpenFileName(&ofn)) return 1;						
			}
			SetWindowText(GetWindow((HWND)lParam,GW_HWNDPREV),str);
			break;
		}
		  break;
		}
		OptDlgChanged(hwndDlg);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			StoreDBCheckState(hwndDlg, IDC_LOG_REMOVE_FILE, "LogSelfRemoveFile");
			if(ICQGetContactSettingByte(NULL, "LogSelfRemoveFile", 0) == 1)
				bLogSelfRemoveFile = TRUE;
			else
				bLogSelfRemoveFile = FALSE;				
			StoreDBCheckState(hwndDlg, IDC_LOG_IGNORECHECK_FILE, "LogIgnoreCheckFile");
			if(ICQGetContactSettingByte(NULL, "LogIgnoreCheckFile", 0) == 1)
				bLogIgnoreCheckFile= TRUE;
			else
				bLogIgnoreCheckFile = FALSE;
			StoreDBCheckState(hwndDlg, IDC_LOG_CHECKSTATUS_FILE, "LogStatusCheckFile");
			if(ICQGetContactSettingByte(NULL, "LogStatusCheckFile", 0) == 1)
				bLogStatusCheckFile = TRUE;
			else
				bLogStatusCheckFile = FALSE;
			StoreDBCheckState(hwndDlg, IDC_LOG_CLIENTCHANGE_FILE, "LogClientChangeFile");
			if(ICQGetContactSettingByte(NULL, "LogClientChangeFile", 0) == 1)
				bLogClientChangeFile = TRUE;
			else
				bLogClientChangeFile = FALSE;

			StoreDBCheckState(hwndDlg, IDC_LOG_AUTH_FILE, "LogAuthFile");
			if(ICQGetContactSettingByte(NULL, "LogAuthFile", 0) == 1)
				bLogAuthFile = TRUE;
			else
				bLogAuthFile = FALSE;

			StoreDBCheckState(hwndDlg, IDC_LOG_REQUEST_FILE, "LogRequestFile");
			if(ICQGetContactSettingByte(NULL, "LogRequestFile", 0) == 1)
				bLogInfoRequestFile = TRUE;
			else
				bLogInfoRequestHistory = FALSE;

			StoreDBCheckState(hwndDlg, IDC_LOG_REMOVE_HISTORY, "LogSelfRemoveHistory");
			if(ICQGetContactSettingByte(NULL, "LogSelfRemoveHistory", 0) == 1)
				bLogSelfRemoveHistory = TRUE;
			else
				bLogSelfRemoveHistory = FALSE;				
			StoreDBCheckState(hwndDlg, IDC_LOG_IGNORECHECK_HISTORY, "LogIgnoreCheckHistory");
			if(ICQGetContactSettingByte(NULL, "LogIgnoreCheckHistory", 0) == 1)
				bLogIgnoreCheckHistory= TRUE;
			else
				bLogIgnoreCheckHistory = FALSE;
			StoreDBCheckState(hwndDlg, IDC_LOG_CHECKSTATUS_HISTORY, "LogStatusCheckHistory");
			if(ICQGetContactSettingByte(NULL, "LogStatusCheckHistory", 0) == 1)
				bLogStatusCheckHistory = TRUE;
			else
				bLogStatusCheckHistory = FALSE;
			StoreDBCheckState(hwndDlg, IDC_LOG_CLIENTCHANGE_HISTORY, "LogClientChangeHistory");
			if(ICQGetContactSettingByte(NULL, "LogClientChangeHistory", 0) == 1)
				bLogClientChangeHistory = TRUE;
			else
				bLogClientChangeHistory = FALSE;

			StoreDBCheckState(hwndDlg, IDC_LOG_AUTH_HISTORY, "LogAuthHistory");
			if(ICQGetContactSettingByte(NULL, "LogAuthHistory", 0) == 1)
				bLogAuthHistory = TRUE;
			else
				bLogAuthHistory = FALSE;

			StoreDBCheckState(hwndDlg, IDC_LOG_REQUEST_HISTORY, "LogRequestHistory");
			if(ICQGetContactSettingByte(NULL, "LogRequestHistory", 0) == 1)
				bLogInfoRequestHistory = TRUE;
			else
				bLogInfoRequestHistory = FALSE;

			StoreDBCheckState(hwndDlg, IDC_LOG_TO_HCONTACT_HISTORY, "LogToHcontact");
			if(ICQGetContactSettingByte(NULL, "LogToHcontact", 0) == 1)
				bHcontactHistory = TRUE;
			else
				bHcontactHistory = FALSE;

	        {
		    char i[1024];
		    GetDlgItemText(hwndDlg, IDC_FILEPATH, i, sizeof(i) );
		    UniWriteContactSettingUtf(NULL, gpszICQProtoName, "EventsLog", i);
	        }

			return TRUE;
		}
		break;
	}
	return FALSE;
}

static const UINT icqVersionControls[] = {IDC_VERSION_CHECKBOX,IDC_SET_VERSION};
static BOOL CALLBACK DlgProcIcqClientIDOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BYTE bIdChanged = FALSE;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			ICQTranslateDialog(hwndDlg);
			LoadDBCheckState(hwndDlg, IDC_HIDEID, "Hide ID", 1);	
			LoadDBCheckState(hwndDlg, IDC_CUSTOM_CAP, "customcap", 1);
			LoadDBCheckState(hwndDlg,	IDC_VERSION_CHECKBOX, "CurrentVer", 0);
			{
				char* CIdComboBox[] =
				{
					"MirandaIM","unknown","QIP2005a","YSM","pyICQ","&RQ","Jimm","Trillian",
					"Licq","Kopete","ICQ for MAC","Miranda IM 6.6.6 [evil]","ICQ Rambler","ICQ 5.1","icq5 (abv)",
					"ICQ netvigator","Sim/MacOS X","Sim/Win32","Centericq","libicq2k","mChat","stICQ","KXicq2",
					"QIP PDA (Windows)","QIP Mobile (Java)","ICQ 2002","ICQ 6","ICQ for Pocket PC",
					"Anastasia","Virus","alicq","mICQ","StrICQ","vICQ","IM2","GAIM","ICQ99","WebICQ","SmartICQ",
					"IM+","uIM","TICQClient","IC@","PreludeICQ","Qnext","ICQ Lite","QIP Infium","JICQ",
					"SpamBot","MIP","Trillian Astra","R&Q","NanoICQ","IMadering","MirandaMobile"
				};
				int i;
				for (i=0; i<sizeof(CIdComboBox)/sizeof(CIdComboBox[0]); i++)
					ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_CHANGEID), CIdComboBox[i],i);
      }
// eternity : Miranda IM ICQ mods
      {
        char* MIMIdComboBox[] =
        {
          "original (Joe@Whale)", "BM Mod", "S7&SSS Mod",
          "S!N Mod", "Plus Mod", "eternity/PlusPlus++"
        };
        int i;
				for (i=0; i<sizeof(MIMIdComboBox)/sizeof(MIMIdComboBox[0]); i++)
			    ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_ENIFMIRCBX), MIMIdComboBox[i],i);
      }
      SendDlgItemMessage(hwndDlg, IDC_ENIFMIRCBX, CB_SETCURSEL, (DBGetContactSettingWord(NULL,gpszICQProtoName,"CurrentICQModID",0)),0);
// eternity : Miranda IM ICQ mods END
			if(DBGetContactSettingWord(NULL,gpszICQProtoName,"CurrentID",0))
				SetDlgItemText(hwndDlg, IDC_FAKEWARNING, Translate("Your client ID is changed!\nChanging client id can result in serious messaging problems!\n\nIf you experience such problems, set your client ID back to \"Miranda\""));
			else
				bIdChanged = TRUE;

			SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_SETCURSEL, (DBGetContactSettingWord(NULL,gpszICQProtoName,"CurrentID",0)),0);
			SetDlgItemInt(hwndDlg, IDC_SET_VERSION, (DBGetContactSettingWord(NULL,gpszICQProtoName,"setVersion", 0)), FALSE );
// eternity : custom capability edit control
			SetDlgItemText(hwndDlg, IDC_CUSTCAPEDIT, UniGetContactSettingUtf(NULL,"ICQCaps","capability", 0));
// eternity : custom capability edit control END
      return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
// eternity : changing ClientID ComboBox to cause ICQ Mod ComboBox enable/disable
      case IDC_CHANGEID:
      {
        int curID;
        curID = SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_GETCURSEL, 0, 0);
        EnableWindow( GetDlgItem( hwndDlg, IDC_ENIFMIR ), curID == 0 );
        EnableWindow( GetDlgItem( hwndDlg, IDC_ENIFMIRCBX ), curID == 0 );
      }
      break;
// eternity : changing ClientID ComboBox to cause ICQ Mod ComboBox enable/disable END
		}
		OptDlgChanged(hwndDlg);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			{
				int id = 0;
				int i, fakeId, fakeICQModID;
				fakeId = SendDlgItemMessage(hwndDlg, IDC_CHANGEID, CB_GETCURSEL, 0, 0);
				if(DBGetContactSettingWord(NULL, gpszICQProtoName, "CurrentID", 0) != fakeId)
				{
					id = 1;
					DBWriteContactSettingWord(NULL, gpszICQProtoName, "CurrentID", (WORD)fakeId);
				}
// eternity : Miranda IM ICQ mods
				fakeICQModID = SendDlgItemMessage(hwndDlg, IDC_ENIFMIRCBX, CB_GETCURSEL, 0, 0);
				if(DBGetContactSettingWord(NULL, gpszICQProtoName, "CurrentICQModID", 0) != fakeICQModID)
				{
					id = 1;
					DBWriteContactSettingWord(NULL, gpszICQProtoName, "CurrentICQModID", (WORD)fakeICQModID);
				}
// eternity : Miranda IM ICQ mods END
				StoreDBCheckState(hwndDlg, IDC_VERSION_CHECKBOX, "CurrentVer");
				if(DBGetContactSettingByte(NULL, gpszICQProtoName, "CurrentVer", 0))
					gbVerEnabled = 1;
				else 
					gbVerEnabled = 0;
				i = GetDlgItemInt(hwndDlg, IDC_SET_VERSION, 0, 0);
				if(DBGetContactSettingWord(NULL, gpszICQProtoName, "setVersion", 0) != i)
				{
					id = 1;
					DBWriteContactSettingWord(NULL, gpszICQProtoName, "setVersion", (WORD)i);
				}
				i = IsDlgButtonChecked(hwndDlg, IDC_HIDEID);
				if(DBGetContactSettingByte(NULL, gpszICQProtoName, "Hide ID", 0) != i)
				{
					id = 1;
					DBWriteContactSettingByte(NULL, gpszICQProtoName, "Hide ID", i);
				}
				if(DBGetContactSettingByte(NULL, gpszICQProtoName, "Hide ID", 0))
					gbHideIdEnabled = 1;
				else 
					gbHideIdEnabled = 0;
				
				i = IsDlgButtonChecked(hwndDlg, IDC_CUSTOM_CAP);
				if(DBGetContactSettingByte(NULL, gpszICQProtoName, "customcap", 0) != i)
				{
					id = 1;
					DBWriteContactSettingByte(NULL, gpszICQProtoName, "customcap", i);
				}
				if (id)
				{
					setUserInfo();
					{
						extern BYTE icq_ver();
						extern int icqGoingOnlineStatus;
						extern bSecureIM();
						icq_packet packet;
						WORD wStatus;
						int nPort = ICQGetContactSettingWord(NULL, "UserPort", 0);
						DWORD dwDirectCookie = rand() ^ (rand() << 16);
						// Get status
						wStatus = MirandaStatusToIcq(icqGoingOnlineStatus);
						wStatus = MirandaStatusToIcq(icqGoingOnlineStatus);
						serverPacketInit(&packet, 71);
						packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
						packDWord(&packet, 0x00060004);             // TLV 6: Status mode and security flags
						packWord(&packet, GetMyStatusFlags());      // Status flags
						packWord(&packet, wStatus);                 // Status
						packTLVWord(&packet, 0x0008, 0x0000);       // TLV 8: Error code
						packDWord(&packet, 0x000c0025);             // TLV C: Direct connection info
						packDWord(&packet, ICQGetContactSettingDword(NULL, "RealIP", 0));
						packDWord(&packet, nPort);
						packByte(&packet, DC_TYPE);                 // TCP/FLAG firewall settings
						packWord(&packet, (WORD)icq_ver());
						packDWord(&packet, dwDirectCookie);         // DC Cookie
						packDWord(&packet, WEBFRONTPORT);           // Web front port
						packDWord(&packet, CLIENTFEATURES);         // Client features
						{
							DWORD a;
							DWORD b;
							DWORD c;
							switch (DBGetContactSettingWord(NULL, gpszICQProtoName, "CurrentID", 0))  //client change dwFT
							{
							case 3:											//ysm
								a=0xFFFFFFAB;           //   Abused timestamp
								b=0x00000000;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 4:											//ICQ lite
								a=0x3AA773EE;           //   Abused timestamp
								b=0x00000000;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 5:													//&RQ
								a=0xFFFFFF7F;           //   Abused timestamp
								b=0x00000000;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 7:										//trillian
								a=0x3B75AC09;           //   Abused timestamp
								b=0x00000000;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 8:													//licq
								a=0x2C0BA3DD;           //   Abused timestamp
								b=0x7D800403;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 2:											//qip
								a=0x08000300;           //   Abused timestamp
								b=0x0000000E;       		// Abused timestamp
								c=0x0000000F;            // Timestamp
								break;
							case 46:       //QIP Infium
								a=0x00002328;           //   Abused timestamp
								b=0x0000000B;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 18:
							case 19:
								a=0x3AA773EE;           //   Abused timestamp
								b=0x3AA66380;       		// Abused timestamp
								c=0x00000000;            // Timestamp
								break;
							case 21:
								a=0x3BA8DBAF;
								b=0x3BEB5373;
								c=0x3BEB5262;
								break;
							case 22:
								a=0x3B4C4C0C;
								b=0x00000000;
								c=0x3B7248ed;
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
								a=0x00000000;
								b=0x00000000;
								c=0x00000000;
								break;
							case 29:
								a=0x44F523B0;
								b=0x44F523A6;
								c=0x44F523A6;
								break;
							case 30:           //alicq
								a=0xffffffbe;
								b=0x00090800;
								c=0x00000000;
								break;
							case 31:           //mICQ
								a=0xFFFFFF42;
								b=0x00000000;
								c=0x00000000;
								break;
							case 32:            //StrICQ 0.4
								a=0xFFFFFF8F;
								b=0x00000000;
								c=0x00000000;
								break;
							case 33:            //vICQ 0.43.0.0
								a=0x04031980;
								b=0x00000000;
								c=0x00000000;
								break;
							case 34:            //IM2
								a=0x3FF19BEB;
								b=0x00000000;
								c=0x3FF19BEB;
								break;
							case 35:           //GAIM
								a=0xffffffff;
								b=0xffffffff;
								c=0xffffffff;
								break;
							case 36:          //ICQ99
								a=0x3AA773EE;
								b=0x00000000;
								c=0x00000000;
								break;
							case 37:          //WebICQ
								a=0xFFFFFFFF;
								b=0x00000000;
								c=0x00000000;
								break;
							case 38:          //SmartICQ
								a=0xDDDDEEFF;
								b=0x00000000;
								c=0x00000000;
								break;
							case 39:           //IM+
								a=0x494D2B00;
								b=0x00000000;
								c=0x00000000;
								break;
							case 48:  //SpamBot
								a=0xffffffff;
								b=0x00000000;
								c=0x3B7248ED;
								break;
							case 51:           //R&Q
								a=0xFFFFF666;
								b=0x00000000;
								c=0x00000000;
								break;
							case 11:
								a=0xFFFFFFFF;         //Abused timestamp
								b=0x06060600;     	//Abused timestamp
								if(gbHideIdEnabled)
									c=gbUnicodeAPI?0x80000000:0x00000000;
								else
									c=bSecureIM()?0x5AFEC0DE:0x00000000;		
								break;
							default :								//miranda
								a=0xffffffff;         //Abused timestamp
								//b=ICQ_PLUG_VERSION;     	//Abused timestamp
								b=ICQ_THISPLUG_VERSION;     	//Abused timestamp //eternity
                if(gbHideIdEnabled)
									c=gbUnicodeAPI?0x80000000:0x00000000;
								else
									c=bSecureIM()?0x5AFEC0DE:0x00000000;
								break;
							}
							ICQWriteContactSettingDword(NULL,  "dwFT1",   a);
							ICQWriteContactSettingDword(NULL,  "dwFT2",   b);
							ICQWriteContactSettingDword(NULL,  "dwFT3",   c);
							packDWord(&packet, a);
							packDWord(&packet, b);
							packDWord(&packet, c);
						}
						packWord(&packet, 0x0000);                  // Unknown
						packTLVWord(&packet, 0x001F, 0x0000);
						sendServPacket(&packet);
					}
				}				
				if(fakeId != 0)
					SetDlgItemText(hwndDlg, IDC_FAKEWARNING, Translate("Your client ID is changed!\nChanging client id can result in serious messaging problems!\n\nIf you experience such problems, set your client ID back to \"Miranda\""));
				else
					SetDlgItemText(hwndDlg, IDC_FAKEWARNING, "");
                if(bIdChanged && (fakeId != 0))
				{
				MessageBox(0,Translate("Changing client ID can result in serious messaging problems!"),Translate("Warning"),MB_OK);
				bIdChanged = FALSE;
				}
			}
			{ // eternity : custom capability edit control
				char cap[24];
				GetDlgItemText(hwndDlg, IDC_CUSTCAPEDIT, cap, sizeof(cap));
				DBWriteContactSettingStringUtf(NULL,"ICQCaps", "capability", cap);
			} // eternity : custom capability edit control END
			return TRUE;
		}
		break;
	}
	return FALSE;
}


static const UINT icqContactsControls[] = {IDC_ADDSERVER,IDC_LOADFROMSERVER,IDC_SAVETOSERVER,IDC_UPLOADNOW,IDC_FORCEREFRESH};
static const UINT icqAvatarControls[] = {IDC_AUTOLOADAVATARS,IDC_BIGGERAVATARS,IDC_STRICTAVATARCHECK};
static const UINT icqAddTempControls[] = {IDC_ADDTEMP,IDC_TMP_CONTACTS_GROUP};
static BOOL CALLBACK DlgProcIcqContactsOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
	  {
    ICQTranslateDialog(hwndDlg);
    LoadDBCheckState(hwndDlg, IDC_ENABLE, "UseServerCList", DEFAULT_SS_ENABLED);
    LoadDBCheckState(hwndDlg, IDC_ADDSERVER, "ServerAddRemove", DEFAULT_SS_ADDSERVER);
    LoadDBCheckState(hwndDlg, IDC_LOADFROMSERVER, "LoadServerDetails", DEFAULT_SS_LOAD);
    LoadDBCheckState(hwndDlg, IDC_SAVETOSERVER, "StoreServerDetails", DEFAULT_SS_STORE);
    LoadDBCheckState(hwndDlg, IDC_ENABLEAVATARS, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED);
    LoadDBCheckState(hwndDlg, IDC_AUTOLOADAVATARS, "AvatarsAutoLoad", DEFAULT_LOAD_AVATARS);
    LoadDBCheckState(hwndDlg, IDC_BIGGERAVATARS, "AvatarsAllowBigger", DEFAULT_BIGGER_AVATARS);
    LoadDBCheckState(hwndDlg, IDC_STRICTAVATARCHECK, "StrictAvatarCheck", DEFAULT_AVATARS_CHECK);

    icq_EnableMultipleControls(hwndDlg, icqContactsControls, sizeof(icqContactsControls)/sizeof(icqContactsControls[0]), 
      ICQGetContactSettingByte(NULL, "UseServerCList", DEFAULT_SS_ENABLED)?TRUE:FALSE);
    icq_EnableMultipleControls(hwndDlg, icqAvatarControls, sizeof(icqAvatarControls)/sizeof(icqAvatarControls[0]), 
		ICQGetContactSettingByte(NULL, "AvatarsEnabled", DEFAULT_AVATARS_ENABLED)?TRUE:FALSE);
	CheckDlgButton(hwndDlg, IDC_DELETE_TMP_CONTACTS, (bTmpContacts == 0));
	CheckDlgButton(hwndDlg, IDC_ADD_TMP_CONTACTS, (bTmpContacts == 1));
	SetDlgItemText(hwndDlg, IDC_TMP_CONTACTS_GROUP, UniGetContactSettingUtf(NULL,gpszICQProtoName,"TmpContactsGroup", 0));
    LoadDBCheckState(hwndDlg, IDC_ADDTEMP, "AddTemp", 0);
	icq_EnableMultipleControls(hwndDlg, icqAddTempControls, sizeof(icqAddTempControls)/sizeof(icqAddTempControls[0]), bTmpContacts?TRUE:FALSE);

    if (icqOnline)
    {
      ShowWindow(GetDlgItem(hwndDlg, IDC_OFFLINETOENABLE), SW_SHOW);
      EnableDlgItem(hwndDlg, IDC_ENABLE, FALSE);
      EnableDlgItem(hwndDlg, IDC_ENABLEAVATARS, FALSE);
    }
    else
    {
      EnableDlgItem(hwndDlg, IDC_UPLOADNOW, FALSE);
    }
	if (!DBGetContactSettingDword(NULL,gpszICQProtoName,"SrvLastUpdate",0) &&
		!DBGetContactSettingWord(NULL,gpszICQProtoName,"SrvRecordCount",0))
		EnableWindow(GetDlgItem(hwndDlg, IDC_FORCEREFRESH), FALSE);

    return TRUE;
	  }
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_UPLOADNOW:
      ShowUploadContactsDialog();
      return TRUE;
  	case IDC_FORCEREFRESH:
  		DBWriteContactSettingDword(NULL,gpszICQProtoName,"SrvLastUpdate",0);
  		DBWriteContactSettingWord(NULL,gpszICQProtoName,"SrvRecordCount",0);
  		EnableWindow(GetDlgItem(hwndDlg, IDC_FORCEREFRESH), FALSE);
  		return TRUE;
    case IDC_ENABLE:
      icq_EnableMultipleControls(hwndDlg, icqContactsControls, sizeof(icqContactsControls)/sizeof(icqContactsControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ENABLE));
      if (icqOnline) 
        ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECTREQD), SW_SHOW);
      else 
        EnableDlgItem(hwndDlg, IDC_UPLOADNOW, FALSE);
      break;
    case IDC_ENABLEAVATARS:
      icq_EnableMultipleControls(hwndDlg, icqAvatarControls, sizeof(icqAvatarControls)/sizeof(icqAvatarControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ENABLEAVATARS));
      break;
    case IDC_ADD_TMP_CONTACTS:
  	  icq_EnableMultipleControls(hwndDlg, icqAddTempControls, sizeof(icqAddTempControls)/sizeof(icqAddTempControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ADD_TMP_CONTACTS));
  	  break;
    case IDC_DELETE_TMP_CONTACTS:
  	  icq_EnableMultipleControls(hwndDlg, icqAddTempControls, sizeof(icqAddTempControls)/sizeof(icqAddTempControls[0]), IsDlgButtonChecked(hwndDlg, IDC_ADD_TMP_CONTACTS));
  	  break;
    }
    OptDlgChanged(hwndDlg);
    break;

  case WM_NOTIFY:
    switch (((LPNMHDR)lParam)->code)
    {
    case PSN_APPLY:
      StoreDBCheckState(hwndDlg, IDC_ENABLE, "UseServerCList");
      StoreDBCheckState(hwndDlg, IDC_ADDSERVER, "ServerAddRemove");
      StoreDBCheckState(hwndDlg, IDC_LOADFROMSERVER, "LoadServerDetails");
      StoreDBCheckState(hwndDlg, IDC_SAVETOSERVER, "StoreServerDetails");
      StoreDBCheckState(hwndDlg, IDC_ENABLEAVATARS, "AvatarsEnabled");
      StoreDBCheckState(hwndDlg, IDC_AUTOLOADAVATARS, "AvatarsAutoLoad");
      StoreDBCheckState(hwndDlg, IDC_BIGGERAVATARS, "AvatarsAllowBigger");
      StoreDBCheckState(hwndDlg, IDC_STRICTAVATARCHECK, "StrictAvatarCheck");
	  if (IsDlgButtonChecked(hwndDlg, IDC_DELETE_TMP_CONTACTS))
	  {
		  ICQWriteContactSettingByte(NULL, "TempContacts", 0);
		  bTmpContacts = 0;
	  }
	  else if (IsDlgButtonChecked(hwndDlg, IDC_ADD_TMP_CONTACTS))
	  {
		  ICQWriteContactSettingByte(NULL, "TempContacts", 1);
		  bTmpContacts = 1;
	  }
	  {
		  char i[128];
		  GetDlgItemText(hwndDlg, IDC_TMP_CONTACTS_GROUP, i, sizeof(i) );
		  if(UniGetContactSettingUtf(NULL,gpszICQProtoName,"TmpContactsGroup", 0)!= i)
		  {
			  DBWriteContactSettingByte(NULL,gpszICQProtoName,"GroupCreated",0);
			  UniWriteContactSettingUtf(NULL,gpszICQProtoName, "TmpContactsGroup", i);
		  }
	  }
      StoreDBCheckState(hwndDlg, IDC_ADDTEMP, "AddTemp");
	  if(ICQGetContactSettingByte(NULL, "AddTemp", 0) == 1)
		  bAddTemp = TRUE;
	  else 
		  bAddTemp = FALSE;
      return TRUE;
    }
    break;
  }
  return FALSE;
}



static BOOL CALLBACK DlgProcIcqNewUINOpts(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			ICQTranslateDialog(hwndDlg);
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LOOKUPLINK:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_FORGOT_PASSWORD);
			return TRUE;
			
		case IDC_NEWUINLINK:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)URL_REGISTER_UIN);
			return TRUE;

		case IDC_PICTURE:
			if(icqOnline)
			{
              if (MessageBox(0,Translate("If you continue, you will lose current connection with a server,\nContinue?"),Translate("Warning"),MB_YESNO) == IDYES)
				icq_requestRegImage(hwndDlg);
			}
			else
                icq_requestRegImage(hwndDlg);
			break;
		case IDC_REGISTER:
			{
			    char password[128];
				char regimage[128];
				GetDlgItemText(hwndDlg, IDC_NEWUIN_PASS, password, sizeof(password));
				GetDlgItemText(hwndDlg, IDC_PICTURE_CONTENT, regimage, sizeof(regimage));
				icq_registerNewUin(password, regimage);
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		hwndRegUin = NULL;
		break;
	}
	return FALSE;
}


void ShowRegUinDialog(void)
{
	if (hwndRegUin == NULL)
	{
		hwndRegUin = CreateDialogUtf(hInst, MAKEINTRESOURCE(IDD_ICQ_REG_UIN), NULL, DlgProcIcqNewUINOpts);
	}
	SetForegroundWindow(hwndRegUin);
}

void ShowPopUpsOpts(void)
{
	if (hwndPopUpsOpts == NULL)
	{
		hwndPopUpsOpts = CreateDialogUtf(hInst, MAKEINTRESOURCE(IDD_OPT_POPUPS2), NULL, DlgProcIcqPopupOpts2);
	}
	SetForegroundWindow(hwndPopUpsOpts);
}
