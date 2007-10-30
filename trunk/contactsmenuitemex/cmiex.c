#include "cmiex.h"

/* #############################################################################
################################################################################
#################################  MIRANDA STUFF  ##############################
################################################################################
############################################################################# */

// eternity : ready for 0.8 core, baby xD
PLUGININFOEX pluginInfoEx = {
  sizeof(PLUGININFOEX),
  MODULENAME,
  __VERSION_DWORD,
  "Get and set user properties by contact menu ("__DATE__") - exclusive merge of MenuItemEx and ContactsEx",
  "Heiko Schillinger, Baloo, jarvis",
  "mike.taussick@seznam.cz, baloo@bk.ru, micron@nexgo.de",
  "© 2007 jarvis, © 2006-07 Baloo, © 2001-03 Heiko Schillinger, © 2002 Robert Rainwater",
  "http://dev.mirandaim.ru/jarvis/",
  0,  //not transient
  0,  //doesn't replace anything built-in
  // Generate your own unique id for your plugin.
  // Do not use this UUID!
  // Use uuidgen.exe to generate the uuuid
  { 0x977be6af, 0xcd5f, 0x4fe1, { 0xaf, 0x2f, 0x53, 0x5b, 0x8a, 0xae, 0xb7, 0x49 } } // {977BE6AF-CD5F-4fe1-AF2F-535B8AAEB749}
};

PLUGININFO pluginInfo = {
  sizeof(PLUGININFO),
    MODULENAME,
    __VERSION_DWORD,
    "Get and set user properties by contact menu ("__DATE__") - exclusive merge of MenuItemEx and ContactsEx",
    "Heiko Schillinger, Baloo, jarvis",
    "mike.taussick@seznam.cz, baloo@bk.ru, micron@nexgo.de",
    "© 2007 jarvis, © 2006-07 Baloo, © 2001-03 Heiko Schillinger, © 2002 Robert Rainwater",
    "http://dev.mirandaim.ru/jarvis/",
    0, 0
};

// called when the contact-menu is built
int BuildMenu( WPARAM wparam, LPARAM lparam )
{
  CLISTMENUITEM cliAV = { 0 }, cliNV, cliHFL, cliIGN, cliPROTO, cliGRP, cliADD,
                cliREQ, cliCID, cliRECV, cliSTAT, cliCIP, cliCMV, cliSLO;
  WORD flags = DBGetContactSettingWord( NULL, MODULENAME, "flags", default_flags );

  cliAV.cbSize = sizeof( CLISTMENUITEM );
  cliAV.flags = CMIM_FLAGS;
  cliAV.hIcon = NULL;
  cliAV.pszContactOwner = NULL;
  cliNV = cliHFL = cliIGN = cliPROTO = cliGRP = cliADD = cliREQ = cliCID =
    cliRECV = cliSTAT = cliCIP = cliCMV = cliSLO = cliAV;

  if ( flags & VF_AV ) ShowItem( &cliAV, hmenuVis );
  else HideItem( &cliAV, hmenuVis );

  if ( flags & VF_NV ) ShowItem( &cliNV, hmenuOff );
  else HideItem( &cliNV, hmenuOff );

  if ( flags & VF_HFL ) ShowItem( &cliHFL, hmenuHide );
  else HideItem( &cliHFL, hmenuHide );

  if ( flags & VF_IGN ) ShowItem( &cliIGN, hmenuIgnore );
  else HideItem( &cliIGN, hmenuIgnore );

  if ( flags & VF_PROTO ) ShowItem( &cliPROTO, hmenuProto );
  else HideItem( &cliPROTO, hmenuProto );

  if ( flags & VF_GRP ) ShowItem( &cliGRP, hmenuGroup );
  else HideItem( &cliGRP, hmenuGroup );

  if ( flags & VF_ADD ) ShowItem( &cliADD, hmenuAdded );
  else HideItem( &cliADD, hmenuAdded );

  if ( flags & VF_REQ ) ShowItem( &cliREQ, hmenuAuthReq );
  else HideItem( &cliREQ, hmenuAuthReq );

  if ( flags & VF_CID )
  {
    ShowItem( &cliCID, hmenuCopyID );
    ModifyCopyID( &cliCID, ( HANDLE )wparam );
  }
  else HideItem( &cliCID, hmenuCopyID );

  if ( flags & VF_RECV && DirectoryExists( ( HANDLE )wparam ) )
    ShowItem( &cliRECV, hmenuRecvFiles );
  else
    HideItem( &cliRECV, hmenuRecvFiles );

  if ( flags & VF_STAT && StatusMsgExists( ( HANDLE )wparam ) )
  {
    ShowItem( &cliSTAT, hmenuStatusMsg );
    ModifyStatusMsg( &cliSTAT, ( HANDLE )wparam );
  }
  else HideItem( &cliSTAT, hmenuStatusMsg );

  if ( flags & VF_CIP && IPExists( ( HANDLE )wparam ) )
  {
    ShowItem( &cliCIP, hmenuCopyIP );
    ModifyCopyIP( &cliCIP, ( HANDLE )wparam );
  }
  else HideItem( &cliCIP, hmenuCopyIP );

  if ( flags & VF_CMV && MirVerExists( ( HANDLE )wparam ) )
  {
    ShowItem( &cliCMV, hmenuCopyMirVer );
    ModifyCopyMirVer( &cliCMV, ( HANDLE )wparam );
  }
  else HideItem( &cliCMV, hmenuCopyMirVer );

  if ( flags & VF_SLO )
  {
    ShowItem( &cliSLO, hmenuShowLastOnline );
    ModifyLastOnline( &cliSLO, ( HANDLE )wparam );
  }
  else HideItem( &cliSLO, hmenuShowLastOnline );

  if( ( flags & VF_AV ) || ( flags & VF_NV ) )
  {
    int apparent = DBGetContactSettingWord( ( HANDLE )wparam, ( const char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, wparam, 0 ), "ApparentMode", 0 );
    if( isVisSupport( ( HANDLE )wparam ) )
      ModifyVisibleSet( &cliAV, apparent == ID_STATUS_ONLINE );
    else HideItem( &cliAV, hmenuVis );
    if( isInvSupport( ( HANDLE )wparam ) )
      ModifyInvisSet( &cliNV, apparent == ID_STATUS_OFFLINE );
    else HideItem( &cliAV, hmenuOff );
  }
  
  // update contact info
  
  { // TESTING: updating user's details
    CallContactService( ( HANDLE )wparam, PSS_GETINFO, 0, 0 );
  }

  ModifyHidden( &cliHFL, DBGetContactSettingByte( ( HANDLE )wparam, "CList", "Hidden", 0 ) );
  return 0;
}

// called when all modules are loaded
static int PluginInit( WPARAM wparam, LPARAM lparam )
{
  CLISTMENUITEM cli;

  //  char path[MAX_PATH];
  //  GetWindowsDirectory(path,MAX_PATH-1);

  CreateServiceFunction( MS_SETINVIS, onSetInvis );
  CreateServiceFunction( MS_SETVIS, onSetVis );
  CreateServiceFunction( MS_HIDE, onHide );
  CreateServiceFunction( MS_IGNORE, onShowIgnore );
  CreateServiceFunction( MS_PROTO, onShowProto );
  CreateServiceFunction( MS_GROUP, onShowGroup );
  CreateServiceFunction( MS_ADDED, onSendAdded );
  CreateServiceFunction( MS_AUTHREQ, onSendAuthRequest );
  CreateServiceFunction( MS_COPYID, onCopyID );
  CreateServiceFunction( MS_RECVFILES, onRecvFiles );
  CreateServiceFunction( MS_STATUSMSG, onCopyStatusMsg );
  CreateServiceFunction( MS_COPYIP, onCopyIP );
  CreateServiceFunction( MS_COPYMIRVER, onCopyMirVer );

  
  // initialize icons
  
  if(ServiceExists(MS_SKIN2_ADDICON))
    initIcoLibIcons();


  // updater section
  {
    static Update upd = {0};
    
//#if defined(_UNICODE)
	  static char szCurrentVersion[30];
	  static char *szVersionUrl = "http://dev.mirandaim.ru/jarvis/";
	  static char *szUpdateUrl = "http://mirandapack.ic.cz/eternity_plugins/cmiex.zip";
	  // todo: upload to FL and complete updating also from FL
	  //static char *szFLVersionUrl = "http://addons.miranda-im.org/details.php?action=viewfile&id=3483";
	  //static char *szFLUpdateurl = "http://addons.miranda-im.org/feed.php?dlfile=3483";
//#endif    
	  static char *szPrefix = "ContactsMenuItemEx</a> ";
  	
	  upd.cbSize = sizeof(upd);
	  upd.szComponentName = pluginInfo.shortName;
    //upd.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO *)&pluginInfo, szCurrentVersion); // TODO support for < 0.8 cores
	  upd.pbVersion = (BYTE *)CreateVersionStringPluginEx((PLUGININFOEX *)&pluginInfoEx, szCurrentVersion);
	  upd.cpbVersion = strlen((char *)upd.pbVersion);
	  //upd.szVersionURL = szFLVersionUrl;
	  //upd.szUpdateURL = szFLUpdateurl;
//#if defined(_UNICODE)
	  upd.pbVersionPrefix = (BYTE *)"ContactsMenuItemEx</a> ";
//#endif
	  upd.cpbVersionPrefix = strlen((char *)upd.pbVersionPrefix);

	  upd.szBetaUpdateURL = szUpdateUrl;
	  upd.szBetaVersionURL = szVersionUrl;
	  upd.pbVersion = szCurrentVersion;
	  upd.cpbVersion = lstrlenA(szCurrentVersion);
	  upd.pbBetaVersionPrefix = (BYTE *)szPrefix;
	  upd.cpbBetaVersionPrefix = strlen((char *)upd.pbBetaVersionPrefix);

	  if(ServiceExists(MS_UPDATE_REGISTER))
		  CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
  }
  // menu items section

  cli.cbSize = sizeof( CLISTMENUITEM );
  cli.flags = 0;
  cli.hIcon = NULL;
  cli.pszContactOwner = NULL;

  cli.position = 120000;
  cli.pszName = Translate( MIT_AV );
  cli.pszService=MS_SETVIS;
  cli.hIcon = LoadIconEx("cmiex_av");
  hmenuVis=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120001;
  cli.pszName=Translate( MIT_NV );
  cli.pszService=MS_SETINVIS;
  cli.hIcon = LoadIconEx("cmiex_nv");
  hmenuOff=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120002;
  cli.pszName=Translate( MIT_HFL );
  cli.pszService=MS_HIDE;
  cli.hIcon = LoadIconEx("cmiex_hfl");
  hmenuHide=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120003;
  cli.pszName=Translate( MIT_IGN );
  cli.pszService=MS_IGNORE;
  cli.hIcon = LoadIconEx("cmiex_ign");
  hmenuIgnore=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120004;
  cli.pszName=Translate( MIT_PROTO );
  cli.pszService=MS_PROTO;
  cli.hIcon = LoadIconEx("cmiex_proto");
  hmenuProto=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120005;
  cli.pszName=Translate( MIT_GRP );
  cli.pszService=MS_GROUP;
  cli.hIcon = LoadIconEx("cmiex_grp");
  hmenuGroup=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120006;
  cli.pszName=Translate( MIT_ADD );
  cli.pszService=MS_ADDED;
  cli.hIcon = LoadIconEx("cmiex_add");
  hmenuAdded=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120007;
  cli.pszName=Translate( MIT_REQ );
  cli.pszService=MS_AUTHREQ;
  cli.hIcon = LoadIconEx("cmiex_req");
  hmenuAuthReq=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);
  
  cli.position=120008;
  cli.pszName=Translate( MIT_CID );
  cli.pszService=MS_COPYID;
  cli.hIcon = LoadIconEx("cmiex_cid");
  hmenuCopyID=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120009;
  cli.pszName=Translate( MIT_RECV );
  cli.pszService=MS_RECVFILES;
  cli.hIcon = LoadIconEx("cmiex_recv");
  hmenuRecvFiles=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120010;
  cli.pszName=Translate( MIT_STAT );
  cli.pszService=MS_STATUSMSG;
  cli.hIcon = LoadIconEx("cmiex_stat");
  hmenuStatusMsg=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120011;
  cli.pszName=Translate( MIT_CIP );
  cli.pszService=MS_COPYIP;
  cli.hIcon = LoadIconEx("cmiex_cip");
  hmenuCopyIP=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120012;
  cli.pszName=Translate( MIT_CMV );
  cli.pszService=MS_COPYMIRVER;
  cli.hIcon = LoadIconEx("cmiex_cmv");
  hmenuCopyMirVer=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&cli);

  cli.position=120013;
  cli.pszName = Translate( MIT_SLO );
  cli.pszService = MS_LASTONLINE;
  cli.hIcon = LoadIconEx("cmiex_slo");
  hmenuShowLastOnline = ( HANDLE )CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&cli );

  HookEvent(ME_CLIST_PREBUILDCONTACTMENU,BuildMenu);
  HookEvent(ME_OPT_INITIALISE,OptionsInit);
  HookEvent( ME_DB_CONTACT_SETTINGCHANGED, cmiex_ContactSettingChanged );

  { // is this first run?
    if ( DBGetContactSettingByte( NULL, MODULENAME, "FirstRun", 0 ) != 1 )
    {
      DBWriteContactSettingWord( NULL, MODULENAME, "flags", 0 );
      DBWriteContactSettingWord( NULL, MODULENAME, "RelNotes", 0 );
      DBWriteContactSettingByte( NULL, MODULENAME, "FirstRun", 1 );
      DBWriteContactSettingString( NULL, MODULENAME, "LastOnlineFormat", DEFAULT_LASTONLINE_FORMAT );
    }
  }
  { // release notes dialog
    int relnotes;
    relnotes = DBGetContactSettingWord( NULL, MODULENAME, "RelNotes", 0 );
    if ( relnotes != CMIEX_RELNOTES )
    {
      DialogBox( hinstance, MAKEINTRESOURCE( IDD_RELNOTES ), NULL, ( DLGPROC )RelNotesProc );
      DBWriteContactSettingWord( NULL, MODULENAME, "RelNotes", CMIEX_RELNOTES );
    }
  }

  /*/ updater plugin support : eternity TODO: my updateinfo
  if(ServiceExists(MS_UPDATE_REGISTERFL)) {
    CallService(MS_UPDATE_REGISTERFL, (WPARAM)2883, (LPARAM)&pluginInfo);
  }/*/

  hdummy=CreateWindow("BUTTON","",0,0,0,0,0,NULL,NULL,hinstance,NULL);
  mainProc=(WNDPROC)SetWindowLong(hdummy,GWL_WNDPROC,(LPARAM)CatchMenuMsg);
  return 0;
}

__declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion )
{
  return &pluginInfoEx;
}

__declspec( dllexport ) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion )
{
  return &pluginInfo;
}

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
  return interfaces;
}

__declspec(dllexport) int Load(PLUGINLINK *link)
{
  pluginLink=link;
  HookEvent(ME_SYSTEM_MODULESLOADED,PluginInit);
  return 0;
}

__declspec(dllexport) int Unload(void)
{
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
  hinstance=hinst;
  return 1;
}

static int cmiex_ContactSettingChanged( WPARAM wParam, LPARAM lParam )
{ // NSN
  DBCONTACTWRITESETTING *cws = ( DBCONTACTWRITESETTING* )lParam;
	WORD newStatus = 0, oldStatus = 0;
	DWORD dwStatuses = 0;

  if( ( HANDLE )wParam == NULL || lstrcmpA( cws->szSetting, "Status" ) )
    return 0;
  newStatus = cws->value.wVal;
  oldStatus = DBGetContactSettingWord((HANDLE)wParam,"UserOnline","OldStatus2",ID_STATUS_OFFLINE );
  if (oldStatus == newStatus) return 0;
  if (oldStatus == ID_STATUS_OFFLINE)
  { // TESTING: updating user's details
    CallContactService( ( HANDLE )wParam, PSS_GETINFO, 0, 0 );
  }
  if (newStatus == ID_STATUS_OFFLINE)
  {
    // set offline timestamp for this contact
    time_t tCurrentTime;
    char *lpzProto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )wParam, 0);

    tCurrentTime = time( NULL );
    DBWriteContactSettingDword( ( HANDLE )wParam, lpzProto, "LogoffTS", ( DWORD )tCurrentTime);
  }
  DBWriteContactSettingWord( ( HANDLE )wParam, "UserOnline", "OldStatus2", newStatus);
  return 0;
}


/* #############################################################################
################################################################################
###################################  VISIBLE  ##################################
################################################################################
############################################################################# */

BOOL isVisSupport(HANDLE hContact)
{
  char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
  if(szProto==NULL) return 0;
  return CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0)&PF1_INVISLIST;
}

// set visible-flag in db
int onSetVis(WPARAM wparam,LPARAM lparam)
{
  CallContactService((HANDLE)wparam,PSS_SETAPPARENTMODE,(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0)==ID_STATUS_ONLINE)?0:ID_STATUS_ONLINE,0);
  return 0;
}

void ModifyVisibleSet(CLISTMENUITEM *cli,int mode)
{
  cli->flags|=CMIM_ICON;
  cli->hIcon = LoadIconEx( "cmiex_av" );
  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuVis,(LPARAM)cli);
}


/* #############################################################################
################################################################################
##################################  INVISIBLE  #################################
################################################################################
############################################################################# */

BOOL isInvSupport(HANDLE hContact)
{
  char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
  if(szProto==NULL) {return 0;}
  return CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_1,0)&PF1_VISLIST;
}

// set the invisible-flag in db
int onSetInvis(WPARAM wparam,LPARAM lparam)
{
  CallContactService((HANDLE)wparam,PSS_SETAPPARENTMODE,(DBGetContactSettingWord((HANDLE)wparam,(const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0),"ApparentMode",0)==ID_STATUS_OFFLINE)?0:ID_STATUS_OFFLINE,0);
  return 0;
}

void ModifyInvisSet(CLISTMENUITEM *cli,int mode)
{
  cli->flags|=CMIM_ICON;
  cli->hIcon = LoadIconEx( "cmiex_nv" );
  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuOff,(LPARAM)cli);
}


/* #############################################################################
################################################################################
###############################  HIDE FROM LIST  ###############################
################################################################################
############################################################################# */

int onHide(WPARAM wparam,LPARAM lparam)
{
  DBWriteContactSettingByte((HANDLE)wparam,"CList","Hidden",(byte)!DBGetContactSettingByte((HANDLE)wparam,"CList","Hidden",0));
  CallService(MS_CLUI_SORTLIST,0,0);
  return 0;
}

void ModifyHidden(CLISTMENUITEM *cli,int mode)
{
  //  cli->flags|=CMIF_CHECKED;
  cli->flags|=CMIM_ICON;
  cli->hIcon = LoadIconEx( "cmiex_hfl" );
  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuHide,(LPARAM)cli);
}


/* #############################################################################
################################################################################
####################################  IGNORE  ##################################
################################################################################
############################################################################# */

// Popup the Ignore - menu
int onShowIgnore(WPARAM wparam,LPARAM lparam)
{
  POINT pos;
  int loop,all;

  hpopupIgnore=GetSubMenu(LoadMenu(hinstance,MAKEINTRESOURCE(IDR_MENU)),0);
  GetCursorPos(&pos);

  for(all=0,loop=1;loop<=5;loop++)
  {
    if(CallService(MS_IGNORE_ISIGNORED,wparam,(LPARAM)loop))
    {
      all++;
      CheckMenuItem(hpopupIgnore,loop,MF_BYPOSITION|MF_CHECKED);
    }
  }
  if(all==5)
    CheckMenuItem(hpopupIgnore,0,MF_BYPOSITION|MF_CHECKED);

  TrackPopupMenu(hpopupIgnore,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,hdummy,NULL);
  DestroyMenu(hpopupIgnore);
  SetWindowLong(hdummy,GWL_USERDATA,(LPARAM)wparam);
  return 0;
}


/* #############################################################################
################################################################################
###################################  PROTOCOL  #################################
################################################################################
############################################################################# */

// Popup the Proto - menu
int onShowProto(WPARAM wparam,LPARAM lparam)
{
  POINT pos;
  int loop,offset=0,pcount;
  MENUITEMINFO mii;
  PROTOCOLDESCRIPTOR** pdesc;

  hpopupProto=GetSubMenu(LoadMenu(hinstance,MAKEINTRESOURCE(IDR_MENU)),1);
  GetCursorPos(&pos);

  ZeroMemory(&mii,sizeof(mii));
  mii.cbSize=sizeof(MENUITEMINFO);
  mii.fMask=MIIM_STRING|MIIM_ID|MIIM_STATE;
  mii.fType=MFT_STRING;

  CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&pcount,(LPARAM)&pdesc);

  for(loop=0;loop<pcount;loop++)
  {
    if(pdesc[loop]->type==PROTOTYPE_PROTOCOL)
    {
      mii.wID=IDM_PROTOS+offset;
      mii.fState=CallService(MS_PROTO_ISPROTOONCONTACT,wparam,(LPARAM)pdesc[loop]->szName)?MFS_CHECKED:MFS_UNCHECKED;
      mii.dwTypeData=pdesc[loop]->szName;
      if(mii.fState==MFS_CHECKED)
        szmainProto=pdesc[loop]->szName;

      InsertMenuItem(hpopupProto,offset,TRUE,&mii);
      protodat[offset++]=pdesc[loop]->szName;
    }
  }

  RemoveMenu(hpopupProto,IDM_DUMMY,MF_BYCOMMAND);

  TrackPopupMenu(hpopupProto,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,hdummy,NULL);
  DestroyMenu(hpopupProto);
  SetWindowLong(hdummy,GWL_USERDATA,(LPARAM)wparam);
  return 0;
}


/* #############################################################################
################################################################################
#####################################  GROUP  ##################################
################################################################################
############################################################################# */

int onShowGroup(WPARAM wparam,LPARAM lparam)
{
  POINT pos;
  int loop,offset=0;
  MENUITEMINFO mii;
  DBVARIANT dbvgrplst,dbvcntctgrp;
  char idstr[4];

  hpopupGroup=GetSubMenu(LoadMenu(hinstance,MAKEINTRESOURCE(IDR_MENU)),1);
  GetCursorPos(&pos);

  ZeroMemory(&mii,sizeof(mii));
  mii.cbSize=sizeof(MENUITEMINFO);
  mii.fMask=MIIM_STRING|MIIM_ID|MIIM_STATE;
  mii.fType=MFT_STRING;

  mii.fState=DBGetContactSetting((HANDLE)wparam,"CList","Group",&dbvcntctgrp)?MFS_CHECKED:MFS_UNCHECKED;
  mii.dwTypeData=Translate("<none>");
  mii.wID=IDM_GROUPS+offset;
  InsertMenuItem(hpopupGroup,offset,TRUE,&mii);
  groupdat[offset++]=Translate("<none>");

  for(loop=0;loop<MAX_GROUPS-1;loop++)
  {
    char *szgrp;
    itoa(loop,idstr,10);

    if(DBGetContactSetting(NULL,"CListGroups",idstr,&dbvgrplst)) break;
    DBGetContactSetting((HANDLE)wparam,"CList","Group",&dbvcntctgrp);

    mii.wID=IDM_GROUPS+offset;
    mii.fState=lstrcmp(dbvgrplst.pszVal+1,dbvcntctgrp.pszVal)?MFS_UNCHECKED:MFS_CHECKED;

    szgrp=strrchr(dbvgrplst.pszVal+1,'\\');

    if(szgrp==NULL)
      szgrp=dbvgrplst.pszVal+1;

    mii.dwTypeData=szgrp;
    InsertMenuItem(hpopupGroup,offset,TRUE,&mii);
    groupdat[offset++]=strdup(dbvgrplst.pszVal+1);
    DBFreeVariant(&dbvgrplst);
    DBFreeVariant(&dbvcntctgrp);
  }

  RemoveMenu(hpopupGroup,IDM_DUMMY,MF_BYCOMMAND);

  TrackPopupMenu(hpopupGroup,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,hdummy,NULL);
  DestroyMenu(hpopupGroup);
  SetWindowLong(hdummy,GWL_USERDATA,(LPARAM)wparam);
  return 0;
}


/* #############################################################################
################################################################################
################################  YOU WERE ADDED  ##############################
################################################################################
############################################################################# */

int onSendAdded(WPARAM wparam,LPARAM lparam)
{
  CallContactService((HANDLE)wparam,PSS_ADDED,0,0);
  return 0;
}


/* #############################################################################
################################################################################
################################  AUTH REQUESTED  ##############################
################################################################################
############################################################################# */

int onSendAuthRequest(WPARAM wparam,LPARAM lparam)
{
  DWORD flags;
  char *szProto;

  szProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,wparam,0);

  flags=CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_4,0);
  if (flags&PF4_NOCUSTOMAUTH)  CallContactService((HANDLE)wparam,PSS_AUTHREQUEST,0,(LPARAM)"");
  else CreateDialogParam(hinstance,MAKEINTRESOURCE(IDD_AUTHREQ),(HWND)CallService(MS_CLUI_GETHWND,0,0),AuthReqWndProc,(LPARAM)wparam);

  return 0;
}

BOOL CALLBACK AuthReqWndProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
  static HANDLE hcontact;

  switch(msg){
  case WM_INITDIALOG:
    hcontact=(HANDLE)lparam;
    break;

  case WM_COMMAND:
    switch(LOWORD(wparam)) {
  case IDOK:
    {
      char szReason[256];

      GetDlgItemText(hdlg,IDC_REASON,szReason,256);
      CallContactService(hcontact,PSS_AUTHREQUEST,0,(LPARAM)szReason);
    } // fall through
  case IDCANCEL:
    DestroyWindow(hdlg);
    break;
    }
    break;
  }

  return 0;
}


/* #############################################################################
################################################################################
####################################  COPY ID  #################################
################################################################################
############################################################################# */

void GetID(HANDLE hContact,LPSTR szProto,LPSTR szID)
{
  DBVARIANT dbv_uniqueid;
  LPSTR uID = (LPSTR) CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
  *szID='\0';
  if( uID && DBGetContactSetting(hContact, szProto, uID ,&dbv_uniqueid)==0 ) {
    if (dbv_uniqueid.type ==DBVT_DWORD)
      wsprintf(szID, "%u", dbv_uniqueid.dVal);
    else if (dbv_uniqueid.type ==DBVT_WORD)
      wsprintf(szID, "%u", dbv_uniqueid.wVal);
    else if (dbv_uniqueid.type ==DBVT_BLOB)
      wsprintf(szID, "%s", dbv_uniqueid.cpbVal);
    else
      wsprintf(szID, "%s", dbv_uniqueid.pszVal);
  }
  DBFreeVariant(&dbv_uniqueid);
}

void ModifyCopyID(CLISTMENUITEM *cli,HANDLE hContact)
{
  LPSTR szProto;
  char szID[256],buffer[256];
  //HICON hIconCID;

  strcpy(buffer,Translate("Copy ID"));

  cli->flags|=CMIM_ICON|CMIM_NAME;

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
  if(!szProto) {
    HideItem(cli,hmenuCopyID);
    return;
  }
  cli->hIcon = LoadIconEx( "cmiex_cid" );
  //DestroyIcon(hIconCID);
  //hIconCID = cli->hIcon;

  GetID(hContact,szProto,(LPSTR)&szID);
  if(szID[0]) {
    strcat(buffer," ["); strcat(buffer,szID); strcat(buffer,"]");
    cli->pszName = (char*)&buffer;
  }
  else {
    cli->flags=CMIM_FLAGS|CMIF_HIDDEN;
  }
  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuCopyID,(LPARAM)cli);
  //DestroyIcon(hIconCID);
}

int onCopyID(WPARAM wparam,LPARAM lparam)
{
  LPSTR szProto;
  char szID[128],buffer[256];

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0);
  GetID((HANDLE)wparam,szProto,(LPSTR)&szID);

  buffer[0]='\0';
  if(DBGetContactSettingWord(NULL,MODULENAME,"flags",0)&VF_CIDN) {
    strcpy(buffer,szProto);
    strcat(buffer,": ");
  }
  strcat(buffer,szID);
  CopyToClipboard((HWND)lparam,buffer);
  return 0;
}


/* #############################################################################
################################################################################
#################################  BROWSE FILES  ###############################
################################################################################
############################################################################# */

BOOL DirectoryExists(HANDLE hContact)
{
  int attr;
  char path[MAX_PATH];
  CallService(MS_FILE_GETRECEIVEDFILESFOLDER, (WPARAM)hContact, (LPARAM)&path);
  attr = GetFileAttributes(path);
  return (attr!=-1) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

int onRecvFiles(WPARAM wparam,LPARAM lparam)
{
  char path[MAX_PATH];
  CallService(MS_FILE_GETRECEIVEDFILESFOLDER, wparam, (LPARAM)&path);
  ShellExecute(0, "open", path, 0, 0, SW_SHOW);
  return 0;
}


/* #############################################################################
################################################################################
################################  COPY STATUS MSG  #############################
################################################################################
############################################################################# */

BOOL StatusMsgExists(HANDLE hContact)
{
  LPSTR msg;
  BOOL ret=0;
  int i;

  if(!ServiceExists(MS_SMR_ENABLE_CONTACT)) return 0;

  for(i=0;status[i].module;i++) {
    msg = MIEXDBGetString(hContact,status[i].module,status[i].name);
    if(msg) {
      if(strlen(msg))  ret |= status[i].flag;
      free(msg);
    }
  }
  return ret;
}

void ModifyStatusMsg(CLISTMENUITEM *cli,HANDLE hContact)
{
  LPSTR szProto;
  HICON hIconSMsg;

  cli->flags|=CMIM_ICON;
  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
  if(!szProto) {
    HideItem(cli,hmenuStatusMsg);
    return;
  }
  hIconSMsg = (HICON) CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
  //cli->hIcon = BindOverlayIcon(hIconSMsg,  (StatusMsgExists(hContact)&2)?MAKEINTRESOURCE(IDI_XSTAT):MAKEINTRESOURCE(IDI_STAT));
  //cli->hIcon = BindOverlayIcon(hIconSMsg, MAKEINTRESOURCE(IDI_STAT));
  cli->hIcon = LoadIconEx( "cmiex_stat" );
  DestroyIcon(hIconSMsg);
  hIconSMsg = cli->hIcon;

  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuStatusMsg,(LPARAM)cli);
  DestroyIcon(hIconSMsg);
}

int onCopyStatusMsg(WPARAM wparam,LPARAM lparam)
{
  LPSTR msg;
  char buffer[2048];
  int i;

  buffer[0]=0;
  for(i=0;status[i].module;i++) {
    msg = MIEXDBGetString((HANDLE)wparam,status[i].module,status[i].name);
    if(msg && strlen(msg)) {
      strcat(buffer,msg);
      strcat(buffer,"\r\n");
      free(msg);
    }
  }

  CopyToClipboard((HWND)lparam,buffer);
  return 0;
}


/* #############################################################################
################################################################################
####################################  COPY IP  #################################
################################################################################
############################################################################# */

BOOL IPExists(HANDLE hContact)
{
  LPSTR szProto;
  DWORD mIP,rIP;

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
  if(!szProto) return 0;

  mIP = DBGetContactSettingDword(hContact, szProto, "IP", 0);
  rIP = DBGetContactSettingDword(hContact, szProto, "RealIP", 0);

  return (mIP!=0 || rIP!=0);
}

void getIP(HANDLE hContact,LPSTR szProto,LPSTR szIP)
{
  char szmIP[64]={0};
  char szrIP[64]={0};
  DWORD mIP = DBGetContactSettingDword(hContact, szProto, "IP", 0);
  DWORD rIP = DBGetContactSettingDword(hContact, szProto, "RealIP", 0);
  if( mIP ) wsprintf(szmIP, "External IP: %d.%d.%d.%d\r\n", mIP>>24,(mIP>>16)&0xFF,(mIP>>8)&0xFF,mIP&0xFF);
  if( rIP ) wsprintf(szrIP, "Internal IP: %d.%d.%d.%d\r\n", rIP>>24,(rIP>>16)&0xFF,(rIP>>8)&0xFF,rIP&0xFF);
  strcpy(szIP,szrIP);
  strcat(szIP,szmIP);
}

void ModifyCopyIP(CLISTMENUITEM *cli,HANDLE hContact)
{
  LPSTR szProto;
  HICON hIconCIP;

  cli->flags |= CMIM_ICON;

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
  if(!szProto) {
    HideItem(cli,hmenuCopyIP);
    return;
  }
  hIconCIP = (HICON) CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);

  //cli->hIcon = BindOverlayIcon(hIconCIP,MAKEINTRESOURCE(IDI_CIP));
  cli->hIcon = LoadIconEx( "cmiex_cip" );
  DestroyIcon(hIconCIP);
  hIconCIP = cli->hIcon;

  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuCopyIP,(LPARAM)cli);
  DestroyIcon(hIconCIP);
}

int onCopyIP(WPARAM wparam,LPARAM lparam)
{
  LPSTR szProto;
  char szIP[128];

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, wparam, 0);
  getIP((HANDLE)wparam,szProto,(LPSTR)&szIP);

  CopyToClipboard((HWND)lparam,szIP);

  return 0;
}


/* #############################################################################
################################################################################
##################################  COPY MIRVER  ###############################
################################################################################
############################################################################# */

int onCopyMirVer(WPARAM wparam,LPARAM lparam)
{
  LPSTR msg = getMirVer((HANDLE)wparam);
  if(msg) {
    CopyToClipboard((HWND)lparam,msg);
    free(msg);
  }
  return 0;
}

LPSTR getMirVer(HANDLE hContact)
{
  LPSTR szProto, msg;

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
  if(!szProto) return NULL;

  msg = MIEXDBGetString(hContact,szProto,"MirVer");
  if(msg) {
    if(strlen(msg))  return msg;
    free(msg);
  }

  return NULL;
}

void ModifyCopyMirVer(CLISTMENUITEM *cli,HANDLE hContact)
{
  LPSTR msg;
  cli->flags|=CMIM_ICON;
  if(ServiceExists(MS_FP_GETCLIENTICON)) {
    cli->hIcon=NULL;
    msg = getMirVer(hContact);
    if(msg) {
      cli->hIcon = (HICON)CallService(MS_FP_GETCLIENTICON,(WPARAM)msg,(LPARAM)1);
      free(msg);
    }
  }
  else cli->hIcon = LoadIconEx( "cmiex_cmv" );
  //if(!cli->hIcon) cli->hIcon = hIcon[6];
  CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hmenuCopyMirVer,(LPARAM)cli);
}

BOOL MirVerExists(HANDLE hContact)
{
  LPSTR szProto, msg;
  BOOL ret=0;

  szProto = (LPSTR) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
  if(!szProto) return 0;

  msg = MIEXDBGetString(hContact,szProto,"MirVer");
  if(msg) {
    if(strlen(msg))  ret = 1;
    free(msg);
  }

  return ret;
}


/* #############################################################################
################################################################################
###############################  SHOW LAST ONLINE  #############################
################################################################################
############################################################################# */

void ModifyLastOnline( CLISTMENUITEM *cli, HANDLE hContact )
{
  time_t tLastOnline;
  struct tm * timeinfo;
  DWORD dLastOnline;
  char buffer[255];
  //char format[255];
  LPSTR szProto;
  DBVARIANT dbv = { 0 };

  szProto = ( LPSTR )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact, 0 );
  dLastOnline = DBGetContactSettingDword( hContact, szProto, "LogoffTS", 0 );
  DBGetContactSettingString( NULL, MODULENAME, "LastOnlineFormat", &dbv );
  if ( ( int )DBGetContactSettingWord(hContact, "UserOnline", "OldStatus", 0) > ID_STATUS_OFFLINE )
    tLastOnline = time( NULL );
  else
    tLastOnline = ( time_t )dLastOnline;
  timeinfo = localtime ( &tLastOnline );

  strftime( buffer, 255, dbv.pszVal, timeinfo );

  //cli->flags |= CMIM_NAME;

  if ( dLastOnline > 1 && timeinfo->tm_year >= 107 ) // 107 is for 1900 + 107 = 2007
  {
    cli->flags |= CMIM_NAME | CMIM_ICON;
    cli->pszName = ( char* ) &buffer;
    cli->hIcon = LoadIconEx( "cmiex_slo" );
  }
  else {
    cli->flags = CMIM_FLAGS | CMIF_HIDDEN;
  }
  CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hmenuShowLastOnline, ( LPARAM ) cli );
}


/* #############################################################################
################################################################################
###############################  SERVICE FUNCTIONS  ############################
################################################################################
############################################################################# */

char *MIEXDBGetString(HANDLE hContact,const char *szModule,const char *szSetting) {
  char *str=NULL;
  DBVARIANT dbv;
  DBGetContactSetting(hContact,szModule,szSetting,&dbv);
  if( dbv.type==DBVT_ASCIIZ || dbv.type==DBVT_UTF8 || dbv.type==DBVT_WCHAR) str=strdup(dbv.pszVal);
  DBFreeVariant(&dbv);
  return str;
}

void CopyToClipboard(HWND hwnd,LPSTR msg)
{
  HGLOBAL hglbCopy;
  LPSTR lpstrCopy;

  hglbCopy = GlobalAlloc(GMEM_MOVEABLE, lstrlen(msg)+1); 
  lpstrCopy = GlobalLock(hglbCopy); 
  lstrcpy(lpstrCopy, msg); 
  GlobalUnlock(hglbCopy); 

  OpenClipboard(NULL);
  EmptyClipboard(); 
  SetClipboardData(CF_TEXT, hglbCopy); 
  CloseClipboard(); 
}

// Window Proc to catch the Ignore-Popupmenu clicks
BOOL CALLBACK CatchMenuMsg(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
  switch(msg) {

  case WM_COMMAND:
    {
      LPARAM flag;
      int isIgnored,all,loop;
      HANDLE hcontact=(HANDLE)GetWindowLong(hwnd,GWL_USERDATA);

      switch(LOWORD(wparam)) {
      case IDM_MSG:
        flag=IGNOREEVENT_MESSAGE;
        break;

      case IDM_ALL:
        flag=IGNOREEVENT_ALL;
        break;

      case IDM_URL:
        flag=IGNOREEVENT_URL;
        break;

      case IDM_FILE:
        flag=IGNOREEVENT_FILE;
        break;

      case IDM_ONLINE:
        flag=IGNOREEVENT_USERONLINE;
        break;

      case IDM_AUTH:
        flag=IGNOREEVENT_AUTHORIZATION;
        break;
      }

      if(flag==IGNOREEVENT_ALL)
      {
        for(all=0,loop=1;loop<=5;loop++)
        {
          if(CallService(MS_IGNORE_ISIGNORED,(WPARAM)hcontact,(LPARAM)loop))
            all++;
        }
        isIgnored=((all==5)?1:0);
      }
      else
        isIgnored=CallService(MS_IGNORE_ISIGNORED,(WPARAM)hcontact,flag);

      CallService(isIgnored?MS_IGNORE_UNIGNORE:MS_IGNORE_IGNORE,(WPARAM)hcontact,flag);


      if(LOWORD(wparam)>=IDM_PROTOS && LOWORD(wparam)<IDM_GROUPS)
      {
        if(szmainProto)
          CallService(MS_PROTO_REMOVEFROMCONTACT,(WPARAM)hcontact,(LPARAM)szmainProto);
        CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hcontact,(LPARAM)protodat[LOWORD(wparam)-IDM_PROTOS]);
      }

      else if(LOWORD(wparam)>=IDM_GROUPS)
      {
        if(LOWORD(wparam)==IDM_GROUPS)
          CallService(MS_CLIST_CONTACTCHANGEGROUP,(WPARAM)hcontact,(LPARAM)NULL);
        else
          DBWriteContactSettingString(hcontact,"CList","Group",groupdat[LOWORD(wparam)-IDM_GROUPS]);
      }
    }

    return 0;
  }

  return CallWindowProc(mainProc,hwnd,msg,wparam,lparam);
}

void ShowItem(CLISTMENUITEM *cli,HMENU hmenu)
{
  cli->flags = CMIM_FLAGS;
  CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hmenu, ( LPARAM )cli );
}

void HideItem(CLISTMENUITEM *cli,HMENU hmenu)
{
  cli->flags |= CMIF_HIDDEN;
  CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hmenu, ( LPARAM )cli );
}

void initIcoLibIcons(void)
{
  int i;
  SKINICONDESC sid = { 0 };
  char szFile[MAX_PATH];
  GetModuleFileNameA( hinstance, szFile, MAX_PATH );

  sid.cbSize = sizeof( SKINICONDESC );
  sid.pszDefaultFile = szFile;
  sid.cx = sid.cy = 16;
  sid.pszSection = Translate( MODULENAME );

  for (i = 0; i < SIZEOF( iconList ); i++ ) {
    char szSettingName[64];
    mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", MODULENAME, iconList[i].szName);
    sid.pszName = szSettingName;
    sid.pszDescription = Translate( iconList[i].szDescr );
    sid.iDefaultIndex = -iconList[i].defIconID;
    iconList[i].hIconLibItem = ( HANDLE )CallService( MS_SKIN2_ADDICON, 0, (LPARAM)&sid );
  }
}

HICON LoadIconEx(const char* name)
{
  char szSettingName[64];
  mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", MODULENAME, name);
  return (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szSettingName);
}

/* #############################################################################
################################################################################
################################  OPTIONS SECTION  #############################
################################################################################
############################################################################# */

BOOL CALLBACK OptionsProc( HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam )
{
  DBVARIANT dbv = { 0 };
  WORD flags = DBGetContactSettingWord( NULL, MODULENAME, "flags", default_flags );
  
  switch ( msg )
  {
    case WM_INITDIALOG:
      CheckDlgButton( hdlg, IDC_ALWAYSVIS, ( flags & VF_AV ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_NEVERVIS, ( flags & VF_NV ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_HIDE, ( flags & VF_HFL ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_IGNORE, ( flags & VF_IGN ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_PROTOS, ( flags & VF_PROTO ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_GROUPS, ( flags & VF_GRP ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_ADDED, ( flags & VF_ADD ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_AUTHREQ, ( flags & VF_REQ ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_COPYID, ( flags & VF_CID ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_COPYIDNAME, ( flags & VF_CIDN ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_RECVFILES, ( flags & VF_RECV ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_SHOWLASTONLINE, ( flags & VF_SLO ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_COPYIP, ( flags & VF_CIP ) ? BST_CHECKED : BST_UNCHECKED );
      CheckDlgButton( hdlg, IDC_COPYMIRVER, ( flags & VF_CMV ) ? BST_CHECKED : BST_UNCHECKED );

      if ( !ServiceExists( MS_SMR_ENABLE_CONTACT ) )
        EnableWindow( GetDlgItem( hdlg, IDC_STATUSMSG ), FALSE );
      CheckDlgButton( hdlg, IDC_STATUSMSG, ( flags & VF_STAT ) ? BST_CHECKED : BST_UNCHECKED );

      DBGetContactSettingString( NULL, MODULENAME, "LastOnlineFormat", &dbv);
      SetDlgItemText(hdlg, IDC_LOFORMAT, dbv.pszVal);

      TranslateDialogDefault( hdlg );
      return 0;

    case WM_NOTIFY:
      switch( ( ( LPNMHDR )lparam )->code )
      {
        case PSN_APPLY:
        {
          WORD mod_flags = 0;
          char format[255];

          mod_flags |= IsDlgButtonChecked( hdlg, IDC_ALWAYSVIS ) ? VF_AV : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_NEVERVIS ) ? VF_NV : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_HIDE ) ? VF_HFL : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_IGNORE ) ? VF_IGN : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_PROTOS ) ? VF_PROTO : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_GROUPS ) ? VF_GRP : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_ADDED ) ? VF_ADD : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_AUTHREQ ) ? VF_REQ : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_COPYID ) ? VF_CID : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_COPYIDNAME ) ? VF_CIDN : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_RECVFILES ) ? VF_RECV : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_STATUSMSG ) ? VF_STAT : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_COPYIP ) ? VF_CIP : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_COPYMIRVER ) ? VF_CMV : 0;
          mod_flags |= IsDlgButtonChecked( hdlg, IDC_SHOWLASTONLINE ) ? VF_SLO : 0;
          DBDeleteContactSetting( NULL, MODULENAME, "flags" );
          DBWriteContactSettingWord( NULL, MODULENAME, "flags", mod_flags );
          GetDlgItemText(hdlg, IDC_LOFORMAT, format, 255);
          DBWriteContactSettingString( NULL, MODULENAME, "LastOnlineFormat", format );
          
          return 1;
        }
      }
      break;

    case WM_COMMAND:
      if( HIWORD( wparam ) == BN_CLICKED && GetFocus() == ( HWND )lparam )
        SendMessage( GetParent( hdlg ), PSM_CHANGED, 0, 0 );

      return 0;

    case WM_CLOSE:
      EndDialog( hdlg, 0 );
      return 0;
  }
  return 0;
}

int OptionsInit( WPARAM wparam, LPARAM lparam )
{
  OPTIONSDIALOGPAGE odp = { 0 };

  odp.cbSize = sizeof( odp );
  odp.position = 955000000;
  odp.hInstance = hinstance;
  odp.pszTemplate = MAKEINTRESOURCE( IDD_OPTIONS );
  //odp.pszTitle = Translate( MODULENAME );
  odp.pszTitle = Translate( "Extended Contacts Menu" );
  odp.pfnDlgProc = OptionsProc;
  //odp.pszGroup = Translate( "Plugins" );
  odp.pszGroup = Translate( "Customize" );
  odp.flags = ODPF_BOLDGROUPS;

  CallService( MS_OPT_ADDPAGE, wparam, ( LPARAM )&odp );
  return 0;
}

/* #############################################################################
################################################################################
#############################  RELEASE NOTES SECTION  ##########################
################################################################################
############################################################################# */

INT_PTR CALLBACK RelNotesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          DBWriteContactSettingWord(NULL, MODULENAME, "RelNotes", CMIEX_RELNOTES);
          EndDialog(hwndDlg, LOWORD(wParam));
          break;
      }
    }
    break;
  }
  return FALSE;
}
