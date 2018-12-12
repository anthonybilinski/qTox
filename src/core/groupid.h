#ifndef GROUP_ID_H
#define GROUP_ID_H

#include "src/core/contactid.h"
#include <QByteArray>
#include <cstdint>

class GroupId : public ContactId
{
public:
    GroupId();
    ~GroupId() = default;
    GroupId(const GroupId& other);
    explicit GroupId(const QByteArray& rawId);
    explicit GroupId(const uint8_t* rawId);
    int getSize() const override;
};

#endif // GROUP_ID_H
