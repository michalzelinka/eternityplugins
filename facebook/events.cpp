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

#include "common.h"

int FacebookProto::Test( WPARAM wparam, LPARAM lparam )
{
	facy.notifications( );
	return FALSE;
}

int FacebookProto::Log(const char *fmt,...)
{
	if ( getByte( FACEBOOK_KEY_ENABLE_LOGGING, 0 ) != TRUE )
		return EXIT_SUCCESS;

	va_list va;
	char text[65535];
	ScopedLock s(log_lock_);

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	return utils::debug::log( m_szModuleName, text );
}

int FacebookProto::ShowNotification(TCHAR* title, TCHAR* info, DWORD flags)
{
	if ( !getByte( FACEBOOK_KEY_NOTIFICATIONS_ENABLE, DEFAULT_NOTIFICATIONS_ENABLE ) )
		return 0;

	int ret; BYTE timeout;

	switch ( getByte( FACEBOOK_KEY_NOTIFICATIONS_TIMEOUT_TYPE, DEFAULT_NOTIFICATIONS_TIMEOUT_TYPE ) ) {
		case 0: timeout = 0; break;
		case 1: timeout = getByte( FACEBOOK_KEY_NOTIFICATIONS_TIMEOUT, DEFAULT_NOTIFICATIONS_TIMEOUT ); break;
		case 2: timeout = -1; }

	if (ServiceExists(MS_POPUP_ADDPOPUP) && DBGetContactSettingByte(NULL,"PopUp","ModuleIsEnabled",1)) {
		POPUPDATAT pd;
		pd.colorBack = getByte( FACEBOOK_KEY_NOTIFICATIONS_COLBACK, DEFAULT_NOTIFICATIONS_COLBACK );
		pd.colorText = getByte( FACEBOOK_KEY_NOTIFICATIONS_COLTEXT, DEFAULT_NOTIFICATIONS_COLTEXT );
		pd.iSeconds = timeout;
		pd.lchContact = NULL;
		pd.lchIcon = NULL;
		pd.PluginData = NULL;
		pd.PluginWindowProc = NULL;
		lstrcpy(pd.lptzContactName, title);
		lstrcpy(pd.lptzText, info);
		ret = PUAddPopUpT(&pd);

		if (ret == 0)
			return 1;
	}

	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY err;
		err.szProto = m_szModuleName;
		err.cbSize = sizeof(err);
		err.dwInfoFlags = NIIF_INTERN_TCHAR | flags;
		err.tszInfoTitle = title;
		err.tszInfo = info;
		err.uTimeout = 1000 * timeout;
		ret = CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & err);

		if (ret == 0)
			return 1;
	} 

//	MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);
	return 0;
}
