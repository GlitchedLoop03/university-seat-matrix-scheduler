#include "seatingplanspage.h"
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QTableWidgetItem>
#include <QBrush>
#include <QColor>
#include <QMap>

SeatingPlansPage::SeatingPlansPage(SeatingPlanRepository *repository,QWidget *parent)
    :QWidget(parent),m_repository(repository),m_examinationCombo(nullptr),m_roomCombo(nullptr),
     m_table(nullptr),m_grid(nullptr),m_summaryLabel(nullptr),m_emptyLabel(nullptr),m_generateButton(nullptr)
{
    setStyleSheet(R"(
      QWidget{background:#f5f7fb;color:#202938;font-size:13px;}
      QLabel#pageTitle{font-size:24px;font-weight:700;} QLabel#muted{color:#687386;}
      QComboBox{background:white;border:1px solid #d7deea;border-radius:7px;padding:9px;}
      QPushButton{background:white;border:1px solid #d7deea;border-radius:7px;padding:9px 14px;font-weight:600;}
      QPushButton#primary{background:#315efb;color:white;border:none;}
      QTableWidget{background:white;color:#202938;border:1px solid #e1e6ef;gridline-color:#dce3ef;}
      QHeaderView::section{background:#f8faff;color:#596579;border:none;border-bottom:1px solid #e1e6ef;padding:8px;font-weight:700;}
    )");
    auto *root=new QVBoxLayout(this);root->setContentsMargins(30,26,30,30);root->setSpacing(12);
    auto *title=new QLabel("Combined Seating Plans");title->setObjectName("pageTitle");root->addWidget(title);
    auto *subtitle=new QLabel("Exams at the same date and time share one physical room layout. Subjects alternate by columns.");
    subtitle->setObjectName("muted");root->addWidget(subtitle);
    auto *selection=new QHBoxLayout();selection->addWidget(new QLabel("Examination / time slot"));
    m_examinationCombo=new QComboBox();m_examinationCombo->setMinimumWidth(250);selection->addWidget(m_examinationCombo,1);
    selection->addWidget(new QLabel("Room"));m_roomCombo=new QComboBox();m_roomCombo->setMinimumWidth(120);selection->addWidget(m_roomCombo);
    m_generateButton=new QPushButton("Generate / Regenerate Combined Plan");m_generateButton->setObjectName("primary");selection->addWidget(m_generateButton);root->addLayout(selection);
    m_summaryLabel=new QLabel("No allocations loaded.");m_summaryLabel->setObjectName("muted");root->addWidget(m_summaryLabel);
    auto *gridTitle=new QLabel("Visual seat map — each cell is one physical seat");gridTitle->setStyleSheet("font-weight:700;font-size:15px;");root->addWidget(gridTitle);
    m_grid=new QTableWidget();m_grid->setEditTriggers(QAbstractItemView::NoEditTriggers);m_grid->setSelectionMode(QAbstractItemView::NoSelection);
    m_grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);m_grid->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_grid->setMinimumHeight(220);root->addWidget(m_grid,2);
    m_emptyLabel=new QLabel("No combined plan exists for this time slot. Generate one to begin.");m_emptyLabel->setObjectName("muted");m_emptyLabel->setAlignment(Qt::AlignCenter);root->addWidget(m_emptyLabel);
    auto *detailTitle=new QLabel("Seat allocation details");detailTitle->setStyleSheet("font-weight:700;font-size:15px;");root->addWidget(detailTitle);
    m_table=new QTableWidget(0,6);m_table->setHorizontalHeaderLabels({"Room","Seat","Subject","Registration","Student","Department"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);m_table->verticalHeader()->setVisible(false);root->addWidget(m_table,2);
    connect(m_examinationCombo,&QComboBox::currentIndexChanged,this,[this](int){loadAllocations();});
    connect(m_roomCombo,&QComboBox::currentIndexChanged,this,[this](int){renderRoomGrid();});
    connect(m_generateButton,&QPushButton::clicked,this,&SeatingPlansPage::generatePlan);
    loadExaminations();
}

int SeatingPlansPage::selectedExaminationId() const { return m_examinationCombo?m_examinationCombo->currentData().toInt():0; }

void SeatingPlansPage::refresh()
{
    loadExaminations();
}

void SeatingPlansPage::loadExaminations()
{
    m_examinationCombo->clear();const auto exams=m_repository->getExaminations();
    for(const auto&e:exams)m_examinationCombo->addItem(e.label,e.id);
    bool available=m_examinationCombo->count()>0;m_examinationCombo->setEnabled(available);m_generateButton->setEnabled(available);
    if(!available)m_summaryLabel->setText("Create an examination before generating a plan.");
    loadAllocations();
}

void SeatingPlansPage::loadAllocations()
{
    m_table->setRowCount(0);m_roomCombo->clear();int examId=selectedExaminationId();
    if(examId<=0){m_emptyLabel->show();m_summaryLabel->setText("No examination selected.");m_grid->setRowCount(0);m_grid->setColumnCount(0);return;}
    const auto allocations=m_repository->getAllocations(examId);QMap<QString,bool> seenRooms;QMap<QString,int> counts;
    for(const auto&a:allocations){
        if(!seenRooms.contains(a.roomNumber)){seenRooms[a.roomNumber]=true;m_roomCombo->addItem(a.roomNumber,a.roomNumber);}
        counts[a.subject]++;
        int row=m_table->rowCount();m_table->insertRow(row);
        m_table->setItem(row,0,new QTableWidgetItem(a.roomNumber));m_table->setItem(row,1,new QTableWidgetItem(QString::number(a.seatNumber)));
        m_table->setItem(row,2,new QTableWidgetItem(a.subject));m_table->setItem(row,3,new QTableWidgetItem(a.registrationNumber));
        m_table->setItem(row,4,new QTableWidgetItem(a.studentName));m_table->setItem(row,5,new QTableWidgetItem(a.department));
    }
    QStringList parts;for(auto it=counts.cbegin();it!=counts.cend();++it)parts<<QString("%1: %2").arg(it.key()).arg(it.value());
    m_summaryLabel->setText(allocations.isEmpty()?"No combined plan saved for this time slot yet.":QString("%1 allocated students — %2").arg(allocations.size()).arg(parts.join(" | ")));
    m_emptyLabel->setVisible(allocations.isEmpty());renderRoomGrid();
}

void SeatingPlansPage::renderRoomGrid()
{
    const QString room=m_roomCombo->currentData().toString();const int examId=selectedExaminationId();
    m_grid->clear();m_grid->setRowCount(0);m_grid->setColumnCount(0);
    if(room.isEmpty()||examId<=0)return;
    const auto allocations=m_repository->getAllocations(examId);int rows=0,columns=0;QMap<int,SeatingAllocation> seats;
    for(const auto&a:allocations)if(a.roomNumber==room){rows=a.roomRows;columns=a.roomColumns;seats[a.seatNumber]=a;}
    if(rows<=0||columns<=0)return;
    m_grid->setRowCount(rows);m_grid->setColumnCount(columns);
    const int cellHeight = 76;
    const int gridHeight = qMin(520, 42 + rows * cellHeight);
    m_grid->setMinimumHeight(gridHeight);
    m_grid->setMaximumHeight(gridHeight);
    m_grid->verticalHeader()->setDefaultSectionSize(cellHeight);
    for(int r=0;r<rows;++r){
        m_grid->setVerticalHeaderItem(r,new QTableWidgetItem(QString::number(r+1)));
        for(int c=0;c<columns;++c){
            int seat=c*rows+r+1;auto *cell=new QTableWidgetItem(QString("Seat %1\nEMPTY").arg(seat));cell->setTextAlignment(Qt::AlignCenter);
            cell->setBackground(QBrush(QColor("#f1f5f9")));cell->setForeground(QBrush(QColor("#64748b")));
            auto it=seats.constFind(seat);
            if(it!=seats.cend()){
                const auto&a=it.value();cell->setText(QString("Seat %1\n%2\n%3\n%4").arg(seat).arg(a.subject,a.registrationNumber,a.studentName));
                cell->setBackground(QBrush(c%2==0?QColor("#dbeafe"):QColor("#dcfce7")));cell->setForeground(QBrush(QColor("#172033")));
                cell->setToolTip(QString("%1 | %2 | %3 | %4").arg(a.subject,a.registrationNumber,a.studentName,a.department));
            }
            m_grid->setItem(r,c,cell);
        }
    }
    for (int r = 0; r < rows; ++r)
        m_grid->setRowHeight(r, cellHeight);
    m_grid->horizontalHeader()->setMinimumSectionSize(105);
}

void SeatingPlansPage::generatePlan()
{
    int id=selectedExaminationId();if(id<=0)return;
    auto answer=QMessageBox::question(this,"Generate Combined Seating Plan",
      "Generate or regenerate the plan for every examination at this date and time? Existing plans for that time slot will be replaced.",
      QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
    if(answer!=QMessageBox::Yes)return;
    if(!m_repository->generatePlan(id)){QMessageBox::warning(this,"Could Not Generate Plan",m_repository->lastError());return;}
    loadAllocations();QMessageBox::information(this,"Combined Plan Generated","The combined seating plan was generated successfully.");
}
