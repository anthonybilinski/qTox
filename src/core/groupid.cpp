#include "groupid.h"

#include <tox/tox.h>

#include <QByteArray>
#include <QString>

/**
 * @class ToxPk
 * @brief This class represents a Tox Public Key, which is a part of Tox ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
GroupId::GroupId()
    : ContactId()
{
}

/**
 * @brief The copy constructor.
 * @param other ToxPk to copy
 */
GroupId::GroupId(const GroupId& other)
    : ContactId(other.id)
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              TOX_CONFERENCE_ID_SIZE, else the ToxPk will be empty.
 */
GroupId::GroupId(const QByteArray& rawId)
    : ContactId(rawId)
{
    assert(rawId.length() == TOX_CONFERENCE_ID_SIZE);
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * TOX_CONFERENCE_ID_SIZE from the specified buffer.
 */
GroupId::GroupId(const uint8_t* rawId)
    : ContactId(QByteArray(reinterpret_cast<const char*>(rawId), TOX_CONFERENCE_ID_SIZE))
{
}

/**
 * @brief Get size of public key in bytes.
 * @return Size of public key in bytes.
 */
int GroupId::getSize() const
{
    return TOX_CONFERENCE_ID_SIZE;
}
