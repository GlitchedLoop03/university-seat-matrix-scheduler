#include "roomrepository.h"
#include <sqlite3.h>

RoomRepository::RoomRepository(sqlite3 *database) : m_db(database)
{
    if (!m_db) m_lastError = "Database connection is not available.";
}

bool RoomRepository::addRoom(const Room &room)
{
    if (!m_db) { m_lastError = "Database connection is not available."; return false; }
    const QString number = room.roomNumber.trimmed();
    if (number.isEmpty() || room.rows <= 0 || room.columns <= 0 || room.capacity() > 100000) {
        m_lastError = "Enter a room number and valid dimensions (maximum 100000 seats)."; return false;
    }
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "INSERT INTO rooms (room_number, capacity, rows, columns) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db)); return false;
    }
    const QByteArray name = number.toUtf8();
    sqlite3_bind_text(stmt, 1, name.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, room.capacity());
    sqlite3_bind_int(stmt, 3, room.rows);
    sqlite3_bind_int(stmt, 4, room.columns);
    const int rc = sqlite3_step(stmt);
    m_lastError = rc == SQLITE_DONE ? QString() : QString::fromUtf8(sqlite3_errmsg(m_db));
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool RoomRepository::updateRoom(const Room &room)
{
    if (!m_db) { m_lastError = "Database connection is not available."; return false; }
    const QString number = room.roomNumber.trimmed();
    if (room.id <= 0 || number.isEmpty() || room.rows <= 0 || room.columns <= 0 || room.capacity() > 100000) {
        m_lastError = "Enter a valid room number and dimensions (maximum 100000 seats)."; return false;
    }
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "UPDATE rooms SET room_number = ?, capacity = ?, rows = ?, columns = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db)); return false;
    }
    const QByteArray name = number.toUtf8();
    sqlite3_bind_text(stmt, 1, name.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, room.capacity());
    sqlite3_bind_int(stmt, 3, room.rows);
    sqlite3_bind_int(stmt, 4, room.columns);
    sqlite3_bind_int(stmt, 5, room.id);
    const int rc = sqlite3_step(stmt), changes = sqlite3_changes(m_db);
    if (rc != SQLITE_DONE) m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else if (changes == 0) m_lastError = "The room record no longer exists.";
    else m_lastError.clear();
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && m_lastError.isEmpty();
}

bool RoomRepository::deleteRoom(int id)
{
    if (!m_db || id <= 0) { m_lastError = "A valid room is required."; return false; }
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, "DELETE FROM rooms WHERE id = ?;", -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db)); return false;
    }
    sqlite3_bind_int(stmt, 1, id);
    const int rc = sqlite3_step(stmt), changes = sqlite3_changes(m_db);
    if (rc != SQLITE_DONE) m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else if (changes == 0) m_lastError = "The room record no longer exists.";
    else m_lastError.clear();
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && changes > 0;
}

QVector<Room> RoomRepository::getAllRooms()
{
    QVector<Room> rooms;
    if (!m_db) { m_lastError = "Database connection is not available."; return rooms; }
    sqlite3_stmt *stmt = nullptr;
    const char *sql = "SELECT id, room_number, rows, columns FROM rooms ORDER BY room_number COLLATE NOCASE;";
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db)); return rooms;
    }
    int rc = SQLITE_OK;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Room room;
        room.id = sqlite3_column_int(stmt, 0);
        const auto *raw = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        room.roomNumber = QString::fromUtf8(raw ? raw : "");
        room.rows = sqlite3_column_int(stmt, 2);
        room.columns = sqlite3_column_int(stmt, 3);
        rooms.append(room);
    }
    if (rc != SQLITE_DONE) m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else m_lastError.clear();
    sqlite3_finalize(stmt);
    return rooms;
}

QString RoomRepository::lastError() const { return m_lastError; }
