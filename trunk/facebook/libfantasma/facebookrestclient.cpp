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
#define TARGET_API_VERSION "1.0"
#define ERROR_TAG "error_response"
#define FB_SERVER "api.facebook.com/restserver.php"
#define SERVER_ADDR QString("http://") + FB_SERVER
#define HTTPS_SERVER_ADDR QString("https://") + FB_SERVER
#define SERVER_URL QUrl(SERVER_ADDR)
#define HTTPS_SERVER_URL QUrl(HTTPS_SERVER_ADDR)

#include "facebookrestclient.h"
#include "facebookmethod.h"
#include "facebooksignatureutil.h"
#include "synchttp.h"

#include <QDateTime>
#include <QString>
#include <QByteArray>
#include <QHash>
#include <QDomDocument>
#include <QUrl>
#include <QDebug>

class FacebookRestClientPrivate {
public:
    FacebookRestClientPrivate() {}
    ~FacebookRestClientPrivate() {}
    
    QString secret;
    QString apiKey;
    QUrl serverUrl;

    QString sessionKey; // filled in when session is established
    bool isDesktop;
    QString sessionSecret; // only used for desktop apps
    int userId;
};

FacebookRestClient::FacebookRestClient(const QString& apiKey,
                                       const QString& secret,
                                       const QString& sessionKey,
                                       const QString& serverUrl) : d(new FacebookRestClientPrivate)
{
    /// Initialize important keys and urls
    d->apiKey = apiKey;
    d->secret = secret;
    d->sessionKey = sessionKey;
    d->serverUrl = (serverUrl.isEmpty()) ? SERVER_URL : QUrl(serverUrl);
}


FacebookRestClient::~FacebookRestClient()
{
    delete d;
}


bool FacebookRestClient::isDesktop() const
{
    return d->isDesktop;
}


void FacebookRestClient::setIsDesktop(bool isDesktop = true)
{
    d->isDesktop = isDesktop;
}


QDomDocument FacebookRestClient::callMethod(const FacebookMethod& method, QHash<QString, QString> params)
{
    /// Set up the initial parameters
    params.insert("method", method.methodName());
    params.insert("api_key", d->apiKey);
    params.insert("v", TARGET_API_VERSION);
    if (method.requiresSession()) {
      params.insert("call_id", QString::number(QDateTime::currentDateTime().toTime_t()));
      params.insert("session_key", d->sessionKey);
    }
    
    /// Append signature for request
    params.insert("sig", FacebookSignatureUtil::generateSignature( FacebookSignatureUtil::convert(params), (isDesktop() && method.requiresSession()) ? d->sessionSecret : d->secret));
    
    /// Post request
    bool doHttps = isDesktop() && FacebookMethod::AUTH_GET_SESSION == method;
    // qDebug() << (FacebookMethod::AUTH_GET_SESSION == method) << doHttps << "\n";
    
    /// Convert response into DOM
    QDomDocument doc;
    doc.setContent(method.takesFile() ? "" : postRequest(method.methodName().toUtf8(), params, doHttps, true));
    doc.normalize();
    
    /// @todo implement error catching
    
    return doc;
}


QString FacebookRestClient::postRequest(const QString& method, const QHash<QString, QString>& params, bool doHttps, bool doEncode)
{
    /// Create URL-encoded byte array from params
    QString buffer = (params.isEmpty()) ? "" : delimit(params, "&", "=", doEncode);
    qDebug() << buffer << "\n";
    QUrl serverUrl = (doHttps) ? HTTPS_SERVER_URL : d->serverUrl;

    /// Post request buffer
    SyncHttp conn(serverUrl.host());
    conn.syncPostURL(serverUrl, buffer.toUtf8());
    
    //qDebug() << conn.readAll().data() << "\n";
    
    //qDebug() << out.buffer();
    /// Return response
    return conn.readAll();
}


QString FacebookRestClient::delimit(const QHash<QString, QString>& params, const QString& delimiter, const QString& equals, bool doEncode)
{
    /// Convert params into delimited, URL-encoded byte array
    QString buffer;
    bool notFirst = false;
    QHash<QString, QString>::ConstIterator i;
    for (i = params.constBegin(); i != params.constEnd(); ++i) {
        if (notFirst) buffer.append(delimiter);
        else notFirst = true;
        buffer.append(i.key()).append(equals).append(doEncode ? QUrl::toPercentEncoding(i.value()) : i.value());
    }
    return buffer;
}

QString FacebookRestClient::delimit(const int* list) {
    QString buffer;
    bool notFirst = false;
    for (int i; i<sizeof(list); i++) {
        if (notFirst) buffer.append(",");
        else notFirst = true;
        buffer.append(list[i]);
    }
    return buffer;
}

/// facebook.auth.*

QString FacebookRestClient::auth_createToken()
{
    //return callMethod(FacebookMethod::AUTH_CREATE_TOKEN, QHash<QString, QString>()).elementsByTagName("auth_createToken_response").item(0).toElement().text();
    return callMethod(FacebookMethod::AUTH_CREATE_TOKEN, QHash<QString, QString>()).firstChildElement().text();
}


QString FacebookRestClient::auth_getSession(const QString& authToken)
{
    QHash<QString,QString> params;
    params.insert("auth_token", authToken);
    QDomDocument response = callMethod( FacebookMethod::AUTH_GET_SESSION, params);
    
    /// Read important tags from response
    d->sessionKey = response.elementsByTagName("session_key").item(0).toElement().text();
    d->userId = response.elementsByTagName("uid").item(0).toElement().text().toInt();
    if (isDesktop()) d->sessionSecret = response.elementsByTagName("secret").item(0).toElement().text();
    return d->sessionKey;
}

/// facebook.profile.*

QDomDocument FacebookRestClient::profile_getFBML(int userId)
{
    QHash<QString,QString> params;
    params.insert("uid", QString::number(userId));
    return callMethod(FacebookMethod::PROFILE_GET_FBML, params);
}

bool FacebookRestClient::profile_setFBML(const QString& fbmlMarkup, int userId)
{
    QHash<QString,QString> params;
    params.insert("uid", QString::number(userId));
    params.insert("markup", fbmlMarkup);
    return extractBool(callMethod(FacebookMethod::PROFILE_SET_FBML, params));
}

/// facebook.fql.*

QDomDocument FacebookRestClient::fql_query(const QString& query)
{
    QHash<QString,QString> params;
    params.insert("query", query);
    return callMethod(FacebookMethod::FQL_QUERY, params);
}

/// facebook.notification.*

QDomDocument FacebookRestClient::notifications_get()
{
    return callMethod(FacebookMethod::NOTIFICATIONS_GET, QHash<QString,QString>());
}

QUrl FacebookRestClient::notifications_sendRequest(const int* recipientIds, const QString& type, const QString& content, const QUrl& image, bool isInvite)
{
    QHash<QString,QString> params;
    params.insert("to_ids", delimit(recipientIds));
    params.insert("type", type);
    params.insert("content", content);
    params.insert("image", image.toString());
    params.insert("invite", isInvite ? "1" : "0");
    return QUrl(callMethod(FacebookMethod::NOTIFICATIONS_SEND_REQUEST, params).firstChildElement().text());
}

QUrl FacebookRestClient::notifications_send(const int* recipientIds, const QString& subject, const QString& body)
{
    QHash<QString,QString> params;
    params.insert("to_ids", delimit(recipientIds));
    params.insert("subject", subject);
    params.insert("body", body);
    return QUrl(callMethod(FacebookMethod::NOTIFICATIONS_SEND, params).firstChildElement().text());
}

/// facebook.fbml.*

bool FacebookRestClient::fbml_refreshRefUrl(const QUrl& url)
{
    QHash<QString,QString> params;
    params.insert("url", url.toString());
    return extractBool(callMethod(FacebookMethod::FBML_REFRESH_REF_URL, params));
}

bool FacebookRestClient::fbml_refreshImgSrc(const QUrl& url)
{
    QHash<QString,QString> params;
    params.insert("url", url.toString());
    return extractBool(callMethod(FacebookMethod::FBML_REFRESH_IMG_SRC, params));
}

/// facebook.feed.*

bool FacebookRestClient::feed_publishActionOfUser(const QString& title, const QString& body, const QHash<QUrl, QUrl>& images, int priority)
{
    return feedHandler(FacebookMethod::FEED_PUBLISH_ACTION_OF_USER, title, body, images, priority);
}

bool FacebookRestClient::feed_publishActionOfUser(const QString& title, const QString& body, int priority)
{
    QHash<QUrl, QUrl> images;
    return feed_publishActionOfUser(title, body, images, priority);
}

bool FacebookRestClient::feed_publishStoryToUser(const QString& title, const QString& body, const QHash<QUrl, QUrl>& images, int priority)
{
    return feedHandler(FacebookMethod::FEED_PUBLISH_STORY_TO_USER, title, body, images, priority);
}

bool FacebookRestClient::feed_publishStoryToUser(const QString& title, const QString& body, int priority)
{
    QHash<QUrl, QUrl> images;
    return feed_publishStoryToUser(title, body, images, priority);
}

bool FacebookRestClient::feedHandler(const FacebookMethod& method, const QString& title, const QString& body, const QHash<QUrl, QUrl>& images, int priority)
{
    QHash<QString, QString> params;
    params.insert("title", title);
    params.insert("body", body);
    if (priority!=0)
        params.insert("priority", QString::number(priority));
    if (!images.isEmpty()) {
        int count = 0;
        QHash<QUrl, QUrl>::ConstIterator i;
        for (i=images.constBegin(); i!=images.constEnd(); ++i) {
            ++count;
            params.insert(QString("image_%1").arg(count), i.key().toString());
            if (!i.value().isEmpty())
            params.insert(QString("image_%1_link").arg(count), i.value().toString());
        }
    }
    return extractBool(callMethod(method, params));
}

bool FacebookRestClient::extractBool(const QDomDocument& doc)
{
    return doc.firstChildElement().text() == "1";
}

/// Attempt at variable argument function
/*
QDomDocument FacebookRestClient::callMethod(const FacebookMethod& method, ...)
{
    va_list list;
    va_start(list, method);
    
    QHash<QString, QString> params;
    char* p1;
    char* p2;
    
    while (true) {
        p1 = va_arg(list, char*);
        p2 = va_arg(list, char*);
        if (p1 == "END" || p2 == "END")
            break;
        params.insert(p1, p2);
    }
    
    va_end(list);
    
    return callMethod(method, params);
}
*/
