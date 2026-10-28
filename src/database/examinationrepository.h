#ifndef EXAMINATIONREPOSITORY_H
#define EXAMINATIONREPOSITORY_H

#include "../models/examination.h"
#include <QString>
#include <QVector>

struct sqlite3;

class ExaminationRepository
{
public:
    explicit ExaminationRepository(sqlite3 *database);

    bool addExamination(const Examination &examination);
    bool updateExamination(const Examination &examination);
    bool deleteExamination(int id);
    QVector<Examination> getAllExaminations();
    QString lastError() const;

private:
    sqlite3 *m_db;
    QString m_lastError;
};

#endif
