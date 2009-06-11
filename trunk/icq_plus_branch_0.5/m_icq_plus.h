// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2008 jarvis
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
// DESCRIPTION:
//
//  ICQ Plus specific shared API stuff
//
// -----------------------------------------------------------------------------

#ifndef M_ICQ_PLUS_H__
#define M_ICQ_PLUS_H__ 1

//Sending tZer in filled structure to contact.
//
//wParam=(WPARAM)&TZERTOSEND
//lParam=0
#define MS_ISEE_SEND_TZER "/SendTzer"

//Showing "Send tZer" dialog -- used when menu item is clicked
//
//wParam=(WPARAM)hContact
//lParam=0
#define MS_ISEE_SEND_TZER_DIALOG "/SendTzerDialog"

//Requesting an Incognito Status Message
//
//wParam=(WPARAM)hContact
//lParam=0
#define MS_ISEE_INCOGNITO_REQUEST "/IncognitoRequest"

#define MS_ISEE_SET_ALWAYS_VISIBLE      "/SetAlwaysVisible"
#define MS_ISEE_SET_NEVER_VISIBLE       "/SetNeverVisible"
#define MS_ISEE_SCAN_USER_STATUS        "/ScanUserStatus"
#define MS_ISEE_SCAN_USER_STATUS_GLOBAL "/ScanUserStatusGlobal"
#define MS_ISEE_ADVANCED_FEATURES       "/AdvFeatures"
#define MS_XSTATUS_UPDATE_FOR_ALL_USERS "/XStatusUpdateForAllUsers"

#define PS_ICQ_ADD_CAPABILITY "/IcqAddCapability"

//

#define ICQEVENTTYPE_AUTHGRANTED       2004 // database event type
#define ICQEVENTTYPE_AUTHDENIED        2005 // database event type
#define ICQEVENTTYPE_AUTHREQUESTED     2006 // database event type
#define ICQEVENTTYPE_SELFREMOVE        2007 // database event type
#define ICQEVENTTYPE_FUTUREAUTH        2008 // database event type
#define ICQEVENTTYPE_CLIENTCHANGE      2009 // database event type
#define ICQEVENTTYPE_CHECKSTATUS       2010 // database event type
#define ICQEVENTTYPE_IGNORECHECKSTATUS 2011 // database event type
#define ICQEVENTTYPE_YOUWEREADDED      2012 // database event type
#define ICQEVENTTYPE_WASFOUND          2013 // database event type

#define MAX_CAPNAME 64
typedef struct
{
	int cbSize;
	char caps[0x10];
	HANDLE hIcon;
	char name[MAX_CAPNAME];
} ICQ_CUSTOMCAP;

#endif /* M_ICQ_PLUS_H__ */
