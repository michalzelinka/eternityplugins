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
#include "facebooksignatureutil.h"

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QHash>
#include <QCryptographicHash>

/*!
    \fn FacebookSignatureUtil::convert(QHash<QString,QByteArray>)
 */
QStringList FacebookSignatureUtil::convert(QHash<QString,QString> params)
{
    /// Convert hashtable into stringlist (format of each entry: key=value)
    QStringList result;
    QHash<QString,QString>::const_iterator i;
    for (i = params.begin(); i != params.end(); ++i) {
        result.append(i.key()+"="+i.value());
    }
    return result;
}


/*!
    \fn FacebookSignatureUtil::generateSignature(QStringList params, QString secret)
 */
QString FacebookSignatureUtil::generateSignature(QStringList params, QString secret)
{
    /// MD5 hash params
    params.sort();
    return QCryptographicHash::hash(params.join("").append(secret).toUtf8(), QCryptographicHash::Md5).toHex();
}
