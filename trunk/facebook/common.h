/*
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
*/

#pragma once

#define MIRANDA_VER    0x0800
#define _WIN32_WINNT   0x0500
#define _WIN32_WINDOWS 0x0500

#include <map>
#include <list>
#include <string>
#include <iostream>
using namespace std;

#include <windows.h>

#pragma warning(push)
#	pragma warning(disable:4312)
#	pragma warning(disable:4996)
#include <newpluginapi.h>
#include <win2k.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
//#include "m_cluiframes.h"
#include <m_database.h>
#include <m_history.h>
#include <m_idle.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <statusmodes.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_xml.h>
#include <m_hotkeys.h>
#include <m_updater.h>

#include "dialogs.h"
#include "http.h"
#include "proto.h"
#include "restmethod.h"
#include "signature.h"
#include "theme.h"
#include "utility.h"
#include "version.h"
#include "resource.h"

#pragma warning(pop)

extern HINSTANCE g_hInstance;

// DB keys
#define FACEBOOK_KEY_UN             "Email"
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

#define FACEBOOK_DB_EVENT_TYPE_MIND 2718

// Facebook request types
#define FACEBOOK_REQUEST_STATUS_GET 1000
#define FACEBOOK_REQUEST_STATUS_SET 1001

#define WM_SETREPLY   WM_APP+10