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

SeatingPlansPage::SeatingPlansPage(
    SeatingPlanRepository *repository, QWidget *parent)
    : QWidget(parent), m_repository(repository)
{
    setStyleSheet(R"(
        QWidget { background: #f5f7fb; color: #202938; font-size: 13px; }
        QLabel#pageTitle { font-size: 24px; font-weight: 700; }
        QLabel#muted { color: #687386; }
        QComboBox {
            background: white; border: 1px solid #d7deea;
            border-radius: 7px; padding: 9px;
        }
        QPushButton {
            background: white; border: 1px solid #d7deea;
            border-radius: 7px; padding: 9px 14px; font-weight: 600;
        }
        QPushButton#primary { background: #315efb; color: white; border: none; }
        QTableWidget {
            background: white; color: #202938;
            alternate-background-color: #f7f9fc;
            border: 1px solid #e1e6ef; gridline-color: #edf0f5;
            selection-background-color: #dce7ff;
            selection-color: #172554;
        }
        QTableWidget::item { background: white; color: #202938; padding: 5px; }
        QTableWidget::item:alternate { background: #f7f9fc; color: #202938; }
        QTableWidget::item:selected { background: #dce7ff; color: #172554; }
        QHeaderView::section {
            background: #f8faff; color: #596579;
            border: none; border-bottom: 1px solid #e1e6ef;
            padding: 11px; font-weight: 700;
        }
    )");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(30, 26, 30, 30);
    root->setSpacing(18);

    auto *title = new QLabel("Seating Plans");
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *subtitle = new QLabel(
        "Generate room-wise seating allocations for registered students.");
    subtitle->setObjectName("muted");
    root->addWidget(subtitle);

    auto *selectionRow = new QHBoxLayout();
    selectionRow->addWidget(new QLabel("Examination"));

    m_examinationCombo = new QComboBox();
    m_examinationCombo->setMinimumWidth(300);
    selectionRow->addWidget(m_examinationCombo, 1);

    m_generateButton = new QPushButton("Generate / Regenerate Plan");
    m_generateButton->setObjectName("primary");
    selectionRow->addWidget(m_generateButton);
    root->addLayout(selectionRow);

    m_summaryLabel = new QLabel("No allocations loaded.");
    m_summaryLabel->setObjectName("muted");
    root->addWidget(m_summaryLabel);

    m_table = new QTableWidget();
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(
        {"Room", "Seat Number", "Registration Number", "Student Name", "Department"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    root->addWidget(m_table, 1);

    m_emptyLabel = new QLabel(
        "No seating plan exists for this examination. Generate one to begin.");
    m_emptyLabel->setObjectName("muted");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_emptyLabel);

    connect(m_examinationCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { loadAllocations(); });
    connect(m_generateButton, &QPushButton::clicked,
            this, &SeatingPlansPage::generatePlan);

    loadExaminations();
}

int SeatingPlansPage::selectedExaminationId() const
{
    return m_examinationCombo->currentData().toInt();
}

void SeatingPlansPage::loadExaminations()
{
    m_examinationCombo->clear();

    const auto examinations = m_repository->getExaminations();
    for (const auto &exam : examinations)
        m_examinationCombo->addItem(exam.label, exam.id);

    const bool available = m_examinationCombo->count() > 0;
    m_examinationCombo->setEnabled(available);
    m_generateButton->setEnabled(available);

    if (!available)
        m_summaryLabel->setText("Create an examination before generating a plan.");

    loadAllocations();
}

void SeatingPlansPage::loadAllocations()
{
    m_table->setRowCount(0);

    const int examId = selectedExaminationId();
    if (examId <= 0) {
        m_emptyLabel->show();
        m_summaryLabel->setText("No examination selected.");
        return;
    }

    const auto allocations = m_repository->getAllocations(examId);

    for (const auto &allocation : allocations) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(allocation.roomNumber));
        m_table->setItem(row, 1,
                         new QTableWidgetItem(QString::number(allocation.seatNumber)));
        m_table->setItem(row, 2,
                         new QTableWidgetItem(allocation.registrationNumber));
        m_table->setItem(row, 3, new QTableWidgetItem(allocation.studentName));
        m_table->setItem(row, 4, new QTableWidgetItem(allocation.department));
    }

    m_summaryLabel->setText(
        QString("%1 student allocation%2")
            .arg(allocations.size())
            .arg(allocations.size() == 1 ? "" : "s"));

    m_emptyLabel->setVisible(allocations.isEmpty());
}

void SeatingPlansPage::generatePlan()
{
    const int examId = selectedExaminationId();
    if (examId <= 0)
        return;

    const auto answer = QMessageBox::question(
        this, "Generate Seating Plan",
        "Generate or regenerate the seating plan for this examination?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    if (!m_repository->generatePlan(examId)) {
        QMessageBox::warning(this, "Could Not Generate Plan",
                             m_repository->lastError());
        return;
    }

    loadAllocations();
    QMessageBox::information(
        this, "Plan Generated",
        "The seating plan was generated successfully.");
}
