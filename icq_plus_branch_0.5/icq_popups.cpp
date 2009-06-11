// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
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
// File name      : $URL$
// Revision       : $Revision$
// Last change on : $Date$
// Last change by : $Author$
//
// DESCRIPTION:
//
//  PopUp Plugin stuff
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

BOOL bPopUpService = FALSE;

void InitPopUps()
{
	if (ServiceExists(MS_POPUP_ADDPOPUPEX))
	{
		bPopUpService = TRUE;
	}
}

static const UINT icqPopupsControls[] = {
	IDC_POPUPS_LOG_ENABLED, IDC_POPUPS_SPAM_ENABLED, IDC_PREVIEW, IDC_USEWINCOLORS, IDC_USESYSICONS, IDC_POPUP_LOG0_TIMEOUT, 
	IDC_POPUP_LOG1_TIMEOUT, IDC_POPUP_LOG2_TIMEOUT, IDC_POPUP_LOG3_TIMEOUT, IDC_POPUP_SPAM_TIMEOUT,
	//
	IDC_POPUP_UNKNOWN_TIMEOUT, IDC_POPUP_VISIBILITY_TIMEOUT, IDC_POPUP_INFOREQUEST_TIMEOUT,
	IDC_POPUP_CLIENTCHANGE_TIMEOUT, IDC_POPUP_IGNORECHECK_TIMEOUT, IDC_POPUP_SELFREMOVE_TIMEOUT,
	IDC_POPUP_AUTH_TIMEOUT, IDC_POPUP_XUPDATER_TIMEOUT, IDC_POPUP_FOUND_TIMEOUT, IDC_POPUP_SCAN_TIMEOUT,
	IDC_POPUP_SPAM_ENABLED, IDC_POPUP_UNKNOWN_ENABLED, IDC_POPUP_VISIBILITY_ENABLED, IDC_POPUP_INFOREQUEST_ENABLED,
	IDC_POPUP_CLIENTCHANGE_ENABLED, IDC_POPUP_IGNORECHECK_ENABLED, IDC_POPUP_SELFREMOVE_ENABLED, IDC_POPUP_AUTH_ENABLED,
	IDC_POPUP_XUPDATER_ENABLED, IDC_POPUP_FOUND_ENABLED, IDC_POPUP_SCAN_ENABLED,
	IDC_POPUPS_UIN_ENABLED, IDC_POPUPS_SHOW_FOR_HIDDEN_ENABLED, IDC_POPUPS_SHOW_FOR_IGNORED_ENABLED, IDC_POPUPS_SHOW_FOR_NOTONLIST_ENABLED

};

static const UINT icqPopupColorControls[] = {
	IDC_POPUP_LOG0_TEXTCOLOR, IDC_POPUP_LOG1_TEXTCOLOR, IDC_POPUP_LOG2_TEXTCOLOR, IDC_POPUP_LOG3_TEXTCOLOR, IDC_POPUP_SPAM_TEXTCOLOR,
	IDC_POPUP_LOG0_BACKCOLOR, IDC_POPUP_LOG1_BACKCOLOR, IDC_POPUP_LOG2_BACKCOLOR, IDC_POPUP_LOG3_BACKCOLOR, IDC_POPUP_SPAM_BACKCOLOR,
	//
	IDC_POPUP_UNKNOWN_TEXTCOLOR, IDC_POPUP_VISIBILITY_TEXTCOLOR, IDC_POPUP_INFOREQUEST_TEXTCOLOR,
	IDC_POPUP_CLIENTCHANGE_TEXTCOLOR, IDC_POPUP_IGNORECHECK_TEXTCOLOR, IDC_POPUP_SELFREMOVE_TEXTCOLOR,
	IDC_POPUP_AUTH_TEXTCOLOR, IDC_POPUP_XUPDATER_TEXTCOLOR, IDC_POPUP_FOUND_TEXTCOLOR, IDC_POPUP_SCAN_TEXTCOLOR,
	IDC_POPUP_UNKNOWN_BACKCOLOR, IDC_POPUP_VISIBILITY_BACKCOLOR, IDC_POPUP_INFOREQUEST_BACKCOLOR,
	IDC_POPUP_CLIENTCHANGE_BACKCOLOR, IDC_POPUP_IGNORECHECK_BACKCOLOR, IDC_POPUP_SELFREMOVE_BACKCOLOR,
	IDC_POPUP_AUTH_BACKCOLOR, IDC_POPUP_XUPDATER_BACKCOLOR, IDC_POPUP_FOUND_BACKCOLOR, IDC_POPUP_SCAN_BACKCOLOR
};

BOOL CALLBACK DlgProcIcqPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BYTE bEnabled;
	CIcqProto* ppro = (CIcqProto*)GetWindowLong( hwndDlg, GWL_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

		ppro->hPopupDialog = hwndDlg;

		CheckDlgButton(hwndDlg, IDC_POPUPS_LOG_ENABLED, ppro->getSettingByte(NULL,"PopupsLogEnabled",DEFAULT_LOG_POPUPS_ENABLED));
		//
		CheckDlgButton(hwndDlg, IDC_POPUPS_UIN_ENABLED, ppro->getSettingByte(NULL,"ShowUinInPopups",DEFAULT_SHOW_UIN_IN_POPUPS));
		CheckDlgButton(hwndDlg, IDC_POPUPS_SHOW_FOR_HIDDEN_ENABLED, ppro->getSettingByte(NULL,"PopUpForHidden",DEFAULT_SHOW_FOR_HIDDEN_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUPS_SHOW_FOR_IGNORED_ENABLED, ppro->getSettingByte(NULL,"PopUpForIgnored",DEFAULT_SHOW_FOR_IGNORED_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUPS_SHOW_FOR_NOTONLIST_ENABLED, ppro->getSettingByte(NULL,"PopUpForNotOnList",DEFAULT_SHOW_FOR_NOTONLIST_ENABLED));
		//
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG0_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups0TextColor",DEFAULT_LOG0_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG0_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups0BackColor",DEFAULT_LOG0_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG0_TIMEOUT, ppro->getSettingDword(NULL,"Popups0Timeout",DEFAULT_LOG0_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG1_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups1TextColor",DEFAULT_LOG1_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG1_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups1BackColor",DEFAULT_LOG1_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG1_TIMEOUT, ppro->getSettingDword(NULL,"Popups1Timeout",DEFAULT_LOG1_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG2_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups2TextColor",DEFAULT_LOG2_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG2_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups2BackColor",DEFAULT_LOG2_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG2_TIMEOUT, ppro->getSettingDword(NULL,"Popups2Timeout",DEFAULT_LOG2_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG3_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups3TextColor",DEFAULT_LOG3_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG3_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups3BackColor",DEFAULT_LOG3_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG3_TIMEOUT, ppro->getSettingDword(NULL,"Popups3Timeout",DEFAULT_LOG3_TIMEOUT),FALSE);
		bEnabled = ppro->getSettingByte(NULL,"PopupsWinColors",DEFAULT_POPUPS_WIN_COLORS);
		CheckDlgButton(hwndDlg, IDC_USEWINCOLORS, bEnabled);
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled);
		icq_EnableMultipleControls(ppro->hPopupMoreDialog, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled); //
		CheckDlgButton(hwndDlg, IDC_USESYSICONS, ppro->getSettingByte(NULL,"PopupsSysIcons",DEFAULT_POPUPS_SYS_ICONS));
		bEnabled = ppro->getSettingByte(NULL,"PopupsEnabled",DEFAULT_POPUPS_ENABLED);
		CheckDlgButton(hwndDlg, IDC_POPUPS_ENABLED, bEnabled);
		icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);
		icq_EnableMultipleControls(ppro->hPopupMoreDialog, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled); //
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
		icq_EnableMultipleControls(ppro->hPopupMoreDialog, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS)); //
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PREVIEW:
			{
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Note"),    LOG_NOTE);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Warning"), LOG_WARNING);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Error"),   LOG_ERROR);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Fatal"),   LOG_FATAL);
			}
			return FALSE;

		case IDC_POPUPS_ENABLED:
			bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
			icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);
			icq_EnableMultipleControls(ppro->hPopupMoreDialog, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled); //

		case IDC_USEWINCOLORS:
			bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
			icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
			icq_EnableMultipleControls(ppro->hPopupMoreDialog, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS)); //
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			ppro->setSettingByte(NULL,"PopupsEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED));
			ppro->setSettingByte(NULL,"PopupsLogEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_LOG_ENABLED));
			//
			ppro->setSettingByte(NULL,"ShowUinInPopups",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_UIN_ENABLED));
			ppro->setSettingByte(NULL,"PopUpForHidden",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_SHOW_FOR_HIDDEN_ENABLED));
			ppro->setSettingByte(NULL,"PopUpForIgnored",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_SHOW_FOR_IGNORED_ENABLED));
			ppro->setSettingByte(NULL,"PopUpForNotOnList",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_SHOW_FOR_NOTONLIST_ENABLED));
			//
			ppro->setSettingDword(NULL,"Popups0TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG0_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups0BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG0_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups0Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG0_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"Popups1TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG1_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups1BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG1_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups1Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG1_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"Popups2TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG2_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups2BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG2_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups2Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG2_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"Popups3TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG3_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups3BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG3_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups3Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG3_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsWinColors",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
			ppro->setSettingByte(NULL,"PopupsSysIcons",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USESYSICONS));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK DlgProcIcqPopupMoreOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BYTE bEnabled;
	CIcqProto* ppro = (CIcqProto*)GetWindowLong( hwndDlg, GWL_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		ICQTranslateDialog(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

		ppro->hPopupMoreDialog = hwndDlg;

		SendDlgItemMessage(hwndDlg, IDC_POPUP_SPAM_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsSpamTextColor",DEFAULT_SPAM_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SPAM_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsSpamBackColor",DEFAULT_SPAM_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_SPAM_TIMEOUT, ppro->getSettingDword(NULL,"PopupsSpamTimeout",DEFAULT_SPAM_TIMEOUT),FALSE);
		//
		SendDlgItemMessage(hwndDlg, IDC_POPUP_UNKNOWN_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsUnknownTextColor",DEFAULT_UNKNOWN_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_UNKNOWN_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsUnknownBackColor",DEFAULT_UNKNOWN_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_UNKNOWN_TIMEOUT, ppro->getSettingDword(NULL,"PopupsUnknownTimeout",DEFAULT_UNKNOWN_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_VISIBILITY_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsVisibilityTextColor",DEFAULT_VISIBILITY_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_VISIBILITY_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsVisibilityBackColor",DEFAULT_VISIBILITY_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_VISIBILITY_TIMEOUT, ppro->getSettingDword(NULL,"PopupsVisibilityTimeout",DEFAULT_VISIBILITY_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_INFOREQUEST_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsInfoRequestTextColor",DEFAULT_INFOREQUEST_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_INFOREQUEST_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsInfoRequestBackColor",DEFAULT_INFOREQUEST_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_INFOREQUEST_TIMEOUT, ppro->getSettingDword(NULL,"PopupsInfoRequestTimeout",DEFAULT_INFOREQUEST_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_CLIENTCHANGE_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsClientChangeTextColor",DEFAULT_CLIENTCHANGE_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_CLIENTCHANGE_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsClientChangeBackColor",DEFAULT_CLIENTCHANGE_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_CLIENTCHANGE_TIMEOUT, ppro->getSettingDword(NULL,"PopupsClientChangeTimeout",DEFAULT_CLIENTCHANGE_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_IGNORECHECK_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsIgnoreCheckTextColor",DEFAULT_IGNORECHECK_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_IGNORECHECK_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsIgnoreCheckBackColor",DEFAULT_IGNORECHECK_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_IGNORECHECK_TIMEOUT, ppro->getSettingDword(NULL,"PopupsIgnoreCheckTimeout",DEFAULT_IGNORECHECK_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SELFREMOVE_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsSelfRemoveTextColor",DEFAULT_SELFREMOVE_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SELFREMOVE_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsSelfRemoveBackColor",DEFAULT_SELFREMOVE_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_SELFREMOVE_TIMEOUT, ppro->getSettingDword(NULL,"PopupsSelfRemoveTimeout",DEFAULT_SELFREMOVE_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_AUTH_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsAuthTextColor",DEFAULT_AUTH_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_AUTH_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsAuthBackColor",DEFAULT_AUTH_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_AUTH_TIMEOUT, ppro->getSettingDword(NULL,"PopupsAuthTimeout",DEFAULT_AUTH_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_XUPDATER_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsXUpdaterTextColor",DEFAULT_XUPDATER_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_XUPDATER_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsXUpdaterBackColor",DEFAULT_XUPDATER_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_XUPDATER_TIMEOUT, ppro->getSettingDword(NULL,"PopupsXUpdaterTimeout",DEFAULT_XUPDATER_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_FOUND_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsFoundTextColor",DEFAULT_FOUND_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_FOUND_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsFoundBackColor",DEFAULT_FOUND_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_FOUND_TIMEOUT, ppro->getSettingDword(NULL,"PopupsFoundTimeout",DEFAULT_FOUND_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SCAN_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsScanTextColor",DEFAULT_SCAN_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SCAN_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsScanBackColor",DEFAULT_SCAN_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_SCAN_TIMEOUT, ppro->getSettingDword(NULL,"PopupsScanTimeout",DEFAULT_SCAN_TIMEOUT),FALSE);
		CheckDlgButton(hwndDlg, IDC_POPUP_SPAM_ENABLED, ppro->getSettingByte(NULL,"PopupsSpamEnabled", DEFAULT_SPAM_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_UNKNOWN_ENABLED, ppro->getSettingByte(NULL,"PopupsUnknownEnabled", DEFAULT_UNKNOWN_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_VISIBILITY_ENABLED, ppro->getSettingByte(NULL,"PopupsVisibilityEnabled", DEFAULT_VISIBILITY_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_INFOREQUEST_ENABLED, ppro->getSettingByte(NULL,"PopupsInfoRequestEnabled", DEFAULT_INFOREQUEST_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_CLIENTCHANGE_ENABLED, ppro->getSettingByte(NULL,"PopupsClientChangeEnabled", DEFAULT_CLIENTCHANGE_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_IGNORECHECK_ENABLED, ppro->getSettingByte(NULL,"PopupsIgnoreCheckEnabled", DEFAULT_IGNORECHECK_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_SELFREMOVE_ENABLED, ppro->getSettingByte(NULL,"PopupsSelfRemoveEnabled", DEFAULT_SELFREMOVE_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_AUTH_ENABLED, ppro->getSettingByte(NULL,"PopupsAuthEnabled", DEFAULT_AUTH_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_XUPDATER_ENABLED, ppro->getSettingByte(NULL,"PopupsXUpdaterEnabled", DEFAULT_XUPDATER_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_FOUND_ENABLED, ppro->getSettingByte(NULL,"PopupsFoundEnabled", DEFAULT_FOUND_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUP_SCAN_ENABLED, ppro->getSettingByte(NULL,"PopupsScanEnabled", DEFAULT_SCAN_POPUPS_ENABLED));
		//
		bEnabled = ppro->getSettingByte(NULL,"PopupsWinColors",DEFAULT_POPUPS_WIN_COLORS);
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled);
		icq_EnableMultipleControls(ppro->hPopupDialog, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled); //
		bEnabled = ppro->getSettingByte(NULL,"PopupsEnabled",DEFAULT_POPUPS_ENABLED);
		icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);
		icq_EnableMultipleControls(ppro->hPopupDialog, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled); //
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
		icq_EnableMultipleControls(ppro->hPopupDialog, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS)); //
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDC_PREVIEW:
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample 'Spambot Found' Event"),      POPTYPE_SPAM);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample 'Unknown User Found' Event"), POPTYPE_UNKNOWN);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample 'User Found' Event"),         POPTYPE_FOUND);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Users Status Scan Started"),  POPTYPE_SCAN);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Users Status Scan Complete"), POPTYPE_SCAN);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Visibility Event"),           POPTYPE_VISIBILITY);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Client Changed Event"),       POPTYPE_CLIENTCHANGE);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Info Request Event"),         POPTYPE_INFOREQUEST);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Ignore Check Event"),         POPTYPE_IGNORECHECK);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Self Remove Event"),          POPTYPE_SELFREMOVE);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Authorization Event"),        POPTYPE_AUTH);
			ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Custom Status Updater"),      POPTYPE_XUPDATER);
			return FALSE;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {

		case PSN_APPLY:
			ppro->setSettingByte(NULL,"PopupsSpamEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_SPAM_ENABLED));
			ppro->setSettingDword(NULL,"PopupsSpamTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SPAM_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsSpamBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SPAM_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsSpamTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_SPAM_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsUnknownEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_UNKNOWN_ENABLED));
			ppro->setSettingDword(NULL,"PopupsUnknownTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_UNKNOWN_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsUnknownBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_UNKNOWN_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsUnknownTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_UNKNOWN_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsVisibilityEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_VISIBILITY_ENABLED));
			ppro->setSettingDword(NULL,"PopupsVisibilityTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_VISIBILITY_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsVisibilityBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_VISIBILITY_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsVisibilityTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_VISIBILITY_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsInfoRequestEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_INFOREQUEST_ENABLED));
			ppro->setSettingDword(NULL,"PopupsInfoRequestTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_INFOREQUEST_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsInfoRequestBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_INFOREQUEST_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsInfoRequestTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_INFOREQUEST_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsClientChangeEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_CLIENTCHANGE_ENABLED));
			ppro->setSettingDword(NULL,"PopupsClientChangeTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_CLIENTCHANGE_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsClientChangeBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_CLIENTCHANGE_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsClientChangeTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_CLIENTCHANGE_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsIgnoreCheckEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_IGNORECHECK_ENABLED));
			ppro->setSettingDword(NULL,"PopupsIgnoreCheckTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_IGNORECHECK_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsIgnoreCheckBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_IGNORECHECK_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsIgnoreCheckTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_IGNORECHECK_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsSelfRemoveEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_SELFREMOVE_ENABLED));
			ppro->setSettingDword(NULL,"PopupsSelfRemoveTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SELFREMOVE_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsSelfRemoveBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SELFREMOVE_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsSelfRemoveTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_SELFREMOVE_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsAuthEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_AUTH_ENABLED));
			ppro->setSettingDword(NULL,"PopupsAuthTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_AUTH_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsAuthBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_AUTH_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsAuthTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_AUTH_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsXUpdaterEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_XUPDATER_ENABLED));
			ppro->setSettingDword(NULL,"PopupsXUpdaterTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_XUPDATER_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsXUpdaterBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_XUPDATER_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsXUpdaterTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_XUPDATER_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsFoundEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_FOUND_ENABLED));
			ppro->setSettingDword(NULL,"PopupsFoundTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_FOUND_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsFoundBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_FOUND_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsFoundTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_FOUND_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsScanEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUP_SCAN_ENABLED));
			ppro->setSettingDword(NULL,"PopupsScanTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SCAN_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsScanBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SCAN_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsScanTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_SCAN_TIMEOUT, NULL, FALSE));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int CIcqProto::ShowPopUpMsg(HANDLE hContact, const char *szTitle, const char *szMsg, BYTE bType)
{
	if (bPopUpService && getSettingByte(NULL, "PopupsEnabled", DEFAULT_POPUPS_ENABLED))
	{
		POPUPDATAEX ppd = {0};
		POPUPDATAW ppdw = {0};
		LPCTSTR rsIcon;
		char szPrefix[32], szSetting[32], szTitleModified[64];

		strcpy(szPrefix, "Popups");
		ppd.iSeconds = 0;

		switch(bType) {
		case LOG_NOTE:
			rsIcon = MAKEINTRESOURCE(IDI_INFORMATION);
			ppd.colorBack = DEFAULT_LOG0_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG0_TEXT_COLORS;
			strcat(szPrefix, "0");
			break;

		case LOG_WARNING:
			rsIcon = MAKEINTRESOURCE(IDI_WARNING);
			ppd.colorBack = DEFAULT_LOG1_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG1_TEXT_COLORS;
			strcat(szPrefix, "1");
			break;

		case LOG_ERROR:
			rsIcon = MAKEINTRESOURCE(IDI_ERROR);
			ppd.colorBack = DEFAULT_LOG2_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG2_TEXT_COLORS;
			strcat(szPrefix, "2");
			break;

		case LOG_FATAL:
			rsIcon = MAKEINTRESOURCE(IDI_ERROR);
			ppd.colorBack = DEFAULT_LOG3_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG3_TEXT_COLORS;
			strcat(szPrefix, "3");
			break;

		case POPTYPE_SPAM:
			if (!(BOOL)getSettingByte(NULL, "PopupsSpamEnabled", DEFAULT_SPAM_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_WARNING);
			ppd.colorBack = DEFAULT_SPAM_BACK_COLORS;
			ppd.colorText = DEFAULT_SPAM_TEXT_COLORS;
			strcat(szPrefix, "Spam");
			break;

		case POPTYPE_UNKNOWN:
			if (!(BOOL)getSettingByte(NULL, "PopupsUnknownEnabled", DEFAULT_UNKNOWN_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_WARNING);
			ppd.colorBack = DEFAULT_UNKNOWN_BACK_COLORS;
			ppd.colorText = DEFAULT_UNKNOWN_TEXT_COLORS;
			strcat(szPrefix, "Unknown");
			break;

		case POPTYPE_VISIBILITY:
			if (!(BOOL)getSettingByte(NULL, "PopupsVisibilityEnabled", DEFAULT_VISIBILITY_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_HIDDEN);
			ppd.colorBack = DEFAULT_VISIBILITY_BACK_COLORS;
			ppd.colorText = DEFAULT_VISIBILITY_TEXT_COLORS;
			strcat(szPrefix, "Visibility");
			break;

		case POPTYPE_INFOREQUEST:
			if (!(BOOL)getSettingByte(NULL, "PopupsInfoRequestEnabled", DEFAULT_INFOREQUEST_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_ICQ);
			ppd.colorBack = DEFAULT_INFOREQUEST_BACK_COLORS;
			ppd.colorText = DEFAULT_INFOREQUEST_TEXT_COLORS;
			strcat(szPrefix, "InfoRequest");
			break;

		case POPTYPE_CLIENTCHANGE:
			if (!(BOOL)getSettingByte(NULL, "PopupsClientChangeEnabled", DEFAULT_CLIENTCHANGE_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_ICQ);
			ppd.colorBack = DEFAULT_CLIENTCHANGE_BACK_COLORS;
			ppd.colorText = DEFAULT_CLIENTCHANGE_TEXT_COLORS;
			strcat(szPrefix, "ClientChange");
			break;

		case POPTYPE_IGNORECHECK:
			if (!(BOOL)getSettingByte(NULL, "PopupsIgnoreCheckEnabled", DEFAULT_IGNORECHECK_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_ICQ);
			ppd.colorBack = DEFAULT_IGNORECHECK_BACK_COLORS;
			ppd.colorText = DEFAULT_IGNORECHECK_TEXT_COLORS;
			strcat(szPrefix, "IgnoreCheck");
			break;

		case POPTYPE_SELFREMOVE:
			if (!(BOOL)getSettingByte(NULL, "PopupsSelfRemoveEnabled", DEFAULT_SELFREMOVE_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_ICQ);
			ppd.colorBack = DEFAULT_SELFREMOVE_BACK_COLORS;
			ppd.colorText = DEFAULT_SELFREMOVE_TEXT_COLORS;
			strcat(szPrefix, "SelfRemove");
			break;

		case POPTYPE_AUTH:
			if (!(BOOL)getSettingByte(NULL, "PopupsAuthEnabled", DEFAULT_AUTH_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_AUTH_ASK);
			ppd.colorBack = DEFAULT_AUTH_BACK_COLORS;
			ppd.colorText = DEFAULT_AUTH_TEXT_COLORS;
			strcat(szPrefix, "Auth");
			break;

		case POPTYPE_XUPDATER:
			if (!(BOOL)getSettingByte(NULL, "PopupsXUpdaterEnabled", DEFAULT_XUPDATER_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_HIDDEN);
			ppd.colorBack = DEFAULT_XUPDATER_BACK_COLORS;
			ppd.colorText = DEFAULT_XUPDATER_TEXT_COLORS;
			strcat(szPrefix, "XUpdater");
			break;

		case POPTYPE_FOUND:
			if (!(BOOL)getSettingByte(NULL, "PopupsFoundEnabled", DEFAULT_FOUND_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_HIDDEN);
			ppd.colorBack = DEFAULT_FOUND_BACK_COLORS;
			ppd.colorText = DEFAULT_FOUND_TEXT_COLORS;
			strcat(szPrefix, "Found");
			break;

		case POPTYPE_SCAN:
			if (!(BOOL)getSettingByte(NULL, "PopupsScanEnabled", DEFAULT_SCAN_POPUPS_ENABLED))
				return -1;
			rsIcon = MAKEINTRESOURCE(IDI_SCAN);
			ppd.colorBack = DEFAULT_SCAN_BACK_COLORS;
			ppd.colorText = DEFAULT_SCAN_TEXT_COLORS;
			strcat(szPrefix, "Scan");
			break;

		default:
			return -1;
		}
		if (!getSettingByte(NULL, "PopupsSysIcons", DEFAULT_POPUPS_SYS_ICONS))
			ppd.lchIcon = GetIcon( PLI_PROTOCOL | PLIF_SMALL | PLIF_ICOLIB );
		else
			ppd.lchIcon = (HICON)LoadImage( NULL, rsIcon, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		if (getSettingByte(NULL, "PopupsWinColors", DEFAULT_POPUPS_WIN_COLORS))
		{
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			ppd.colorBack = GetSysColor(COLOR_WINDOW);
		}
		else
		{
			strcpy(szSetting, szPrefix);
			strcat(szSetting, "TextColor");
			ppd.colorText = getSettingDword(NULL, szSetting, ppd.colorText);
			strcpy(szSetting, szPrefix);
			strcat(szSetting, "BackColor");
			ppd.colorBack = getSettingDword(NULL, szSetting, ppd.colorBack);
		}
		strcpy(szSetting, szPrefix);
		strcat(szSetting, "Timeout");
		ppd.iSeconds = getSettingDword(NULL, szSetting, ppd.iSeconds);

		//if (ppd.iSeconds == 0) ppd.iSeconds = 10;
		//hPopUpWnd = WindowList_Find(hPopUpsList, hContact);
		//if ((bType == POPTYPE_VIS) && hPopUpWnd)
		//	return -1;

		if (getSettingByte(NULL, "ShowUinInPopups", DEFAULT_SHOW_UIN_IN_POPUPS))
		{
			DWORD dwUin = this->getSettingDword(hContact, "UIN", 0);
			if (dwUin)
				mir_snprintf(szTitleModified, sizeof(szTitleModified), "%s (UIN: %u)", szTitle, dwUin);
			else
				lstrcpyA(szTitleModified, szTitle);
		}

#if defined( _UNICODE )
		// call unicode popup module - only on unicode OS otherwise it will not work properly :(
		// due to Popup Plug bug in ADDPOPUPW implementation
		if ( ServiceExists( MS_POPUP_ADDPOPUPW ))
		{
			char str[MAX_PATH];

			make_unicode_string_static(ICQTranslateUtfStatic(szTitleModified, str, MAX_PATH), ppdw.lpwzContactName, MAX_CONTACTNAME);
			make_unicode_string_static(ICQTranslateUtfStatic(szMsg, str, MAX_PATH), ppdw.lpwzText, MAX_SECONDLINE);
			ppdw.lchContact = hContact;
			ppdw.lchIcon = ppd.lchIcon;
			ppdw.colorBack = ppd.colorBack;
			ppdw.colorText = ppd.colorText;
			ppdw.PluginWindowProc = NULL;
			ppdw.PluginData = NULL;
			ppdw.iSeconds = ppd.iSeconds;
			return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&ppdw, 0);
		}
		else 
#endif
		{
			char str[MAX_PATH];

			utf8_decode_static(ICQTranslateUtfStatic(szTitle, str, MAX_PATH), ppd.lpzContactName, MAX_CONTACTNAME);
			utf8_decode_static(ICQTranslateUtfStatic(szMsg, str, MAX_PATH), ppd.lpzText, MAX_SECONDLINE);
			ppd.lchContact = hContact;
			ppd.PluginWindowProc = NULL;
			ppd.PluginData = NULL;

			return CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
		}
	}
	return -1; // Failure
}
