/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009 Michal Zelinka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

#pragma once

// Version management
#define __VERSION_DWORD             PLUGIN_MAKE_VERSION(0, 0, 0, 1)
#define __VERSION_STRING            "0.0.0.1"
#define __VERSION_VS_FILE           0,0,0,1
#define __VERSION_VS_PROD           0,8,0,34
#define __VERSION_VS_FILE_STRING    "0, 0, 0, 1"
#define __VERSION_VS_PROD_STRING    "0, 8, 0, 34"

// Product management
#define FACEBOOK_NAME               "Facebook"
#define FACEBOOK_URL_HOMEPAGE       "http://www.facebook.com/"

// Connection
#define FACEBOOK_SERVER_REGULAR     "http://www.facebook.com/"
#define FACEBOOK_SERVER_CHAT        "http://%d.channel%d.facebook.com/"
#define FACEBOOK_SERVER_LOGIN       "https://login.facebook.com/"
#define FACEBOOK_SERVER_APPS        "http://apps.facebook.com/"

// User-Agents (stored only)
/*
Miranda/0.9
Lynx/2.8.4rel.1 libwww-FM/2.14
Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; SV1; .NET CLR 1.1.4322)
Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)
Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/4.0; MRA 5.3 (build 02552); SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0; InfoPath.2; .NET CLR 1.1.4322)
Mozilla/4.08 [en] (WinNT; U ;Nav)
Mozilla/5.0 (compatible; Konqueror/3.5; Linux 2.6.21-rc1; x86_64; cs, en_US) KHTML/3.5.6 (like Gecko)
Mozilla/5.0 (Windows; U; Windows NT 5.1; ru; rv:1.8.1.9) Gecko/20071025 Firefox/2.0.0.9
Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.2) Gecko/20070221 SeaMonkey/1.1.1
Opera/8.01 (J2ME/MIDP; Opera Mini/3.0.6306/1528; nb; U; ssr)
Opera/9.27 (Windows NT 5.1; U; en)
Opera/9.54 (Windows NT 5.1; U; en)
Opera/9.80 (Windows NT 5.1; U; en) Presto/2.2.15 Version/10.00
*/

// Input constants
#define FACEBOOK_MESSAGE_LIMIT      1024
#define FACEBOOK_MESSAGE_LIMIT_TEXT "1024"
#define FACEBOOK_MIND_LIMIT         420
#define FACEBOOK_MIND_LIMIT_TEXT    "420"

// Defaults
#define FACEBOOK_DEFAULT_POLL_RATE              8 // in seconds

// Facebook request types // TODO: Convert to MS_ and release in FB plugin SDK?
#define FACEBOOK_REQUEST_LOGIN                  100 // connecting physically
#define FACEBOOK_REQUEST_POPOUT                 110 // getting __post_form_id__
#define FACEBOOK_REQUEST_UPDATE                 120 // regular updates (friends online, ...)
#define FACEBOOK_REQUEST_RECONNECT              130 // getting __sequence_num__ and __channel_id__
#define FACEBOOK_REQUEST_SETTINGS               140 // looks like nothing special...
#define FACEBOOK_REQUEST_STATUS_GET             200 // getting others' statuses (TODO: Obsolete?
                                                    // Isn't this distributed via UPDATE?)
#define FACEBOOK_REQUEST_STATUS_SET             201 // setting my "What's on my mind?"
#define FACEBOOK_REQUEST_MESSAGE_SEND           300 // sending message
#define FACEBOOK_REQUEST_MESSAGES_RECEIVE       301 // receiving messages
#define FACEBOOK_REQUEST_NOTIFICATIONS_RECEIVE  401 // receiving notifications

#define WM_SETREPLY   WM_APP+10
