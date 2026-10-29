#ifndef ROOM_H
#define ROOM_H
#include <QString>
struct Room {
    int id = 0;
    QString roomNumber;
    int rows = 5;
    int columns = 6;
    int capacity() const { return rows * columns; }
};
#endif // ROOM_H
