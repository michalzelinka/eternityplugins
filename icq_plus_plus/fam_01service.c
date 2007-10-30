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
// Revision       : $Revision: 36 $
// Last change on : $Date: 2007-08-05 03:45:10 +0300 (Ð’Ñ, 05 Ð°Ð²Ð³ 2007) $
// Last change by : $Author: sss123next $
//
// DESCRIPTION:
//
//  Describe me here please...
//
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#define MD5_BLOCK_SIZE 1024*1024 // use 1MB blocks
extern int gbIdleAllow;
extern int icqGoingOnlineStatus;
extern int pendingAvatarsStart;
extern WORD wListenPort;
extern CRITICAL_SECTION modeMsgsMutex;
extern const capstr capXStatus[];

extern char* detectUserClient(HANDLE hContact, DWORD dwUin, WORD wVersion, DWORD dwFT1, DWORD dwFT2, DWORD dwFT3, DWORD dwOnlineSince, BYTE bDirectFlag, DWORD dwDirectCookie, DWORD dwWebPort, BYTE* caps, WORD wLen, BYTE* bClientId, char* szClientBuf);

void setUserInfo();

char* calcMD5Hash(char* szFile);

int bSecureIM()
{
  if (ServiceExists("SecureIM/IsContactSecured"))
	  return 1;
  else
	  return 0;
}

WORD icq_ver()
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



void handleServiceFam(unsigned char* pBuffer, WORD wBufferLength, snac_header* pSnacHeader, serverthread_info *info)
{
  icq_packet packet;

  switch (pSnacHeader->wSubtype)
  {

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
    gRates = ratesCreate(pBuffer, wBufferLength);
    /* ack rate levels */
    serverPacketInit(&packet, 20);
    packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_RATE_ACK);
    packDWord(&packet, 0x00010002);
    packDWord(&packet, 0x00030004);
    packWord(&packet, 0x0005);
    sendServPacket(&packet);

    /* CLI_REQINFO - This command requests from the server certain information about the client that is stored on the server. */
#ifdef _DEBUG
    NetLog_Server("Sending CLI_REQINFO");
#endif
    serverPacketInit(&packet, 10);
    packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_REQINFO);
    sendServPacket(&packet);

    if (gbSsiEnabled)
    {
      DWORD dwLastUpdate;
      WORD wRecordCount;
      servlistcookie* ack;
      DWORD dwCookie;

      dwLastUpdate = ICQGetContactSettingDword(NULL, "SrvLastUpdate", 0);
      wRecordCount = ICQGetContactSettingWord(NULL, "SrvRecordCount", 0);

      // CLI_REQLISTS - we want to use SSI
#ifdef _DEBUG
      NetLog_Server("Requesting roster rights");
#endif
      serverPacketInit(&packet, 10);
      packFNACHeader(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQLISTS);
      sendServPacket(&packet);

      if (!wRecordCount) // CLI_REQROSTER
      { // we do not have any data - request full list
#ifdef _DEBUG
        NetLog_Server("Requesting full roster");
#endif
        serverPacketInit(&packet, 10);
        ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
        if (ack)
        { // we try to use standalone cookie if available
          ack->dwAction = SSA_CHECK_ROSTER; // loading list
          dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_REQUEST, 0, ack);
        }
        else // if not use that old fake
          dwCookie = ICQ_LISTS_CLI_REQUEST<<0x10;

        packFNACHeaderFull(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_REQUEST, 0, dwCookie);
        sendServPacket(&packet);
      }
      else // CLI_CHECKROSTER
      {
#ifdef _DEBUG
        NetLog_Server("Requesting roster check");
#endif
        serverPacketInit(&packet, 16);
        ack = (servlistcookie*)SAFE_MALLOC(sizeof(servlistcookie));
        if (ack)  // TODO: rewrite - use get list service for empty list
        { // we try to use standalone cookie if available
          ack->dwAction = SSA_CHECK_ROSTER; // loading list
          dwCookie = AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_CLI_CHECK, 0, ack);
        }
        else // if not use that old fake
          dwCookie = ICQ_LISTS_CLI_CHECK<<0x10;

        packFNACHeaderFull(&packet, ICQ_LISTS_FAMILY, ICQ_LISTS_CLI_CHECK, 0, dwCookie);
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

    // CLI_REALOCATION
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
    serverPacketInit(&packet, 10);
    packFNACHeader(&packet, ICQ_BUDDY_FAMILY, ICQ_USER_CLI_REQBUDDY);
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
      oscar_tlv_chain *chain = NULL;

#ifdef _DEBUG
      NetLog_Server("Server migration requested (Flags: %u)", pSnacHeader->wFlags);
#endif
      pBuffer += 2; // Unknown, seen: 0
      wBufferLength -= 2;
      chain = readIntoTLVChain(&pBuffer, wBufferLength, 0);

      if (info->cookieDataLen > 0)
        SAFE_FREE(&info->cookieData);

      info->newServer = getStrFromChain(chain, 0x05, 1);
      info->cookieData = getStrFromChain(chain, 0x06, 1);
      info->cookieDataLen = getLenFromChain(chain, 0x06, 1);

      if (!info->newServer || !info->cookieData)
      {
        icq_LogMessage(LOG_FATAL, "A server migration has failed because the server returned invalid data. You must reconnect manually.");
        SAFE_FREE(&info->newServer);
        SAFE_FREE(&info->cookieData);
        info->cookieDataLen = 0;
        info->newServerReady = 0;
        return;
      }

      disposeChain(&chain);
      NetLog_Server("Migration has started. New server will be %s", info->newServer);

      icqGoingOnlineStatus = gnCurrentStatus;
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
        dwValue = getDWordFromChain(chain, 10, 1); 
        ICQWriteContactSettingDword(NULL, "IP", dwValue);
		ICQWriteContactSettingDword(NULL, "OldIP", dwValue);

        // Save member since timestamp
        dwValue = getDWordFromChain(chain, 5, 1); 
        if (dwValue) ICQWriteContactSettingDword(NULL, "MemberTS", dwValue);

        dwValue = getDWordFromChain(chain, 3, 1);
        ICQWriteContactSettingDword(NULL, "LogonTS", dwValue ? dwValue : time(NULL));

        disposeChain(&chain);

        // If we are in SSI mode, this is sent after the list is acked instead
        // to make sure that we don't set status before seing the visibility code
        if (!gbSsiEnabled || info->isMigrating)
          handleServUINSettings(wListenPort, info);
      }
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
      ratesUpdateLevel(gRates, wClass, dwLevel);
      LeaveCriticalSection(&ratesMutex);

      if (wStatus == 2 || wStatus == 3)
      { // this is only the simplest solution, needs rate management to every section
        ICQBroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
        if (wStatus == 2)
          NetLog_Server("Rates #%u: Alert", wClass);
        else
          NetLog_Server("Rates #%u: Limit", wClass);
      }
      else if (wStatus == 4)
      {
        ICQBroadcastAck(NULL, ICQACKTYPE_RATEWARNING, ACKRESULT_STATUS, (HANDLE)wClass, wStatus);
        NetLog_Server("Rates #%u: Clear", wClass);
      }
	icq_CheckSpeed(wStatus);
    }

    break;

  case ICQ_SERVER_REDIRECT_SERVICE: // reply to family request, got new connection point
  {
    oscar_tlv_chain* pChain = NULL;
    WORD wFamily;
    familyrequest_rec* reqdata;

    if (!(pChain = readIntoTLVChain(&pBuffer, wBufferLength, 0)))
    {
      NetLog_Server("Received Broken Redirect Service SNAC(1,5).");
      break;
    }
    wFamily = getWordFromChain(pChain, 0x0D, 1);

    // pick request data
    if ((!FindCookie(pSnacHeader->dwRef, NULL, &reqdata)) || (reqdata->wFamily != wFamily))
    {
      disposeChain(&pChain);
      NetLog_Server("Received unexpected SNAC(1,5), skipping.");
      break;
    }

    FreeCookie(pSnacHeader->dwRef);

    { // new family entry point received
      char* pServer;
      WORD wPort;
      char* pCookie;
      WORD wCookieLen;
      NETLIBOPENCONNECTION nloc = {0};
      HANDLE hConnection;

      pServer = getStrFromChain(pChain, 0x05, 1);
      pCookie = getStrFromChain(pChain, 0x06, 1);
      wCookieLen = getLenFromChain(pChain, 0x06, 1);

      if (!pServer || !pCookie)
      {
        NetLog_Server("Server returned invalid data, family unavailable.");

        SAFE_FREE(&pServer);
        SAFE_FREE(&pCookie);
        SAFE_FREE(&reqdata);
        break;
      }

      // Get new family server ip and port
      wPort = info->wServerPort; // get default port
      parseServerAddress(pServer, &wPort);

      // establish connection
      nloc.flags = 0;
      nloc.szHost = pServer; 
      nloc.wPort = wPort;

      hConnection = NetLib_OpenConnection(ghServerNetlibUser, wFamily == ICQ_AVATAR_FAMILY ? "Avatar " : NULL, &nloc);
      
      if (hConnection == NULL)
      {
        NetLog_Server("Unable to connect to ICQ new family server.");
      } // we want the handler to be called even if the connecting failed
      reqdata->familyhandler(hConnection, pCookie, wCookieLen);

      // Free allocated memory
      // NOTE: "cookie" will get freed when we have connected to the avatar server.
      disposeChain(&pChain);
      SAFE_FREE(&pServer);
      SAFE_FREE(&reqdata);
    }

    break;
  }

  case ICQ_SERVER_EXTSTATUS: // our avatar
  {
    NetLog_Server("Received our avatar hash & status.");

    if (wBufferLength > 4 && pBuffer[1] == AVATAR_HASH_PHOTO)
    { // skip photo item
      if (wBufferLength >= pBuffer[3] + 4)
      {
        wBufferLength -= pBuffer[3] + 4;
        pBuffer += pBuffer[3] + 4;
      }
      else
      {
        pBuffer += wBufferLength;
        wBufferLength = 0;
      }
    }

    if ((wBufferLength >= 0x14) && gbAvatarsEnabled)
    {
      if (!info->bMyAvatarInited) // signal the server after login
      { // this refreshes avatar state - it used to work automatically, but now it does not
        if (ICQGetContactSettingByte(NULL, "ForceOurAvatar", 0))
        { // keep our avatar
          char* file = loadMyAvatarFileName();

          IcqSetMyAvatar(0, (LPARAM)file);
          SAFE_FREE(&file);
        }
        else // only change avatar hash to the same one
        {
          char hash[0x14];

          memcpy(hash, pBuffer, 0x14);
          hash[2] = 1; // update image status
          updateServAvatarHash(hash, 0x14);
        }
        info->bMyAvatarInited = TRUE;
        break;
      }

      switch (pBuffer[2])
      {
        case 1: // our avatar is on the server
        {
          char* file;
          char* hash;

          ICQWriteContactSettingBlob(NULL, "AvatarHash", pBuffer, 0x14);
          
          setUserInfo();
          // here we need to find a file, check its hash, if invalid get avatar from server
          file = loadMyAvatarFileName();
          if (!file)
          { // we have no avatar file, download from server
            char szFile[MAX_PATH + 4];
#ifdef _DEBUG
            NetLog_Server("We have no avatar, requesting from server.");
#endif
            GetAvatarFileName(0, NULL, szFile, MAX_PATH);
            GetAvatarData(NULL, dwLocalUIN, NULL, pBuffer, 0x14, szFile);
          }
          else
          { // we know avatar filename
            hash = calcMD5Hash(file);
            if (!hash)
            { // hash could not be calculated - probably missing file, get avatar from server
              char szFile[MAX_PATH + 4];
#ifdef _DEBUG
              NetLog_Server("We have no avatar, requesting from server.");
#endif
              GetAvatarFileName(0, NULL, szFile, MAX_PATH);
              GetAvatarData(NULL, dwLocalUIN, NULL, pBuffer, 0x14, szFile);
            } // check if we had set any avatar if yes set our, if not download from server
            else if (memcmp(hash, pBuffer+4, 0x10))
            { // we have different avatar, sync that
              if (ICQGetContactSettingByte(NULL, "ForceOurAvatar", 1))
              { // we want our avatar, update hash
                DWORD dwPaFormat = DetectAvatarFormat(file);
                char* pHash = _alloca(0x14);

                NetLog_Server("Our avatar is different, set our new hash.");

                pHash[0] = 0;
                pHash[1] = dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC;
                pHash[2] = 1; // state of the hash
                pHash[3] = 0x10; // len of the hash
                memcpy(pHash + 4, hash, 0x10);
                updateServAvatarHash(pHash, 0x14);
              }
              else
              { // get avatar from server
                char szFile[MAX_PATH + 4];
#ifdef _DEBUG
                NetLog_Server("We have different avatar, requesting new from server.");
#endif
                GetAvatarFileName(0, NULL, szFile, MAX_PATH);
                GetAvatarData(NULL, dwLocalUIN, NULL, pBuffer, 0x14, szFile);
              }
            }
            SAFE_FREE(&hash);
            SAFE_FREE(&file);
          }
          break;
        }
        case 0x41: // request to upload avatar data
        case 0x81:
        { // request to re-upload avatar data
          char* file;
          char* hash;
          DWORD dwPaFormat;

          if (!gbSsiEnabled) break; // we could not change serv-list if it is disabled...

          file = loadMyAvatarFileName();
          if (!file)
          { // we have no file to upload, remove hash from server
            NetLog_Server("We do not have avatar, removing hash.");
            IcqSetMyAvatar(0, 0);
            break;
          }
          dwPaFormat = DetectAvatarFormat(file);
          hash = calcMD5Hash(file);
          if (!hash)
          { // the hash could not be calculated, remove from server
            NetLog_Server("We could not obtain hash, removing hash.");
            IcqSetMyAvatar(0, 0);
          }
          else if (!memcmp(hash, pBuffer+4, 0x10))
          { // we have the right file
            HANDLE hFile = NULL, hMap = NULL;
            BYTE* ppMap = NULL;
            long cbFileSize = 0;

            NetLog_Server("Uploading our avatar data.");

            if ((hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL )) != INVALID_HANDLE_VALUE)
              if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
                if ((ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) != NULL)
                  cbFileSize = GetFileSize( hFile, NULL );

            if (cbFileSize != 0)
            {
              SetAvatarData(NULL, (WORD)(dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC), ppMap, cbFileSize);
            }

            if (ppMap != NULL) UnmapViewOfFile(ppMap);
            if (hMap  != NULL) CloseHandle(hMap);
            if (hFile != NULL) CloseHandle(hFile);
            SAFE_FREE(&hash);
          }
          else
          {
            char* pHash = _alloca(0x14);

            NetLog_Server("Our file is different, set our new hash.");

            pHash[0] = 0;
            pHash[1] = dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC;
            pHash[2] = 1; // state of the hash
            pHash[3] = 0x10; // len of the hash
            memcpy(pHash + 4, hash, 0x10);
            updateServAvatarHash(pHash, 0x14);

            SAFE_FREE(&hash);
          }

          SAFE_FREE(&file);
          break;
        default:
          NetLog_Server("Received UNKNOWN Avatar Status.");
        }
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




char* calcMD5Hash(char* szFile)
{
  char* res = NULL;

  if (szFile)
  {
    HANDLE hFile = NULL, hMap = NULL;
    BYTE* ppMap = NULL;

    if ((hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL )) != INVALID_HANDLE_VALUE)
    {
      if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
      {
        long cbFileSize = GetFileSize( hFile, NULL );
        res = (char*)SAFE_MALLOC(16*sizeof(char));
        if (cbFileSize != 0 && res)
        {
          mir_md5_state_t state;
          mir_md5_byte_t digest[16];
          int dwOffset = 0;
          mir_md5_init(&state);
          while (dwOffset < cbFileSize)
          {
            int dwBlockSize = min(MD5_BLOCK_SIZE, cbFileSize-dwOffset);
            if (!(ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, dwOffset, dwBlockSize)))
              break;
            mir_md5_append(&state, (const mir_md5_byte_t *)ppMap, dwBlockSize);
            UnmapViewOfFile(ppMap);
            dwOffset += dwBlockSize;
          }
          mir_md5_finish(&state, digest);
          memcpy(res, digest, 16);
        }
      }
    }


    if (hMap  != NULL) CloseHandle(hMap);
    if (hFile != NULL) CloseHandle(hFile);
  }
  
  return res;
}


static char* buildUinList(int subtype, WORD wMaxLen, HANDLE* hContactResume)
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
    hContact = ICQFindFirstContact();

  while (hContact != NULL)
  {
    if (!ICQGetContactSettingUID(hContact, &dwUIN, &szUID))
    {
      szLen[0] = strlennull(strUID(dwUIN, szUID));

      switch (subtype)
      {

      case BUL_VISIBLE:
        add = ID_STATUS_ONLINE == ICQGetContactSettingWord(hContact, "ApparentMode", 0);
        break;

      case BUL_INVISIBLE:
        add = ID_STATUS_OFFLINE == ICQGetContactSettingWord(hContact, "ApparentMode", 0);
        break;

      case BUL_TEMPVISIBLE:
        add = ICQGetContactSettingByte(hContact, "TemporaryVisible", 0);
        // clear temporary flag
        // Here we assume that all temporary contacts will be in one packet
        ICQWriteContactSettingByte(hContact, "TemporaryVisible", 0);
        break;

      default:
        add = 1;

        // If we are in SS mode, we only add those contacts that are
        // not in our SS list, or are awaiting authorization, to our
        // client side list
        if (gbSsiEnabled && ICQGetContactSettingWord(hContact, "ServerId", 0) &&
          !ICQGetContactSettingByte(hContact, "Auth", 0))
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

    hContact = ICQFindNextContact(hContact);
  }
  *hContactResume = NULL;


  return szList;
}



void sendEntireListServ(WORD wFamily, WORD wSubtype, int listType)
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
      packBuffer(&packet, szList, (WORD)nListLen);
      sendServPacket(&packet);
    }

    SAFE_FREE(&szList);
  }
  while (hResumeContact);
}




void setUserInfo()
{ // CLI_SETUSERINFO
  icq_packet packet;
  WORD wAdditionalData = 0;
  BYTE bXStatus = gbXStatusEnabled?ICQGetContactSettingByte(NULL, DBSETTING_XSTATUSID, 0):0;
  BYTE cID = DBGetContactSettingWord(NULL, gpszICQProtoName, "CurrentID", 0);
  BYTE cICQModID = DBGetContactSettingWord(NULL, gpszICQProtoName, "CurrentICQModID", 0); // eternity BYTE

// eternity :: added last six 4s for new custom caps
  static char wID[] = { 4, 1, 8, 0, 4, 1, 1, 1, 1, 1, 1, 4,//12
					   10, 10, 10, 10, 1, 1, 2, 0, 1, 0, 0, 6,//24
					   6, 1, 12, 2, 1, 0, 0, 0, 0, 0, 1, 4,//36
					   0, 0, 0, 0, 1, 3, 2, 2, 1, 0, 8, 0,//48
					   0, 1, 3, 0, 0, 0,//54
					   4}; // eternity : 55 xo) last is MirandaMobile


  if(!cID||cID==11||cID==54) 
  { // if miranda or as 'miranda' or as 'mirandamobile'
#ifdef DBG_XTRAZ_MUC
	wAdditionalData += 16;
#endif
	if(gbAimEnabled)
		wAdditionalData += 16;	
	if(gbAvatarsEnabled)
		wAdditionalData += 16;
	if(gbUtfEnabled)
		wAdditionalData += 16;
	if(gbRTFEnabled)
		wAdditionalData += 16;
	if(gbXStatusEnabled)
		wAdditionalData += 16;
	if(gbTzerEnabled)
        wAdditionalData += 16;
	if(bXStatus)
		wAdditionalData += 16;
  }
  else if(gbHideIdEnabled)
	  wAdditionalData += 16;
  wAdditionalData += wID[cID]*16;
  if(gbCustomCapEnabled)
  {
	  wAdditionalData += 16 * lstCustomCaps->realCount;
  }
  if(cID==2||cID==23||cID==24||cID==46) //qip caps
  {
	  if(gbXStatusEnabled)
		  wAdditionalData += 16;
	  if(bXStatus)
		  wAdditionalData += 16;
  }
  if(cID>=12&&cID<=15||cID==26) //icq lite group caps
  {
	  if(gbXStatusEnabled)
		  wAdditionalData += 16;
	  if(bXStatus)
		  wAdditionalData += 16;
  }
  serverPacketInit(&packet, (WORD)(62+wAdditionalData));
  packFNACHeader(&packet, ICQ_LOCATION_FAMILY, ICQ_LOCATION_SET_USER_INFO);

  /* TLV(5): capability data */
  packWord(&packet, 0x0005);
  packWord(&packet, (WORD)(wAdditionalData+32));
  packBuffer(&packet, capNewCap, 0x10);
  if(gbHideIdEnabled&&cID!=11)
  {
    /* sending my cap is useless when no support x)
	  packBuffer(&packet, capIcqJen, 4); // send eternity cap
	  packDWord(&packet, MIRANDA_VERSION); // add mirver
	  packDWord(&packet, ICQ_THISPLUG_VERSION); // add icqplug ver
	  if (bSecureIM()!=0)
		  packDWord(&packet, 0x5AFEC0DE);
	  else
		  packDWord(&packet, 0x00000000);
    */
    packBuffer(&packet, capMirandaIm, 8);
    packDWord(&packet, MIRANDA_VERSION);
    packDWord(&packet, ICQ_THISPLUG_VERSION);
  }
  switch (cID) //client change caps
  {
  case 11:
	  if(gbHideIdEnabled)
	  {
		  packBuffer(&packet, capIcqJen, 4);
		  packBuffer(&packet, "\x06\x06\x06\x00\x06\x06\x06\x00",8);
	    if (bSecureIM()!=0)
		    packDWord(&packet, 0x5AFEC0DE);
	    else
		    packDWord(&packet, 0x00000000);
	  }
	  else
	  {
		  packBuffer(&packet, capMirandaIm, 8); 
		  packBuffer(&packet, "\x06\x06\x06\x00\x06\x06\x06\x00",8);
	  }
  case 0:								//miranda
	  {
		  if(!cID && !gbHideIdEnabled)
		  { // Miranda Signature - eternity : added customization of various ICQ Mods
        switch (cICQModID) {
          default:
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
          case 4:
            packBuffer(&packet, capIcqJp, 4);
            packDWord(&packet, MIRANDA_VERSION);
            packDWord(&packet, ICQ_PLUG_VERSION);
            break;
          case 5:
            packBuffer(&packet, capIcqJen, 4);
            packDWord(&packet, MIRANDA_VERSION);
            packDWord(&packet, ICQ_THISPLUG_VERSION);
            break;
        }
        if (cICQModID > 1 && cICQModID < 6) // send SecureIM or not for Mods
        {
	      if (bSecureIM()!=0)
		      packDWord(&packet, 0x5AFEC0DE);
	      else
		      packDWord(&packet, 0x00000000);
        }
      } // eternity : added customization of various ICQ Mods END
		  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|(gbRTFEnabled?CAPF_RTF:0)|(gbXStatusEnabled?CAPF_XTRAZ:0)
			  |(gbUtfEnabled?CAPF_UTF:0)|(gbAvatarsEnabled?CAPF_ICQ_DEVIL:0)|CAPF_AIM_FILE|CAPF_DIRECT);
		  if (bXStatus)
			  packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
		  if (gbAimEnabled)
			  packBuffer(&packet, capAimIcq, 0x10); 
		  if (gbTzerEnabled)
		      packBuffer(&packet, captZers, 16);
#ifdef DBG_CAPXTRAZ_MUC
		  packNewCap(&packet, 0x134C);    // CAP_XTRAZ_MUC
#endif
	  }
	break;
  case 2:								//QIP
	  packBuffer(&packet, capQip, 0x10);
	  packBuffer(&packet, capQip_1, 0x10);
	  break;
  case 23:
	  packBuffer(&packet, capQipPDA, 0x10); 	  
	  break;
  case 24:
	  packBuffer(&packet, capQipMobile, 0x10); 	  
	  break;
  case 46:  //QIP Infium
	  packBuffer(&packet, capQipInfium, 0x10);
	  break; 
  case 4:								//icq 
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_AIM_FILE|CAPF_XTRAZ_CHAT|CAPF_PUSH2TALK|CAPF_VOICE_CHAT|CAPF_UTF);
	break;
  case 9:
	  packBuffer(&packet, capKopete, 0xC); 
	  packDWord(&packet, 0x000C0001);
	  break;
  case 10:
	  packBuffer(&packet, capMacIcq, 0x10); 
	  break;	  
  case 5:
	  packBuffer(&packet, capAndRQ, 9); 
	  packBuffer(&packet, "\x04\x07\x09\x00\x00\x00\x00",7);
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY);
	  break;
  case 6:
	  packBuffer(&packet, capJimm, 5); 
	  packBuffer(&packet, "\x30\x2E\x35\x2E\x32\x62\x00\x00\x00\x00\x00",11);
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
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY|CAPF_DIRECT|CAPF_RTF);
	break;
  case 31:
	  packBuffer(&packet, capmIcq, 0xC);  //mIcq
	  packBuffer(&packet, "\x08\x02\x00\x00",4);	  
	  break;
  case 34:    	  //IM2
	  packBuffer(&packet, capIm2, 0x10);
	  break; 
  case 35:           //GAIM
	  packBuffer(&packet, capAimIcon, 0x10);
	  packBuffer(&packet, capAimDirect, 0x10);
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_AIM_FILE|CAPF_UTF);
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
	  packBuffer(&packet, "\x00\x01\x00\x05",4);
	  break;
  case 49:    //MIP
      packBuffer(&packet, capMipClient, 0xC);
	  packBuffer(&packet, "\x00\x06\x00\x00",4);
	  break;
  case 50:  //Trillian Astra
      packBuffer(&packet, capTrillian, 0x10);
	  packBuffer(&packet, capTrilCrypt, 0x10);
      AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_RTF|CAPF_AIM_FILE);
	  break; 
  case 51: //R&Q
	  packBuffer(&packet, capRAndQ, 9);
      packBuffer(&packet, "\x01\x00\x06\x01",4);
	  break;
  case 52:   //NanoICQ
      AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_UTF);
      break;
  case 53: //IMadering
	  packBuffer(&packet, capIMadering, 0x10);
	  break;
  case 54: // eternity : MirandaMobile
    packBuffer(&packet, capMirandaMobile, 16);
    break;
  default:
	  {
		  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY);
	  }
	  break;
  }
  if(cID==2||cID==23||cID==24||cID==46) //qip caps
  {
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|CAPF_DIRECT|CAPF_ICQ_DEVIL|CAPF_UTF|CAPF_RTF|(gbXStatusEnabled?CAPF_XTRAZ:0)|CAPF_AIM_FILE);
	  if (bXStatus)
	  {
		  packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	  }
  }
  if(cID==10||cID>=6&&cID<=10||cID==16||cID==17||cID==20||cID==25||cID>=27&&cID<=40||cID==44||cID==45||cID==47||cID>=49&&cID<=52) //default caps
  {
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_SRV_RELAY);
  }
  if(cID>=12&&cID<=15||cID==26) //icq lite group caps
  {
	  AddCapabilitiesToBuffer((BYTE*)&packet, CAPF_TYPING|CAPF_SRV_RELAY|CAPF_DIRECT|CAPF_PUSH2TALK|CAPF_ICQ_DEVIL|CAPF_UTF|CAPF_RTF|(gbXStatusEnabled?CAPF_XTRAZ:0)|CAPF_XTRAZ_CHAT|CAPF_AIM_FILE|CAPF_VOICE_CHAT);
	  if (bXStatus)
	  {
		  packBuffer(&packet, capXStatus[bXStatus-1], 0x10);
	  }
  }
  
  { // store own client capabilities, and detect own client
	  DBCONTACTWRITESETTING dbcws;
	  dbcws.value.type = DBVT_BLOB;
	  dbcws.value.cpbVal = wAdditionalData+46;
	  dbcws.value.pbVal = packet.pData+20;
	  dbcws.szModule = gpszICQProtoName;
	  dbcws.szSetting = "CapBuf";
	  CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)NULL, (LPARAM)&dbcws);
  }
  if(gbCustomCapEnabled)
  {
	  int i;
	  for (i = 0; i < lstCustomCaps->realCount; ++i)
		  packBuffer(&packet, ((ICQ_CUSTOMCAP *)lstCustomCaps->items[i])->caps, 0x10); 
  }
  sendServPacket(&packet);
}


void handleServUINSettings(int nPort, serverthread_info *info)
{
  icq_packet packet;

  setUserInfo();

  /* SNAC 3,4: Tell server who's on our list */
  sendEntireListServ(ICQ_BUDDY_FAMILY, ICQ_USER_ADDTOLIST, BUL_ALLCONTACTS);

  if (icqGoingOnlineStatus == ID_STATUS_INVISIBLE)
  {
    /* Tell server who's on our visible list */
    if (!gbSsiEnabled)
      sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDVISIBLE, BUL_VISIBLE);
    else
      updateServVisibilityCode(3);
  }

  if (icqGoingOnlineStatus != ID_STATUS_INVISIBLE)
  {
    /* Tell server who's on our invisible list */
    if (!gbSsiEnabled)
      sendEntireListServ(ICQ_BOS_FAMILY, ICQ_CLI_ADDINVISIBLE, BUL_INVISIBLE);
    else
      updateServVisibilityCode(4);
  }

  // SNAC 1,1E: Set status
  {
    WORD wStatus;
    DWORD dwDirectCookie = rand() ^ (rand() << 16);


    // Get status
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
  	packDWord(&packet, ICQGetContactSettingDword(NULL,"dwFT1", 0));
  	packDWord(&packet, ICQGetContactSettingDword(NULL,"dwFT2", 0));
  	packDWord(&packet, ICQGetContactSettingDword(NULL,"dwFT3", 0));
    packWord(&packet, 0x0000);                  // Unknown
    packTLVWord(&packet, 0x001F, 0x0000);
    sendServPacket(&packet);
  }
	{
		LPSTR szClient = {0};
		static DWORD dwFT1, dwFT2, dwFT3;
		char szStrBuf[MAX_PATH];
		DBVARIANT dbv;
		DBCONTACTGETSETTING dbcgs;
		dbcgs.szModule = gpszICQProtoName;
		dbcgs.szSetting = "CapBuf";
		dbcgs.pValue = &dbv;
		CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&dbcgs);     
		if (dbv.type == DBVT_BLOB)
		{
			char bClient=0;
			WORD bufsize = dbv.cpbVal;
			BYTE *buf = dbv.pbVal;
			dwFT1 = DBGetContactSettingDword(NULL,gpszICQProtoName,"dwFT1",0);
			dwFT2 = DBGetContactSettingDword(NULL,gpszICQProtoName,"dwFT2",0);
			dwFT3 = DBGetContactSettingDword(NULL,gpszICQProtoName,"dwFT3",0);
			szClient = detectUserClient(NULL, 0, icq_ver(), dwFT1, dwFT2, dwFT3, 0, 0, 0, 0, buf, bufsize, &bClient, szStrBuf);
			if (!szClient||szClient<0)
				szClient = "Unknown";
			ICQWriteContactSettingUtf(NULL,   "MirVer",       szClient);
		}
	} 


  /* SNAC 1,11 */
  serverPacketInit(&packet, 14);
  packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_IDLE);
  packDWord(&packet, 0x00000000);

  sendServPacket(&packet);
  gbIdleAllow = 0;

  // Change status
  SetCurrentStatus(icqGoingOnlineStatus);

  // Finish Login sequence
  serverPacketInit(&packet, 98);
  packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_READY);
  packDWord(&packet, 0x00220001); // imitate ICQ 6 behaviour
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00010004);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00130004);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00020001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00030001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00150001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00040001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00060001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x00090001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x000A0001);
  packDWord(&packet, 0x0110157f);
  packDWord(&packet, 0x000B0001);
  packDWord(&packet, 0x0110157f);

  sendServPacket(&packet);

  NetLog_Server(" *** Yeehah, login sequence complete");

  // login sequence is complete enter logged-in mode
  info->bLoggedIn = 1;

  // enable auto info-update routine
  icq_EnableUserLookup(TRUE);

  if (!info->isMigrating)
  { /* Get Offline Messages Reqeust */
    serverPacketInit(&packet, 24);
    packFNACHeaderFull(&packet, ICQ_EXTENSIONS_FAMILY, ICQ_META_CLI_REQ, 0, 0x00020001);
    packDWord(&packet, 0x0001000a);    /* TLV */
    packLEWord(&packet, 8);            /* bytes remaining */
    packLEDWord(&packet, dwLocalUIN);
    packDWord(&packet, 0x3c000200);    /* get offline msgs */

    sendServPacket(&packet);

    // Update our information from the server
    sendOwnerInfoRequest();

    // Request info updates on all contacts
    icq_RescanInfoUpdate();

    // Start sending Keep-Alive packets
    StartKeepAlive(info);
  
    if (gbAvatarsEnabled)
    { // Send SNAC 1,4 - request avatar family 0x10 connection
      icq_requestnewfamily(ICQ_AVATAR_FAMILY, StartAvatarThread);

      pendingAvatarsStart = 1;
      NetLog_Server("Requesting Avatar family entry point.");
    }
  }
  info->isMigrating = 0;

  if (gbAimEnabled)
  {
    char** szMsg = MirandaStatusToAwayMsg(gnCurrentStatus);

    EnterCriticalSection(&modeMsgsMutex);
    if (szMsg && !bNoStatusReply)
      icq_sendSetAimAwayMsgServ(*szMsg);
    LeaveCriticalSection(&modeMsgsMutex);
  }
  CheckSelfRemove();
}
