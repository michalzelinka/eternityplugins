// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright � 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001-2002 Jon Keating, Richard Hughes
// Copyright � 2002-2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004-2009 Joe Kucera
// Copyright � 2006-2009 BM, SSS, jarvis, S!N, persei and others
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
// File name      : $URL: http://sss.chaoslab.ru:81/svn/icqjplus/branches/0.5_branch/fam_01service.cpp $
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

void CIcqProto::handleServiceFam(BYTE *pBuffer, WORD wBufferLength, snac_header *pSnacHeader, serverthread_info *info)
{
	icq_packet packet;

	switch (pSnacHeader->wSubtype) {

	case ICQ_SERVER_READY:
#ifdef _DEBUG
		NetLog_Server("Server is ready and is requesting my Family versions");
		NetLog_Server("Sending my Families");
#endif

		// This packet is a response to SRV_FAMILIES SNAC(1,3).
		// This tells the server which SNAC families and their corresponding
		// versions which the client understands. This also seems to identify
		// the client as an ICQ vice AIM client to the server.
		// Miranda mimics the behaviour of ICQ 6
		serverPacketInit(&packet, 54);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_FAMILIES);
		packDWord(&packet, 0x00220001);
		packDWord(&packet, 0x00010004);
		packDWord(&packet, 0x00130004);
		packDWord(&packet, 0x00020001);
		packDWord(&packet, 0x00030001);
		packDWord(&packet, 0x00150001);
		packDWord(&packet, 0x00040001);
		packDWord(&packet, 0x00060001);
		packDWord(&packet, 0x00090001);
		packDWord(&packet, 0x000a0001);
		packDWord(&packet, 0x000b0001);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_FAMILIES2:
		/* This is a reply to CLI_FAMILIES and it tells the client which families and their versions that this server understands.
		* We send a rate request packet */
#ifdef _DEBUG
		NetLog_Server("Server told me his Family versions");
		NetLog_Server("Requesting Rate Information");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQ_RATE_INFO);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_RATE_INFO:
#ifdef _DEBUG
		NetLog_Server("Server sent Rate Info");
		NetLog_Server("Sending Rate Info Ack");
#endif
		/* init rates management */
		m_rates = new rates(this, pBuffer, wBufferLength);
		/* ack rate levels */
		m_rates->initAckPacket(&packet);
		sendServPacket(&packet);

		/* CLI_REQINFO - This command requests from the server certain information about the client that is stored on the server. */
#ifdef _DEBUG
		NetLog_Server("Sending CLI_REQINFO");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQINFO);
		sendServPacket(&packet);

		if (m_bSsiEnabled)
		{
			cookie_servlist_action* ack;
			DWORD dwCookie;

			DWORD dwLastUpdate = getSettingDword(NULL, "SrvLastUpdate", 0);
			WORD wRecordCount = getSettingWord(NULL, "SrvRecordCount", 0);

			// CLI_REQLISTS - we want to use SSI
#ifdef _DEBUG
			NetLog_Server("Requesting roster rights");
#endif
			serverPacketInit(&packet, 16);
			packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQLISTS);
			packTLVWord(&packet, 0x0B, 0x000F); // mimic ICQ 6
			sendServPacket(&packet);

			if (!wRecordCount) // CLI_REQROSTER
			{ // we do not have any data - request full list
#ifdef _DEBUG
				NetLog_Server("Requesting full roster");
#endif
				serverPacketInit(&packet, 10);
				ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				if (ack)
				{ // we try to use standalone cookie if available
					ack->dwAction = SSA_CHECK_ROSTER; // loading list
					dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_REQUEST, 0, ack);
				}
				else // if not use that old fake
					dwCookie = ICQ_LISTS_CLI_REQUEST<<0x10;

				packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQUEST, 0, dwCookie);
				sendServPacket(&packet);
			}
			else // CLI_CHECKROSTER
			{
#ifdef _DEBUG
				NetLog_Server("Requesting roster check");
#endif
				serverPacketInit(&packet, 16);
				ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				if (ack)  // TODO: rewrite - use get list service for empty list
				{ // we try to use standalone cookie if available
					ack->dwAction = SSA_CHECK_ROSTER; // loading list
					dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_CHECK, 0, ack);
				}
				else // if not use that old fake
					dwCookie = ICQ_LISTS_CLI_CHECK<<0x10;

				packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_CHECK, 0, dwCookie);
				// check if it was not changed elsewhere (force reload, set that setting to zero)
				if (IsServerGroupsDefined())
				{
					packDWord(&packet, dwLastUpdate);  // last saved time
					packWord(&packet, wRecordCount);   // number of records saved
				}
				else
				{ // we need to get groups info into DB, force receive list
					packDWord(&packet, 0);  // last saved time
					packWord(&packet, 0);   // number of records saved
				}
				sendServPacket(&packet);
			}
		}

		// CLI_REQLOCATION
#ifdef _DEBUG
		NetLog_Server("Requesting Location rights");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_CLI_REQ_RIGHTS);
		sendServPacket(&packet);

		// CLI_REQBUDDY
#ifdef _DEBUG
		NetLog_Server("Requesting Client-side contactlist rights");
#endif
		serverPacketInit(&packet, 16);
		packFNACHeader(&packet, ICQ_BUDDY_FAMILY, ICQ_USER_CLI_REQBUDDY);
		// Query flags: 1 = Enable Avatars
		//              2 = Enable offline status message notification
		//              4 = Enable Avatars for offline contacts
    //              8 = Use reject for not authorized contacts
		packTLVWord(&packet, 0x05, 0x0003); // mimic ICQ 6
		sendServPacket(&packet);

		// CLI_REQICBM
#ifdef _DEBUG
		NetLog_Server("Sending CLI_REQICBM");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_REQICBM);
		sendServPacket(&packet);

		// CLI_REQBOS
#ifdef _DEBUG
		NetLog_Server("Sending CLI_REQBOS");
#endif
		serverPacketInit(&packet, 10);
		packFNACHeader(&packet, ICQ_BOS_FAMILY, ICQ_PRIVACY_REQ_RIGHTS);
		sendServPacket(&packet);
		break;

	case ICQ_SERVER_PAUSE:
		NetLog_Server("Server is going down in a few seconds... (Flags: %u)", pSnacHeader->wFlags);
		// This is the list of groups that we want to have on the next server
		serverPacketInit(&packet, 30);
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_PAUSE_ACK);
		packWord(&packet,ICQ_SERVICE_FAMILY);
		packWord(&packet,ICQ_LISTS_FAMILY);
		packWord(&packet,ICQ_LOCATION_FAMILY);
		packWord(&packet,ICQ_BUDDY_FAMILY);
		packWord(&packet,ICQ_EXTENSIONS_FAMILY);
		packWord(&packet,ICQ_MSG_FAMILY);
		packWord(&packet,0x06);
		packWord(&packet,ICQ_BOS_FAMILY);
		packWord(&packet,ICQ_LOOKUP_FAMILY);
		packWord(&packet,ICQ_STATS_FAMILY);
		sendServPacket(&packet);
#ifdef _DEBUG
		NetLog_Server("Sent server pause ack");
#endif
		break;

	case ICQ_SERVER_MIGRATIONREQ:
		{
#ifdef _DEBUG
			NetLog_Server("Server migration requested (Flags: %u)", pSnacHeader->wFlags);
#endif
			pBuffer += 2; // Unknown, seen: 0
			wBufferLength -= 2;

			oscar_tlv_chain *chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

			if (info->cookieDataLen > 0)
				SAFE_FREE((void**)&info->cookieData);

			info->newServer = chain->getString(0x05, 1);
			info->newServerSSL = chain->getNumber(0x8E, 1);
			info->cookieData = (BYTE*)chain->getString(0x06, 1);
			info->cookieDataLen = chain->getLength(0x06, 1);

			disposeChain(&chain);

			if (!info->newServer || !info->cookieData)
			{
				icq_LogMessage(LOG_FATAL, LPGEN("A server migration has failed because the server returned invalid data. You must reconnect manually."));
				SAFE_FREE(&info->newServer);
				SAFE_FREE((void**)&info->cookieData);
				info->cookieDataLen = 0;
				info->newServerReady = 0;
				return;
			}

			NetLog_Server("Migration has started. New server will be %s", info->newServer);

			m_iDesiredStatus = m_iStatus;
			SetCurrentStatus(ID_STATUS_CONNECTING); // revert to connecting state

			info->newServerReady = 1;
			info->isMigrating = 1;
		}
		break;

	case ICQ_SERVER_NAME_INFO: // This is the reply to CLI_REQINFO
		{
			BYTE bUinLen;
			oscar_tlv_chain *chain;

#ifdef _DEBUG
			NetLog_Server("Received self info");
#endif
			unpackByte(&pBuffer, &bUinLen);
			pBuffer += bUinLen;
			pBuffer += 4;      /* warning level & user class */
			wBufferLength -= 5 + bUinLen;

			if (pSnacHeader->dwRef == ICQ_CLIENT_REQINFO<<0x10)
			{ // This is during the login sequence
				DWORD dwValue;

				// TLV(x01) User type?
				// TLV(x0C) Empty CLI2CLI Direct connection info
				// TLV(x0A) External IP
				// TLV(x0F) Number of seconds that user has been online
				// TLV(x03) The online since time.
				// TLV(x0A) External IP again
				// TLV(x22) Unknown
				// TLV(x1E) Unknown: empty.
				// TLV(x05) Member of ICQ since.
				// TLV(x14) Unknown
				chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

				// Save external IP
				dwValue = chain->getDWord(0x0A, 1); 
				setSettingDword(NULL, "IP", dwValue);
				setSettingDword(NULL, "OldIP", dwValue);

				// Save member since timestamp
				dwValue = chain->getDWord(0x05, 1); 
				if (dwValue) setSettingDword(NULL, "MemberTS", dwValue);

				dwValue = chain->getDWord(0x03, 1);
				setSettingDword(NULL, "LogonTS", dwValue ? dwValue : time(NULL));

				disposeChain(&chain);

				// If we are in SSI mode, this is sent after the list is acked instead
				// to make sure that we don't set status before seing the visibility code
				if (!m_bSsiEnabled || info->isMigrating)
					handleServUINSettings(wListenPort, info);
			}
			else if (m_hNotifyNameInfoEvent)
				// Just notify that the set status note & mood process is finished
				SetEvent(m_hNotifyNameInfoEvent);
		}
		break;

	case ICQ_SERVER_RATE_CHANGE:

		if (wBufferLength >= 2)
		{
			WORD wStatus;
			WORD wClass;
			DWORD dwLevel;
			// We now have global rate management, although controlled are only some
			// areas. This should not arrive in most cases. If it does, update our
			// local rate levels & issue broadcast.
			unpackWord(&pBuffer, &wStatus);
			unpackWord(&pBuffer, &wClass);
			pBuffer += 20;
			unpackDWord(&pBuffer, &dwLevel);

			EnterCriticalSection(&ratesMutex);
			m_rates->updateLevel(wClass, dwLevel);
			LeaveCriticalSection(&ratesMutex);

			if (wStatus == 2 || wStatus == 3)
			{ // this is only the simplest solution, needs rate management to every section
				BroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
				if (wStatus == 2)
					NetLog_Server("Rates #%u: Alert", wClass);
				else
					NetLog_Server("Rates #%u: Limit", wClass);
			}
			else if (wStatus == 4)
			{
				BroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
				NetLog_Server("Rates #%u: Clear", wClass);
			}
			icq_CheckSpeed(wStatus);
		}

		break;

	case ICQ_SERVER_REDIRECT_SERVICE: // reply to family request, got new connection point
		{
			oscar_tlv_chain* pChain = NULL;
			cookie_family_request *pCookieData;

			if (!(pChain = readIntoTLVChain(&pBuffer, wBufferLength, 0)))
			{
				NetLog_Server("Received Broken Redirect Service SNAC(1,5).");
				break;
			}
			WORD wFamily = pChain->getWord(0x0D, 1);

			// pick request data
			if ((!FindCookie(pSnacHeader->dwRef, NULL, (void**)&pCookieData)) || (pCookieData->wFamily != wFamily))
			{
				disposeChain(&pChain);
				NetLog_Server("Received unexpected SNAC(1,5), skipping.");
				break;
			}

			FreeCookie(pSnacHeader->dwRef);

			{ // new family entry point received
				char *pServer = pChain->getString(0x05, 1);
				BYTE bServerSSL = pChain->getNumber(0x8E, 1);
				char *pCookie = pChain->getString(0x06, 1);
				WORD wCookieLen = pChain->getLength(0x06, 1);

				if (!pServer || !pCookie)
				{
					NetLog_Server("Server returned invalid data, family unavailable.");

					SAFE_FREE(&pServer);
					SAFE_FREE(&pCookie);
					SAFE_FREE((void**)&pCookieData);
          disposeChain(&pChain);
					break;
				}

				// Get new family server ip and port
				WORD wPort = info->wServerPort; // get default port
				parseServerAddress(pServer, &wPort);

				// establish connection
				NETLIBOPENCONNECTION nloc = {0};
				nloc.flags = 0;
				nloc.szHost = pServer; 
				nloc.wPort = wPort;

				HANDLE hConnection = NetLib_OpenConnection(m_hServerNetlibUser, wFamily == ICQ_AVATAR_FAMILY ? "Avatar " : NULL, &nloc);

				if (hConnection == NULL)
				{
					NetLog_Server("Unable to connect to ICQ new family server.");
				} // we want the handler to be called even if the connecting failed
				else if (bServerSSL)
				{ /* Start SSL session if requested */
#ifdef _DEBUG
					NetLog_Server("(%d) Starting SSL negotiation", CallService(MS_NETLIB_GETSOCKET, (WPARAM)hConnection, 0));
#endif
					CallService(MS_NETLIB_STARTSSL, (WPARAM)hConnection, 0);
				}

				(this->*pCookieData->familyHandler)(hConnection, pCookie, wCookieLen);

				// Free allocated memory
				// NOTE: "cookie" will get freed when we have connected to the avatar server.
				disposeChain(&pChain);
				SAFE_FREE(&pServer);
				SAFE_FREE((void**)&pCookieData);
			}

			break;
		}

	case ICQ_SERVER_EXTSTATUS: // our session data
		{
#ifdef _DEBUG
			NetLog_Server("Received owner session data.");
#endif
      while (wBufferLength > 4)
      { // loop thru all items
        WORD wItemID = pBuffer[0] * 0x10 | pBuffer[1];
        BYTE bFlags = pBuffer[2];
        BYTE nDataLen = pBuffer[3];

			  if (wItemID == AVATAR_HASH_PHOTO)
			  { // skip photo item
#ifdef _DEBUG
          NetLog_Server("Photo item recognized");
#endif
			  }
			  else if ((wItemID == AVATAR_HASH_STATIC || wItemID == AVATAR_HASH_FLASH) && (nDataLen >= 0x10))
			  {
#ifdef _DEBUG
          NetLog_Server("Avatar item recognized");
#endif
				  if (m_bAvatarsEnabled && !info->bMyAvatarInited) // signal the server after login
				  { // this refreshes avatar state - it used to work automatically, but now it does not
					  if (getSettingByte(NULL, "ForceOurAvatar", 0))
					  { // keep our avatar
						  char *file = loadMyAvatarFileName();

						  SetMyAvatar(0, (LPARAM)file);
						  SAFE_FREE(&file);
					  }
					  else // only change avatar hash to the same one
					  {
						  BYTE hash[0x14];

						  memcpy(hash, pBuffer, 0x14);
						  hash[2] = 1; // update image status
						  updateServAvatarHash(hash, 0x14);
					  }
					  info->bMyAvatarInited = TRUE;
					  break;
				  }
          // process owner avatar hash changed notification
          handleAvatarOwnerHash(wItemID, bFlags, pBuffer, nDataLen + 4);
        }
        else if (wItemID == 0x02)
        {
#ifdef _DEBUG
          NetLog_Server("Status message item recognized");
#endif
        }
        else if (wItemID == 0x0E)
        {
#ifdef _DEBUG
          NetLog_Server("Status mood item recognized");
#endif
        }

        // move to next item
			  if (wBufferLength >= nDataLen + 4)
			  {
 					wBufferLength -= nDataLen + 4;
  				pBuffer += nDataLen + 4;
	  		}
		  	else
			  {
				  pBuffer += wBufferLength;
				  wBufferLength = 0;
			  }
			}
			break;
		}

	case ICQ_ERROR:
		{ // Something went wrong, probably the request for avatar family failed
			WORD wError;

			if (wBufferLength >= 2)
				unpackWord(&pBuffer, &wError);
			else 
				wError = 0;

			LogFamilyError(ICQ_SERVICE_FAMILY, wError);
			break;
		}

		// Stuff we don't care about
	case ICQ_SERVER_MOTD:
#ifdef _DEBUG
		NetLog_Server("Server message of the day");
#endif
		break;

	default:
		NetLog_Server("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_SERVICE_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;

	}
}

char* CIcqProto::buildUinList(int subtype, WORD wMaxLen, HANDLE* hContactResume)
{
	char* szList;
	HANDLE hContact;
	WORD wCurrentLen = 0;
	DWORD dwUIN;
	uid_str szUID;
	char szLen[2];
	int add;

	szList = (char*)SAFE_MALLOC(CallService(MS_DB_CONTACT_GETCOUNT, 0, 0) * UINMAXLEN);
	szLen[1] = '\0';

	if (*hContactResume)
		hContact = *hContactResume;
	else
		hContact = FindFirstContact();

	while (hContact != NULL)
	{
		if (!getContactUid(hContact, &dwUIN, &szUID))
		{
			szLen[0] = strlennull(strUID(dwUIN, szUID));

			switch (subtype)
			{

			case BUL_VISIBLE:
				add = ID_STATUS_ONLINE == getSettingWord(hContact, "ApparentMode", 0);
				break;

			case BUL_INVISIBLE:
				add = ID_STATUS_OFFLINE == getSettingWord(hContact, "ApparentMode", 0);
				break;

			case BUL_TEMPVISIBLE:
				add = getSettingByte(hContact, "TemporaryVisible", 0);
				// clear temporary flag
				// Here we assume that all temporary contacts will be in one packet
				setSettingByte(hContact, "TemporaryVisible", 0);
				break;

			default:
				add = 1;

				// If we are in SS mode, we only add those contacts that are
				// not in our SS list, or are awaiting authorization, to our
				// client side list
				if (m_bSsiEnabled && getSettingWord(hContact, DBSETTING_SERVLIST_ID, 0) &&
					!getSettingByte(hContact, "Auth", 0))
					add = 0;

				// Never add hidden contacts to CS list
				if (DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
					add = 0;

				break;
			}

			if (add)
			{
				wCurrentLen += szLen[0] + 1;
				if (wCurrentLen > wMaxLen)
				{
					*hContactResume = hContact;
					return szList;
				}

				strcat(szList, szLen);
				strcat(szList, szUID);
			}
		}

		hContact = FindNextContact(hContact);
	}
	*hContactResume = NULL;

	return szList;
}

void CIcqProto::sendEntireListServ(WORD wFamily, WORD wSubtype, int listType)
{
	HANDLE hResumeContact;
	char* szList;
	int nListLen;
	icq_packet packet;


	hResumeContact = NULL;

	do
	{ // server doesn't seem to be able to cope with packets larger than 8k
		// send only about 100contacts per packet
		szList = buildUinList(listType, 0x3E8, &hResumeContact);
		nListLen = strlennull(szList);

		if (nListLen)
		{
			serverPacketInit(&packet, (WORD)(nListLen + 10));
			packFNACHeader(&packet, wFamily, wSubtype);
			packBuffer(&packet, (LPBYTE)szList, (WORD)nListLen);
			sendServPacket(&packet);
		}

		SAFE_FREE((void**)&szList);
	}
		while (hResumeContact);
}

static void packNewCap(icq_packet* packet, WORD wNewCap)
{ // pack standard capability
	DWORD dwQ1 = 0x09460000 | wNewCap;

	packDWord(packet, dwQ1); 
	packDWord(packet, 0x4c7f11d1);
	packDWord(packet, 0x82224445);
	packDWord(packet, 0x53540000);
}

// CLI_SETUSERINFO
void CIcqProto::setUserInfo()
{ 
	icq_packet packet;
	WORD wAdditionalData = 0;
	BYTE bXStatus = getContactXStatus(NULL);
	BYTE cID = getSettingByte(NULL, "CurrentID", 0);
	BYTE cICQModID = getSettingByte(NULL, "CurrentModID", 0);

	static char wID[] ={ 4, 1, 8, 0, 4, 1, 1, 1, 1, 1, 1, 4, // 12
	                     12, 10, 10, 10, 1, 1, 2, 0, 1, 0, 0, 6, // 24
	                     6, 1, 13, 2, 1, 0, 0, 0, 0, 0, 1, 4, // 36
	                     0, 0, 0, 0, 1, 3, 2, 2, 1, 1, 8, 0, // 48
	                     0, 1, 3, 0, 0, 0, 4, 6, 6, 6, 7}; // 58


	// force reupdate of custom capabilities
	updateCustomCapabilities();
	
	if (!cID || cID == 11) // || cID == 54)
	{ // if Miranda, as 'Miranda' or as 'MirandaMobile'

		if (m_bAimEnabled)
			wAdditionalData += 16;	
		if (m_bAvatarsEnabled)
			wAdditionalData += 16;
		if (m_bUtfEnabled)
			wAdditionalData += 16;
		if (m_bRTFEnabled)
			wAdditionalData += 16;
		if (m_bXStatusEnabled)
			wAdditionalData += 16;
		if (m_bTzersEnabled)
			wAdditionalData += 16;
		if (bXStatus)
			wAdditionalData += 16;
#ifdef DBG_CAPXTRAZ_MUC
		wAdditionalData += 16;
#endif
#ifdef DBG_CAPMTN
		wAdditionalData += 16;
#endif
#ifdef DBG_NEWCAPS
		wAdditionalData += 16;
#endif
#ifdef DBG_CAPXTRAZ
		wAdditionalData += 16;
#endif
#ifdef DBG_OSCARFT
		wAdditionalData += 16;
#endif
#ifdef DBG_CAPHTML
		wAdditionalData += 16;
#endif
#ifdef DBG_AIMCONTACTSEND
		wAdditionalData += 16;
#endif
	}
	else if (m_bHideIdEnabled)
		wAdditionalData += 16; // TODO: DESC

	wAdditionalData += wID[cID]*16; // TODO: Recheck spaces

	if (m_bCustomCapsEnabled)
		wAdditionalData += 16 * lstCustomCaps->realCount;

	if (cID == 2 || cID == 23 || cID == 24 || cID == 46) // QIP caps
	{
		if (m_bXStatusEnabled)
			wAdditionalData += 16;
		if (bXStatus)
			wAdditionalData += 16;
	}
	if (cID >= 12 && cID <= 15 || cID == 26) // ICQLite 5 + ICQ 6 group caps
	{
		if (m_bXStatusEnabled)
			wAdditionalData += 16;
		if (bXStatus)
			wAdditionalData += 16;
	}
	if (cID == 6 || cID == 51 || cID == 45 || cID == 49 || cID == 55 || cID == 56 || cID == 57 || cID == 58)
	{
		if (m_bXStatusEnabled)
			wAdditionalData += 16;
		if (bXStatus)
			wAdditionalData += 16;
	}

	serverPacketInit(&packet, (WORD)(62 + wAdditionalData));
	packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO);


	/* TLV(5): capability data */
	//packWord(&packet, 0x0005);
	//packWord(&packet, (WORD)(48 + wAdditionalData));

	// VS //

	/* TLV(5): capability data */
	packWord(&packet, 0x0005);
	packWord(&packet, (WORD)(32 + wAdditionalData));
	packBuffer(&packet, capNewCap, 0x10);

	if (m_bHideIdEnabled && cID != 11)
	{
		packBuffer(&packet, capIcqJp, 4);
		packDWord(&packet, MIRANDA_VERSION);
		packDWord(&packet, ICQ_PLUG_VERSION);
		packDWord(&packet, m_bSecureIM ? 0x5AFEC0DE : 0x00000000);
	}
	switch (cID) //client change caps
	{

	case 11: // Miranda Evil
		packBuffer(&packet, m_bHideIdEnabled ? capIcqJp : capMirandaIm, m_bHideIdEnabled ? 4 : 8); 
		packBuffer(&packet, (BYTE*)"\x06\x06\x06\x00\x06\x06\x06\x00", 8);
		m_bHideIdEnabled ? packDWord(&packet, m_bSecureIM ? 0x5AFEC0DE : 0x00000000) : 0;

	case 0:								//miranda
		if (!cID && !m_bHideIdEnabled)
		{ // Miranda Signature + variable ICQ mods
			switch (cICQModID)
			{
			case 0:
				packBuffer(&packet, capMirandaIm, 8);
				packDWord(&packet, MIRANDA_VERSION);
				packDWord(&packet, ICQ_JOEWHALE_VERSION);
				break;
			case 1:
				packBuffer(&packet, capMirandaIm, 8);
				packDWord(&packet, MIRANDA_VERSION);
				packDWord(&packet, ICQ_BM_VERSION);
				break;
			case 2:
				packBuffer(&packet, capIcqJs7, 4);
				packDWord(&packet, MIRANDA_VERSION);
	            packDWord(&packet, ICQ_S7SSS_VERSION);
				break;
			case 3:
				packBuffer(&packet, capIcqJSin, 4);
				packDWord(&packet, MIRANDA_VERSION);
				packDWord(&packet, ICQ_SIN_VERSION);
				break;
			default:
			case 4:
				packBuffer(&packet, capIcqJp, 4);
				packDWord(&packet, MIRANDA_VERSION);
				packDWord(&packet, ICQ_PLUG_VERSION);
				break;
			case 5:
				packBuffer(&packet, capIcqJen, 4);
				packDWord(&packet, MIRANDA_VERSION);
				packDWord(&packet, ICQ_PLUSPLUS_VERSION);
			break;
			}
			switch (cICQModID)
			{ // send SecureIM or not for Mods
			case 2: // s7sss,
			case 3: // sin,
			case 4: // plus and
			case 5: // eternity mods may decode SecureIM flag
				packDWord( &packet, m_bSecureIM ? 0x5AFEC0DE : 0x00000000 );
			case 0: // icqj and bm mod
			case 1: // packets are already full
			default :
				break;
			}
		}
		AddCapabilitiesToBuffer((BYTE*)&packet,
		    // CAPF_TYPING | // OBSOLETE, moved below
			// CAPF_SRV_RELAY | // OBSOLETE, moved below
			(m_bRTFEnabled ? CAPF_RTF : 0) //|
			// (m_bXStatusEnabled ? CAPF_XTRAZ : 0) | // OBSOLETE, moved below
 			// (m_bUtfEnabled?CAPF_UTF:0) | // OBSOLETE, moved below
			// (m_bAvatarsEnabled?CAPF_ICQ_DEVIL:0) | // OBSOLETE, moved below
			// CAPF_OSCAR_FILE|CAPF_ICQDIRECT // OBSOLETE, moved below
			);

		packNewCap(&packet, 0x1349);    // AIM_CAPS_ICQSERVERRELAY
		// Broadcasts the capability to receive UTF8 encoded messages
		if (m_bUtfEnabled)
			packNewCap(&packet, 0x134E);    // CAP_UTF8MSGS
		if (bXStatus)
			packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
		if (m_bDCMsgEnabled) // TODO: Maybe something else?
			packNewCap(&packet, 0x1344);      // AIM_CAPS_ICQDIRECT
		if (m_bAimEnabled)
			//packBuffer(&packet, capAimIcq, 0x10); // OBSOLETE
			packNewCap(&packet, 0x134D);    // Tells the server we can speak to AIM
		if (m_bAvatarsEnabled)
			packNewCap(&packet, 0x134C);    // CAP_DEVILS
		if (bXStatus)
			packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
		if (m_bTzersEnabled)
			packBuffer(&packet, captZers, 0x10);

#ifdef DBG_CAPHTML
	{
// DESCRIPTION {
//		packDWord(&packet, 0x0138ca7b); // CAP_HTMLMSGS
//		packDWord(&packet, 0x769a4915); // Broadcasts the capability to receive
//		packDWord(&packet, 0x88f213fc); // HTML messages
//		packDWord(&packet, 0x00979ea8);
// }
		packBuffer(&packet, capHtmlMsgs, 0x10);
	}
#endif
#ifdef DBG_AIMCONTACTSEND
		// packBuffer(&packet, capAimSendbuddylist, 0x10); // OBSOLETE
		packNewCap(&packet, 0x134B);    // CAP_AIM_SENDBUDDYLIST
#endif
#ifdef DBG_CAPXTRAZ_MUC
		packNewCap(&packet, 0x134C);    // CAP_XTRAZ_MUC
#endif
#ifdef DBG_CAPMTN
// DESCRIPTION {
//		packDWord(&packet, 0x563FC809); // CAP_TYPING
//		packDWord(&packet, 0x0B6F41BD);
//		packDWord(&packet, 0x9F794226);
//		packDWord(&packet, 0x09DFA2F3);
// }
		packBuffer(&packet, capTyping, 0x10);
#endif
#ifdef DBG_NEWCAPS
		// Tells server we understand to new format of caps
		packNewCap(&packet, 0x0000);    // CAP_NEWCAPS
#endif
#ifdef DBG_CAPXTRAZ
// DESCRIPTION {
//		packDWord(&packet, 0x1a093c6c); // CAP_XTRAZ
//		packDWord(&packet, 0xd7fd4ec5); // Broadcasts the capability to handle
//		packDWord(&packet, 0x9d51a647); // Xtraz
//		packDWord(&packet, 0x4e34f5a0);
// }
		packBuffer(&packet, capXtraz, 0x10);
#endif
#ifdef DBG_OSCARFT
		// Broadcasts the capability to receive Oscar File Transfers
		packNewCap(&packet, 0x1343);    // CAP_AIM_FILE
#endif

/*		packDWord(&packet, 0xb99708b5); /// voice chat ???? unknown
		packDWord(&packet, 0x3a924202);
		packDWord(&packet, 0xb069f1e7);
		packDWord(&packet, 0x57bb2e17);*/

/*		packDWord(&packet, 0x67361515); /// icq lite audio chat Xtraz "ICQTalk" plugin
		packDWord(&packet, 0x612d4c07);
		packDWord(&packet, 0x8f3dbde6);
		packDWord(&packet, 0x408ea041);*/

/*		packDWord(&packet, 0x178c2d9b); // Unknown cap
		packDWord(&packet, 0xdaa545bb); /// icq lite video chat Xtraz "VideoRcv" plugin
		packDWord(&packet, 0x8ddbf3bd);
		packDWord(&packet, 0xbd53a10a);*/

		break;

	case 2:								//QIP
		packBuffer(&packet, capQip, 0x10);
		packBuffer(&packet, capQip_1, 0x10);
		break;
	case 23:
		packBuffer(&packet, capQipPDA, 0x10); 	  
		break;
	case 24:
		packBuffer(&packet, capQipSymbian, 0x10); 	  
		break;
	case 46:  //QIP Infium
		packBuffer(&packet, capQipInfium, 0x10);
		packBuffer(&packet, capQipPlugins, 0x10);
		break; 
	case 4:								//icq 
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_OSCAR_FILE|CAPF_XTRAZ_CHAT|CAPF_PUSH2TALK|CAPF_VOICE_CHAT|CAPF_UTF);
		break;
	case 9:
		packBuffer(&packet, capKopete, 0xC); 
		packDWord(&packet, 0x000C0007);
		break;
	case 10:
		packBuffer(&packet, capMacIcq, 0x10); 
		break;	  
	case 5:
		packBuffer(&packet, capAndRQ, 9); 
		packBuffer(&packet, (BYTE*)"\x04\x07\x09\x00\x00\x00\x00",7);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY);
		break;
	case 6:
		packBuffer(&packet, capJimm, 5); 
		packBuffer(&packet, (BYTE*)"\x30\x2E\x36\x2E\x33\x61\x00\x00\x00\x00\x00",11);
		break;
	case 7:
		packBuffer(&packet, capTrillian, 0x10); 
		break;
	case 8:
		packBuffer(&packet, capLicq, 0xC); 
		packDWord(&packet, 0x01030200);
		break;
	case 12:
		packBuffer(&packet, capRambler, 0x10);
		packBuffer(&packet, captZers, 0x10); 
		packBuffer(&packet, capIcqLite, 0x10); 
		break;	  
	case 13:
		packBuffer(&packet, captZers, 0x10); 
		break;	  
	case 14:
		packBuffer(&packet, capAbv, 0x10); 
		break;
	case 15:
		packBuffer(&packet, capNetvigator, 0x10); 
		break;
	case 20:
		packBuffer(&packet, capmChat, 0x10); 
		break;
	case 25:
		packBuffer(&packet, capIs2002, 0x10);	  
		break;
	case 26:	
		packBuffer(&packet, captZers, 0x10); 
		packBuffer(&packet, capHtmlMsgs, 0x10); 
		packBuffer(&packet, capLiveVideo, 0x10);
		packBuffer(&packet, capAimContactSnd, 0x10);
		packBuffer(&packet, capAimAudio, 0x10);
		packBuffer(&packet, capAimChat, 0x10); 
		packBuffer(&packet, capIcqLite, 0x10); 
		break;
	case 27:
		packBuffer(&packet, capComm20012, 0x10);
		packBuffer(&packet, capIs2001, 0x10);		 
		break;
	case 28:
		packBuffer(&packet, capAnastasia, 0x10);	  
		break;
	case 16:
		packBuffer(&packet, capSim, 0xC); 
		packDWord(&packet, 0x00090540);
		break;
	case 17:
		packBuffer(&packet, capSim, 0xC); 
		packDWord(&packet, 0x00090580);
		break;
	case 18:
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY|CAPF_ICQDIRECT|CAPF_RTF);
		break;
	case 31:
		packBuffer(&packet, capmIcq, 0xC);  //mIcq
		packBuffer(&packet, (BYTE*)"\x08\x02\x00\x00",4);	  
		break;
	case 34:    	  //IM2
		packBuffer(&packet, capIm2, 0x10);
		break; 
	case 35:           //GAIM
		packBuffer(&packet, capAimIcon, 0x10);
		packBuffer(&packet, capAimDirect, 0x10);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_OSCAR_FILE|CAPF_UTF);
		break;
	case 40:      //uIM
		packBuffer(&packet, capUim, 0x10);
		break;
	case 41:     //TICQClient
		packBuffer(&packet, capComm20012, 0x10);
		packBuffer(&packet, capIs2001, 0x10);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY|CAPF_RTF); 
		break;
	case 42:     //IC@
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY|CAPF_RTF|CAPF_UTF);
		break;
	case 43:    //PreludeICQ
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY|CAPF_UTF|CAPF_TYPING);
		break;
	case 44:  //QNEXT
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_RTF);
		break;
	case 47:  //JICQ
		packBuffer(&packet, capPalmJicq, 0xC);
		packBuffer(&packet, (BYTE*)"\x00\x01\x00\x05",4);
		break;
	case 49:    //MIP
		packBuffer(&packet, capMipClient, 0xC);
		packBuffer(&packet, (BYTE*)"\x00\x06\x00\x00",4);
		break;
	case 50:  //Trillian Astra
		packBuffer(&packet, capTrillian, 0x10);
		packBuffer(&packet, capTrilCrypt, 0x10);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_RTF|CAPF_OSCAR_FILE);
		break; 
	case 51: //R&Q
		packBuffer(&packet, capRAndQ, 9);
		packBuffer(&packet, (BYTE*)"\x07\x00\x01\x01",4);
		break;
	case 52:   //NanoICQ
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_UTF);
		break;
	case 53: //IMadering
		packBuffer(&packet, capIMadering, 0x10);
		break;
	case 54: // MirandaMobile
		packBuffer(&packet, capMimMobile, 16);
		break;
	case 55: // D[i]Chat
		packBuffer(&packet,(BYTE*) "D[i]Chat v.0.71",16);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|CAPF_UTF|(m_bXStatusEnabled?CAPF_XTRAZ:0)|CAPF_XTRAZ_CHAT|CAPF_OSCAR_FILE);
		break;
	case 56: // LocID
		packBuffer(&packet, (BYTE*)"LocID", 0x10);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|CAPF_UTF|(m_bXStatusEnabled?CAPF_XTRAZ:0)|CAPF_XTRAZ_CHAT|CAPF_OSCAR_FILE);
	    break;
	case 57: // BayanICQ
		packBuffer(&packet, (BYTE*)"bayanICQ0.1d", 16);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|CAPF_UTF|(m_bXStatusEnabled?CAPF_XTRAZ:0)|CAPF_OSCAR_FILE|CAPF_ICQ_DEVIL);
		break;
	case 58: // Core Pager
		packBuffer(&packet, capCorePager, 16);
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|CAPF_UTF|(m_bXStatusEnabled?CAPF_XTRAZ:0)|CAPF_OSCAR_FILE|CAPF_ICQ_DEVIL|CAPF_ICQDIRECT);
		break;
	default:
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY);
		break;
	}

	if (cID == 2 || cID == 23 || cID == 24 || cID == 46) // QIP caps
	{
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING | CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_ICQ_DEVIL | CAPF_UTF | CAPF_RTF | (m_bXStatusEnabled ? CAPF_XTRAZ : 0) | CAPF_OSCAR_FILE);
		if (bXStatus)
			packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	}
	if (cID == 6 || cID == 51 || cID == 45 || cID == 49 || cID == 55 || cID == 56 || cID == 57 || cID == 58) //active xstatus
	{
		if (bXStatus)
			packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	}
	if (cID >= 6 && cID <= 10 || cID == 16 || cID == 17 || cID == 20 || cID == 25 || cID >= 27 && cID <= 40 || cID == 44||cID == 45||cID == 47 || cID >= 49 && cID <= 52) //default caps
	{
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY);
	}
	if (cID >= 12 && cID <= 15) // ICQLite 5 group caps
	{
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING | CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_PUSH2TALK | CAPF_ICQ_DEVIL | CAPF_UTF | CAPF_RTF | (m_bXStatusEnabled ? CAPF_XTRAZ : 0) | CAPF_XTRAZ_CHAT | CAPF_OSCAR_FILE | CAPF_VOICE_CHAT);
		if (bXStatus)
			packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	}
	if (cID == 26) // ICQ 6 group caps
	{
		AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING | CAPF_SRV_RELAY | CAPF_UTF | (m_bXStatusEnabled ? CAPF_XTRAZ : 0) | CAPF_XTRAZ_CHAT | CAPF_OSCAR_FILE);
		if (bXStatus)
			packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	}
  
	if (m_bCustomCapsEnabled)
		for (int i = 0; i < lstCustomCaps->realCount; ++i)
			packBuffer(&packet, (BYTE*)((ICQ_CUSTOMCAP *)lstCustomCaps->items[i])->caps, 0x10);

	{ // All capabilites set, let's store own client capabilities
		DBCONTACTWRITESETTING dbcws = {0};
		dbcws.value.type = DBVT_BLOB;
		dbcws.value.cpbVal = wAdditionalData + 46; // ??
		dbcws.value.pbVal = packet.pData + 20; // ??
		dbcws.szModule = m_szModuleName;
		dbcws.szSetting = "CapBuf";
		CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)NULL, (LPARAM)&dbcws);
	}

	sendServPacket(&packet);
}


void CIcqProto::updateClientID(void)
{
	icq_packet packet;
	DWORD dwFT1, dwFT2, dwFT3;
	int nPort = getSettingWord(NULL, "UserPort", 0);
	DWORD dwDirectCookie = rand() ^ (rand() << 16);

	// Get status
	WORD wStatus = MirandaStatusToIcq(m_iDesiredStatus);
	serverPacketInit(&packet, 71);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
	packDWord(&packet, 0x00060004);             // TLV 6: Status mode and security flags
	packWord(&packet, GetMyStatusFlags());      // Status flags
	packWord(&packet, wStatus);                 // Status
	packTLVWord(&packet, 0x0008, 0x0000);       // TLV 8: Error code
	packDWord(&packet, 0x000c0025);             // TLV C: Direct connection info
	packDWord(&packet, getSettingDword(NULL, "RealIP", 0));
	packDWord(&packet, nPort);
	packByte(&packet, DC_TYPE);                 // TCP/FLAG firewall settings
	packWord(&packet, (WORD)GetProtoVersion());
	packDWord(&packet, dwDirectCookie);         // DC Cookie
	packDWord(&packet, WEBFRONTPORT);           // Web front port
	packDWord(&packet, CLIENTFEATURES);         // Client features

	SetTimeStamps(&dwFT1, &dwFT2, &dwFT3);

	packDWord(&packet, dwFT1);
	packDWord(&packet, dwFT2);
	packDWord(&packet, dwFT3);
	packWord(&packet, 0x0000);                  // Unknown
	packTLVWord(&packet, 0x001F, 0x0000);
	sendServPacket(&packet);
}


void CIcqProto::handleServUINSettings(int nPort, serverthread_info *info)
{
	icq_packet packet;

	setUserInfo();

	/* SNAC 3,4: Tell server who's on our list */
	sendEntireListServ(ICQ_BUDDY_FAMILY, ICQ_USER_ADDTOLIST, BUL_ALLCONTACTS);

	if (m_iDesiredStatus == ID_STATUS_INVISIBLE)
	{
		/* Tell server who's on our visible list */
		if (!m_bSsiEnabled)
			sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDVISIBLE, BUL_VISIBLE);
		else
			updateServVisibilityCode(3);
	}

	if (m_iDesiredStatus != ID_STATUS_INVISIBLE)
	{
		/* Tell server who's on our invisible list */
		if (!m_bSsiEnabled)
			sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDINVISIBLE, BUL_INVISIBLE);
		else
			updateServVisibilityCode(4);
	}

	// SNAC 1,1E: Set status
	{
		DWORD dwDirectCookie = rand() ^ (rand() << 16);
		DWORD dwFT1, dwFT2, dwFT3;

		// Get status
		WORD wStatus = MirandaStatusToIcq(m_iDesiredStatus);

		// Get status note & mood
		char *szStatusNote = PrepareStatusNote(m_iDesiredStatus);
		BYTE bXStatus = getContactXStatus(NULL);
		char szMoodData[32];

		// prepare mood id
		if (bXStatus && moodXStatus[bXStatus-1] != -1)
			null_snprintf(szMoodData, SIZEOF(szMoodData), "icqmood%d", moodXStatus[bXStatus-1]);
		else
			szMoodData[0] = '\0';

		//! Tricky code, this ensures that the status note will be saved to the directory
		SetStatusNote(szStatusNote, m_bGatewayMode ? 5000 : 2500, TRUE);

		WORD wStatusNoteLen = strlennull(szStatusNote);
		WORD wStatusMoodLen = strlennull(szMoodData);
		WORD wSessionDataLen = (wStatusNoteLen ? wStatusNoteLen + 4 : 0) + 4 + wStatusMoodLen + 4;

		serverPacketInit(&packet, (WORD)(71 + (wSessionDataLen ? wSessionDataLen + 4 : 0)));
		packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
		packDWord(&packet, 0x00060004);             // TLV 6: Status mode and security flags
		packWord(&packet, GetMyStatusFlags());      // Status flags
		packWord(&packet, wStatus);                 // Status
		packTLVWord(&packet, 0x0008, 0x0000);       // TLV 8: Error code
		packDWord(&packet, 0x000c0025);             // TLV C: Direct connection info
		packDWord(&packet, getSettingDword(NULL, "RealIP", 0));
		packDWord(&packet, nPort);
		packByte(&packet, DC_TYPE);                 // TCP/FLAG firewall settings
		packWord(&packet, GetProtoVersion());
		packDWord(&packet, dwDirectCookie);         // DC Cookie
		packDWord(&packet, WEBFRONTPORT);           // Web front port
		packDWord(&packet, CLIENTFEATURES);         // Client features
		SetTimeStamps(&dwFT1, &dwFT2, &dwFT3);      // Client detection caps
		packDWord(&packet, dwFT1);                  // -||-
		packDWord(&packet, dwFT2);                  // -||-
		packDWord(&packet, dwFT3);                  // -||-
		packWord(&packet, 0x0000);                  // Unknown
		packTLVWord(&packet, 0x001F, 0x0000);

		if (wSessionDataLen)
		{ // Pack session data
			packWord(&packet, 0x1D);                  // TLV 1D
			packWord(&packet, wSessionDataLen);       // TLV length
			packWord(&packet, 0x02);                  // Item Type
			if (wStatusNoteLen)
			{
				packWord(&packet, 0x400 | (WORD)(wStatusNoteLen + 4)); // Flags + Item Length
				packWord(&packet, wStatusNoteLen);      // Text Length
				packBuffer(&packet, (LPBYTE)szStatusNote, wStatusNoteLen);
				packWord(&packet, 0);                   // Encoding not specified (utf-8 is default)
			}
			else
				packWord(&packet, 0);                   // Flags + Item Length
			packWord(&packet, 0x0E);                  // Item Type
			packWord(&packet, wStatusMoodLen);        // Flags + Item Length
			if (wStatusMoodLen)
				packBuffer(&packet, (LPBYTE)szMoodData, wStatusMoodLen); // Mood

			// Save current status note & mood
			setSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, szStatusNote);
			setSettingString(NULL, DBSETTING_STATUS_MOOD, szMoodData);
		}
		// Release memory
		SAFE_FREE(&szStatusNote);

		sendServPacket(&packet);
	}

	{ // PLUS BEGIN
		LPSTR szClient = {0};
		static DWORD dwFT1, dwFT2, dwFT3;
		char szStrBuf[MAX_PATH];
		DBVARIANT dbv = {0};
		DBCONTACTGETSETTING dbcgs;
		dbcgs.szModule = m_szProtoName;
		dbcgs.szSetting = "CapBuf";
		dbcgs.pValue = &dbv;
		CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&dbcgs);
		if (dbv.type == DBVT_BLOB)
		{
			char bClient = 0;
			WORD bufsize = dbv.cpbVal;
			BYTE *buf = dbv.pbVal;
			SetTimeStamps(&dwFT1, &dwFT2, &dwFT3);
			// TODO: Check if own client detection works properly -- UInfo still shows wrong icon
			szClient = detectUserClient(NULL, 0, m_wClass, GetProtoVersion(), dwFT1, dwFT2, dwFT3, 0, 0, 0, 0, buf, bufsize, (LPBYTE)&bClient, szStrBuf);
			if (!szClient || szClient < 0)
				szClient = "Unknown";
			setSettingStringUtf(NULL, "MirVer", szClient);
		}
	} // PLUS END

	/* SNAC 1,11 */
	serverPacketInit(&packet, 14);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_IDLE);
	packDWord(&packet, 0x00000000);

	sendServPacket(&packet);
	m_bIdleAllow = 0;

	// Change status
	SetCurrentStatus(m_iDesiredStatus);

	// Finish Login sequence
	serverPacketInit(&packet, 98);
	packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_READY);
	packDWord(&packet, 0x00220001); // imitate ICQ 6 behaviour
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00010004);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00130004);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00020001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00030001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00150001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00040001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00060001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x00090001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x000A0001);
	packDWord(&packet, 0x0110164f);
	packDWord(&packet, 0x000B0001);
	packDWord(&packet, 0x0110164f);

	sendServPacket(&packet);

	NetLog_Server(" *** Yeehah, login sequence complete");

	// login sequence is complete enter logged-in mode
	info->bLoggedIn = 1;
	m_bConnectionLost = FALSE;

	// enable auto info-update routine
	icq_EnableUserLookup(TRUE);

	if (!info->isMigrating)
	{ /* Get Offline Messages Reqeust */
		cookie_offline_messages *ack = (cookie_offline_messages*)SAFE_MALLOC(sizeof(cookie_offline_messages));
		if (ack)
		{
			DWORD dwCookie = AllocateCookie(CKT_OFFLINEMESSAGE, ICQ_MSG_CLI_REQ_OFFLINE, 0, ack);

			serverPacketInit(&packet, 10);
			packFNACHeader(&packet, ICQ_MSG_FAMILY, ICQ_MSG_CLI_REQ_OFFLINE, 0, dwCookie);

			sendServPacket(&packet);
		}
		else
			icq_LogMessage(LOG_WARNING, LPGEN("Failed to request offline messages. They may be received next time you log in."));

		// Update our information from the server
		sendOwnerInfoRequest();

		// Request info updates on all contacts
		icq_RescanInfoUpdate();

		// Start sending Keep-Alive packets
		StartKeepAlive(info);

		if (m_bAvatarsEnabled)
		{ // Send SNAC 1,4 - request avatar family 0x10 connection
			icq_requestnewfamily(ICQ_AVATAR_FAMILY, &CIcqProto::StartAvatarThread);

			m_pendingAvatarsStart = 1;
			NetLog_Server("Requesting Avatar family entry point.");
		}
	}
	info->isMigrating = 0;

	if (m_bAimEnabled)
	{
		char **szMsg = MirandaStatusToAwayMsg(m_iStatus);

		EnterCriticalSection(&m_modeMsgsMutex);
		if (szMsg && !m_bNoStatusReply)
			icq_sendSetAimAwayMsgServ(*szMsg);
		LeaveCriticalSection(&m_modeMsgsMutex);
	}
	CheckSelfRemove();
}
