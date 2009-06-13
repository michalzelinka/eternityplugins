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
#ifndef FACEBOOKRESTCLIENT_H
#define FACEBOOKRESTCLIENT_H

#include <QHash>

class QString;
class QUrl;
class QByteArray;
class QDomDocument;

class FacebookRestClientPrivate;
class FacebookMethod;

/**
	@author jahshan
*/
class FacebookRestClient{
public:
    //QString & TARGET_API_VERSION;
    //QString & ERROR_TAG;
    //QString & FB_SERVER;
    //QString & SERVER_ADDR;
    //QString & HTTPS_SERVER_ADDR;
    //QUrl & SERVER_URL;
    //QUrl & HTTPS_SERVER_URL;
    static const int NUM_AUTOAPPENDED_PARAMS = 5;
    
    FacebookRestClient(const QString&, const QString&, const QString& =0, const QString& =0);

    ~FacebookRestClient();
    bool isDesktop() const;
    void setIsDesktop(bool isDesktop);
    /// facebook.auth.*
    QString auth_getSession(const QString&);
    QString auth_createToken();
    /// facebook.profile.*
    QDomDocument profile_getFBML(int=0);
    bool profile_setFBML(const QString&, int=0);
    /// facebook.fql.*
    QDomDocument fql_query(const QString&);
    /// facebook.notifications.*
    QDomDocument notifications_get();
    QUrl notifications_sendRequest(const int*, const QString&, const QString&, const QUrl&, bool);
    QUrl notifications_send(const int*, const QString&, const QString&);
    /// facebook.fbml.*
    bool fbml_refreshRefUrl(const QUrl&);
    bool fbml_refreshImgSrc(const QUrl&);
    /// facebook.feed.*
    bool feed_publishActionOfUser(const QString&, const QString&, const QHash<QUrl, QUrl>&, int=0);
    bool feed_publishActionOfUser(const QString&, const QString&, int=0);
    bool feed_publishStoryToUser(const QString&, const QString&, const QHash<QUrl, QUrl>&, int=0);
    bool feed_publishStoryToUser(const QString&, const QString&, int=0);
private:
    FacebookRestClientPrivate* const d;
protected:
    //QDomDocument callMethod(const FacebookMethod&, ...);
    QDomDocument callMethod(const FacebookMethod&, QHash<QString, QString>);
    bool feedHandler(const FacebookMethod&, const QString&, const QString&, const QHash<QUrl, QUrl>&, int);
    static QString delimit(const QHash<QString, QString>&, const QString&, const QString&, bool);
    static QString delimit(const int*);
    static bool extractBool(const QDomDocument&);
private:
    QString postRequest(const QString&, const QHash<QString, QString>&, bool, bool);
};

#endif
