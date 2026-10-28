#include "registrationrepository.h"

#include <sqlite3.h>

RegistrationRepository::RegistrationRepository(sqlite3 *database)
    : m_db(database)
{
    if (!m_db)
        m_lastError = "Database connection is not available.";
}

QVector<RegistrationOption> RegistrationRepository::getExaminations()
{
    QVector<RegistrationOption> results;
    if (!m_db) {
        m_lastError = "Database connection is not available.";
        return results;
    }

    sqlite3_stmt *stmt = nullptr;
    const char *sql =
        "SELECT id, subject, exam_date, exam_time FROM examinations "
        "ORDER BY exam_date, exam_time, subject COLLATE NOCASE";

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return results;
    }

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        RegistrationOption item;
        item.id = sqlite3_column_int(stmt, 0);
        auto textAt = [stmt](int column) {
            const auto *value = reinterpret_cast<const char *>(
                sqlite3_column_text(stmt, column));
            return QString::fromUtf8(value ? value : "");
        };
        item.label = QString("%1 — %2 %3")
                         .arg(textAt(1), textAt(2), textAt(3));
        results.append(item);
    }

    m_lastError = rc == SQLITE_DONE ? QString()
                                    : QString::fromUtf8(sqlite3_errmsg(m_db));
    sqlite3_finalize(stmt);
    return results;
}

QVector<RegisteredStudent>
RegistrationRepository::getRegisteredStudents(int examinationId)
{
    QVector<RegisteredStudent> results;
    if (!m_db || examinationId <= 0) {
        m_lastError = "Select a valid examination.";
        return results;
    }

    sqlite3_stmt *stmt = nullptr;
    const char *sql =
        "SELECT s.id, s.registration_number, s.name, s.department "
        "FROM registrations r JOIN students s ON s.id = r.student_id "
        "WHERE r.examination_id = ? ORDER BY s.registration_number COLLATE NOCASE";

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return results;
    }
    sqlite3_bind_int(stmt, 1, examinationId);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        RegisteredStudent item;
        item.id = sqlite3_column_int(stmt, 0);
        auto textAt = [stmt](int column) {
            const auto *value = reinterpret_cast<const char *>(
                sqlite3_column_text(stmt, column));
            return QString::fromUtf8(value ? value : "");
        };
        item.registrationNumber = textAt(1);
        item.name = textAt(2);
        item.department = textAt(3);
        results.append(item);
    }

    m_lastError = rc == SQLITE_DONE ? QString()
                                    : QString::fromUtf8(sqlite3_errmsg(m_db));
    sqlite3_finalize(stmt);
    return results;
}

QVector<RegisteredStudent>
RegistrationRepository::getAvailableStudents(int examinationId)
{
    QVector<RegisteredStudent> results;
    if (!m_db || examinationId <= 0) {
        m_lastError = "Select a valid examination.";
        return results;
    }

    sqlite3_stmt *stmt = nullptr;
    const char *sql =
        "SELECT s.id, s.registration_number, s.name, s.department "
        "FROM students s WHERE NOT EXISTS ("
        "SELECT 1 FROM registrations r WHERE r.student_id = s.id "
        "AND r.examination_id = ?) "
        "ORDER BY s.registration_number COLLATE NOCASE";

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return results;
    }
    sqlite3_bind_int(stmt, 1, examinationId);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        RegisteredStudent item;
        item.id = sqlite3_column_int(stmt, 0);
        auto textAt = [stmt](int column) {
            const auto *value = reinterpret_cast<const char *>(
                sqlite3_column_text(stmt, column));
            return QString::fromUtf8(value ? value : "");
        };
        item.registrationNumber = textAt(1);
        item.name = textAt(2);
        item.department = textAt(3);
        results.append(item);
    }

    m_lastError = rc == SQLITE_DONE ? QString()
                                    : QString::fromUtf8(sqlite3_errmsg(m_db));
    sqlite3_finalize(stmt);
    return results;
}

bool RegistrationRepository::registerStudent(int examinationId, int studentId)
{
    if (!m_db || examinationId <= 0 || studentId <= 0) {
        m_lastError = "Select a valid examination and student.";
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(
            m_db,
            "INSERT INTO registrations (examination_id, student_id) VALUES (?, ?)",
            -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, examinationId);
    sqlite3_bind_int(stmt, 2, studentId);
    const int rc = sqlite3_step(stmt);
    m_lastError = rc == SQLITE_DONE ? QString()
                                    : QString::fromUtf8(sqlite3_errmsg(m_db));
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool RegistrationRepository::unregisterStudent(int examinationId, int studentId)
{
    if (!m_db || examinationId <= 0 || studentId <= 0) {
        m_lastError = "Select a valid examination and student.";
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(
            m_db,
            "DELETE FROM registrations WHERE examination_id = ? AND student_id = ?",
            -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    sqlite3_bind_int(stmt, 1, examinationId);
    sqlite3_bind_int(stmt, 2, studentId);
    const int rc = sqlite3_step(stmt);
    const int changes = sqlite3_changes(m_db);

    if (rc != SQLITE_DONE)
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else if (changes == 0)
        m_lastError = "The registration no longer exists.";
    else
        m_lastError.clear();

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE && changes > 0;
}

QString RegistrationRepository::lastError() const
{
    return m_lastError;
}
