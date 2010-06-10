// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright � 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright � 2001-2002 Jon Keating, Richard Hughes
// Copyright � 2002-2004 Martin �berg, Sam Kothari, Robert Rainwater
// Copyright � 2004-2009 Joe Kucera
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
// File name      : $URL: http://sss.chaoslab.ru:81/svn/icqjplus/branches/0.5_branch/families.h $
// Revision       : $Revision: 298 $
// Last change on : $Date: 2009-06-19 11:03:16 +0200 (Fri, 19 Jun 2009) $
// Last change by : $Author: persei $
//
// DESCRIPTION:
//
//  Declaration for handlers of Channel 2 SNAC Families
//
// -----------------------------------------------------------------------------

#ifndef __FAMILIES_H
#define __FAMILIES_H


struct snac_header 
{
  BOOL  bValid;
  WORD  wFamily;
  WORD  wSubtype;
  WORD  wFlags;
  DWORD dwRef;
  WORD  wVersion;
};


struct message_ack_params 
{
  BYTE bType;
  DWORD dwUin;
  DWORD dwMsgID1;
  DWORD dwMsgID2;
  directconnect *pDC;
  WORD wCookie;
  int msgType;
  BYTE bFlags;
};

#define MAT_SERVER_ADVANCED 0
#define MAT_DIRECT          1


/* handleMessageTypes(): mMsgFlags constants */
#define MTF_DIRECT          1
#define MTF_PLUGIN          2

struct UserInfoRecordItem 
{
  WORD wTLV;
  int dbType;
  char *szDbSetting;
};

/*---------* Functions *---------------*/

int getPluginTypeIdLen(int nTypeID);
void packPluginTypeId(icq_packet *packet, int nTypeID);
int unpackSessionDataItem(oscar_tlv_chain *pChain, WORD wItemType, BYTE **ppItemData, WORD *pwItemSize, BYTE *pbItemFlags);

#define BUL_ALLCONTACTS   0
#define BUL_VISIBLE       1
#define BUL_INVISIBLE     2
#define BUL_TEMPVISIBLE   4


#endif /* __FAMILIES_H */