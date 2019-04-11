#include "src/grouplist.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/settings.h"
#include "src/widget/contentdialog.h"

#include <QCollator>

namespace {

QString getShortName(const QString& name)
{
    constexpr auto MAX_NAME_LENGTH = 30;
    if (name.length() <= MAX_NAME_LENGTH) {
        return name;
    }

    return name.left(MAX_NAME_LENGTH).trimmed() + "…";
}

}

FriendChatroom::FriendChatroom(Friend* frnd)
    : frnd{frnd}
{
}

Friend* FriendChatroom::getFriend()
{
    return frnd;
}

Contact* FriendChatroom::getContact()
{
    return frnd;
}

void FriendChatroom::setActive(bool _active)
{
    if (active != _active) {
        active = _active;
        emit activeChanged(active);
    }
}

bool FriendChatroom::canBeInvited() const
{
    return frnd->isOnline();
}

int FriendChatroom::getCircleId() const
{
    return Settings::getInstance().getFriendCircleID(frnd->getPublicKey());
}

QString FriendChatroom::getCircleName() const
{
    const auto circleId = getCircleId();
    return Settings::getInstance().getCircleName(circleId);
}

void FriendChatroom::inviteToNewGroup()
{
    auto core = Core::getInstance();
    const auto friendId = frnd->getId();
    const auto groupId = core->createGroup();
    core->groupInviteFriend(friendId, groupId);
}

QString FriendChatroom::getAutoAcceptDir() const
{
    const auto pk = frnd->getPublicKey();
    return Settings::getInstance().getAutoAcceptDir(pk);
}

void FriendChatroom::setAutoAcceptDir(const QString& dir)
{
    const auto pk = frnd->getPublicKey();
    Settings::getInstance().setAutoAcceptDir(pk, dir);
}

void FriendChatroom::disableAutoAccept()
{
    setAutoAcceptDir(QString{});
}

bool FriendChatroom::autoAcceptEnabled() const
{
    return getAutoAcceptDir().isEmpty();
}

bool FriendChatroom::getBlocked() const
{
    const auto pk = frnd->getPublicKey();
    return Settings::getInstance().getFriendBlocked(pk);
}

void FriendChatroom::setBlocked(const bool blocked)
{
    const auto pk = frnd->getPublicKey();
    auto core = Core::getInstance();
    const auto friendId = frnd->getId();
    if (blocked) {
        core->removeFriend(friendId);
    } else {
        core->acceptFriendRequest(pk);
    }
    Settings::getInstance().setFriendBlocked(pk, blocked);
}

void FriendChatroom::inviteFriend(const Group* group)
{
    const auto friendId = frnd->getId();
    const auto groupId = group->getId();
    Core::getInstance()->groupInviteFriend(friendId, groupId);
}

QVector<GroupToDisplay> FriendChatroom::getGroups() const
{
    QVector<GroupToDisplay> groups;
    for (const auto group : GroupList::getAllGroups()) {
        const auto name = getShortName(group->getName());
        const GroupToDisplay groupToDisplay = { name, group };
        groups.push_back(groupToDisplay);
    }

    return groups;
}

/**
 * @brief Return sorted list of circles exclude current circle.
 */
QVector<CircleToDisplay> FriendChatroom::getOtherCircles() const
{
    QVector<CircleToDisplay> circles;
    const auto currentCircleId = getCircleId();
    const auto& s = Settings::getInstance();
    for (int i = 0; i < s.getCircleCount(); ++i) {
        if (i == currentCircleId) {
            continue;
        }

        const auto name = getShortName(s.getCircleName(i));
        const CircleToDisplay circle = { name, i };
        circles.push_back(circle);
    }

    std::sort(circles.begin(), circles.end(),
              [](const CircleToDisplay& a, const CircleToDisplay& b) -> bool {
                  QCollator collator;
                  collator.setNumericMode(true);
                  return collator.compare(a.name, b.name) < 0;
              });

    return circles;
}

void FriendChatroom::resetEventFlags()
{
    frnd->setEventFlag(false);
}
