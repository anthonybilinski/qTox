/*
    Copyright © 2015-2018 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef HISTORY_H
#define HISTORY_H

#include "src/core/contactid.h"
#include "src/core/toxpk.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/widget/searchtypes.h"

#include <tox/toxencryptsave.h>

#include <QDateTime>
#include <QHash>
#include <QVector>

#include <cstdint>
#include <memory>

class Profile;
class HistoryKeeper;

class History
{
public:
    struct HistMessage
    {
        HistMessage(qint64 id, bool isSent, QDateTime timestamp, QString chat, QString dispName,
                    QString sender, QString message)
            : chat{chat}
            , sender{sender}
            , message{message}
            , dispName{dispName}
            , timestamp{timestamp}
            , id{id}
            , isSent{isSent}
        {
        }

        QString chat;
        QString sender;
        QString message;
        QString dispName;
        QDateTime timestamp;
        qint64 id;
        bool isSent;
    };

    struct DateMessages
    {
        uint offsetDays;
        uint count;
    };

public:
    explicit History(std::shared_ptr<RawDatabase> db);
    ~History();

    bool isValid();

    bool isHistoryExistence(ContactIdPtr contactId);

    void eraseHistory();
    void removeContactHistory(ContactIdPtr contactId);
    void removeGroupHistory(ContactIdPtr groupPersistentId);
    void addNewMessage(ContactIdPtr contactId, const QString& message, ContactIdPtr sender,
                       const QDateTime& time, bool isSent, QString dispName,
                       const std::function<void(int64_t)>& insertIdCallback = {});

    QList<HistMessage> getChatHistoryFromDate(ContactIdPtr contactId, const QDateTime& from,
                                              const QDateTime& to);
    QList<HistMessage> getChatHistoryDefaultNum(ContactIdPtr contactId);
    QList<DateMessages> getChatHistoryCounts(ContactIdPtr contactId, const QDate& from, const QDate& to);
    QDateTime getDateWhereFindPhrase(ContactIdPtr contactId, const QDateTime& from, QString phrase,
                                     const ParameterSearch& parameter);
    QDateTime getStartDateChatHistory(ContactIdPtr contactId);

    void markAsSent(qint64 messageId);

protected:
    QVector<RawDatabase::Query>
    generateNewMessageQueries(ContactIdPtr contactId, const QString& message,
                              ContactIdPtr sender, const QDateTime& time, bool isSent,
                              QString dispName, std::function<void(int64_t)> insertIdCallback = {});

private:
    QList<HistMessage> getChatHistory(ContactIdPtr contactId, const QDateTime& from,
                                      const QDateTime& to, int numMessages);
    std::shared_ptr<RawDatabase> db;
    QHash<ContactIdPtr, int64_t> peers;
};

#endif // HISTORY_H
