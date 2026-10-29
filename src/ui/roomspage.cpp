#include "roomspage.h"
#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>

RoomsPage::RoomsPage(RoomRepository *repository, QWidget *parent)
    : QWidget(parent), m_repository(repository), m_searchInput(nullptr), m_table(nullptr),
      m_emptyLabel(nullptr), m_editButton(nullptr), m_deleteButton(nullptr)
{
    setupUi();
    loadRooms();
}

void RoomsPage::setupUi()
{
    setObjectName("roomsPage");
    setStyleSheet(
        "QWidget#roomsPage { background:#f7f8fc; color:#20283b; }"
        "QLabel#pageTitle { font-size:25px; font-weight:700; color:#20283b; }"
        "QLabel#pageSubtitle { color:#758097; font-size:13px; }"
        "QLineEdit,QSpinBox { background:white; border:1px solid #dce1eb; border-radius:7px; padding:8px; color:#20283b; }"
        "QPushButton { border:none; border-radius:7px; padding:9px 15px; font-weight:600; }"
        "QPushButton#primaryButton { background:#526de5; color:white; }"
        "QPushButton#secondaryButton { background:white; color:#34405b; border:1px solid #dce1eb; }"
        "QTableWidget { background:white; border:1px solid #e3e7ef; border-radius:9px; gridline-color:#edf0f5; selection-background-color:#e9edff; selection-color:#20283b; }"
        "QHeaderView::section { background:#f5f7fb; color:#68738b; padding:11px; border:none; border-bottom:1px solid #e3e7ef; font-weight:600; }");
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28,25,28,28); root->setSpacing(16);
    auto *title = new QLabel("Rooms"); title->setObjectName("pageTitle"); root->addWidget(title);
    auto *subtitle = new QLabel("Set the physical row and column layout for every room. Capacity is calculated automatically.");
    subtitle->setObjectName("pageSubtitle"); root->addWidget(subtitle);
    auto *toolbar = new QHBoxLayout();
    m_searchInput = new QLineEdit(); m_searchInput->setPlaceholderText("Search by room, layout, or capacity...");
    m_searchInput->setClearButtonEnabled(true); toolbar->addWidget(m_searchInput,1);
    m_editButton = new QPushButton("Edit room"); m_editButton->setObjectName("secondaryButton"); toolbar->addWidget(m_editButton);
    m_deleteButton = new QPushButton("Delete room"); m_deleteButton->setObjectName("secondaryButton"); toolbar->addWidget(m_deleteButton);
    auto *addButton = new QPushButton("+ Add room"); addButton->setObjectName("primaryButton"); toolbar->addWidget(addButton);
    root->addLayout(toolbar);
    m_table = new QTableWidget(0,3);
    m_table->setHorizontalHeaderLabels({"Room number","Layout (rows × columns)","Capacity"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false); m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(43);
    m_table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    root->addWidget(m_table,1);
    m_emptyLabel = new QLabel("No rooms yet. Add your first examination room.");
    m_emptyLabel->setAlignment(Qt::AlignCenter); m_emptyLabel->setStyleSheet("color:#758097;padding:12px;"); root->addWidget(m_emptyLabel);
    connect(addButton,&QPushButton::clicked,this,[this](){showRoomDialog(false);});
    connect(m_editButton,&QPushButton::clicked,this,[this](){showRoomDialog(true);});
    connect(m_deleteButton,&QPushButton::clicked,this,[this](){deleteSelectedRoom();});
    connect(m_searchInput,&QLineEdit::textChanged,this,[this](){applySearch();});
    connect(m_table,&QTableWidget::itemSelectionChanged,this,[this](){updateActionButtons();});
    connect(m_table,&QTableWidget::itemDoubleClicked,this,[this](){showRoomDialog(true);});
}

void RoomsPage::loadRooms()
{
    const auto rooms = m_repository->getAllRooms();
    m_table->setRowCount(0);
    for (const Room &room : rooms) {
        int row=m_table->rowCount(); m_table->insertRow(row);
        auto *number = new QTableWidgetItem(room.roomNumber); number->setData(Qt::UserRole,room.id);
        m_table->setItem(row,0,number);
        m_table->setItem(row,1,new QTableWidgetItem(QString("%1 × %2").arg(room.rows).arg(room.columns)));
        m_table->setItem(row,2,new QTableWidgetItem(QString::number(room.capacity())));
    }
    m_emptyLabel->setText(rooms.isEmpty() ? "No rooms yet. Add your first examination room." : "No rooms match your search.");
    applySearch();
}

void RoomsPage::applySearch()
{
    const QString query=m_searchInput->text().trimmed(); bool any=false;
    for(int row=0;row<m_table->rowCount();++row) {
        QStringList fields;
        for(int col=0;col<m_table->columnCount();++col) fields << m_table->item(row,col)->text();
        bool matches=fields.join(" ").contains(query,Qt::CaseInsensitive);
        m_table->setRowHidden(row,!matches); any=any||matches;
    }
    m_emptyLabel->setVisible(m_table->rowCount()==0||!any); updateActionButtons();
}

void RoomsPage::updateActionButtons()
{
    bool selected=selectedRoomId()>0; m_editButton->setEnabled(selected); m_deleteButton->setEnabled(selected);
}

int RoomsPage::selectedRoomId() const
{
    int row=m_table->currentRow();
    if(row<0||!m_table->item(row,0)||m_table->isRowHidden(row)) return 0;
    return m_table->item(row,0)->data(Qt::UserRole).toInt();
}

void RoomsPage::showRoomDialog(bool editing)
{
    int roomId=editing?selectedRoomId():0; if(editing&&roomId==0)return;
    QString currentNumber; int currentRows=5,currentColumns=6;
    if(editing) {
        int row=m_table->currentRow(); currentNumber=m_table->item(row,0)->text();
        auto dimensions=m_table->item(row,1)->text().split(" × ");
        if(dimensions.size()==2){currentRows=dimensions[0].toInt();currentColumns=dimensions[1].toInt();}
    }
    QDialog dialog(this); dialog.setWindowTitle(editing?"Edit room layout":"Add room layout"); dialog.setMinimumWidth(420);
    dialog.setStyleSheet("QDialog{background:white;color:#20283b;} QLabel{color:#34405b;} QLineEdit,QSpinBox{background:white;border:1px solid #dce1eb;border-radius:6px;padding:8px;} QPushButton{padding:8px 14px;border-radius:6px;}");
    auto *layout=new QVBoxLayout(&dialog); auto *form=new QFormLayout();
    auto *number=new QLineEdit(currentNumber); number->setPlaceholderText("e.g. B-204");
    auto *rows=new QSpinBox(); rows->setRange(1,100000); rows->setValue(currentRows);
    auto *columns=new QSpinBox(); columns->setRange(1,100000); columns->setValue(currentColumns);
    auto *capacity=new QLabel(); capacity->setStyleSheet("font-weight:700;color:#3155b7;");
    auto updateCapacity=[rows,columns,capacity](){capacity->setText(QString("%1 seats").arg(qint64(rows->value())*columns->value()));};
    connect(rows,&QSpinBox::valueChanged,&dialog,[updateCapacity](int){updateCapacity();});
    connect(columns,&QSpinBox::valueChanged,&dialog,[updateCapacity](int){updateCapacity();}); updateCapacity();
    form->addRow("Room number",number); form->addRow("Number of rows",rows); form->addRow("Number of columns",columns); form->addRow("Calculated capacity",capacity);
    layout->addLayout(form);
    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel); layout->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept); connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if(dialog.exec()!=QDialog::Accepted)return;
    qint64 cap=qint64(rows->value())*columns->value();
    if(number->text().trimmed().isEmpty()||cap>100000){QMessageBox::warning(this,"Invalid room","Enter a room number and a layout with 1 to 100000 seats.");return;}
    Room room; room.id=roomId; room.roomNumber=number->text().trimmed(); room.rows=rows->value(); room.columns=columns->value();
    bool ok=editing?m_repository->updateRoom(room):m_repository->addRoom(room);
    if(!ok){QMessageBox::warning(this,"Could not save room",m_repository->lastError().isEmpty()?"Check the room details and try again.":m_repository->lastError());return;}
    loadRooms();
}

void RoomsPage::deleteSelectedRoom()
{
    int id=selectedRoomId(); if(id==0)return;
    int answer=QMessageBox::warning(this,"Delete room","Delete this room? Any seat allocations associated with it may also be removed.",QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel);
    if(answer!=QMessageBox::Yes)return;
    if(!m_repository->deleteRoom(id)){QMessageBox::warning(this,"Could not delete room",m_repository->lastError());return;}
    loadRooms();
}
