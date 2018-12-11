#include <QByteArray>
#include <QString>
#include <cstdint>
#include <QHash>
#include "src/core/contactid.h"

/**
 * @brief The default constructor. Creates an empty id.
 */
ContactId::ContactId()
    : id()
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              TOX_PUBLIC_KEY_SIZE, else the ToxPk will be empty.
 */
ContactId::ContactId(const QByteArray& rawId)
{
    id = QByteArray(rawId);
}

/**
 * @brief Compares the equality of the ContactId.
 * @param other ContactId to compare.
 * @return True if both ContactId are equal, false otherwise.
 */
bool ContactId::operator==(const ContactId& other) const
{
    return id == other.id;
}

/**
 * @brief Compares the inequality of the ToxPk.
 * @param other ToxPk to compare.
 * @return True if both ToxPks are not equal, false otherwise.
 */
bool ContactId::operator!=(const ContactId& other) const
{
    return id != other.id;
}

/**
 * @brief Compares two ToxPks
 * @param other ToxPk to compare.
 * @return True if this ToxPks is less than the other ToxPk, false otherwise.
 */
bool ContactId::operator<(const ContactId& other) const
{
    return id < other.id;
}

/**
 * @brief Converts the ToxPk to a uppercase hex string.
 * @return QString containing the hex representation of the key
 */
QString ContactId::toString() const
{
    return id.toHex().toUpper();
}

/**
 * @brief Returns a pointer to the raw key data.
 * @return Pointer to the raw key data, which is exactly `ToxPk::getPkSize()`
 *         bytes long. Returns a nullptr if the ToxPk is empty.
 */
const uint8_t* ContactId::getData() const
{
    if (id.isEmpty()) {
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(id.constData());
}

/**
 * @brief Get a copy of the key
 * @return Copied key bytes
 */
QByteArray ContactId::getByteArray() const
{
    return QByteArray(id); // TODO: Is a copy really necessary?
}

/**
 * @brief Checks if the ToxPk contains a key.
 * @return True if there is a key, False otherwise.
 */
bool ContactId::isEmpty() const
{
    return id.isEmpty();
}

