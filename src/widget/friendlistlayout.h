/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef FRIENDLISTLAYOUT_H
#define FRIENDLISTLAYOUT_H

#include "genericchatitemlayout.h"
#include "src/model/status.h"
#include "src/core/core.h"
#include <QBoxLayout>

class FriendWidget;
class FriendListWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout();
    explicit FriendListLayout(QWidget* parent);

    void addFriendWidget(FriendWidget* widget, Status::Status s);
    void removeFriendWidget(FriendWidget* widget, Status::Status s);
    int indexOfFriendWidget(GenericChatItemWidget* widget, Status::Status s) const;
    void moveFriendWidgets(FriendListWidget* listWidget);
    int friendOnlineCount() const;
    int friendTotalCount() const;

    bool hasChatrooms() const;
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false, bool hideBlocked = false);

    QLayout* getLayoutOnline() const;
    QLayout* getLayoutOffline() const;
    QLayout* getLayoutBlocked() const;
    QLayout* getFriendLayout(Status s) const;

private:
    void init();

    GenericChatItemLayout friendOnlineLayout;
    GenericChatItemLayout friendOfflineLayout;
    GenericChatItemLayout friendBlockedLayout;
};

#endif // FRIENDLISTLAYOUT_H
