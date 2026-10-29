#include "database.h"
#include <QStringList>
#include <sqlite3.h>

Database::Database() : m_db(nullptr) {}
Database::~Database() { if (m_db) sqlite3_close(m_db); }

bool Database::open(const QString &path)
{
    if (m_db) { m_lastError = "Database connection is already open."; return false; }
    const QByteArray filePath = path.toUtf8();
    const int result = sqlite3_open(filePath.constData(), &m_db);
    if (result != SQLITE_OK) {
        m_lastError = m_db ? QString::fromUtf8(sqlite3_errmsg(m_db))
                           : QStringLiteral("Unable to allocate a SQLite connection.");
        if (m_db) { sqlite3_close(m_db); m_db = nullptr; }
        return false;
    }
    m_lastError.clear();
    return true;
}

bool Database::initializeSchema()
{
    if (!m_db) { m_lastError = "Cannot initialize schema: database is not open."; return false; }
    const char *sql = R"SQL(
        PRAGMA foreign_keys = ON;
        CREATE TABLE IF NOT EXISTS students (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            registration_number TEXT NOT NULL UNIQUE,
            name TEXT NOT NULL,
            department TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS rooms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            room_number TEXT NOT NULL UNIQUE,
            capacity INTEGER NOT NULL CHECK (capacity > 0),
            rows INTEGER NOT NULL DEFAULT 1 CHECK (rows > 0),
            columns INTEGER NOT NULL DEFAULT 1 CHECK (columns > 0)
        );
        CREATE TABLE IF NOT EXISTS examinations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            subject TEXT NOT NULL,
            exam_date TEXT NOT NULL,
            exam_time TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS registrations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            examination_id INTEGER NOT NULL,
            student_id INTEGER NOT NULL,
            UNIQUE (examination_id, student_id),
            FOREIGN KEY (examination_id) REFERENCES examinations(id) ON DELETE CASCADE,
            FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE
        );
        CREATE TABLE IF NOT EXISTS seat_allocations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            examination_id INTEGER NOT NULL,
            room_id INTEGER NOT NULL,
            student_id INTEGER NOT NULL,
            seat_number INTEGER NOT NULL CHECK (seat_number > 0),
            UNIQUE (examination_id, student_id),
            UNIQUE (examination_id, room_id, seat_number),
            FOREIGN KEY (examination_id) REFERENCES examinations(id) ON DELETE CASCADE,
            FOREIGN KEY (room_id) REFERENCES rooms(id) ON DELETE CASCADE,
            FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE
        );
    )SQL";
    char *error = nullptr;
    if (sqlite3_exec(m_db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        m_lastError = error ? QString::fromUtf8(error) : QString::fromUtf8(sqlite3_errmsg(m_db));
        sqlite3_free(error);
        return false;
    }

    bool hasRows = false, hasColumns = false;
    sqlite3_stmt *info = nullptr;
    if (sqlite3_prepare_v2(m_db, "PRAGMA table_info(rooms);", -1, &info, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db)); return false;
    }
    while (sqlite3_step(info) == SQLITE_ROW) {
        const auto *raw = reinterpret_cast<const char *>(sqlite3_column_text(info, 1));
        const QString name = QString::fromUtf8(raw ? raw : "");
        if (name == "rows") hasRows = true;
        if (name == "columns") hasColumns = true;
    }
    sqlite3_finalize(info);
    if (!hasRows && sqlite3_exec(m_db,
        "ALTER TABLE rooms ADD COLUMN rows INTEGER NOT NULL DEFAULT 1 CHECK (rows > 0);",
        nullptr, nullptr, &error) != SQLITE_OK) {
        m_lastError = error ? QString::fromUtf8(error) : QString::fromUtf8(sqlite3_errmsg(m_db));
        sqlite3_free(error); return false;
    }
    if (!hasColumns) {
        if (sqlite3_exec(m_db,
            "ALTER TABLE rooms ADD COLUMN columns INTEGER NOT NULL DEFAULT 1 CHECK (columns > 0);",
            nullptr, nullptr, &error) != SQLITE_OK) {
            m_lastError = error ? QString::fromUtf8(error) : QString::fromUtf8(sqlite3_errmsg(m_db));
            sqlite3_free(error); return false;
        }
        if (sqlite3_exec(m_db, "UPDATE rooms SET columns = capacity;", nullptr, nullptr, &error) != SQLITE_OK) {
            m_lastError = error ? QString::fromUtf8(error) : QString::fromUtf8(sqlite3_errmsg(m_db));
            sqlite3_free(error); return false;
        }
        sqlite3_free(error);
    }
    m_lastError.clear();
    return true;
}

QString Database::lastError() const { return m_lastError; }
bool Database::isOpen() const { return m_db != nullptr; }
sqlite3 *Database::handle() const { return m_db; }

int Database::countRows(const QString &tableName) const
{
    if (!m_db) return -1;
    const QStringList allowedTables = {"students", "rooms", "examinations"};
    if (!allowedTables.contains(tableName)) return -1;
    const QString sql = "SELECT COUNT(*) FROM " + tableName + ";";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) != SQLITE_OK) return -1;
    int count = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}
