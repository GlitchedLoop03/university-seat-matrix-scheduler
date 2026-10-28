#include "database.h"

#include <sqlite3.h>

Database::Database()
    : m_db(nullptr)
{
}

Database::~Database()
{
    if (m_db != nullptr) {
        sqlite3_close(m_db);
    }
}

bool Database::open(const QString &path)
{
    if (m_db != nullptr) {
        m_lastError = "Database connection is already open.";
        return false;
    }

    const QByteArray filePath = path.toUtf8();

    const int result = sqlite3_open(filePath.constData(), &m_db);

    if (result != SQLITE_OK) {
        m_lastError = m_db != nullptr
            ? QString::fromUtf8(sqlite3_errmsg(m_db))
            : QStringLiteral("Unable to allocate a SQLite connection.");

        if (m_db != nullptr) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }

        return false;
    }

    m_lastError.clear();
    return true;
}

bool Database::initializeSchema()
{
    if (m_db == nullptr) {
        m_lastError = "Cannot initialize schema: database is not open.";
        return false;
    }

    const char *sql = R"(
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
            capacity INTEGER NOT NULL CHECK (capacity > 0)
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
            FOREIGN KEY (examination_id)
                REFERENCES examinations(id) ON DELETE CASCADE,
            FOREIGN KEY (student_id)
                REFERENCES students(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS seat_allocations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            examination_id INTEGER NOT NULL,
            room_id INTEGER NOT NULL,
            student_id INTEGER NOT NULL,
            seat_number INTEGER NOT NULL CHECK (seat_number > 0),
            UNIQUE (examination_id, student_id),
            UNIQUE (examination_id, room_id, seat_number),
            FOREIGN KEY (examination_id)
                REFERENCES examinations(id) ON DELETE CASCADE,
            FOREIGN KEY (room_id)
                REFERENCES rooms(id) ON DELETE CASCADE,
            FOREIGN KEY (student_id)
                REFERENCES students(id) ON DELETE CASCADE
        );
    )";

    char *errorMessage = nullptr;

    const int result = sqlite3_exec(
        m_db, sql, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK) {
        m_lastError = errorMessage != nullptr
            ? QString::fromUtf8(errorMessage)
            : QString::fromUtf8(sqlite3_errmsg(m_db));

        sqlite3_free(errorMessage);
        return false;
    }

    m_lastError.clear();
    return true;
}

QString Database::lastError() const
{
    return m_lastError;
}

bool Database::isOpen() const
{
    return m_db != nullptr;
}

sqlite3 *Database::handle() const
{
    return m_db;
}
