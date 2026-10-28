#ifndef ROOMSPAGE_H
#define ROOMSPAGE_H

#include "../database/roomrepository.h"

#include <QWidget>

class QLineEdit;
class QTableWidget;
class QPushButton;
class QLabel;

class RoomsPage : public QWidget
{
    Q_OBJECT

public:
    explicit RoomsPage(RoomRepository *repository,
                       QWidget *parent = nullptr);

private:
    RoomRepository *m_repository;
    QLineEdit *m_searchInput;
    QTableWidget *m_table;
    QLabel *m_emptyLabel;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;

    void setupUi();
    void loadRooms();
    void applySearch();
    void updateActionButtons();
    void showRoomDialog(bool editing);
    void deleteSelectedRoom();
    int selectedRoomId() const;
};

#endif
