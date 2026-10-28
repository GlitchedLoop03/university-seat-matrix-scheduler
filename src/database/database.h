#ifndef DATABASE_H
#define DATABASE_H

#include <QString>

struct sqlite3;

class Database
{
public:
    Database();
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    bool open(const QString &path);
    bool initializeSchema();

    QString lastError() const;
    bool isOpen() const;
    sqlite3 *handle() const;
    int countRows(const QString &tableName) const;

private:
    sqlite3 *m_db;
    QString m_lastError;
};

#endif // DATABASE_H
