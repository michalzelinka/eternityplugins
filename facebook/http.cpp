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

#include "http.h"

int FacebookProto::Test(WPARAM,LPARAM)
{
	this->m_szAccountId = "<<<user id goes here, on proto init>>>";
	PerformHttpRequest( FACEBOOK_REQUEST_STATUS_SET, "TEST_STATUS_VIA_MIRANDA_HTTP_REQUEST" );
	return FALSE;
}

BOOL FacebookProto::PerformHttpRequest( int requestType, char* requestData )
{
	int     httpMethod   = 0;
	char*   httpAction   = NULL;
	char*   httpPostData = NULL;
	char    szBuffer[1024] = {0};

	switch ( requestType )
	{

	case FACEBOOK_REQUEST_STATUS_SET:
		httpMethod = REQUEST_POST;
		httpAction = "/ajax/updatestatus.php";
		mir_snprintf( szBuffer, 1024, "<<<post data go here>>>" );
		httpPostData = szBuffer;
		break;

	}

	// Perform a HTTP request
	HttpGetPost( httpMethod, httpAction, httpPostData );

	return FALSE;
}

BOOL FacebookProto::HttpGetPost( int httpMethod, char* httpAction, char* httpPostData )
{
    char url[64];
    sprintf( url, "www.facebook.com%s", httpAction );

	NETLIBHTTPREQUEST nlhr = {0};

	nlhr.cbSize = sizeof( nlhr );
	nlhr.requestType = httpMethod;
	nlhr.szUrl = url;
	nlhr.flags = NLHRF_NODUMP | NLHRF_HTTP11;

	// Prepare headers
	NETLIBHTTPHEADER headers[10];
	headers[0].szName = "Host";
	headers[0].szValue = "www.facebook.com";
	headers[1].szName = "Accept";
	headers[1].szValue = "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5";
	headers[2].szName = "Accept-Language";
	headers[2].szValue = "en-us,en;q=0.5";
	headers[3].szName = "Accept-Charset";
	headers[3].szValue = "ISO-8859-1,utf-8;q=0.7,*;q=0.7";
	headers[4].szName = "X-SVN-Rev";
	headers[4].szValue = "171324";
	headers[5].szName = "Content-Type";
	headers[5].szValue = "application/x-www-form-urlencoded";
	headers[6].szName = "Referer";
	headers[6].szValue = "http://www.facebook.com/home.php?";
	headers[7].szName = "Cookie";
	headers[7].szValue = "<<<cookies go here>>>";
	headers[8].szName = "Pragma";
	headers[8].szValue = "no-cache";
	headers[9].szName = "Cache-Control";
	headers[9].szValue = "no-cache";

	nlhr.headers = headers;
	nlhr.headersCount = 10;

	nlhr.pData = httpPostData;
	nlhr.dataLength = strlen( nlhr.pData );

	MessageBoxA( NULL, httpPostData, url, MB_OK );

	NETLIBHTTPREQUEST* pnlhr = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&nlhr);

	MessageBoxA( NULL, pnlhr->pData, "", MB_OK );
	/*if(!pnlhr || (pnlhr->resultCode != 200) || (pnlhr->dataLength != 1))
	{
        MessageBoxA( NULL, "Error", "", MB_OK );
	}
	else
	{ // check if OK
		MessageBoxA( NULL, pnlhr->pData, "OK", MB_OK );
	}*/

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);

	return FALSE;
}
