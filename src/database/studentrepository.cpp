#include "studentrepository.h"

#include <sqlite3.h>

StudentRepository::StudentRepository(sqlite3 *database)
    : m_db(database)
{
    if (m_db == nullptr) {
        m_lastError = "Database connection is not available.";
    }
}

bool StudentRepository::addStudent(const Student &student)
{
    if (m_db == nullptr) {
        m_lastError = "Database connection is not available.";
        return false;
    }

    const QString registrationNumber =
        student.registrationNumber.trimmed();
    const QString name = student.name.trimmed();
    const QString department = student.department.trimmed();

    if (registrationNumber.isEmpty() ||
        name.isEmpty() ||
        department.isEmpty()) {
        m_lastError = "Registration number, name, and department are required.";
        return false;
    }

    const char *sql = R"(
        INSERT INTO students (registration_number, name, department)
        VALUES (?, ?, ?);
    )";

    sqlite3_stmt *statement = nullptr;

    int result = sqlite3_prepare_v2(
        m_db, sql, -1, &statement, nullptr);

    if (result != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return false;
    }

    const QByteArray registrationBytes = registrationNumber.toUtf8();
    const QByteArray nameBytes = name.toUtf8();
    const QByteArray departmentBytes = department.toUtf8();

    sqlite3_bind_text(
        statement, 1, registrationBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        statement, 2, nameBytes.constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(
        statement, 3, departmentBytes.constData(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        sqlite3_finalize(statement);
        return false;
    }

    sqlite3_finalize(statement);
    m_lastError.clear();
    return true;
}

QVector<Student> StudentRepository::getAllStudents()
{
    QVector<Student> students;

    if (m_db == nullptr) {
        m_lastError = "Database connection is not available.";
        return students;
    }

    const char *sql = R"(
        SELECT id, registration_number, name, department
        FROM students
        ORDER BY registration_number COLLATE NOCASE;
    )";

    sqlite3_stmt *statement = nullptr;

    const int result = sqlite3_prepare_v2(
        m_db, sql, -1, &statement, nullptr);

    if (result != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return students;
    }

    int stepResult;

    while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW) {
        Student student;

        student.id = sqlite3_column_int(statement, 0);

        const auto *registration = sqlite3_column_text(statement, 1);
        const auto *name = sqlite3_column_text(statement, 2);
        const auto *department = sqlite3_column_text(statement, 3);

        student.registrationNumber = QString::fromUtf8(
            reinterpret_cast<const char *>(registration));

        student.name = QString::fromUtf8(
            reinterpret_cast<const char *>(name));

        student.department = QString::fromUtf8(
            reinterpret_cast<const char *>(department));

        students.append(student);
    }

    if (stepResult != SQLITE_DONE) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        sqlite3_finalize(statement);
        students.clear();
        return students;
    }

    sqlite3_finalize(statement);
    m_lastError.clear();

    return students;
}

QString StudentRepository::lastError() const
{
    return m_lastError;
}
