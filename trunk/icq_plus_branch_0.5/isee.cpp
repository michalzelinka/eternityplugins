
#include "icqoscar.h"

// 0 - delete
// 1 - add
void CIcqProto::sendVisListServ(int mode)
{
/*	char *szList;
	char szUin[20];
	char szLen[2];
	WORD wListLen = 0;
	int ID = 0;
	icq_packet packet;
	icq_contacts_cache icc;
	WORD type = (mode) ? ICQ_CLI_ADDTEMPVISIBLE : ICQ_CLI_REMOVETEMPVISIBLE;

	int count = CacheIDCount();

	if (!count)	return;

	szList = (char*)alloca((count + 1) * 11);
	szList[0] = 0;
	szLen[1] = 0;

	while(GetCacheByID(ID++, &icc))
	{

	if (icc.dwUin == dwLocalUIN)
	continue;

	if (mode) // for adding
	{
	if (DBGetContactSettingByte(icc.hContact, "CList", "NotOnList", 0) ||
	(icc.flags & F_TEMPVIS)) // skip already added
	continue;

	// mark contact
	SetCacheFlagsByID(ID - 1, icc.flags|F_TEMPVIS);
	}
	else
	if (icc.flags & F_TEMPVIS)
	{
	// unmark contact
	SetCacheFlagsByID(ID - 1, icc.flags &~ F_TEMPVIS);
	}
	else
	continue;

	_itoa(icc.dwUin, szUin, 10);
	szLen[0] = strlen(szUin);

	strcat(szList, szLen);
	strcat(szList, szUin);

	wListLen += szLen[0] + 1;

	if (wListLen > MAX_MESSAGESNACSIZE)
	break;
	}

	if (wListLen)
	{
	packet.wLen = wListLen + 10;
	write_flap(&packet, 2);
	packFNACHeader(&packet, ICQ_BOS_FAMILY, type);
	packBuffer(&packet, szList, wListLen);
	sendServPacket(&packet);
	}
*/}


// status checking stuff
void CIcqProto::icq_CheckSpeed(WORD wStatus)
{
	// 2 - slowly! 3 - really slowly! 4 - speed up!
	switch (wStatus)
	{
		case 2: if (m_FLCStatus > 1) m_wCheckSpeed += 1000; break;
		case 3: if (m_FLCStatus > 1) m_wCheckSpeed += 5000; break;
		case 4: m_wCheckSpeed = m_wCheckSpeedDefault; break;
		default: break;
	}
	Netlib_Logf(m_hServerNetlibUser, "Rate status received: %u. Delay: %u ms", wStatus, m_wCheckSpeed);
}


void CIcqProto::ToggleAdvancedFeaturesItems( BOOL bEnable )
{
	CLISTMENUITEM clmi = { 0 };
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS | (( bEnable ) ? 0 : CMIF_HIDDEN);

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hCustomStatusRoot,    ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hMenuAdvFeaturesRoot, ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )m_hMenuUsersStatusScan, ( LPARAM )&clmi );
}


void CIcqProto::InitAdvancedFeaturesItems()
{

	CLISTMENUITEM mi = { 0 };
	int i = 0, len = strlennull(m_szModuleName);
	//char tszName[MAX_PATH + 64];
	char svcFnc[MAX_PATH + 64];

	m_bVisibility = getSettingByte(NULL, "SrvVisibility", DEFAULT_VISIBILITY_MODE);
	if (m_bVisibility > 6 || m_bVisibility < 0) m_bVisibility = 0;

		//if (bStatusMenu)
		//	lstrcpy(tszName, TranslateT("Advanced Features"));
		//else
		//	mir_sntprintf(tszName, SIZEOF(tszName), _T("%s %s"), m_tszUserName, TranslateT("Advanced Features"));

	// Advanced Features

	mi.cbSize = sizeof(mi);
	mi.position = 1002;
	mi.pszContactOwner = m_szModuleName;
	mi.pszPopupName = (char *)pcli->pfnGetProtocolMenu( m_szModuleName );
	mi.ptszName = LPGENT("Advanced Features");
	mi.icolibItem = hStaticIcons[ISI_ADVANCED_FEATURES]->Handle();
	mi.flags = CMIF_ROOTPOPUP | CMIF_ICONFROMICOLIB | CMIF_TCHAR | CMIF_HIDDEN;
	m_hMenuAdvFeaturesRoot = ( HANDLE )CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );

	// Global Users Status Scan

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s", m_szModuleName, MS_ISEE_SCAN_USER_STATUS_GLOBAL);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);
	// TODO: proper service

	mi.position = 1003;
	mi.pszService = svcFnc;

	mi.icolibItem = hStaticIcons[ISI_SCAN_STOP]->Handle();
	mi.ptszName = LPGENT("&Users Status Scan");

	m_hMenuUsersStatusScan = ( HANDLE )CallService( MS_CLIST_ADDSTATUSMENUITEM, 0, ( LPARAM )&mi );

	// Advanced Features Items

	mi.icolibItem = hStaticIcons[ISI_DISABLED_ITEM]->Handle();
	mi.flags = CMIF_TCHAR | CMIF_CHILDPOPUP | CMIF_ICONFROMICOLIB;
	mi.pszPopupName = (char*)m_hMenuAdvFeaturesRoot;

 	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.position = 1000010000;
	mi.ptszName = LPGENT("Default, corresponding to status");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                            bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                            0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                            (LPARAM)&mi
	                         );

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.position = 1100010000;
	mi.ptszName = LPGENT("Allow all users to see you");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
                               bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	mi.position++;

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.ptszName = LPGENT("Block all users from seeing you");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	mi.position++;

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.ptszName = LPGENT("Allow only users in the Visible list to see you");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	mi.position++;

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.ptszName = LPGENT("Block only users in the Invisible list from seeing you");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	mi.position++;

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.ptszName = LPGENT("Allow only users in the Contact list to see you");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	mi.position++;

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.ptszName = LPGENT("Allow only users in the Contact list to see you, except Invisible list users");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.position = 1200010000;
	mi.icolibItem = m_hIconProtocol->Handle();
	mi.ptszName = LPGENT("Send global authorization request to all users who haven't authorized you yet");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.position = 1300010000;
	mi.icolibItem = (m_bASD) ? hStaticIcons[ISI_ENABLED_ITEM]->Handle() : hStaticIcons[ISI_DISABLED_ITEM]->Handle();
	mi.ptszName = LPGENT("Active Status Discovery");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	mi.position++;

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.ptszName = LPGENT("Web Aware");
	mi.icolibItem = (getSettingByte(NULL, "WebAware", 0)) ? hStaticIcons[ISI_ENABLED_ITEM]->Handle() : hStaticIcons[ISI_DISABLED_ITEM]->Handle();
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	null_snprintf(svcFnc, sizeof(svcFnc), "%s/%s%d", m_szModuleName, MS_ISEE_ADVANCED_FEATURES, i);
	CreateProtoServiceParam(svcFnc + len, &CIcqProto::menuAdvFeatures, i);

	mi.position = 1400010000;
	mi.icolibItem = hStaticIcons[ISI_MANAGE_SERVER_LIST]->Handle();
	mi.ptszName = LPGENT("Manage server's list...");
	mi.pszService = svcFnc;

	hAdvFeaturesItems[i++] = (HANDLE)CallService(
	                           bStatusMenu ? MS_CLIST_ADDSTATUSMENUITEM : MS_CLIST_ADDMAINMENUITEM,
	                           0, //bStatusMenu ? (WPARAM)&hAdvFeaturesRoot : 0,
	                           (LPARAM)&mi
	                         );

	setAdvFeaturesIcons(-1);
}


int __cdecl CIcqProto::menuAdvFeatures(WPARAM wParam, LPARAM lParam, LPARAM fParam)
{
	BYTE i = (BYTE)fParam;
	switch (i)
	{
	case 0: // Privacy levels
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		SetVisibility(i);
		break;
	case 7: // send global auth
		SendAuthRequestToAllUnauthorized( );
		break;
	case 8: // ASD
		m_bASD = !m_bASD;
		setSettingByte( NULL, "ASD", m_bASD );
		// TODO: toggle ASD
		break;
	case 9: // Web Aware
		m_bWebAware = !m_bWebAware;
		setSettingByte( NULL, "WebAware", m_bWebAware );
		// TODO: send web aware
		break;
	case 10: // Server List
		ShowUploadContactsDialog();
		break;
	case 11: // Users status scan
		icq_CheckAllUsersStatus(0, 0);
		break;
	}
	setAdvFeaturesIcons(i);
	return 0;
}


// o = position of icon to update (use -1 to update them all)
void CIcqProto::setAdvFeaturesIcons(int o)
{
	int i;
	CLISTMENUITEM tmi = {0};

	tmi.cbSize = sizeof(tmi);
	tmi.flags = CMIM_ICON | CMIM_FLAGS | CMIF_ICONFROMICOLIB | CMIF_TCHAR;

	if (o >= -1 && o <= 6) {
		int setMode = getSettingByte(NULL, "Privacy", DEFAULT_VISIBILITY_MODE);
		for (i = 0; i <= 6; i++)
		{
			tmi.icolibItem = (i == o || i == setMode) ? hStaticIcons[ISI_ENABLED_ITEM]->Handle() : hStaticIcons[ISI_DISABLED_ITEM]->Handle();
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hAdvFeaturesItems[i], (LPARAM)&tmi);
		}
	}
	if (o == -1 || o == 8)
	{
		tmi.icolibItem = (m_bASD) ? hStaticIcons[ISI_ENABLED_ITEM]->Handle() : hStaticIcons[ISI_DISABLED_ITEM]->Handle();
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hAdvFeaturesItems[o], (LPARAM)&tmi);
	}
	if (o == -1 || o == 9)
	{
		tmi.icolibItem = (m_bWebAware) ? hStaticIcons[ISI_ENABLED_ITEM]->Handle() : hStaticIcons[ISI_DISABLED_ITEM]->Handle();
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hAdvFeaturesItems[o], (LPARAM)&tmi);
	}
	if (o == -1 || o == 11)
	{
		switch(m_FLCStatus)
		{
		case 0 : tmi.icolibItem = hStaticIcons[ISI_SCAN_STOP]->Handle(); break;   // stopped
		case 1 : tmi.icolibItem = hStaticIcons[ISI_SCAN_PAUSE]->Handle(); break;  // paused
		default: tmi.icolibItem = hStaticIcons[ISI_SCAN_START]->Handle(); break;  // running
		}
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hAdvFeaturesItems[o], (LPARAM)&tmi);
	}

	ToggleAdvancedFeaturesItems( this->icqOnline( ) );
}
