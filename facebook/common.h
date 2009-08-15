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

#pragma warning( disable: 4996 )

#define MIRANDA_VER    0x0800
#define _WIN32_WINNT   0x0500
#define _WIN32_WINDOWS 0x0500

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include <time.h>
#include <cstring>
#include <ctime>
#include <assert.h>
using namespace std;

#include <windows.h>

//#pragma warning(push)
//#	pragma warning(disable:4312)
//#	pragma warning(disable:4996)
#include <newpluginapi.h>
#include <win2k.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h> // causes Boost/tinyJSON throwing marvellous errors -> tinyJSON must be included first
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
//#include <m_cluiframes.h>
#include <m_database.h>
#include <m_history.h>
#include <m_idle.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_protomod.h>
#include <m_skin.h>
#include <statusmodes.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_xml.h>
#include <m_hotkeys.h>
#include <m_updater.h>

#include "JSON_parser/JSON_parser.h"

#include "http.h"
#include "communication.h"
#include "utils.h"
#include "proto.h"
#include "json.h"
#include "db.h"
#include "constants.h"
#include "dialogs.h"
#include "theme.h"
#include "resource.h"

//#pragma warning(pop)

extern HINSTANCE g_hInstance;
