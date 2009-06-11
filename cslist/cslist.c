/* ########################################################################## 80

                              Custom Status List
                              __________________

  Custom Status List plugin for Miranda-IM (www.miranda-im.org)
  Follower of Custom Status History List by HANAX
  Copyright © 2006,2007 HANAX
  Copyright © 2007,2008 jarvis

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  ----------------------------------------------------------------------------  

  File name      : $URL$
  Revision       : $Rev$
  Last change on : $Date$
  Last change by : $Author$

  ----------------------------------------------------------------------------  


  DESCRIPTION:

  Offers List of your Custom Statuses.

  ----------------------------------------------------------------------------  


  HISTORY:

  TODO
  ----
  - duplicities detection (during Import, adding, ..)
  - stage 2 of code revision
  - save window positions
  - resetting custom status on when selected users log in
  - exactly, CSList-like system
  - optionally set Away/NA/DND/Occ
  - saving dialog positions
  - all TODOs listed in this source x)
  - feature requests maybe ..

  0.0.0.18 - fixing trip :)
  --------
  - icon selector shows only icons, that are supported by installed ICQ plugin
  - fixed memory leak and possible crash when modifying status and then importing 0 DB entries (helpItem not nulled)
  - merged Add/Modify processes together :)
  - Unicode build, finally :)

  0.0.0.17 - I become to be useful :)
  --------
  - CSList menu item can be placed elsewhere, also instead of Xstatus menu (currently ICQ eternity/PlusPlus++ mod only)
  - importing statuses from database (ICQ key)
  - list sorting (Procedure + calls)
  - dialog openable only once :)
  - new icons by inducti0n :)
  - no more infinite width when adding x-message
  - new Options dialog

  0.0.0.16 - rework
  --------
  - code redesigned
  - fix for Windows Vista crash (free) and ANSI Windows crash (init extended controls)
  - preimplementation for new features that will come (favourites, ...)
  
  0.0.0.15 - make me nice
  --------
  - stage 1 of code revision
  - Options dialog (in development) :)
  - IcoLib group renamed to CSLIST_MODULE_LONG_NAME
  - status titles in Add/Modify ComboBox were strangely corrected x)
  - possibility to turn off "Release Notes" dialog

  0.0.0.14 - will you use me? :)
  --------
  - new lines in message input creatable via Enter
  - NULLED XSTATUS now available :)
  
  0.0.0.13 - oooops x))
  --------
  - hotfix for Miranda cores < 0.7
    - CSList appears in Main Menu
    - fixed icons not loaded when IcoLib is missing
  
  0.0.0.12 - goin' my way now x)
  --------
  - DB saving method/format slightly modified
    [+] multi-line messages now working
    [-] LOST compatibility with HANAX's plugin (but is auto-imported if exists)
    NOTE: I think "importing-only" of older releases will stay,
          I have many Feature Requests that would need changing
          DB format anyway, so this problem looks solved x)
  - "Release notes" dialog on first run
  - Updater support
  
  0.0.0.11 - will this work? x)
  --------
  - some similar procedures merged together :P 
  - another procedures more generalized 8) 
  - separated Add/Modify dialog finally in use :)
    [+] solves some bahaviour problems x)
  
  0.0.0.10 - ohhh, basics ]:)
  --------
  - icons support - complete xstatus service :) 
    [+] no more static angry ;) :D
  - iconized buttons - supporting IcoLib :)
  - better method for DB entry format - old was little.."dizzy" x) :D
  - TEMPORARILY DISABLED modifying of items - needs some fixes x))
  
  0.0.0.9 - working a little? O:)
  -------
  - enhanced List Control behaviour (selecting, deselecting, editation, adding)
  - LOADING from + SAVING to DB!
    [+] my parser is also compatible with HANAX plugin's DB entries ;)
  
  0.0.0.8 - behaviour testing :o)
  -------
  - setting status by double-clicking list item
  - much better procedures, debugging, better exceptions, dialog behaviour
  
  0.0.0.7 - crash testing :o)
  -------
  - basic functions coding
  
  0.0.0.6 - resource testing
  -------
  - basic resources modeling


// ############################ INCLUDES & DEFINITIONS ###################### */

#include "cslist.h"


// ############################### MAIN ########################################

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {
  hInst = hinstDLL;
  return TRUE;
}


// ########################### MIM MENU COMMAND ################################

static int PluginMenuCommand( WPARAM wParam, LPARAM lParam ) {
  if ( !opened )
    DialogBox( hInst, MAKEINTRESOURCE( IDD_CSLIST ), NULL, ( DLGPROC )CSListProc );
  else if ( opened == 1 )
  {
    opened++;
    MessageBox( NULL, _T( "Some instance of Custom Status List is already opened." ), _T( "Custom Status List" ), MB_OK );
    opened--;
  }
  return 0;
}


// ############################# PLUGIN INFO ###################################

__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion ) {
  gMirandaVersion = mirandaVersion;
  return &pluginInfoEx;
}

__declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion ) {
  gMirandaVersion = mirandaVersion;
	return &pluginInfo;
}


// ############################# INTERFACES ####################################

__declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void ) {
  return interfaces;
}


// ############################### LOADER ######################################

int __declspec( dllexport ) Load( PLUGINLINK *link ) {
  pluginLink = link;

  { // are we running under Unicode core?
		char szVer[MAX_PATH];

		CallService( MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, ( LPARAM )szVer );
		_strlwr( szVer ); // make sure it is lowercase
		gbUnicodeCore = ( strstr( szVer, "unicode" ) != NULL );
    if ( !gbUnicodeCore )
    {
      pluginInfoEx.flags = 0;
      pluginInfo.flags = 0;
    }
  }

  hSvcShowList = CreateServiceFunction( MS_CSLIST_SHOWLIST, PluginMenuCommand );

  cslist_init_icons();

  if ( DBGetContactSettingByte( NULL, "ICQ", "XStatusEnabled", 1 ) )
  { // if xstatuses are enabled, declare, if we can use status menu or main menu only
    if ( bStatusMenu = ServiceExists( MS_CLIST_ADDSTATUSMENUITEM ) )
      hHookMenuBuild = HookEvent( ME_CLIST_PREBUILDSTATUSMENU, cslist_init_menu_item );
    else
      cslist_init_menu_item( 0, 0 );
  }

  hHookOnPluginsLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, onPluginsLoaded );
  hHookOnOptionsInit = HookEvent( ME_OPT_INITIALISE, onOptionsInit );
  { // Katy's easter-egg :))
    hHookOnKatynkaIsLoggedIn = HookEvent( ME_DB_CONTACT_SETTINGCHANGED, cslist_KatysEasterEgg );
  }
  { // fix for ANSI Windows - Add and Modify dialogs not loaded when clicking - troubles with not loaded ComboBoxEx
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof( icc );
    icc.dwICC = ICC_USEREX_CLASSES;
    InitCommonControlsEx( &icc );
  }
  return 0;
}


// ############################## UNLOADER #####################################

int __declspec( dllexport ) Unload( void ) {
  int i;
  // unhooking, unservicing & unhandling
  UnhookEvent( hHookMenuBuild );
  UnhookEvent( hHookOnPluginsLoaded );
  UnhookEvent( hHookOnOptionsInit );
  UnhookEvent( hHookOnKatynkaIsLoggedIn );
  DestroyServiceFunction( hSvcShowList );
  for ( i = 0; i < SIZEOF( cslforms ); i++ )
    cslforms[i].hIcoLibItem = NULL;
  hList = NULL;
  hXCombo = NULL;
  hIml = NULL;
  hDlg = NULL;
  return 0;
}


// ############################# AFTER-LOADER ##################################

static int onPluginsLoaded( WPARAM wparam, LPARAM lparam )
{
  // updater section - needs optimalization x)
  if( ServiceExists( MS_UPDATE_REGISTER ) )
  {
    static Update upd = { 0 };
    
//#if defined( _UNICODE )
	  static char szCurrentVersion[30];
	  static char *szVersionUrl = CSLIST_UPD_VERURL;
	  static char *szUpdateUrl = CSLIST_UPD_UPDURL;
	  // todo: complete FL updating
	  static char *szFLVersionUrl = CSLIST_UPD_FLVERURL;
	  static char *szFLUpdateurl = CSLIST_UPD_FLUPDURL;
//#endif    
	  static char *szPrefix = CSLIST_UPD_SZPREFIX;
  	
	  upd.cbSize = sizeof( upd );
	  upd.szComponentName = pluginInfo.shortName;
	  if ( gMirandaVersion < PLUGIN_MAKE_VERSION( 0, 7, 0, 0 ) ) 
		  upd.pbVersion = ( BYTE * )CreateVersionStringPlugin( ( PLUGININFO * )&pluginInfo, szCurrentVersion ); // updater support for < 0.7 core
	  else
		  upd.pbVersion = ( BYTE * )CreateVersionStringPluginEx( ( PLUGININFOEX * )&pluginInfoEx, szCurrentVersion );
	  upd.cpbVersion = strlen( ( char * )upd.pbVersion );
	  //upd.szVersionURL = szFLVersionUrl;
	  //upd.szUpdateURL = szFLUpdateurl;
//#if defined( _UNICODE )
	  upd.pbVersionPrefix = ( BYTE * )CSLIST_UPD_SZPREFIX;
//#endif
	  upd.cpbVersionPrefix = strlen( ( char * )upd.pbVersionPrefix );

	  upd.szBetaUpdateURL = szUpdateUrl;
	  upd.szBetaVersionURL = szVersionUrl;
	  upd.pbVersion = szCurrentVersion;
	  upd.cpbVersion = lstrlenA( szCurrentVersion );
	  upd.pbBetaVersionPrefix = ( BYTE * )szPrefix;
	  upd.cpbBetaVersionPrefix = strlen( ( char * )upd.pbBetaVersionPrefix );

	  CallService( MS_UPDATE_REGISTER, 0, ( LPARAM )&upd );
  }
  // 0.8+ hotkeys service
	if ( ServiceExists( MS_HOTKEY_REGISTER ) )
	{
		HOTKEYDESC hotkey = { 0 };
		hotkey.cbSize = sizeof( hotkey );
		hotkey.pszName = MS_CSLIST_SHOWLIST;
		hotkey.pszDescription = "Show Custom Status List";
		hotkey.pszSection = "Status";
		hotkey.pszService = MS_CSLIST_SHOWLIST;
		hotkey.DefHotKey = MAKEWORD( 'L', HOTKEYF_CONTROL | HOTKEYF_SHIFT );
		CallService( MS_HOTKEY_REGISTER, 0, ( LPARAM )&hotkey );
	}
  // modern toolbar
  if ( ServiceExists( MS_TB_ADDBUTTON ) )
  {
	  TBButton button = { 0 };
		SKINICONDESC sid = { 0 };
    TCHAR szFile[MAX_PATH];
    GetModuleFileName( hInst, szFile, MAX_PATH );

    sid.cbSize = sizeof( sid );
    sid.flags = SIDF_ALL_TCHAR;
		sid.ptszSection = L"ToolBar";
		sid.ptszDefaultFile = ( TCHAR * )szFile;
		sid.cx = sid.cy = 16;

		sid.pszName = "CSList_TB_Icon";
		sid.ptszDescription = L"Custom Status List";
		sid.iDefaultIndex = -IDI_CSLIST;
		hMainIcon = CallService( MS_SKIN2_ADDICON, 0, ( LPARAM )&sid );

	  button.cbSize = sizeof( button );
	  button.pszButtonID = MS_CSLIST_SHOWLIST;
    button.pszTooltipUp = button.pszTooltipDn = button.pszButtonName = CSLIST_MODULE_LONG_NAME;
	  button.pszServiceName = MS_CSLIST_SHOWLIST;
	  button.defPos = 200;
	  button.tbbFlags = TBBF_SHOWTOOLTIP | TBBF_VISIBLE;
    button.hPrimaryIconHandle = ( HANDLE )hMainIcon;
    button.hSecondaryIconHandle = button.hPrimaryIconHandle;
	  CallService( MS_TB_ADDBUTTON, 0, ( LPARAM )&button );
  }
  // release notes dialog
  {
    int relnotes, showit;
    relnotes = DBGetContactSettingWord( NULL, CSLIST_MODULE_SHORT_NAME, "RelNotes", 0 );
    showit = DBGetContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "ShowRelNotes", 1 );
    if ( relnotes != CSLIST_RELNOTES && showit )
      DialogBox( hInst, MAKEINTRESOURCE(IDD_RELNOTES), NULL, ( DLGPROC )RelNotesProc );
  }
  { // take care of DB entries
    // not needed :) current structure allows to write them into DB when it's needed :)
  }
  return 0;
}


// #############################  OPTIONS LOADER  ##############################

int onOptionsInit( WPARAM wparam, LPARAM lparam )
{
  OPTIONSDIALOGPAGE odp = { 0 };

  odp.cbSize = sizeof( odp );
  odp.position = 955000000;
  odp.hInstance = hInst;
  odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPTIONS );
  odp.ptszTitle = TranslateTS( ( TCHAR * )CSLIST_MODULE_LONG_NAME );
  odp.pfnDlgProc = CSListOptionsProc;
  odp.ptszGroup = TranslateTS( ( TCHAR * )"Status" );
  odp.flags = ODPF_BOLDGROUPS;

  CallService( MS_OPT_ADDPAGE, wparam, ( LPARAM )&odp );
  return 0;
}


// #############################################################################
// #############################################################################
// #############################################################################
// #############################################################################
// ############################# MAIN PROCEDURE ################################
// #############################################################################
// #############################################################################
// #############################################################################

INT_PTR CALLBACK CSListProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  NMHDR* pnmh	= 0;
  hDlg = hwndDlg;
  switch ( uMsg )
  {
    case WM_NOTIFY: // some notifying message is sent
    {
      if ( wParam == IDC_CSLIST ) {
        pnmh = ( NMHDR * )lParam; // TODO : optimize? maybe cleaner solution is possible (switching lParam)
  // ######################## DBLCLICKING THE LIST ###############################
        if ( pnmh->code == NM_DBLCLK ) {
          cslist_set_status( hwndDlg );
          //cslist_clear_selection();
          goto savenexit; // stick #1 - edit: not needed ;) so TODO
          // EndDialog( hwndDlg, LOWORD( wParam ) ); // OPTIONALLY: end dialog when ListItem is DblClk'd?
        }
  // #################### CLICKING TO SORT THE LIST ##############################
        else if ( pnmh->code == LVN_COLUMNCLICK && pnmh->idFrom == IDC_CSLIST ) {
          cslist_sort_list();
        }
  // ######################## CLICKING THE LIST ##################################
        else {
            iSelect = SendMessage( hList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED | LVNI_SELECTED ); // set which row is selected (cannot be only focused, but fully selected - second param ;))
            if ( iSelect == -1 ) // if no selection
              flag = 0;
            else
              flag = 1;
            EnableWindow( GetDlgItem( hDlg, IDC_MODIFY ), flag );
            EnableWindow( GetDlgItem( hDlg, IDC_REMOVE ), flag );
            break;
        }
      }
    }
    break;
// ################## INITIALIZING MAIN DIALOG #################################
    case WM_INITDIALOG:
    {
      int i;
      char szTemp[MAX_PATH + 128];

      opened = 1; // instance opened
      // ..................................................... creating the list
      hList = GetDlgItem( hwndDlg, IDC_CSLIST );
      memset( &LvCol, 0, sizeof( LvCol ) );
      hIml = ImageList_Create( 16, 16,	IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 37, 1 );
      if ( hIml ) {
        for ( i = 0; i < 37; i++ ) 
        {
          null_snprintf( szTemp, sizeof( szTemp ), "%s_%s%d", "ICQ", "xstatus", i );
          ImageList_AddIcon( hIml, ( HANDLE )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szTemp ) );
        }
			  ListView_SetImageList( hList, hIml, LVSIL_SMALL );
		  }
      LvCol.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
      LvCol.fmt = LVCFMT_LEFT;
      LvCol.cx = 0x00;
      LvCol.pszText = L"";
      LvCol.cx = 0x16;
      //ListView_SetExtendedListViewStyle( hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
      ListView_SetExtendedListViewStyleEx( hList, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP );
      SendMessage( hList, LVM_INSERTCOLUMN , 0, ( LPARAM )&LvCol );
      LvCol.pszText = TranslateT( "Title" );
      LvCol.cx = 0x64;
      SendMessage( hList, LVM_INSERTCOLUMN, 1, ( LPARAM )&LvCol );
      LvCol.pszText = TranslateT( "Message" );
      LvCol.cx = 0xa8;
      SendMessage( hList, LVM_INSERTCOLUMN, 2, ( LPARAM )&LvCol );
      memset( &LvItem, 0, sizeof( LvItem ) );

      // ................................................ load + sort list items
      cslist_initialize_list_content( hwndDlg );
      cslist_sort_list();

      // ............................................. creating iconized buttons
			for ( i = 0; i < SIZEOF( cslforms ); i++ )
			{
        if ( cslforms[i].idc < 0 )
          continue;
				SendDlgItemMessage( hwndDlg, cslforms[i].idc, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )LoadIconExEx( cslforms[i].iconIcoLib, cslforms[i].iconNoIcoLib ) );
				SendDlgItemMessage( hwndDlg, cslforms[i].idc, BUTTONSETASFLATBTN, 0, 0 ); //maybe set as BUTTONSETDEFAULT?
				SendDlgItemMessage( hwndDlg, cslforms[i].idc, BUTTONADDTOOLTIP, ( WPARAM )TranslateTS( cslforms[i].title ), BATF_TCHAR );
			}
      SetWindowText( hwndDlg, TranslateT( "Custom Status List" ) );
    }
    break;
// ######################## COMMANDS ###########################################
    case WM_COMMAND:
      switch ( LOWORD( wParam ) )
      {
        //case IDC_CSLIST: TODO: deselection
// ------------------------- EXITING THE LIST ----------------------------------
savenexit: // stick #1
        case IDC_NOCHNG: // save, no status change and close
        case IDCANCEL:   // save, no status change and close
        case IDOK:       // not used
        case IDC_APPLY:  // save, set selected and close
        case IDC_EXIT:   // save, set nulled status and close
          if ( ( LOWORD( wParam ) ) == IDC_APPLY ) //set selected status
            cslist_set_status( hwndDlg );
          else if ( ( LOWORD( wParam ) ) == IDC_EXIT )
          { // set null status
            ICQ_CUSTOM_STATUS helpStatus;
            int icon = 0;
            char* title = "";
            char* message = "";
            helpStatus.cbSize = sizeof( ICQ_CUSTOM_STATUS );
            helpStatus.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE;
            helpStatus.status = &icon;
            helpStatus.ptszName = ( TCHAR * )title;
            helpStatus.ptszMessage = ( TCHAR * )message;
            CallService( PS_ICQ_SETCUSTOMSTATUSEX, 0, ( LPARAM )&helpStatus );
          }
          flag = 0;
          iSelect = -1;
          if ( bChanged ) cslist_save_list_content( hwndDlg );
          ListView_DeleteAllItems( hList );
          bChanged = 0;
          opened = 0; // closing instance
          EndDialog( hwndDlg, LOWORD( wParam ) );
          break;
// -------------------------- OPTIONS BUTTON -----------------------------------
        case IDC_OPTS:
          DialogBox( hInst, MAKEINTRESOURCE( IDD_OPTIONS ), hwndDlg, ( DLGPROC )CSListOptionsProc );
          break;
// --------------------- ADDING/MODIFYING ITEM  --------------------------------
        case IDC_MODIFY:
          action = 1;
        case IDC_ADD:
          {
            DialogBox( hInst, MAKEINTRESOURCE( IDD_ADDMODIFY ), hwndDlg, ( DLGPROC )CSListAddModifyProc );
            if ( AMResult == 1 )
            {
              if ( action == 1 )
              {
                ModifiedPos = iSelect;
                cslist_modify_item();
                ModifiedPos = -1;
              }
              else
              {
                cslist_add_item();
              }
              AMResult = 0;
              bChanged = 1;
            }
            action = 0;
            cslist_clear_help_item();
            cslist_sort_list();
          }
          break;
// ----------------- REMOVING ITEM FROM THE LIST -------------------------------
        case IDC_REMOVE:
          {
            cslist_remove_item();
            bChanged = 1;
          }
          break;
        case IDC_IMPORT:
          cslist_import_statuses_from_icq();
          break;
      }
    break;
  }
  return FALSE;
}

// #############################################################################
// ########################### END OF MAIN PROCEDURE ###########################
// #############################################################################
// #############################################################################
// ########################## ADD/MODIFY DLG PROCEDURE #########################
// #############################################################################

INT_PTR CALLBACK CSListAddModifyProc( HWND hwndAMDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
    case WM_INITDIALOG:
    {
      // .........................................creating comboboxex for xicons
      // TODO: changing statics and dialog title in order of action :)
      int i, maxCount;
      if ( action ^ 1 ) // action == 1
      {
        SetWindowText( hwndAMDlg, TranslateT( "Add new item" ) );
        SetDlgItemText( hwndAMDlg, IDC_AM_OK, TranslateT( "Add" ) );
      }
      else
      {
        SetWindowText( hwndAMDlg, TranslateT( "Modify selected item" ) );
        SetDlgItemText( hwndAMDlg, IDC_AM_OK, TranslateT( "Modify" ) );
      }
      SetDlgItemText( hwndAMDlg, IDC_AM_CN, TranslateT( "Cancel" ) );
      SetDlgItemText( hwndAMDlg, IDC_ST_XICON, TranslateT( "Icon:" ) );
      SetDlgItemText( hwndAMDlg, IDC_ST_XTITLE, TranslateT( "Title:" ) );
      SetDlgItemText( hwndAMDlg, IDC_ST_XMESSAGE, TranslateT( "Message:" ) );

      hXCombo = GetDlgItem( hwndAMDlg, IDC_CB_ICON );
      SendMessage( hXCombo, CBEM_SETIMAGELIST, 0, ( LPARAM )hIml );
      ZeroMemory( &CbItem, sizeof( CbItem ) );

      if ( DBGetContactSettingByte( NULL, "ICQ", "NonStandartXstatus", 0 ) )
        maxCount = SIZEOF( xstatuses );
      else
        maxCount = 32;
      for ( i = 0; i < maxCount; i++ )
      {
        CbItem.mask = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
        CbItem.pszText = TranslateTS( xstatuses[i].xname );
        CbItem.iImage = i; // or xstatuses[i].xnum? x)
        CbItem.iItem = i;
        CbItem.iSelectedImage = i;
        SendMessage( hXCombo, CBEM_INSERTITEM, 0, ( LPARAM )&CbItem );
      }
      SendMessage( hXCombo, CB_SETCURSEL, 0, 0 ); // first zero sets selection to top
      if ( action == 1 ) // modifying
      {
        // get values from selection..
        LvItem.iItem = iSelect;
        LvItem.iSubItem = 0;
        ListView_GetItem( hList, &LvItem );
        ListView_GetItemText( hList, iSelect, 1, helpItem.ItemTitle, CSLIST_XTITLE_LIMIT );
        ListView_GetItemText( hList, iSelect, 2, helpItem.ItemMessage, CSLIST_XMESSAGE_LIMIT );
        // ..and send them to edit controls
        SendMessage( hXCombo, CB_SETCURSEL, LvItem.iImage, 0 );
        SetDlgItemText( hwndAMDlg, IDC_XTITLE, helpItem.ItemTitle );
        SetDlgItemText( hwndAMDlg, IDC_XMESSAGE, helpItem.ItemMessage );
      }
    }
    break;
    case WM_COMMAND:
    {
      switch ( LOWORD( wParam ) )
      {
	      case IDC_XTITLE:
	        if ( ( HIWORD( wParam ) == EN_CHANGE ) )
	        {
	          if ( GetWindowTextLength( GetDlgItem( hwndAMDlg, IDC_XTITLE ) ) > CSLIST_XTITLE_LIMIT )
	            SendDlgItemMessage( hwndAMDlg, IDC_XTITLE, WM_APP, ( WPARAM )Translate( "Warning" ), ( LPARAM )Translate( "This field do not accept string longer than 64 characters. String will be truncated." ) );
	          else
	            SendDlgItemMessage( hwndAMDlg, IDC_XTITLE, WM_APP, 0, 0 );
	        }
		      break;
	      case IDC_XMESSAGE:
	        if ( ( HIWORD( wParam ) == EN_CHANGE ) )
	        {
	          if ( GetWindowTextLength( GetDlgItem( hwndAMDlg, IDC_XMESSAGE ) ) > CSLIST_XMESSAGE_LIMIT )
              SendDlgItemMessage( hwndAMDlg, IDC_XMESSAGE, WM_APP, ( WPARAM )Translate( "Warning" ), ( LPARAM )Translate( "This field do not accept string longer than 2048 characters. String will be truncated." ) );
	          else
	            SendDlgItemMessage( hwndAMDlg, IDC_XMESSAGE, WM_APP, 0, 0 );
	        }
		      break;
        case IDOK:
        case IDC_AM_OK:
          cslist_AM_set_help_item( hwndAMDlg );
          AMResult = 1;
          EndDialog( hwndAMDlg, LOWORD( wParam ) );
          break;
        case IDCANCEL:
        case IDC_AM_CN:
          AMResult = 0;
          EndDialog( hwndAMDlg, LOWORD( wParam ) );
          break;
      }
    }
    break;
  }
  return FALSE;
}


// #############################################################################
// ####################### END OF ADD/MODIFY DLG PROCEDURE #####################
// #############################################################################
// ############################ OPTIONS DLG PROCEDURE ##########################
// #############################################################################

BOOL CALLBACK CSListOptionsProc( HWND hwndOptsDlg, UINT msg, WPARAM wparam, LPARAM lparam )
{
  // WORD flags = DBGetContactSettingWord( NULL, MODULENAME, "flags", default_flags );
  // EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL ), 1 );
  // IsDlgButtonChecked( hwndRNDlg, IDC_SHOWIT )
  // CheckDlgButton(hwndDlg, IDC_META, DBGetContactSettingByte(NULL,"CLC","Meta",SETTING_USEMETAICON_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
  
  switch ( msg )
  {
    case WM_INITDIALOG:
    {
      int hideCS = DBGetContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "hideCS", 0 );
      int place = DBGetContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "placement", 11 );
      CheckDlgButton( hwndOptsDlg, IDC_HIDE_CSMENU, hideCS ? BST_CHECKED : BST_UNCHECKED );
      switch ( place ) {
        case 0:
        case 1:
        case 11:
        case 12:
          CheckDlgButton( hwndOptsDlg, IDC_PLACE_GLOBAL, 1 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ), 1 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ), 1 );
          break;
        case 2:
          CheckDlgButton( hwndOptsDlg, IDC_PLACE_ICQ_STATUS, 1 );
          break;
        case 3:
          CheckDlgButton( hwndOptsDlg, IDC_PLACE_MAIN, 1 );
          break;
      }
      switch ( place ) {
        case 0:
        case 1:
        case 11 :
          CheckDlgButton( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP, 1 );
          break;
        case 12 :
          CheckDlgButton( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM, 1 );
          break;
      }
    }
    break;

    case WM_COMMAND:
    {
      switch ( LOWORD( wparam ) )
      {
        case IDC_PLACE_GLOBAL:
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ), 1 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ), 1 );
          break;
        case IDC_PLACE_ICQ_STATUS:
        case IDC_PLACE_MAIN:
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ), 0 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ), 0 );
          break;
        case IDOK:
        case IDCANCEL:
        {
          int place, hideCS;
          hideCS = IsDlgButtonChecked( hwndOptsDlg, IDC_HIDE_CSMENU );
          DBWriteContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "hideCS", hideCS );
          if( IsDlgButtonChecked( hwndOptsDlg, IDC_PLACE_GLOBAL ) )
          {
            if ( IsDlgButtonChecked( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ) )
              place = 11;
            else if ( IsDlgButtonChecked( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ) )
              place = 12;
            else
              place = 11;
          }
          else if ( IsDlgButtonChecked( hwndOptsDlg, IDC_PLACE_ICQ_STATUS ) )
            place = 2;
          else if ( IsDlgButtonChecked( hwndOptsDlg, IDC_PLACE_MAIN ) )
            place = 3;
          if ( place != DBGetContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "placement", 0 ) )
            DBWriteContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "placement", place );
        }
        EndDialog( hwndOptsDlg, LOWORD( wparam ) );
        break;
      }
    }
    break;
  }
  return 0;
}


// #############################################################################
// ######################## END OF OPTIONS DLG PROCEDURE #######################
// #############################################################################
// ############################# LIST SORT PROCEDURE ###########################
// #############################################################################

static int CALLBACK CSListSortProc( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
  LVITEM first, second;
 
  first.mask = LVIF_IMAGE;
  first.iItem = lParam1;
  first.iSubItem = lParamSort;

  second.mask = LVIF_IMAGE;
  second.iItem = lParam2;
  second.iSubItem = lParamSort;

  ListView_GetItem( hList, &first );
  ListView_GetItem( hList, &second );

  if ( first.iImage > second.iImage ) return 1;
  else if ( first.iImage < second.iImage ) return -1;
  else return 0;
}


// #############################################################################
// ########################## END OF LIST SORT PROCEDURE #######################
// #############################################################################
// ######################## RELEASE NOTES DIALOG PROCEDURE #####################
// #############################################################################

INT_PTR CALLBACK RelNotesProc( HWND hwndRNDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
    case WM_INITDIALOG:
    {
      SetDlgItemText( hwndRNDlg, IDC_TXHTEXT, rnthanks );
    }
    break;
    case WM_COMMAND:
    {
      switch ( LOWORD( wParam ) )
      {
        case IDOK:
        case IDCANCEL:
        {
          int showit;
          showit = IsDlgButtonChecked( hwndRNDlg, IDC_SHOWIT ) ? 1 : 0;
          DBWriteContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "ShowRelNotes", !showit );
          DBWriteContactSettingWord( NULL, CSLIST_MODULE_SHORT_NAME, "RelNotes", CSLIST_RELNOTES );
          EndDialog( hwndRNDlg, LOWORD( wParam ) );
        }
        break;
      }
    }
    break;
  }
  return FALSE;
}


// #############################################################################
// #################### END OF RELEASE NOTES DIALOG PROCEDURE ##################
// #############################################################################

// ############################# HELP FUNCTIONS ################################

int null_snprintf( char *buffer, size_t count, const char* fmt, ... ) // deprecated? O_o
{
  va_list va;
  int len;

  ZeroMemory( buffer, count );
  va_start( va, fmt );
  len = mir_vsnprintf( buffer, count - 1, fmt, va );
  va_end( va );
  return len;
}

static int cslist_init_menu_item( WPARAM wParam, LPARAM lParam )
{
  CLISTMENUITEM mi = { 0 };
  int miPlacement = 3;
  if ( hHookMenuBuild != NULL )
    miPlacement = DBGetContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "placement", 11 );
  ZeroMemory( &mi, sizeof( mi ) );
  mi.cbSize = sizeof( mi );
  mi.position = -0x7FFFFFFF; // top
  mi.flags = 0;
  mi.hIcon = LoadIconExEx( "csl_icon", IDI_CSLIST );
  mi.ptszName = TranslateTS( ( TCHAR * )"Custom Status List..." );
  mi.pszService = "CSList/ShowList";
  mi.pszContactOwner = "ICQ";
  if ( miPlacement == 12 ) mi.position = 2000040000; // bottom
  else if ( miPlacement == 2 ) {
    int protoCount;
    PROTOCOLDESCRIPTOR** pdesc;
    CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&protoCount,(LPARAM)&pdesc);
    if ( protoCount > 1 ) mi.ptszPopupName = ( TCHAR * )"ICQ";
  }
  switch ( miPlacement ) {
    case 0 : // obsolete I think :))
    case 1 :
    case 11:
    case 12:
    case 2:
      if( DBGetContactSettingByte( NULL, CSLIST_MODULE_SHORT_NAME, "hideCS", 0 ) )
      {
        mi.position = 00000001; // when hiding CSMenu, move to its place
        mi.ptszName = TranslateTS( ( TCHAR * )"Custom status" );
      }
      CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );
      break;
    case 3: // add to main menu
      CallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
      break;
    //case 2: // add to tray menu
    //  CallService( "CList/AddTrayMenuItem", 0, ( LPARAM )&mi );
    //  break;
  }
  return 0;
}

// dialog box ANSI or Unicode?


// ################## COMMANDS OF CSLIST #######################################

int cslist_add_item()
{ // final adding with helpItem
  int iItem;

// -------------------------------------------------------- define list style --
  if ( ModifiedPos >= 0 )
    iItem = ModifiedPos;
  else
    iItem = SendMessage( hList, LVM_GETITEMCOUNT, 0, 0 );
  memset( &LvItem, 0, sizeof( LvItem ) );
  LvItem.mask = LVIF_TEXT | LVIF_IMAGE;

  LvItem.cchTextMax = 256;
  LvItem.iItem = iItem;
// -------------------------------------------------------------------- first --
  LvItem.iSubItem = 0;
  LvItem.iImage = helpItem.ItemIcon; // use selected xicon

  SendMessage( hList, LVM_INSERTITEM, 0, ( LPARAM )&LvItem );
// ------------------------------------------------------------------- second --

  LvItem.iSubItem = 1;
  LvItem.pszText = helpItem.ItemTitle;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );
// -------------------------------------------------------------------- third --
  LvItem.iSubItem = 2;
  LvItem.pszText = helpItem.ItemMessage;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );
// --------------------------------------------------------------------- end ---
  cslist_clear_selection();
  return 0;
}

int cslist_modify_item()
{
  if ( ModifiedPos < 0 )
    return FALSE;
  memset( &LvItem, 0, sizeof( LvItem ) );
  LvItem.mask = LVIF_TEXT | LVIF_IMAGE;
  LvItem.cchTextMax = 256;
  LvItem.iItem = ModifiedPos;
  LvItem.iSubItem = 0;
  LvItem.iImage = helpItem.ItemIcon;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );
  LvItem.iSubItem = 1;
  LvItem.pszText = helpItem.ItemTitle;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );
  LvItem.iSubItem = 2;
  LvItem.pszText = helpItem.ItemMessage;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );
  return TRUE;
}

int cslist_remove_item()
{
  if ( flag ) SendMessage( hList, LVM_DELETEITEM, iSelect, 0 );
  cslist_clear_selection();
  return 0;
}

int cslist_AM_set_help_item(HWND hwndAMDlg)
{
  // get and set helpItem xicon
	ZeroMemory( &CbItem, sizeof( CbItem ) );
	CbItem.mask = CBEIF_IMAGE | CBEIF_TEXT;
	CbItem.iItem = SendDlgItemMessage( hwndAMDlg, IDC_CB_ICON, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( hwndAMDlg, IDC_CB_ICON, CBEM_GETITEM, 0, ( LPARAM )&CbItem );
  helpItem.ItemIcon = CbItem.iImage;

  GetDlgItemText( hwndAMDlg, IDC_XTITLE, helpItem.ItemTitle, CSLIST_XTITLE_LIMIT ); // set helpItem xtitle
  GetDlgItemText( hwndAMDlg, IDC_XMESSAGE, helpItem.ItemMessage, CSLIST_XMESSAGE_LIMIT ); // set helpItem xmessage
  return 0;
}


// ################## SERVICES FOR CSLIST ######################################

void cslist_clear_selection() {
  flag = 0;
  iSelect = -1;
  SendMessage( hList, LVM_SETITEMSTATE, -1, 0 );
  //SendMessage( hList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED | LVNI_SELECTED );
}

void cslist_clear_help_item() {
  ZeroMemory( &helpItem, sizeof( helpItem ) );
}

void cslist_sort_list() {
  ListView_SortItemsEx( hList, CSListSortProc, (LPARAM)0 );
}

void cslist_import_statuses_from_icq() {
  int i, result;
  result = MessageBox( hDlg, L"Do you want to delete those DB entries after Import?", L"Custom Status List", MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION );
  for ( i = 0; i < 37; i++ )
  {
    DBVARIANT dbv = { 0 };
    char bufName[64], bufMsg[64];
    cslist_clear_help_item( );
    mir_snprintf( bufName, 32, "XStatus%luName", i );
    mir_snprintf( bufMsg, 32, "XStatus%luMsg", i );
    helpItem.ItemIcon = i - 1;
    DBGetContactSettingTString( NULL, "ICQ", bufName, &dbv );
    lstrcpy( helpItem.ItemTitle, dbv.ptszVal );
    DBGetContactSettingTString( NULL, "ICQ", bufMsg, &dbv );
    lstrcpy( helpItem.ItemMessage, dbv.ptszVal );
    if ( lstrlen( helpItem.ItemTitle ) || lstrlen( helpItem.ItemMessage ) )
      cslist_add_item();
    if ( result == IDYES )
    {
      DBDeleteContactSetting( NULL, "ICQ", bufName );
      DBDeleteContactSetting( NULL, "ICQ", bufMsg );
    }
  }
  cslist_sort_list();
  bChanged = 1;
}

int cslist_KatysEasterEgg( WPARAM wParam, LPARAM lParam )
{
  DBCONTACTWRITESETTING *cws = ( DBCONTACTWRITESETTING* )lParam;
  if ( ( HANDLE )wParam == NULL || lstrcmpA( cws->szSetting, "Status" ) )
    return 0;
  if ( ( DBGetContactSettingDword( ( HANDLE )wParam, "ICQ", "UIN", 0 ) == 0x1831BA1A ) &&
       ( DBGetContactSettingWord( ( HANDLE )wParam, "UserOnline", "OldStatus2", 0 ) == ID_STATUS_OFFLINE ) &&
       ( cws->value.wVal > ID_STATUS_OFFLINE ) )
  {
    ICQ_CUSTOM_STATUS helpStatus;
    int icon = 0;
    char * title = "";
    char * message = "";
    helpStatus.cbSize = sizeof( ICQ_CUSTOM_STATUS );
    helpStatus.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE;
    helpStatus.status = &icon;
    helpStatus.ptszName = ( TCHAR * )title;
    helpStatus.ptszMessage = ( TCHAR * )message;
    CallService( PS_ICQ_SETCUSTOMSTATUSEX, 0, ( LPARAM )&helpStatus );
  }
  return 0;
}

// ################## DB - LOAD AND SAVE #######################################

int cslist_initialize_list_content( HWND hwndDlg )
{
  // get DB string, parse to statuses, add by helpItem
  DBVARIANT dbv = { DBVT_TCHAR };
  int parseResult;
  int dbLoadResult;
  const TCHAR* rowDelim = L""; // new line
  TCHAR *row = NULL;

  dbLoadResult = DBGetContactSettingTString( NULL, CSLIST_MODULE_SHORT_NAME, "listhistory", &dbv );
  if ( dbv.ptszVal )
  {
    row = wcstok( dbv.ptszVal, rowDelim );
    while( row != NULL ) {
      // process current row..
      parseResult = cslist_parse_row( row );
      // ..add item..
      if ( parseResult == TRUE ) cslist_add_item();
      // ..and go to the other, while some remains
      row = wcstok( NULL, rowDelim );
    }
  }
  //free( rowDelim );
  //free( row );
  return 0;
}

int cslist_parse_row( TCHAR *row ) // parse + helpItem
{
  int pIconInt;
  TCHAR pIcon[4], pTitle[CSLIST_XTITLE_LIMIT+2], pMsg[CSLIST_XMESSAGE_LIMIT+2], pFav[4];
  //if ( sscanf( row, "%2[^]%64[^]%2048[^]%2[^]", pIcon, pTitle, pMsg, pFav ) == 4 ) // PLEASE!! x) use DEFs xO
  if ( swscanf( row, L"%2[^]%64[^]%2048[^]%2[^]", &pIcon, &pTitle, &pMsg, &pFav ) == 4 ) // PLEASE!! x) use DEFs xO
  {
    pIconInt = _wtoi( pIcon );
    helpItem.ItemIcon = pIconInt;
    lstrcpy( helpItem.ItemTitle, pTitle );
    lstrcpy( helpItem.ItemMessage, pMsg );
  }
  else if ( swscanf( row, L"%2[^]%2048[^]%2[^]", &pIcon, &pMsg, &pFav ) == 3 )
  {
    pIconInt = _wtoi( pIcon );
    helpItem.ItemIcon = pIconInt;
    lstrcpy( helpItem.ItemTitle, L"" );
    lstrcpy( helpItem.ItemMessage, pMsg );
  }
  else if ( swscanf( row, L"%2[^]%64[^]%2[^]", &pIcon, &pTitle, &pFav ) == 3 )
  {
    pIconInt = _wtoi( pIcon );
    helpItem.ItemIcon = pIconInt;
    lstrcpy( helpItem.ItemTitle, pTitle );
    lstrcpy( helpItem.ItemMessage, L"" );
  }
  //else if( swscanf( row, L"%2[^]%2[^]", pIcon, pFav ) == 2 )
  //{
  //  pIconInt = _wtoi( pIcon );
  //  helpItem.ItemIcon = pIconInt;
  //  lstrcpy( helpItem.ItemTitle, "" );
  //  lstrcpy( helpItem.ItemMessage, "" );
  //} // why allow empty xstatuses? O_o
  else {
    //free(row);
    //free(pIcon);
    //free(pTitle);
    //free(pMsg);
    //free(pFav);
    return FALSE;
  }
  //free(row);
  //free(pIcon);
  //free(pTitle);
  //free(pMsg);
  //free(pFav);
  return TRUE;
}


int cslist_save_list_content( HWND hwndDlg )
{
  TCHAR cImageToString[64];
  int i = 0;
  LPTSTR dbString = { 0 };
  TCHAR dbStringTmp[65536];
  lstrcpy( dbStringTmp, L"" ); // set to empty - caused "dizzy" string begin x) O_o
  lstrcpy( cImageToString, L"" );
  for ( i = 0; i < ListView_GetItemCount( hList ); i++ )
  {
    // insert xint to string
    LvItem.iItem = i;
    LvItem.iSubItem = 0;
    ListView_GetItem( hList, &LvItem );
    swprintf( cImageToString, 8, L"%d", LvItem.iImage );
    lstrcat( dbStringTmp, cImageToString );
    lstrcpy( cImageToString, L"" );
    lstrcat( dbStringTmp, L"" );
    ListView_GetItemText( hList, i, 1, helpItem.ItemTitle, CSLIST_XTITLE_LIMIT );
    if ( wcslen( helpItem.ItemTitle ) > 0 ) lstrcat( dbStringTmp, helpItem.ItemTitle );
    lstrcat( dbStringTmp, L"" );
    ListView_GetItemText( hList, i, 2, helpItem.ItemMessage, CSLIST_XMESSAGE_LIMIT );
    if ( wcslen( helpItem.ItemMessage ) > 0 ) lstrcat( dbStringTmp, helpItem.ItemMessage );
    lstrcat( dbStringTmp, L"0" );
  }
  dbString = dbStringTmp;
  // save to DB
  DBWriteContactSettingTString( NULL, CSLIST_MODULE_SHORT_NAME, "listhistory", dbString );
  free( dbString );
  free( dbStringTmp );
  lstrcpy( helpItem.ItemTitle, L"" );
  lstrcpy( helpItem.ItemMessage, L"" );
  return 0;
}


// ######################### CREATING DIALOG ###################################

int cslist_set_status( HWND hwndDlg )
{
  ICQ_CUSTOM_STATUS helpStatus;
  memset( &helpStatus, 0, sizeof( helpStatus ) );
  helpStatus.cbSize = sizeof( ICQ_CUSTOM_STATUS );
  helpStatus.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_UNICODE;
  iSelect = SendMessage( hList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED | LVNI_SELECTED ); // set which row is selected
  if ( iSelect == -1 ) // no status selected
    flag = 0;
  else
  { // else get values from selection
    LvItem.iItem = iSelect;
    LvItem.iSubItem = 0;
    ListView_GetItem( hList, &LvItem );
    helpItem.ItemIcon = LvItem.iImage; // TODO: get xicon int and set ..
    ListView_GetItemText( hList, iSelect, 1, helpItem.ItemTitle, CSLIST_XTITLE_LIMIT );
    ListView_GetItemText( hList, iSelect, 2, helpItem.ItemMessage, CSLIST_XMESSAGE_LIMIT );
    // ..and set xstatus
    helpItem.ItemIcon++; // need revision of redirected xstatus numbers xo)
    helpStatus.status = &helpItem.ItemIcon;
    helpStatus.ptszName = helpItem.ItemTitle;
    helpStatus.ptszMessage = helpItem.ItemMessage;
    CallService( PS_ICQ_SETCUSTOMSTATUSEX, 0, ( LPARAM )&helpStatus ); // f**king function, 4 hours of thinking xDD
    // ..and clean helper + selection
    cslist_clear_selection();
  }
  return 0;
}

void cslist_init_icons( void )
{
  int i;
  SKINICONDESC sid = { 0 };
  char szFile[MAX_PATH];
  GetModuleFileNameA( hInst, szFile, MAX_PATH );

  sid.cbSize = sizeof( SKINICONDESC );
  sid.ptszDefaultFile = ( TCHAR * )szFile;
  sid.cx = sid.cy = 16;
  sid.ptszSection = TranslateTS( CSLIST_MODULE_LONG_NAME );

  for ( i = 0; i < SIZEOF( cslforms ); i++ )
  {
    char szSettingName[64];
    mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", CSLIST_MODULE_SHORT_NAME, cslforms[i].iconIcoLib );
    sid.pszName = szSettingName;
    sid.ptszDescription = TranslateTS( cslforms[i].szDescr );
    sid.iDefaultIndex = -cslforms[i].iconNoIcoLib;
    cslforms[i].hIcoLibItem = ( HANDLE )CallService( MS_SKIN2_ADDICON, 0, ( LPARAM )&sid );
  }
}


// ######################### OTHER FUNCTIONS ###################################

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon )
{
  if ( ServiceExists( MS_SKIN2_GETICON ) ) {
    char szSettingName[64];
    mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", CSLIST_MODULE_SHORT_NAME, IcoLibName );
    return ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szSettingName );
  }
  else
    return ( HICON )LoadImage( hInst, MAKEINTRESOURCE( NonIcoLibIcon ), IMAGE_ICON, 0, 0, 0 );
}


/* ########################################################################## 80

                              Custom Status List
                              __________________

                                  by jarvis

// ############################################################ 2008/01/28 ## */
