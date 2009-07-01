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
#define FACEBOOK_SERVER_LOGIN       "https://login.facebook.com/"

// DB keys
#define FACEBOOK_KEY_ID             "Email"
#define FACEBOOK_KEY_PASS           "Password"

#define FACEBOOK_KEY_POPUP_SHOW     "Popup/Show"
#define FACEBOOK_KEY_POPUP_SIGNON   "Popup/Signon"
#define FACEBOOK_KEY_POPUP_COLBACK  "Popup/ColorBack"
#define FACEBOOK_KEY_POPUP_COLTEXT  "Popup/ColorText"
#define FACEBOOK_KEY_POPUP_TIMEOUT  "Popup/Timeout"

#define FACEBOOK_KEY_SINCEID        "SinceID"
#define FACEBOOK_KEY_DMSINCEID      "DMSinceID"
#define FACEBOOK_KEY_NEW            "NewAcc"

#define FACEBOOK_KEY_AV_URL         "AvatarURL"

// Input constants
#define FACEBOOK_MIND_LIMIT         420
#define FACEBOOK_MIND_LIMIT_TEXT    "420"

// DB event types
#define FACEBOOK_DB_EVENT_TYPE_MIND 2718

// Facebook request types // TODO: Convert to MS_ and release in FB plugin SDK?
#define FACEBOOK_REQUEST_LOGIN                  100
#define FACEBOOK_REQUEST_STATUS_GET             200
#define FACEBOOK_REQUEST_STATUS_SET             201
#define FACEBOOK_REQUEST_POST_FORM_ID_GET       250
#define FACEBOOK_REQUEST_MESSAGE_SEND           300
#define FACEBOOK_REQUEST_MESSAGES_RECEIVE       301
#define FACEBOOK_REQUEST_NOTIFICATIONS_RECEIVE  401

#define WM_SETREPLY   WM_APP+10
