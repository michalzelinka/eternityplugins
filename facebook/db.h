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

// DB macros
#define getByte( setting, error )           DBGetContactSettingByte( NULL, m_szModuleName, setting, error )
#define setByte( setting, value )           DBWriteContactSettingByte( NULL, m_szModuleName, setting, value )
#define getWord( setting, error )           DBGetContactSettingWord( NULL, m_szModuleName, setting, error )
#define setWord( setting, value )           DBWriteContactSettingWord( NULL, m_szModuleName, setting, value )
#define getDword( setting, error )          DBGetContactSettingDword( NULL, m_szModuleName, setting, error )
#define setDword( setting, value )          DBWriteContactSettingDword( NULL, m_szModuleName, setting, value )
#define getTString( setting, dest )         DBGetContactSettingTString( NULL, m_szModuleName, setting, dest )
#define setTString( setting, value )        DBWriteContactSettingTString( NULL, m_szModuleName, setting, value )
#define deleteSetting( setting )            DBDeleteContactSetting( NULL, m_szModuleName, setting )

// DB keys
#define FACEBOOK_KEY_LOGIN          "Email"
#define FACEBOOK_KEY_ID             "ID"
#define FACEBOOK_KEY_NAME           "RealName"
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

#define FACEBOOK_KEY_POLL_RATE      "PollRate"

// DB event types
#define FACEBOOK_DB_EVENT_TYPE_MIND 2718
