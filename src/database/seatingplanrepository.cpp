#include "seatingplanrepository.h"

#include <sqlite3.h>

SeatingPlanRepository::SeatingPlanRepository(sqlite3 *database)
    : m_db(database)
{
}

QString SeatingPlanRepository::lastError() const
{
    return m_lastError;
}

QVector<SeatingExamination> SeatingPlanRepository::getExaminations()
{
    QVector<SeatingExamination> results;
    m_lastError.clear();

    const char *sql =
        "SELECT id, subject, exam_date, exam_time "
        "FROM examinations ORDER BY exam_date, exam_time, subject";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return results;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SeatingExamination exam;
        exam.id = sqlite3_column_int(stmt, 0);

        const QString subject = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        const QString date = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        const QString time = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));

        exam.label = subject + " — " + date + " " + time;
        results.append(exam);
    }

    sqlite3_finalize(stmt);
    return results;
}

QVector<SeatingAllocation>
SeatingPlanRepository::getAllocations(int examinationId)
{
    QVector<SeatingAllocation> results;
    m_lastError.clear();

    const char *sql =
        "SELECT r.room_number, a.seat_number, "
        "s.registration_number, s.name, s.department "
        "FROM seat_allocations a "
        "JOIN rooms r ON r.id = a.room_id "
        "JOIN students s ON s.id = a.student_id "
        "WHERE a.examination_id = ? "
        "ORDER BY r.room_number, a.seat_number";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));
        return results;
    }

    sqlite3_bind_int(stmt, 1, examinationId);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        SeatingAllocation item;
        item.roomNumber = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
        item.seatNumber = sqlite3_column_int(stmt, 1);
        item.registrationNumber = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        item.studentName = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));
        item.department = QString::fromUtf8(
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));
        results.append(item);
    }

    if (rc != SQLITE_DONE)
        m_lastError = QString::fromUtf8(sqlite3_errmsg(m_db));

    sqlite3_finalize(stmt);
    return results;
}

bool SeatingPlanRepository::generatePlan(int examinationId)
{
    m_lastError.clear();

    auto fail = [this](const QString &message) {
        m_lastError = message;
        return false;
    };

    if (examinationId <= 0)
        return fail("Select a valid examination.");

    if (sqlite3_exec(m_db, "BEGIN IMMEDIATE", nullptr, nullptr, nullptr)
        != SQLITE_OK) {
        return fail(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    auto rollback = [this, &fail](const QString &message) {
        sqlite3_exec(m_db, "ROLLBACK", nullptr, nullptr, nullptr);
        return fail(message);
    };

    int registeredCount = 0;
    {
        sqlite3_stmt *stmt = nullptr;
        const char *sql =
            "SELECT COUNT(*) FROM registrations WHERE examination_id = ?";

        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));

        sqlite3_bind_int(stmt, 1, examinationId);
        const int rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW)
            registeredCount = sqlite3_column_int(stmt, 0);

        sqlite3_finalize(stmt);

        if (rc != SQLITE_ROW)
            return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    if (registeredCount == 0)
        return rollback("No students are registered for this examination.");

    struct RoomCapacity {
        int id;
        QString number;
        int capacity;
    };

    QVector<RoomCapacity> rooms;
    int totalCapacity = 0;

    {
        sqlite3_stmt *stmt = nullptr;
        const char *sql =
            "SELECT id, room_number, capacity FROM rooms ORDER BY room_number";

        if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));

        int rc;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            RoomCapacity room;
            room.id = sqlite3_column_int(stmt, 0);
            room.number = QString::fromUtf8(
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
            room.capacity = sqlite3_column_int(stmt, 2);
            rooms.append(room);
            totalCapacity += room.capacity;
        }

        if (rc != SQLITE_DONE) {
            const QString error = QString::fromUtf8(sqlite3_errmsg(m_db));
            sqlite3_finalize(stmt);
            return rollback(error);
        }

        sqlite3_finalize(stmt);
    }

    if (rooms.isEmpty())
        return rollback("Add at least one room before generating a plan.");

    if (totalCapacity < registeredCount) {
        return rollback(
            QString("Insufficient room capacity. Registered students: %1. "
                    "Available seats: %2. Add rooms or increase capacity.")
                .arg(registeredCount).arg(totalCapacity));
    }

    // Build the new plan inside the transaction. Existing allocations
    // remain intact unless the complete replacement succeeds.
    if (sqlite3_exec(m_db, "SAVEPOINT new_plan", nullptr, nullptr, nullptr)
        != SQLITE_OK) {
        return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    if (sqlite3_exec(
            m_db,
            "DELETE FROM seat_allocations WHERE examination_id = "
            "(SELECT id FROM examinations WHERE id = 0)",
            nullptr, nullptr, nullptr) != SQLITE_OK) {
        return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    {
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(
                m_db,
                "DELETE FROM seat_allocations WHERE examination_id = ?",
                -1, &stmt, nullptr) != SQLITE_OK) {
            return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
        }

        sqlite3_bind_int(stmt, 1, examinationId);
        const int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE)
            return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    sqlite3_stmt *students = nullptr;
    sqlite3_stmt *insert = nullptr;

    if (sqlite3_prepare_v2(
            m_db,
            "SELECT reg.student_id FROM registrations AS reg "
            "JOIN students AS stu ON stu.id = reg.student_id "
            "WHERE reg.examination_id = ? "
            "ORDER BY stu.registration_number COLLATE NOCASE, stu.id",
            -1, &students, nullptr) != SQLITE_OK) {
        return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    if (sqlite3_prepare_v2(
            m_db,
            "INSERT INTO seat_allocations "
            "(examination_id, room_id, student_id, seat_number) "
            "VALUES (?, ?, ?, ?)",
            -1, &insert, nullptr) != SQLITE_OK) {
        sqlite3_finalize(students);
        return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    sqlite3_bind_int(students, 1, examinationId);

    int roomIndex = 0;
    int seatNumber = 0;
    int rc;

    while ((rc = sqlite3_step(students)) == SQLITE_ROW) {
        while (roomIndex < rooms.size()
               && seatNumber >= rooms[roomIndex].capacity) {
            ++roomIndex;
            seatNumber = 0;
        }

        if (roomIndex >= rooms.size()) {
            sqlite3_finalize(students);
            sqlite3_finalize(insert);
            return rollback("Not enough seats to complete the allocation.");
        }

        sqlite3_reset(insert);
        sqlite3_clear_bindings(insert);
        sqlite3_bind_int(insert, 1, examinationId);
        sqlite3_bind_int(insert, 2, rooms[roomIndex].id);
        sqlite3_bind_int(insert, 3, sqlite3_column_int(students, 0));
        sqlite3_bind_int(insert, 4, seatNumber + 1);

        if (sqlite3_step(insert) != SQLITE_DONE) {
            const QString error = QString::fromUtf8(sqlite3_errmsg(m_db));
            sqlite3_finalize(students);
            sqlite3_finalize(insert);
            return rollback(error);
        }

        ++seatNumber;
    }

    sqlite3_finalize(students);
    sqlite3_finalize(insert);

    if (rc != SQLITE_DONE)
        return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));

    if (sqlite3_exec(m_db, "RELEASE SAVEPOINT new_plan",
                     nullptr, nullptr, nullptr) != SQLITE_OK) {
        return rollback(QString::fromUtf8(sqlite3_errmsg(m_db)));
    }

    if (sqlite3_exec(m_db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
        const QString error = QString::fromUtf8(sqlite3_errmsg(m_db));
        sqlite3_exec(m_db, "ROLLBACK", nullptr, nullptr, nullptr);
        return fail(error);
    }

    return true;
}
