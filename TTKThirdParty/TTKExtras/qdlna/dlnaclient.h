#ifndef DLNACLIENT_H
#define DLNACLIENT_H

/* =================================================
 * This file is part of the TTK Music Player project
 * Copyright (C) 2015 - 2019 Greedysky Studio

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 ================================================= */

#include <QMap>
#include "dlnaxml.h"
#include "musicextrasglobaldefine.h"

class DlnaXml;

/*! @brief The class of the dlna client.
 * @author Greedysky <greedysky@163.com>
 */
class MUSIC_EXTRAS_EXPORT DlnaClient
{
public:
    explicit DlnaClient(const QString &data);
    ~DlnaClient();

    QString server();
    QString serverName();

    bool connect();
    bool isConnected() const;

    QString tryToPlayFile(const QString &url);
    QString uploadFileToPlay(const QString &url);

    QString startPlay(int instance);
    QString stopPlay(int instance);
    QString pause(int instance);

    QString getPosition();
    int totalSeconds(const QString &value);

private:
    bool m_isConnected;

    DlnaXml *m_xml;
    QString m_serverIP, m_serverPort;
    QString m_smp, m_controlURL;
    QString m_friendlyName;
    QMap<QString, DlnaService> m_services;

};

#endif // DLNACLIENT_H
