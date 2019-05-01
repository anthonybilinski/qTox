/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "src/persistence/db/rawdatabase.h"
// normally we should only test public API instead of implementation,  but there's no reason to expose db schema
// upgrade externally, and the complexity of each version upgrade benefits from being individually testable
#include "src/persistence/history.cpp"

#include <QtTest/QtTest>
#include <QString>

#include <memory>

class TestDbSchema : public QObject
{
    friend class History;
    Q_OBJECT
private slots:
    void test1to2();
private:
    std::shared_ptr<RawDatabase> create1to2TestDb();
};

void TestDbSchema::test1to2()
{
    auto rawdb = create1to2TestDb();
    dbSchema1to2(rawdb);
}

std::shared_ptr<RawDatabase> TestDbSchema::create1to2TestDb()
{
    auto db = std::shared_ptr<RawDatabase>{new RawDatabase{"test1to2.db", {}, {}}};
    // set up naughty tables
    return db;
}

QTEST_GUILESS_MAIN(TestDbSchema)
#include "dbschema_test.moc"
