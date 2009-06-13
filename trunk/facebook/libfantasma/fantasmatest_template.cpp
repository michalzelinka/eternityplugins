/***************************************************************************
 *   Copyright (C) 2007 by Jahshan Bhatti                                  *
 *   jabhatti91@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/// Copy this file to fantasmatest.cpp and place your keys here
#define API_KEY   "YOUR_API_KEY"
#define SECRET    "YOUR_SECRET"

#include "facebookrestclient.h"

#include <QString>
#include <QByteArray>
#include <QCoreApplication>

#include <stdio.h>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    FacebookRestClient client = FacebookRestClient(QString(API_KEY), QString(SECRET));
    client.setIsDesktop(true);
    QString auth = client.auth_createToken();
    printf("http://www.facebook.com/login.php?v=1.0&api_key=%s&auth_token=%s\n", API_KEY, auth.toUtf8().data());
    getchar();
    
    client.auth_getSession(auth);
    //org::w3c::dom::Node* d = new org::w3c::dom::Node(client->friends_get()->getJavaObject());
    //client->printDom(d, "  ");
    return 0;
}
