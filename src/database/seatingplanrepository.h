#ifndef SEATINGPLANREPOSITORY_H
#define SEATINGPLANREPOSITORY_H

#include <QString>
#include <QVector>

struct sqlite3;

struct SeatingExamination {
    int id = 0;
    QString label;
};

struct SeatingAllocation {
    QString roomNumber;
    int seatNumber = 0;
    QString registrationNumber;
    QString studentName;
    QString department;
};

class SeatingPlanRepository
{
public:
    explicit SeatingPlanRepository(sqlite3 *database);

    QVector<SeatingExamination> getExaminations();
    QVector<SeatingAllocation> getAllocations(int examinationId);
    bool generatePlan(int examinationId);
    QString lastError() const;

private:
    sqlite3 *m_db;
    QString m_lastError;
};

#endif
