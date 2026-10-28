#include "roomspage.h"

#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QToolButton>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalBlocker>

RoomsPage::RoomsPage(RoomRepository *repository, QWidget *parent)
    : QWidget(parent),
      m_repository(repository),
      m_searchInput(nullptr),
      m_table(nullptr),
      m_emptyLabel(nullptr),
      m_editButton(nullptr),
      m_deleteButton(nullptr)
{
    setupUi();
    loadRooms();
}

void RoomsPage::setupUi()
{
    setObjectName("roomsPage");
    setStyleSheet(
        "QWidget#roomsPage { background: #f7f8fc; color: #20283b; }"
        "QLabel#pageTitle { font-size: 25px; font-weight: 700; color: #20283b; }"
        "QLabel#pageSubtitle { color: #758097; font-size: 13px; }"
        "QLineEdit, QSpinBox { background: white; border: 1px solid #dce1eb;"
        " border-radius: 7px; padding: 9px; color: #20283b; }"
        "QLineEdit:focus, QSpinBox:focus { border: 1px solid #7188e8; }"
        "QSpinBox { padding-right: 22px; }"
        "QSpinBox::up-button, QSpinBox::down-button {"
        " subcontrol-origin: border; subcontrol-position: right;"
        " width: 20px; background: #eef2f9; border-left: 1px solid #dce1eb; }"
        "QSpinBox::up-button { subcontrol-position: right top;"
        " border-top-right-radius: 6px; }"
        "QSpinBox::down-button { subcontrol-position: right bottom;"
        " border-bottom-right-radius: 6px; }"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
        " background: #dfe7f7; }"
        "QPushButton { border: none; border-radius: 7px; padding: 9px 15px;"
        " font-weight: 600; }"
        "QPushButton#primaryButton { background: #526de5; color: white; }"
        "QPushButton#primaryButton:hover { background: #405bd0; }"
        "QPushButton#secondaryButton { background: white; color: #34405b;"
        " border: 1px solid #dce1eb; }"
        "QPushButton#secondaryButton:hover { background: #f0f3fa; }"
        "QPushButton:disabled { color: #a6adbc; background: #edf0f5; }"
        "QTableWidget { background: white; border: 1px solid #e3e7ef;"
        " border-radius: 9px; gridline-color: #edf0f5; selection-background-color: #e9edff;"
        " selection-color: #20283b; }"
        "QHeaderView::section { background: #f5f7fb; color: #68738b;"
        " padding: 11px; border: none; border-bottom: 1px solid #e3e7ef;"
        " font-weight: 600; }"
    );

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 25, 28, 28);
    root->setSpacing(16);

    auto *title = new QLabel("Rooms");
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *subtitle = new QLabel(
        "Manage examination rooms and the number of students each room can accommodate.");
    subtitle->setObjectName("pageSubtitle");
    root->addWidget(subtitle);

    auto *toolbar = new QHBoxLayout();
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search by room number or capacity...");
    m_searchInput->setClearButtonEnabled(true);
    toolbar->addWidget(m_searchInput, 1);

    m_editButton = new QPushButton("Edit room");
    m_editButton->setObjectName("secondaryButton");
    toolbar->addWidget(m_editButton);

    m_deleteButton = new QPushButton("Delete room");
    m_deleteButton->setObjectName("secondaryButton");
    toolbar->addWidget(m_deleteButton);

    auto *addButton = new QPushButton("+ Add room");
    addButton->setObjectName("primaryButton");
    toolbar->addWidget(addButton);
    root->addLayout(toolbar);

    m_table = new QTableWidget(0, 2);
    m_table->setHorizontalHeaderLabels({"Room number", "Capacity"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(43);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    root->addWidget(m_table, 1);

    m_emptyLabel = new QLabel("No rooms yet. Add your first examination room.");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #758097; padding: 12px;");
    root->addWidget(m_emptyLabel);

    connect(addButton, &QPushButton::clicked, this,
            [this]() { showRoomDialog(false); });
    connect(m_editButton, &QPushButton::clicked, this,
            [this]() { showRoomDialog(true); });
    connect(m_deleteButton, &QPushButton::clicked, this,
            [this]() { deleteSelectedRoom(); });
    connect(m_searchInput, &QLineEdit::textChanged, this,
            [this]() { applySearch(); });
    connect(m_table, &QTableWidget::itemSelectionChanged, this,
            [this]() { updateActionButtons(); });
    connect(m_table, &QTableWidget::itemDoubleClicked, this,
            [this]() { showRoomDialog(true); });
}

void RoomsPage::loadRooms()
{
    const QVector<Room> rooms = m_repository->getAllRooms();

    m_table->setRowCount(0);
    for (const Room &room : rooms) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto *numberItem = new QTableWidgetItem(room.roomNumber);
        numberItem->setData(Qt::UserRole, room.id);
        m_table->setItem(row, 0, numberItem);
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(room.capacity)));
    }

    m_emptyLabel->setText(
        rooms.isEmpty() ? "No rooms yet. Add your first examination room."
                        : "No rooms match your search.");
    applySearch();
}

void RoomsPage::applySearch()
{
    const QString query = m_searchInput->text().trimmed();
    bool anyVisible = false;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QString number = m_table->item(row, 0)->text();
        const QString capacity = m_table->item(row, 1)->text();
        const bool matches = number.contains(query, Qt::CaseInsensitive)
                             || capacity.contains(query, Qt::CaseInsensitive);
        m_table->setRowHidden(row, !matches);
        anyVisible = anyVisible || matches;
    }

    const bool noRooms = m_table->rowCount() == 0;
    m_emptyLabel->setVisible(noRooms || !anyVisible);
    updateActionButtons();
}

void RoomsPage::updateActionButtons()
{
    const bool selected = selectedRoomId() > 0;
    m_editButton->setEnabled(selected);
    m_deleteButton->setEnabled(selected);
}

int RoomsPage::selectedRoomId() const
{
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0) || m_table->isRowHidden(row))
        return 0;

    return m_table->item(row, 0)->data(Qt::UserRole).toInt();
}

void RoomsPage::showRoomDialog(bool editing)
{
    const int roomId = editing ? selectedRoomId() : 0;
    if (editing && roomId == 0)
        return;

    QString currentNumber;
    int currentCapacity = 30;

    if (editing) {
        const int row = m_table->currentRow();
        currentNumber = m_table->item(row, 0)->text();
        currentCapacity = m_table->item(row, 1)->text().toInt();
    }

    QDialog dialog(this);
    dialog.setWindowTitle(editing ? "Edit room" : "Add room");
    dialog.setMinimumWidth(440);
    dialog.setStyleSheet(
        "QDialog { background: white; color: #20283b; }"
        "QLabel { color: #34405b; }"
        "QLineEdit, QSpinBox { background: white; border: 1px solid #dce1eb;"
        " border-radius: 6px; padding: 8px; }"
        "QPushButton { padding: 8px 14px; border-radius: 6px; }"
    );

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *numberInput = new QLineEdit(currentNumber);
    numberInput->setPlaceholderText("e.g. B-204");

    auto *capacityInput = new QLineEdit(QString::number(currentCapacity));
    capacityInput->setValidator(new QIntValidator(1, 100000, capacityInput));
    capacityInput->setMinimumWidth(100);
    capacityInput->setAlignment(Qt::AlignCenter);

    auto *decreaseButton = new QPushButton("-");
    decreaseButton->setFixedSize(38, 36);

    auto *increaseButton = new QPushButton("+");
    increaseButton->setFixedSize(38, 36);

    const QString stepButtonStyle =
        "QPushButton { background: #e8edf8; color: #243d69;"
        " border: 1px solid #cbd5e7; border-radius: 6px;"
        " font-size: 18px; font-weight: 700; padding: 0; }"
        "QPushButton:hover { background: #d4def3; }"
        "QPushButton:pressed { background: #bdccef; }";

    decreaseButton->setStyleSheet(stepButtonStyle);
    increaseButton->setStyleSheet(stepButtonStyle);

    auto *capacityWidget = new QWidget();
    auto *capacityLayout = new QHBoxLayout(capacityWidget);
    capacityLayout->setContentsMargins(0, 0, 0, 0);
    capacityLayout->setSpacing(8);
    capacityLayout->addWidget(decreaseButton);
    capacityLayout->addWidget(capacityInput, 1);
    capacityLayout->addWidget(increaseButton);

    auto changeCapacity = [capacityInput](int delta) {
        bool ok = false;
        int value = capacityInput->text().toInt(&ok);
        if (!ok)
            value = 1;

        value = qBound(1, value + delta, 100000);
        capacityInput->setText(QString::number(value));
    };

    connect(decreaseButton, &QPushButton::clicked, this,
            [changeCapacity]() { changeCapacity(-1); });
    connect(increaseButton, &QPushButton::clicked, this,
            [changeCapacity]() { changeCapacity(1); });

    form->addRow("Room number", numberInput);
    form->addRow("Seat capacity", capacityWidget);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    Room room;
    room.id = roomId;
    room.roomNumber = numberInput->text().trimmed();
    bool capacityOk = false;
    room.capacity = capacityInput->text().toInt(&capacityOk);
    if (!capacityOk || room.capacity < 1 || room.capacity > 100000) {
        QMessageBox::warning(
            &dialog, "Invalid capacity",
            "Enter a seat capacity between 1 and 100000.");
        return;
    }

    const bool success = editing ? m_repository->updateRoom(room)
                                 : m_repository->addRoom(room);

    if (!success) {
        QMessageBox::warning(
            this, "Could not save room",
            m_repository->lastError().isEmpty()
                ? "Check the room details and try again."
                : m_repository->lastError());
        return;
    }

    loadRooms();
}

void RoomsPage::deleteSelectedRoom()
{
    const int roomId = selectedRoomId();
    if (roomId == 0)
        return;

    const int answer = QMessageBox::warning(
        this, "Delete room",
        "Delete this room? Any seat allocations associated with it may also be removed.",
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if (answer != QMessageBox::Yes)
        return;

    if (!m_repository->deleteRoom(roomId)) {
        QMessageBox::warning(this, "Could not delete room", m_repository->lastError());
        return;
    }

    loadRooms();
}
