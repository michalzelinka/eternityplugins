/* ########################################################################## 80

                              Custom Status List
                              __________________

  Custom Status List plugin for Miranda-IM (www.miranda-im.org)
  Follower of Custom Status History List by HANAX
  Copyright © 2006-2008 HANAX
  Copyright © 2007-2008 jarvis

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
  - conversion to C++
  - favourites
  - watching statuses set by x-status menu
  - all TODOs listed in this source x)
  - feature requests maybe :)

  MAYBE-TODO
  ----------
  - save window positions

  NOT-TODO
  --------
  - setting Away/NA/DND/Occ - there are already many status managers :)

  0.0.0.20 - calm before the storm Ox)
  --------
  - some corrections of working with strings (ANSI/Unicode <-> TCHAR)
  - simplified loading of list items :)
  - the Options button replaced by the Undo button

  0.0.0.19 - enjoy multi'ing, baby :)
  --------
  - status is changing on all ICQ based accounts/protos
  - when dialog is opened, and user tryes to open another one, forgotten dialog is brought to front instead
  - menu placement optimalization
  - removed old nasty GOTO because of double-clicking the list x)
  - active checking of input data length during editing custom status
  - optimalizations in translations
  - support for Variables plugin
  - other enhancements related to the possibility of using multiple ICQ accounts

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
  if ( hDlg == NULL )
    DialogBox( hInst, MAKEINTRESOURCE( IDD_CSLIST ), NULL, ( DLGPROC )CSListProc );
  else
    SetForegroundWindow( hDlg );
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

  /*
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
  */

  gbAccountsSupported = ( gMirandaVersion > PLUGIN_MAKE_VERSION( 0, 8, 0, 7 ) );

  hSvcShowList = CreateServiceFunction( MS_CSLIST_SHOWLIST, PluginMenuCommand );

  cslist_init_icons();

  if ( DBReadByte( "ICQ", "XStatusEnabled", ICQ_XSTATUS_ENABLED_DEFAULT ) )
  { // if xstatuses are enabled, declare, if we can use status menu or main menu only
    if ( bStatusMenu = ServiceExists( MS_CLIST_ADDSTATUSMENUITEM ) )
      hHookMenuBuild = HookEvent( ME_CLIST_PREBUILDSTATUSMENU, cslist_init_menu_items );
    else
      cslist_init_menu_items( 0, 0 );
  }

  hHookOnPluginsLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, onPluginsLoaded );
  hHookOnOptionsInit = HookEvent( ME_OPT_INITIALISE, onOptionsInit );
  { // (originally) Katy's easter-egg :))
    hHookOnDBSettingChanged = HookEvent( ME_DB_CONTACT_SETTINGCHANGED, cslist_setting_changed );
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
  UnhookEvent( hHookOnDBSettingChanged );
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
    if ( gMirandaVersion > PLUGIN_MAKE_VERSION( 0, 7, 0, 0 ) )
      upd.pbVersion = ( BYTE * )CreateVersionStringPluginEx( ( PLUGININFOEX * )&pluginInfoEx, szCurrentVersion );
    else // updater support for < 0.7 cores
      upd.pbVersion = ( BYTE * )CreateVersionStringPlugin( ( PLUGININFO * )&pluginInfo, szCurrentVersion );
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
  // 0.8+ core hotkeys service
  if ( ServiceExists( MS_HOTKEY_REGISTER ) )
  {
    HOTKEYDESC hotkey = { 0 };
    hotkey.cbSize = sizeof( hotkey );
    hotkey.pszName = MS_CSLIST_SHOWLIST;
    hotkey.pszDescription = CSLIST_STRING_SHOW_CSLIST;
    hotkey.pszSection = "Status";
    hotkey.pszService = MS_CSLIST_SHOWLIST;
    hotkey.DefHotKey = MAKEWORD( 'L', HOTKEYF_CONTROL | HOTKEYF_SHIFT );
    CallService( MS_HOTKEY_REGISTER, 0, ( LPARAM )&hotkey );
  }
  // 0.7+ clist modern toolbar
  if ( ServiceExists( MS_TB_ADDBUTTON ) )
  {
    TBButton button = { 0 };
    SKINICONDESC sid = { 0 };
    TCHAR szFile[MAX_PATH];
    GetModuleFileName( hInst, szFile, MAX_PATH );

    sid.cbSize = sizeof( sid );
    sid.flags = SIDF_ALL_TCHAR;
    sid.ptszSection = TranslateT( "ToolBar" );
    sid.ptszDefaultFile = ( TCHAR * )szFile;
    sid.cx = sid.cy = 16;

    sid.pszName = "CSList_TB_Icon";
    sid.ptszDescription = TranslateT( CSLIST_MODULE_LONG_NAME );
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
    relnotes = DBReadWord( CSLIST_MODULE_SHORT_NAME, "RelNotes", 0 );
    showit = DBReadByte( CSLIST_MODULE_SHORT_NAME, "ShowRelNotes", CSLIST_SHOW_RELNOTES_DEFAULT );
    if ( relnotes != CSLIST_RELNOTES && showit )
      DialogBox( hInst, MAKEINTRESOURCE( IDD_RELNOTES ), NULL, ( DLGPROC )RelNotesProc );
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
  odp.ptszTitle = TranslateT( CSLIST_MODULE_LONG_NAME );
  odp.pfnDlgProc = CSListOptionsProc;
  odp.ptszGroup = TranslateT( "Status" );
  odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

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
  NMHDR* pnmh  = 0;
  switch ( uMsg )
  {
    case WM_NOTIFY: // some notifying message is sent
    {
      if ( wParam == IDC_CSLIST ) {
        pnmh = ( NMHDR * )lParam; // TODO : optimize? maybe cleaner solution is possible (switching lParam)
  // ######################## DBLCLICKING THE LIST ###############################
        if ( pnmh->code == NM_DBLCLK ) {
          cslist_set_status( hwndDlg, IDC_APPLY );
          PostMessage(hwndDlg, WM_COMMAND, IDOK, 0L);
          break;
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
      int i, j;
      char szTemp[MAX_PATH + 128];

      hDlg = hwndDlg;
      // ..................................................... creating the list
      hList = GetDlgItem( hwndDlg, IDC_CSLIST );
      memset( &LvCol, 0, sizeof( LvCol ) );
      hIml = ImageList_Create( 16, 16,  IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 37, 1 );
      if ( hIml ) {
        int protoCount;
        PROTOCOLDESCRIPTOR** pdesc;
        CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );
        for ( j = 0; j < protoCount; j++ ) {
          if ( DBReadDword( pdesc[j]->szName, "UIN", 0 ) == 0 )
            continue;
          for ( i = 0; i < 37; i++ ) 
          {
            mir_snprintf( szTemp, sizeof( szTemp ), "%s_%s%d", pdesc[j]->szName, "xstatus", i );
            ImageList_AddIcon( hIml, ( HANDLE )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szTemp ) );
          }
          break;
        }
        ListView_SetImageList( hList, hIml, LVSIL_SMALL );
      }
      LvCol.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
      LvCol.fmt = LVCFMT_LEFT;
      LvCol.cx = 0x00;
      LvCol.pszText = _T( "" );
      LvCol.cx = 0x16;
      //ListView_SetExtendedListViewStyle( hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
      ListView_SetExtendedListViewStyleEx( hList, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP );
      SendMessage( hList, LVM_INSERTCOLUMN , 0, ( LPARAM )&LvCol );
      LvCol.pszText = TranslateT( CSLIST_STRING_TITLE );
      LvCol.cx = 0x64;
      SendMessage( hList, LVM_INSERTCOLUMN, 1, ( LPARAM )&LvCol );
      LvCol.pszText = TranslateT( CSLIST_STRING_MESSAGE );
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
        SendDlgItemMessage( hwndDlg, cslforms[i].idc, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )LoadIconExEx( cslforms[i].pszIconIcoLib, cslforms[i].iconNoIcoLib ) );
        SendDlgItemMessage( hwndDlg, cslforms[i].idc, BUTTONSETASFLATBTN, 0, 0 ); //maybe set as BUTTONSETDEFAULT?
        SendDlgItemMessage( hwndDlg, cslforms[i].idc, BUTTONADDTOOLTIP, ( WPARAM )TranslateTS( cslforms[i].ptszTitle ), BATF_TCHAR );
      }
      SetWindowText( hwndDlg, TranslateT( CSLIST_MODULE_LONG_NAME ) );
    }
    break;
// ######################## COMMANDS ###########################################
    case WM_COMMAND:
      switch ( LOWORD( wParam ) )
      {
// ------------------------- EXITING THE LIST ----------------------------------
        case IDC_NOCHNG: // save, no status change and close
        case IDCANCEL:   // save, no status change and close
        case IDOK:       // not used
        case IDC_APPLY:  // save, set selected and close
        case IDC_EXIT:   // save, set nulled status and close
          cslist_set_status( hwndDlg, LOWORD( wParam ) );
          cslist_clear_selection();
          if ( bChanged ) cslist_save_list_content( hwndDlg );
          ListView_DeleteAllItems( hList );
          bChanged = 0;
          EndDialog( hwndDlg, LOWORD( wParam ) );
          hDlg = NULL;
          break;
// --------------------------- UNDO BUTTON -------------------------------------
        case IDC_UNDO:
          ListView_DeleteAllItems( hList );
          cslist_initialize_list_content( hwndDlg );
          cslist_sort_list();
          bChanged = 0;
          EnableWindow( GetDlgItem( hwndDlg, IDC_UNDO ), FALSE );
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
              EnableWindow( GetDlgItem( hwndDlg, IDC_UNDO ), TRUE );
            }
            action = 0;
            cslist_clear_help_item();
            cslist_sort_list();
          }
          break;
// ----------------- REMOVING ITEM FROM THE LIST -------------------------------
        case IDC_REMOVE:
          {
            if ( DBReadByte( CSLIST_MODULE_SHORT_NAME, "deletionConfirm", CSLIST_ITEMS_DELETION_DEFAULT ) )
              if ( MessageBox( hDlg, TranslateT( CSLIST_STRING_ITEM_DELETE_CONFIRM ), TranslateT( CSLIST_MODULE_LONG_NAME ), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION ) == IDNO )
                break;
            cslist_remove_item();
            bChanged = 1;
            EnableWindow( GetDlgItem( hwndDlg, IDC_UNDO ), TRUE );
          }
          break;
// -------------------------- IMPORTING ----------------------------------------
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
      int i, maxCount;
      TCHAR buf[64];

      lstrcpy( buf, TranslateT( CSLIST_STRING_TITLE ) );
      lstrcat( buf, _T( ":" ) );
      SetDlgItemText( hwndAMDlg, IDC_ST_XTITLE, buf );
      lstrcpy( buf, TranslateT( CSLIST_STRING_MESSAGE ) );
      lstrcat( buf, _T( ":" ) );
      SetDlgItemText( hwndAMDlg, IDC_ST_XMESSAGE, buf );
      SetDlgItemText( hwndAMDlg, IDC_AM_CN, TranslateT( CSLIST_STRING_CANCEL ) );

      hXCombo = GetDlgItem( hwndAMDlg, IDC_CB_ICON );
      SendMessage( hXCombo, CBEM_SETIMAGELIST, 0, ( LPARAM )hIml );
      ZeroMemory( &CbItem, sizeof( CbItem ) );

      if ( DBReadByte( "ICQ", "NonStandartXstatus", CSLIST_SHOW_NONSTANDARD_XSTATUSES_DEFAULT ) )
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

      if ( action ^ 1 ) // action XOR 1
      {
        SetWindowText( hwndAMDlg, TranslateT( CSLIST_STRING_ADD_NEW_ITEM ) );
        SetDlgItemText( hwndAMDlg, IDC_AM_OK, TranslateT( CSLIST_STRING_ADD ) );
        SetDlgItemText( hwndAMDlg, IDC_XTITLE, TranslateT( CSLIST_STRING_TITLE ) );
        SetDlgItemText( hwndAMDlg, IDC_XMESSAGE, TranslateT( CSLIST_STRING_MESSAGE ) );
      }
      else
      {
        SetWindowText( hwndAMDlg, TranslateT( CSLIST_STRING_MODIFY_ITEM ) );
        SetDlgItemText( hwndAMDlg, IDC_AM_OK, TranslateT( CSLIST_STRING_MODIFY ) );
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
        case IDC_XMESSAGE:
          {
            int idc = LOWORD( wParam );
            unsigned int limit = ( idc == IDC_XTITLE ) ? CSLIST_XTITLE_LIMIT : CSLIST_XMESSAGE_LIMIT;
            BOOL type = ( idc == IDC_XTITLE ) ? 1 : 0;
            if ( ( HIWORD( wParam ) == EN_CHANGE ) )
            {
              TCHAR bufMsgTitle[CSLIST_XTITLE_LIMIT + 2], bufTrimmedTitle[CSLIST_XTITLE_LIMIT + 1];
              TCHAR bufMsgMessage[CSLIST_XMESSAGE_LIMIT], bufTrimmedMessage[CSLIST_XMESSAGE_LIMIT + 1];
              TCHAR bufInfo[256];
              EDITBALLOONTIP ebtt;
              mir_sntprintf(
                bufInfo,
                sizeof( bufInfo ),
                _T( CSLIST_STRING_FIELD_STRING_LIMIT ),
                limit
              );
              ebtt.cbStruct = sizeof( ebtt );
              ebtt.pszTitle = TranslateT( CSLIST_STRING_WARNING );
              ebtt.pszText = bufInfo;
              ebtt.ttiIcon = TTI_WARNING;
              GetDlgItemText( hwndAMDlg, idc, type ? bufMsgTitle : bufMsgMessage, limit + 2 );
              if ( tcslen( type ? bufMsgTitle : bufMsgMessage ) > limit ) {
                SendMessage( GetDlgItem( hwndAMDlg, idc ), EM_SHOWBALLOONTIP, 0, ( LPARAM )&ebtt );
                GetDlgItemText( hwndAMDlg, idc, type ? bufTrimmedTitle : bufTrimmedMessage, limit + 1 );
                SetDlgItemText( hwndAMDlg, idc, type ? bufTrimmedTitle : bufTrimmedMessage );
              }
            }
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
  switch ( msg )
  {
    case WM_INITDIALOG:
    {
      int place = DBReadByte( CSLIST_MODULE_SHORT_NAME, "placement", CSLIST_MENU_PLACEMENT_DEFAULT );

      cslist_translate_dialog( hwndOptsDlg );

      CheckDlgButton( hwndOptsDlg,
                      IDC_DEL_CONFIRM,
                      DBReadByte( CSLIST_MODULE_SHORT_NAME, "deletionConfirm", CSLIST_ITEMS_DELETION_DEFAULT ) ?
                      BST_CHECKED : BST_UNCHECKED
      );

      CheckDlgButton( hwndOptsDlg,
                      IDC_DEL_AFTER_IMPORT,
                      DBReadByte( CSLIST_MODULE_SHORT_NAME, "deleteAfterImport", CSLIST_IMPORT_DELETION_DEFAULT ) ?
                      BST_CHECKED : BST_UNCHECKED
      );

      CheckDlgButton( hwndOptsDlg,
                      IDC_HIDE_CSMENU,
                      DBReadByte( CSLIST_MODULE_SHORT_NAME, "hideCS", CSLIST_HIDE_CSUI_DEFAULT ) ?
                      BST_CHECKED : BST_UNCHECKED
      );
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
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_HIDE_CSMENU ), 1 );
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
      SendMessage( GetParent( hwndOptsDlg ), PSM_CHANGED, 0, 0 );
      switch ( LOWORD( wparam ) )
      {
        case IDC_PLACE_GLOBAL:
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ), 1 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ), 1 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_HIDE_CSMENU ), 0 );
          break;
        case IDC_PLACE_ICQ_STATUS:
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_HIDE_CSMENU ), 1 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ), 0 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ), 0 );
          break;
        case IDC_PLACE_MAIN:
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_TOP ), 0 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_PLACE_GLOBAL_BOTTOM ), 0 );
          EnableWindow( GetDlgItem( hwndOptsDlg, IDC_HIDE_CSMENU ), 0 );
          break;
      }
    }
    break;
    case WM_NOTIFY:
    {
      switch ( ( ( LPNMHDR )lparam )->idFrom )
      {
        case 0:
        {
          switch ( ( ( LPNMHDR )lparam )->code )
          {
            case PSN_APPLY:
            {
              int place = 0;

              DBWriteByte( CSLIST_MODULE_SHORT_NAME,
                           "deletionConfirm",
                           IsDlgButtonChecked( hwndOptsDlg, IDC_DEL_CONFIRM ) ?
                           1 : 0
              );

              DBWriteByte( CSLIST_MODULE_SHORT_NAME,
                           "deleteAfterImport",
                           IsDlgButtonChecked( hwndOptsDlg, IDC_DEL_AFTER_IMPORT ) ?
                           1 : 0
              );

              DBWriteByte( CSLIST_MODULE_SHORT_NAME,
                           "hideCS",
                           IsDlgButtonChecked( hwndOptsDlg, IDC_HIDE_CSMENU ) ?
                           1 : 0
              );

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
              if ( place != DBReadByte( CSLIST_MODULE_SHORT_NAME, "placement", CSLIST_MENU_PLACEMENT_DEFAULT ) )
                DBWriteByte( CSLIST_MODULE_SHORT_NAME, "placement", place );

              // rebuilding status menu - TODO: stand-alone function?
              {
                char protoService[64];
                PROTOCOLDESCRIPTOR** pdesc;
                int protoCount = 0, i;
                ICQ_CUSTOM_STATUS hideStatusMenu;
                WPARAM wp;
                CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );
                for ( i = 0; i < protoCount; i++ ) {
                  if ( DBReadDword( pdesc[i]->szName, "UIN", 0 ) == 0 )
                    continue;
                  wp = ( IsDlgButtonChecked( hwndOptsDlg, IDC_PLACE_ICQ_STATUS ) && IsDlgButtonChecked( hwndOptsDlg, IDC_HIDE_CSMENU ) ) ? 0 : 1;
                  lstrcpyA( protoService, pdesc[i]->szName );
                  lstrcatA( protoService, PS_ICQ_SETCUSTOMSTATUSEX );
                  hideStatusMenu.cbSize = sizeof( ICQ_CUSTOM_STATUS );
                  hideStatusMenu.flags = CSSF_DISABLE_UI;
                  hideStatusMenu.wParam = &wp;
                  CallService( protoService, 0, ( LPARAM )&hideStatusMenu );
                }
                cslist_reset_status_menu();
              }
            }
            break;
          }
        }
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
      SetDlgItemText( hwndRNDlg, IDC_RN_VERSION, _T( CSLIST_VERSION ) );
      cslist_translate_dialog( hwndRNDlg );
	    {
		    LOGFONT lf = {0};
		    HFONT hfnt = (HFONT)SendDlgItemMessage(hwndRNDlg, IDC_RN_PLUGIN_NAME, WM_GETFONT, 0, 0);
		    GetObject(hfnt, sizeof(lf), &lf);
		    lf.lfWeight = FW_BOLD;
		    SendDlgItemMessage(hwndRNDlg, IDC_RN_PLUGIN_NAME, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);
	    }
    }
    break;
    case WM_COMMAND:
    {
      switch ( LOWORD( wParam ) )
      {
        case IDOK:
        case IDCANCEL:
        {
          DBWriteByte( CSLIST_MODULE_SHORT_NAME, "ShowRelNotes", IsDlgButtonChecked( hwndRNDlg, IDC_SHOWIT ) ? 0 : 1 );
          DBWriteWord( CSLIST_MODULE_SHORT_NAME, "RelNotes", CSLIST_RELNOTES );
          EndDialog( hwndRNDlg, LOWORD( wParam ) );
        }
        break;
      }
    }
    break;

    case WM_CTLCOLORSTATIC:
    {
		  if ( ((HWND)lParam == GetDlgItem(hwndRNDlg, IDC_WHITERECT)) ||
			   ((HWND)lParam == GetDlgItem(hwndRNDlg, IDC_RN_PLUGIN_ICON)) ||
         ((HWND)lParam == GetDlgItem(hwndRNDlg, IDC_RN_PLUGIN_NAME)) ||
			   ((HWND)lParam == GetDlgItem(hwndRNDlg, IDC_RN_DESCRIPTION)) )
        return ( BOOL )GetStockObject( WHITE_BRUSH );
      return FALSE;
    }
    break;
  }
  return FALSE;
}


// #############################################################################
// #################### END OF RELEASE NOTES DIALOG PROCEDURE ##################
// #############################################################################

// ############################# HELP FUNCTIONS ################################

static int cslist_init_menu_items( WPARAM wParam, LPARAM lParam )
{
  CLISTMENUITEM mi = { 0 };
  int miPlacement = CSLIST_MENU_PLACEMENT_DEFAULT;
  int protoCount, i;
  PROTOCOLDESCRIPTOR** pdesc;

  CallService( MS_CLIST_REMOVEMAINMENUITEM, ( WPARAM )hMainMenuItem, 0 );  
  if ( hHookMenuBuild != NULL )
    miPlacement = DBReadByte( CSLIST_MODULE_SHORT_NAME, "placement", CSLIST_MENU_PLACEMENT_DEFAULT );
  CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );
  
  ZeroMemory( &mi, sizeof( mi ) );
  mi.cbSize = sizeof( mi );
  mi.flags = CMIF_TCHAR;
  mi.hIcon = LoadIconExEx( "icon", IDI_CSLIST );
  mi.pszService = "CSList/ShowList";
  mi.position = 2000040000; // default bottom
  if ( miPlacement == CSLIST_MENU_PLACEMENT_ICQ && protoCount > 1 ) {
    mi.ptszName = TranslateT( CSLIST_STRING_CUSTOM_STATUS );
    if ( DBReadByte( CSLIST_MODULE_SHORT_NAME, "hideCS", 0 ) )
      mi.position = 00000001; // instead of cs ui
  }
  else {
    mi.ptszName = TranslateT( CSLIST_MODULE_LONG_NAME );
    if ( miPlacement == CSLIST_MENU_PLACEMENT_GLOBAL_TOP )
      mi.position = -0x7fffffff; // top
  }
  switch ( miPlacement ) {
    case CSLIST_MENU_PLACEMENT_ICQ:
      if ( protoCount > 1 )
        for ( i = 0; i < protoCount; i++ ) {
          if ( DBReadDword( pdesc[i]->szName, "UIN", 0 ) == 0 )
            continue;
          if ( gbAccountsSupported ) {
            PROTOACCOUNT* pacc = ( PROTOACCOUNT* )CallService( MS_PROTO_GETACCOUNT, 0, ( LPARAM )pdesc[i]->szName );
            mi.ptszPopupName = pacc->tszAccountName;
          }
          else {
#if defined( _UNICODE )
            TCHAR protoName[64];
            MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pdesc[i]->szName, -1, protoName, _MAX_PATH );
            mi.ptszPopupName = protoName;
#else
            mi.ptszPopupName = pdesc[i]->szName;
#endif
          }
          CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );
        }
      break;
    case CSLIST_MENU_PLACEMENT_GLOBAL:
    case CSLIST_MENU_PLACEMENT_GLOBAL_TOP:
    case CSLIST_MENU_PLACEMENT_GLOBAL_BOTTOM:
      CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );
      break;
    case CSLIST_MENU_PLACEMENT_MAIN:
      hMainMenuItem = ( HANDLE )CallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );
      break;
    //case CSLIST_MENU_PLACEMENT_TRAY:
    //  CallService( "CList/AddTrayMenuItem", 0, ( LPARAM )&mi );
    //  break;
  }
  return 0;
}


// ################## COMMANDS OF CSLIST #######################################

int cslist_add_item()
{ // final adding with helpItem
  int iItem;

  // define style

  if ( ModifiedPos >= 0 )
    iItem = ModifiedPos;
  else
    iItem = SendMessage( hList, LVM_GETITEMCOUNT, 0, 0 );

  memset( &LvItem, 0, sizeof( LvItem ) );
  LvItem.mask = LVIF_TEXT | LVIF_IMAGE;

  LvItem.cchTextMax = 256;
  LvItem.iItem = iItem;

  // first column

  LvItem.iSubItem = 0;
  LvItem.iImage = helpItem.ItemIcon; // use selected xicon
  SendMessage( hList, LVM_INSERTITEM, 0, ( LPARAM )&LvItem );

  // second column

  LvItem.iSubItem = 1;
  LvItem.pszText = helpItem.ItemTitle;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );

  // third column

  LvItem.iSubItem = 2;
  LvItem.pszText = helpItem.ItemMessage;
  SendMessage( hList, LVM_SETITEM, 0, ( LPARAM )&LvItem );

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
  cslist_clear_help_item();
  //SendMessage( hList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED | LVNI_SELECTED );
}

void cslist_clear_help_item() {
  ZeroMemory( &helpItem, sizeof( helpItem ) );
}

void cslist_sort_list() {
  ListView_SortItemsEx( hList, CSListSortProc, (LPARAM)0 );
}

void cslist_reset_status_menu()
{
  CLIST_INTERFACE* pcli;
  pcli = ( CLIST_INTERFACE* )CallService( MS_CLIST_RETRIEVE_INTERFACE, 0, 0 );
  if ( pcli && pcli->version > 4 )
    pcli->pfnReloadProtoMenus();
}

void cslist_import_statuses_from_icq() {
  int i, j, result, protoCount;
  PROTOCOLDESCRIPTOR** pdesc;
  result = DBReadByte( CSLIST_MODULE_SHORT_NAME, "deleteAfterImport", CSLIST_IMPORT_DELETION_DEFAULT );
  if ( result == 1 )
    result = IDYES;
  else
    result = MessageBox( hDlg, TranslateT( CSLIST_STRING_INPORT_DELETE_CONFIRM ), TranslateT( CSLIST_MODULE_LONG_NAME ), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION );
  CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );

  for ( j = 0; j < protoCount; j++ )
  {
    if ( DBReadDword( pdesc[j]->szName, "UIN", 0 ) == 0 )
      continue;
    for ( i = 0; i < 37; i++ )
    {
      DBVARIANT dbv = { 0 };
      char bufProto[64], bufName[64], bufMsg[64];
      lstrcpyA( bufProto, pdesc[j]->szName );
      cslist_clear_help_item( );
      mir_snprintf( bufName, 32, "XStatus%luName", i );
      mir_snprintf( bufMsg, 32, "XStatus%luMsg", i );
      helpItem.ItemIcon = i - 1;
      DBReadTString( bufProto, bufName, &dbv );
      lstrcpy( helpItem.ItemTitle, dbv.ptszVal );
      DBReadTString( bufProto, bufMsg, &dbv );
      lstrcpy( helpItem.ItemMessage, dbv.ptszVal );
      if ( lstrlen( helpItem.ItemTitle ) || lstrlen( helpItem.ItemMessage ) )
        cslist_add_item();
      if ( result == IDYES )
      {
        DBDelete( bufProto, bufName );
        DBDelete( bufProto, bufMsg );
      }
    }
  }
  cslist_sort_list();
  bChanged = 1;
}

int cslist_setting_changed( WPARAM wParam, LPARAM lParam )
{
  HANDLE hContact = ( HANDLE )wParam;
  DBCONTACTWRITESETTING *cws = ( DBCONTACTWRITESETTING* )lParam;
  DBVARIANT dbv = { 0 };
  DWORD dwUin = 0x0;
  DWORD dwEasteredUin = 0x0;

  if ( hContact == NULL && !lstrcmpA( cws->szSetting, "UIN" ) ) // if user changes his UIN on any account
  {
    cslist_reset_status_menu();
    return 0;
  }

  else if ( hContact == NULL || lstrcmpA( cws->szSetting, "Status" ) ) // own setting or not a status-related setting
    return 0;

  dwEasteredUin = DBReadDword( CSLIST_MODULE_SHORT_NAME, "EasterEggedUIN", 0x0 );

  if ( dwEasteredUin == 0x0 ) // no spyed UIN found
    return 0;

  DBGetContactSettingString( hContact, "Protocol", "p", &dbv );

  dwUin = DBGetContactSettingDword( hContact, dbv.pszVal, "UIN", 0x0 );

  if ( dwUin != 0x0 ) // if this is an ICQ user
    if ( dwUin == dwEasteredUin ) // if spyed UIN is equivalent to this contact
      if ( DBGetContactSettingWord( hContact, "UserOnline", "OldStatus", 0 ) == ID_STATUS_OFFLINE )
        if ( cws->value.wVal > ID_STATUS_OFFLINE )
          cslist_set_status( NULL, IDC_EXIT );

  return 0;
}

// ################## DB - LOAD AND SAVE #######################################

int cslist_initialize_list_content( HWND hwndDlg )
{
  // get DB string, parse to statuses, add by helpItem
  DBVARIANT dbv = { DBVT_TCHAR };
  int parseResult;
  int dbLoadResult;
  const TCHAR* rowDelim = _T( "" ); // new line
  TCHAR *row = NULL;

  dbLoadResult = DBReadTString( CSLIST_MODULE_SHORT_NAME, "listhistory", &dbv );
  if ( dbv.ptszVal )
  {
    row = tcstok( dbv.ptszVal, rowDelim );
    while( row != NULL ) {
      // process current row..
      parseResult = cslist_parse_row( row );
      // ..add item..
      if ( parseResult == TRUE ) cslist_add_item();
      // ..and go to the other, while some remains
      row = tcstok( NULL, rowDelim );
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
  TCHAR scanfFormat[64];

  lstrcpy( pTitle, _T( "" ) );
  lstrcpy( pMsg, _T( "" ) );

  // database row format: "%2[^]%64[^]%2048[^]%2[^]"
  
  mir_sntprintf(
    scanfFormat,
    sizeof( scanfFormat ),
    _T( "%%%d[^]%%%d[^]%%%d[^]%%%d[^]" ), // %% makes %, %d makes number :)
    2,
    CSLIST_XTITLE_LIMIT,
    CSLIST_XMESSAGE_LIMIT,
    2
  );

  tsscanf( row, scanfFormat, pIcon, pTitle, pMsg, pFav );

  if ( lstrlen( pTitle ) + lstrlen( pMsg ) > 0 )
  {
    pIconInt = ttoi( pIcon );
    helpItem.ItemIcon = pIconInt;
    if ( lstrlen( pTitle ) )
      lstrcpy( helpItem.ItemTitle, pTitle );
    if ( lstrlen( pMsg ) )
      lstrcpy( helpItem.ItemMessage, pMsg );
  }
  else {
    return FALSE;
  }
  return TRUE;
}

int cslist_save_list_content( HWND hwndDlg )
{
  TCHAR cImageToString[64];
  int i = 0;
  LPTSTR dbString = { 0 };
  TCHAR dbStringTmp[65536];
  lstrcpy( dbStringTmp, _T( "" ) ); // set to empty - caused "dizzy" string begin x) O_o
  lstrcpy( cImageToString, _T( "" ) );
  for ( i = 0; i < ListView_GetItemCount( hList ); i++ )
  {
    // insert xint to string
    LvItem.iItem = i;
    LvItem.iSubItem = 0;
    ListView_GetItem( hList, &LvItem );
    tcprintf( cImageToString, 8, _T( "%d" ), LvItem.iImage );
    lstrcat( dbStringTmp, cImageToString );
    lstrcpy( cImageToString, _T( "" ) );
    lstrcat( dbStringTmp, _T( "" ) );
    ListView_GetItemText( hList, i, 1, helpItem.ItemTitle, CSLIST_XTITLE_LIMIT );
    if ( tcslen( helpItem.ItemTitle ) > 0 ) lstrcat( dbStringTmp, helpItem.ItemTitle );
    lstrcat( dbStringTmp, _T( "" ) );
    ListView_GetItemText( hList, i, 2, helpItem.ItemMessage, CSLIST_XMESSAGE_LIMIT );
    if ( tcslen( helpItem.ItemMessage ) > 0 ) lstrcat( dbStringTmp, helpItem.ItemMessage );
    lstrcat( dbStringTmp, _T( "0" ) );
  }
  dbString = dbStringTmp;
  // save to DB
  DBWriteTString( CSLIST_MODULE_SHORT_NAME, "listhistory", dbString );
  free( dbString );
  //free( dbStringTmp );
  lstrcpy( helpItem.ItemTitle, _T( "" ) );
  lstrcpy( helpItem.ItemMessage, _T( "" ) );
  return 0;
}


// ######################### CREATING DIALOG ###################################

int cslist_set_status( HWND hwndDlg, WORD setAction )
{
  ICQ_CUSTOM_STATUS helpStatus = { 0 };
  memset( &helpStatus, 0, sizeof( helpStatus ) );
  helpStatus.cbSize = sizeof( ICQ_CUSTOM_STATUS );
  helpStatus.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_TCHAR;
  iSelect = SendMessage( hList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED | LVNI_SELECTED ); // get which row is selected
  if ( iSelect == -1 && ( setAction == IDC_APPLY || setAction == IDOK ) ) // no status selected
    flag = 0;
  else
  { // else continue setting
    if ( setAction == IDC_EXIT )
    { // set null status
      helpItem.ItemIcon = 0;
      lstrcpy( helpItem.ItemTitle, _T( "" ) );
      lstrcpy( helpItem.ItemMessage, _T( "" ) );
      helpStatus.status = &helpItem.ItemIcon;
      helpStatus.ptszName = helpItem.ItemTitle;
      helpStatus.ptszMessage = helpItem.ItemMessage;
    }
    else if ( setAction == IDC_APPLY || setAction == IDOK )
    { //set selected status
      LvItem.iItem = iSelect;
      LvItem.iSubItem = 0;
      ListView_GetItem( hList, &LvItem );
      helpItem.ItemIcon = LvItem.iImage; // TODO: get xicon int and set ..
      ListView_GetItemText( hList, iSelect, 1, helpItem.ItemTitle, CSLIST_XTITLE_LIMIT );
      ListView_GetItemText( hList, iSelect, 2, helpItem.ItemMessage, CSLIST_XMESSAGE_LIMIT );
      helpItem.ItemIcon++; // need revision of redirected xstatus numbers xo)
      helpStatus.status = &helpItem.ItemIcon;
      helpStatus.ptszName = variables_parsedup( helpItem.ItemTitle, NULL, NULL );
      helpStatus.ptszMessage = variables_parsedup( helpItem.ItemMessage, NULL, NULL );
    }
    else {
      return 0;
    }
    { // setting xstatus on all ICQ accounts
      char protoService[64];
      PROTOCOLDESCRIPTOR** pdesc;
      int protoCount = 0, i;
      CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );
      for ( i = 0; i < protoCount; i++ ) {
        if ( DBReadDword( pdesc[i]->szName, "UIN", 0 ) == 0 )
          continue;
        lstrcpyA( protoService, pdesc[i]->szName );
        lstrcatA( protoService, PS_ICQ_SETCUSTOMSTATUSEX );
        CallService( protoService, 0, ( LPARAM )&helpStatus ); // f**king function, 4 hours of thinking xDD
      }
    }
    // ..and clean helper + selection
    cslist_clear_selection();
  }
  return 0;
}

void cslist_init_icons( void )
{
  int i;
  SKINICONDESC sid = { 0 };
  TCHAR szFile[MAX_PATH];
  GetModuleFileName( hInst, szFile, MAX_PATH );

  sid.cbSize = sizeof( SKINICONDESC );
  sid.ptszDefaultFile = szFile;
  sid.flags = SIDF_ALL_TCHAR;
  sid.cx = sid.cy = 16;
  sid.ptszSection = TranslateT( CSLIST_MODULE_LONG_NAME );

  for ( i = 0; i < SIZEOF( cslforms ); i++ )
  {
    char szSettingName[64];
    mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", CSLIST_MODULE_SHORT_NAME, cslforms[i].pszIconIcoLib );
    sid.pszName = szSettingName;
    sid.ptszDescription = TranslateTS( cslforms[i].ptszDescr );
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

void cslist_translate_dialog( HWND windowHandle ) {
  LANGPACKTRANSLATEDIALOG lptd;

  lptd.cbSize = sizeof( lptd );
  lptd.flags = 0;
  lptd.hwndDlg = windowHandle;
  lptd.ignoreControls = NULL;
  CallService( MS_LANGPACK_TRANSLATEDIALOG, 0, ( LPARAM )&lptd );
}


/* ########################################################################## 80

                              Custom Status List
                              __________________

                                  by jarvis

// ########################################################################## */
