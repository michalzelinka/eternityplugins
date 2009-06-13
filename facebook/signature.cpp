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

#include <map>
#include <list>
#include <string>
#include <iostream>
using namespace std;
#include "signature.h"

/*!
    \fn FacebookSignatureUtil::convert(QHash<QString,QByteArray>)
 */
//list< string > SignatureUtil::convert( map< string, string > params )
//{
//    /// Convert hashtable into stringlist (format of each entry: key=value)
//    list< string > result;
//	for ( int i = 0; i < params.size; i++ )
//        result.insert( params[i] + "=" + params[i] );
//
//    return result;
//}


/*!
    \fn FacebookSignatureUtil::generateSignature(QStringList params, QString secret)
 */
string SignatureUtil::generateSignature( list< string > params, string secret )
{
	string signature = "";

    params.sort( );

	for ( list< string >::iterator it = params.begin( ); it != params.end( ); it++ )
		signature += *it;

	signature += secret;

    BYTE* input = new BYTE[signature.length( ) + 1];
	BYTE output[16] = { 0 };
	strcpy( ( char* )input, signature.c_str( ) );

	MessageBoxA( NULL, signature.c_str( ), "", MB_OK );

	md5i.md5_hash( ( BYTE* )signature.c_str( ), signature.length( ), output );

	char output2[255];

	signature = ( char* )output;

	MessageBoxA( NULL, signature.c_str( ), "", MB_OK );
    //return QCryptographicHash::hash(params.join("").append(secret).toUtf8(), QCryptographicHash::Md5).toHex();

	return signature;
}
