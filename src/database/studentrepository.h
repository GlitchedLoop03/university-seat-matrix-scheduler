#ifndef STUDENTREPOSITORY_H
#define STUDENTREPOSITORY_H

#include "../models/student.h"

#include <QString>
#include <QVector>

struct sqlite3;

class StudentRepository
{
public:
    explicit StudentRepository(sqlite3 *database);

    bool addStudent(const Student &student);
    bool deleteStudent(int id);
    QVector<Student> getAllStudents();

    QString lastError() const;

private:
    sqlite3 *m_db;
    QString m_lastError;
};

#endif // STUDENTREPOSITORY_H
