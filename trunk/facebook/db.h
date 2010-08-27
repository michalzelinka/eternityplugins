/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-10 Michal Zelinka

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

File name      : $HeadURL$
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
#define getString( setting, dest )          DBGetContactSettingString( NULL, m_szModuleName, setting, dest )
#define setString( setting, value )         DBWriteContactSettingString( NULL, m_szModuleName, setting, value )
#define getTString( setting, dest )         DBGetContactSettingTString( NULL, m_szModuleName, setting, dest )
#define setTString( setting, value )        DBWriteContactSettingTString( NULL, m_szModuleName, setting, value )
#define getU8String( setting, dest )        DBGetContactSettingUTF8String( NULL, m_szModuleName, setting, dest )
#define setU8String( setting, value )       DBWriteContactSettingUTF8String( NULL, m_szModuleName, setting, value )
#define deleteSetting( setting )            DBDeleteContactSetting( NULL, m_szModuleName, setting )

// DB keys
#define FACEBOOK_KEY_LOGIN          "Email"
#define FACEBOOK_KEY_ID             "ID"
#define FACEBOOK_KEY_NAME           "RealName"
#define FACEBOOK_KEY_UPD_NAMES      "UpdateNames"
#define FACEBOOK_KEY_PASS           "Password"
#define FACEBOOK_KEY_DEF_GROUP      "DefaultGroup"
#define FACEBOOK_KEY_AV_URL         "AvatarURL"
#define FACEBOOK_KEY_POLL_RATE      "PollRate"
#define FACEBOOK_KEY_USER_AGENT     "UserAgent"
#define FACEBOOK_KEY_LOGGING_ENABLE "LoggingEnable"
#define FACEBOOK_KEY_SET_MIRANDA_STATUS "SetMirandaStatus"
#define FACEBOOK_KEY_TIMEOUTS_LIMIT "TimeoutsLimit" // [HIDDEN]
#define FACEBOOK_KEY_DISABLE_LOGOUT "DisableLogout" // [HIDDEN]
#define FACEBOOK_KEY_DISABLE_STATUS_NOTIFY  "DisableStatusNotify" // [HIDDEN]

#define FACEBOOK_KEY_NOTIFICATIONS_ENABLE   "NotificationsEnable"
#define FACEBOOK_KEY_FEEDS_ENABLE           "FeedsEnable"
#define FACEBOOK_KEY_EVENTS_COLBACK         "EventsColorBack"
#define FACEBOOK_KEY_EVENTS_COLTEXT         "EventsColorText"
#define FACEBOOK_KEY_EVENTS_TIMEOUT_TYPE    "EventsTimeoutType"
#define FACEBOOK_KEY_EVENTS_TIMEOUT         "EventsTimeout"
