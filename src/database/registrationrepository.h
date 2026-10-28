#ifndef REGISTRATIONREPOSITORY_H
#define REGISTRATIONREPOSITORY_H

#include <QString>
#include <QVector>

struct sqlite3;

struct RegistrationOption
{
    int id = 0;
    QString label;
};

struct RegisteredStudent
{
    int id = 0;
    QString registrationNumber;
    QString name;
    QString department;
};

class RegistrationRepository
{
public:
    explicit RegistrationRepository(sqlite3 *database);

    QVector<RegistrationOption> getExaminations();
    QVector<RegisteredStudent> getRegisteredStudents(int examinationId);
    QVector<RegisteredStudent> getAvailableStudents(int examinationId);

    bool registerStudent(int examinationId, int studentId);
    bool unregisterStudent(int examinationId, int studentId);

    QString lastError() const;

private:
    sqlite3 *m_db;
    QString m_lastError;
};

#endif
