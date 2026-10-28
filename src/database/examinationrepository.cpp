#include "examinationrepository.h"

#include <sqlite3.h>
#include <QByteArray>

ExaminationRepository::ExaminationRepository(sqlite3 *database)
    : m_db(database)
{
    if (!m_db)
        m_lastError = "Database connection is not available.";
}

bool ExaminationRepository::addExamination(const Examination &examination)
{
    if (!m_db) {
        m_lastError = "Database connection is not available.";
        return false;
    }

    const QString subject = examination.subject.trimmed();
    if (subject.isEmpty() || examination.examDate.isEmpty()
        || examination.examTime.isEmpty()) {
        m_lastError = "Subject, date, and time are required.";
        return false;
    }

    const char *sql =
        "INSERT INTO examinations (subject, exam_date, exam_time) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    const QByteArray subjectBytes = subject.toUtf8();
    const QByteArray dateBytes = examination.examDate.toUtf8();
    const QByteArray timeBytes = examination.examTime.toUtf8();

    sqlite3_bind_text(statement, 1, subjectBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, dateBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, timeBytes.constData(), -1, SQLITE_TRANSIENT);

    const int result = sqlite3_step(statement);
    if (result != SQLITE_DONE)
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else
        m_lastError.clear();

    sqlite3_finalize(statement);
    return result == SQLITE_DONE;
}

bool ExaminationRepository::updateExamination(const Examination &examination)
{
    if (!m_db || examination.id <= 0) {
        m_lastError = "A valid examination is required.";
        return false;
    }

    const QString subject = examination.subject.trimmed();
    if (subject.isEmpty() || examination.examDate.isEmpty()
        || examination.examTime.isEmpty()) {
        m_lastError = "Subject, date, and time are required.";
        return false;
    }

    const char *sql =
        "UPDATE examinations SET subject = ?, exam_date = ?, exam_time = ? "
        "WHERE id = ?;";

    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    const QByteArray subjectBytes = subject.toUtf8();
    const QByteArray dateBytes = examination.examDate.toUtf8();
    const QByteArray timeBytes = examination.examTime.toUtf8();

    sqlite3_bind_text(statement, 1, subjectBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, dateBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, timeBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 4, examination.id);

    const int result = sqlite3_step(statement);
    const int changes = sqlite3_changes(m_db);

    if (result != SQLITE_DONE)
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else if (changes == 0)
        m_lastError = "No examination record was changed.";
    else
        m_lastError.clear();

    sqlite3_finalize(statement);
    return result == SQLITE_DONE && changes > 0;
}

bool ExaminationRepository::deleteExamination(int id)
{
    if (!m_db || id <= 0) {
        m_lastError = "A valid examination is required.";
        return false;
    }

    sqlite3_stmt *statement = nullptr;
    if (sqlite3_prepare_v2(m_db,
            "DELETE FROM examinations WHERE id = ?;", -1,
            &statement, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    sqlite3_bind_int(statement, 1, id);
    const int result = sqlite3_step(statement);
    const int changes = sqlite3_changes(m_db);

    if (result != SQLITE_DONE)
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
    else if (changes == 0)
        m_lastError = "The examination record no longer exists.";
    else
        m_lastError.clear();

    sqlite3_finalize(statement);
    return result == SQLITE_DONE && changes > 0;
}

QVector<Examination> ExaminationRepository::getAllExaminations()
{
    QVector<Examination> examinations;

    if (!m_db) {
        m_lastError = "Database connection is not available.";
        return examinations;
    }

    sqlite3_stmt *statement = nullptr;
    const char *sql =
        "SELECT id, subject, exam_date, exam_time FROM examinations "
        "ORDER BY exam_date, exam_time, subject COLLATE NOCASE;";

    if (sqlite3_prepare_v2(m_db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return examinations;
    }

    int result;
    while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
        Examination examination;
        examination.id = sqlite3_column_int(statement, 0);
        examination.subject = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(statement, 1)));
        examination.examDate = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(statement, 2)));
        examination.examTime = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(statement, 3)));
        examinations.append(examination);
    }

    if (result == SQLITE_DONE)
        m_lastError.clear();
    else
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));

    sqlite3_finalize(statement);
    return examinations;
}

QString ExaminationRepository::lastError() const
{
    return m_lastError;
}
