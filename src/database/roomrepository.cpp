#include "roomrepository.h"

#include <sqlite3.h>

RoomRepository::RoomRepository(sqlite3 *database)
    : m_db(database)
{
    if (!m_db) {
        m_lastError = "Database connection is not available.";
    }
}

bool RoomRepository::addRoom(const Room &room)
{
    if (!m_db) {
        m_lastError = "Database connection is not available.";
        return false;
    }

    const QString number = room.roomNumber.trimmed();

    if (number.isEmpty() || room.capacity <= 0) {
        m_lastError = "Enter a room number and a positive capacity.";
        return false;
    }

    const char *sql =
        "INSERT INTO rooms (room_number, capacity) VALUES (?, ?);";

    sqlite3_stmt *statement = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr)
        != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    const QByteArray roomNumber = number.toUtf8();

    sqlite3_bind_text(statement, 1, roomNumber.constData(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, room.capacity);

    const int result = sqlite3_step(statement);

    if (result != SQLITE_DONE) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    } else {
        m_lastError.clear();
    }

    sqlite3_finalize(statement);
    return result == SQLITE_DONE;
}

bool RoomRepository::updateRoom(const Room &room)
{
    if (!m_db) {
        m_lastError = "Database connection is not available.";
        return false;
    }

    const QString number = room.roomNumber.trimmed();

    if (room.id <= 0 || number.isEmpty() || room.capacity <= 0) {
        m_lastError = "Enter a valid room number and positive capacity.";
        return false;
    }

    const char *sql =
        "UPDATE rooms SET room_number = ?, capacity = ? WHERE id = ?;";

    sqlite3_stmt *statement = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr)
        != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    const QByteArray roomNumber = number.toUtf8();

    sqlite3_bind_text(statement, 1, roomNumber.constData(),
                      -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, room.capacity);
    sqlite3_bind_int(statement, 3, room.id);

    const int result = sqlite3_step(statement);

    if (result != SQLITE_DONE) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    } else {
        m_lastError.clear();
    }

    sqlite3_finalize(statement);
    return result == SQLITE_DONE;
}

bool RoomRepository::deleteRoom(int id)
{
    if (!m_db || id <= 0) {
        m_lastError = "A valid room is required.";
        return false;
    }

    const char *sql = "DELETE FROM rooms WHERE id = ?;";
    sqlite3_stmt *statement = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr)
        != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    sqlite3_bind_int(statement, 1, id);
    const int result = sqlite3_step(statement);
    const int changes = sqlite3_changes(m_db);

    if (result != SQLITE_DONE) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    } else if (changes == 0) {
        m_lastError = "The room record no longer exists.";
    } else {
        m_lastError.clear();
    }

    sqlite3_finalize(statement);
    return result == SQLITE_DONE && changes > 0;
}

QVector<Room> RoomRepository::getAllRooms()
{
    QVector<Room> rooms;

    if (!m_db) {
        m_lastError = "Database connection is not available.";
        return rooms;
    }

    const char *sql =
        "SELECT id, room_number, capacity "
        "FROM rooms ORDER BY room_number COLLATE NOCASE;";

    sqlite3_stmt *statement = nullptr;

    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr)
        != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return rooms;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        Room room;
        room.id = sqlite3_column_int(statement, 0);
        room.roomNumber = QString::fromUtf8(
            reinterpret_cast<const char *>(
                sqlite3_column_text(statement, 1)));
        room.capacity = sqlite3_column_int(statement, 2);
        rooms.append(room);
    }

    sqlite3_finalize(statement);
    m_lastError.clear();
    return rooms;
}

QString RoomRepository::lastError() const
{
    return m_lastError;
}
