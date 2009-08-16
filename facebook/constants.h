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

File name      : $URL$
Revision       : $Revision$
Last change by : $Author$
Last change on : $Date$

*/

#pragma once

// Version management
#define __VERSION_DWORD             PLUGIN_MAKE_VERSION(0, 0, 0, 2)
#define __PRODUCT_DWORD             PLUGIN_MAKE_VERSION(0, 8, 0, 34)
#define __VERSION_STRING            "0.0.0.2"
#define __PRODUCT_STRING            "0.8.0.34"
#define __VERSION_VS_FILE           0,0,0,2
#define __VERSION_VS_PROD           0,8,0,34
#define __VERSION_VS_FILE_STRING    "0, 0, 0, 2"
#define __VERSION_VS_PROD_STRING    "0, 8, 0, 34"

// Product management
#define FACEBOOK_NAME               "Facebook"
#define FACEBOOK_URL_HOMEPAGE       "http://www.facebook.com/"

// Connection
#define FACEBOOK_SERVER_REGULAR     "http://www.facebook.com/"
#define FACEBOOK_SERVER_CHAT        "http://%d.channel%d.facebook.com/"
#define FACEBOOK_SERVER_LOGIN       "https://login.facebook.com/"
#define FACEBOOK_SERVER_APPS        "http://apps.facebook.com/"

#define FACEBOOK_DEFAULT_AVATAR_URL "http://static.ak.fbcdn.net/pics/q_silhouette.gif"

// Input constants
#define FACEBOOK_MESSAGE_LIMIT      1024
#define FACEBOOK_MESSAGE_LIMIT_TEXT "1024"
#define FACEBOOK_MIND_LIMIT         420
#define FACEBOOK_MIND_LIMIT_TEXT    "420"

// Defaults
#define FACEBOOK_MINIMAL_POLL_RATE              5  // 10?
#define FACEBOOK_DEFAULT_POLL_RATE              15 // in seconds
#define FACEBOOK_MAXIMAL_POLL_RATE              60 // 45?

// Facebook request types // TODO: Provide MS_ and release in FB plugin API?
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

// User-Agents
static const char* user_agents[] = {
	"Miranda (default)",
	"Lynx/2.8.4rel.1 libwww-FM/2.14",
	"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; SV1; .NET CLR 1.1.4322)",
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.0)",
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/4.0; MRA 5.3 (build 02552); SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0; InfoPath.2; .NET CLR 1.1.4322)",
	"Mozilla/4.08 [en] (WinNT; U ;Nav)",
	"Mozilla/5.0 (compatible; Konqueror/3.5; Linux 2.6.21-rc1; x86_64; cs, en_US) KHTML/3.5.6 (like Gecko)",
	"Mozilla/5.0 (Windows; U; Windows NT 5.1; ru; rv:1.8.1.9) Gecko/20071025 Firefox/2.0.0.9",
	"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.2) Gecko/20070221 SeaMonkey/1.1.1",
	"Opera/8.01 (J2ME/MIDP; Opera Mini/3.0.6306/1528; nb; U; ssr)",
	"Opera/9.27 (Windows NT 5.1; U; en)",
	"Opera/9.54 (Windows NT 5.1; U; en)",
	"Opera/9.80 (Windows NT 5.1; U; en) Presto/2.2.15 Version/10.00",
	"Opera/9.80 (Macintosh; Intel Mac OS X; U; en) Presto/2.2.15 Version/10.00",
	"Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_7; en-us) AppleWebKit/525.28.3 (KHTML, like Gecko) Version/3.2.3 Safari/525.28.3",
	"Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_7; en-US) AppleWebKit/531.0 (KHTML, like Gecko) Chrome/3.0.183 Safari/531.0",
	"Mozilla/5.0 (iPod; U; CPU iPhone OS 2_2_1 like Mac OS X; en-us) AppleWebKit/525.18.1 (KHTML, like Gecko) Version/3.1.1 Mobile/5H11a Safari/525.20",
	"HTC-8900/1.2 Mozilla/4.0 (compatible; MSIE 6.0; Windows CE; IEMobile 7.6) UP.Link/6.3.0.0.0",
	"BlackBerry8320/4.3.1 Profile/MIDP-2.0 Configuration/CLDC-1.1",
	"Opera/9.60 (J2ME/MIDP; Opera Mini/4.2.13337/504; U; en) Presto/2.2.0",
	"Nokia6230/2.0+(04.43)+Profile/MIDP-2.0+Configuration/CLDC-1.1+UP.Link/6.3.0.0.0",
	"Mozilla/5.0 (webOS/1.0; U; en-US) AppleWebKit/525.27.1 (KHTML, like Gecko) Version/1.0 Safari/525.27.1 Pre/1.0 // palmOne",
};
