#ifndef ROOMREPOSITORY_H
#define ROOMREPOSITORY_H

#include "../models/room.h"

#include <QString>
#include <QVector>

struct sqlite3;

class RoomRepository
{
public:
    explicit RoomRepository(sqlite3 *database);

    bool addRoom(const Room &room);
    bool updateRoom(const Room &room);
    bool deleteRoom(int id);
    QVector<Room> getAllRooms();

    QString lastError() const;

private:
    sqlite3 *m_db;
    QString m_lastError;
};

#endif // ROOMREPOSITORY_H
