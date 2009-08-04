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

// Sending tZer in filled structure to contact.
//
// wParam=(WPARAM)&TZERTOSEND
// lParam=0
#define PS_ICQP_SEND_TZER "/SendTzer"

// Showing "Send tZer" dialog -- used when menu item is clicked
//
// wParam=(WPARAM)hContact
// lParam=0
#define PS_ICQP_SEND_TZER_DIALOG "/SendTzerDialog"

// Requesting an Incognito Status Message
//
// wParam=(WPARAM)hContact
// lParam=0
#define PS_ICQP_INCOGNITO_REQUEST "/IncognitoRequest"


#define PS_ISEE_SET_ALWAYS_VISIBLE      "/SetAlwaysVisible"
#define PS_ISEE_SET_NEVER_VISIBLE       "/SetNeverVisible"
#define PS_ISEE_SCAN_USER_STATUS        "/ScanUserStatus"
#define PS_ISEE_SCAN_USER_STATUS_GLOBAL "/ScanUserStatusGlobal"
#define PS_ISEE_ADVANCED_FEATURES       "/AdvFeatures"
#define PS_ICQP_XSTATUS_UPDATE_FOR_ALL_USERS    "/XStatusUpdateForAllUsers"


// Add a custom ICQ capability
//
// wParam = 0;
// lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
#define PS_ICQP_ADD_CAPABILITY         "/IcqAddCapability"


// Check if capability is supported (only icqCustomCap caps are)
//
// wParam = (WPARAM)(HANDLE)hContact;
// lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
// returns non-zero if capability is supported
#define PS_ICQP_CHECK_CAPABILITY       "/IcqCheckCapability"

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
