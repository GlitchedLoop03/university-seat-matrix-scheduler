#include "examinationspage.h"

#include <QAbstractItemView>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimeEdit>
#include <QVBoxLayout>

ExaminationsPage::ExaminationsPage(ExaminationRepository *repository,
                                   QWidget *parent)
    : QWidget(parent),
      m_repository(repository),
      m_searchInput(nullptr),
      m_table(nullptr),
      m_emptyLabel(nullptr),
      m_editButton(nullptr),
      m_deleteButton(nullptr)
{
    setupUi();
    loadExaminations();
}

void ExaminationsPage::setupUi()
{
    setObjectName("examinationsPage");
    setStyleSheet(
        "QWidget#examinationsPage { background: #f7f8fc; color: #20283b; }"
        "QLabel#pageTitle { font-size: 25px; font-weight: 700; color: #20283b; }"
        "QLabel#pageSubtitle { color: #758097; font-size: 13px; }"
        "QLineEdit, QDateEdit, QTimeEdit { background: white;"
        " border: 1px solid #dce1eb; border-radius: 7px;"
        " padding: 9px; color: #20283b; }"
        "QLineEdit:focus, QDateEdit:focus, QTimeEdit:focus {"
        " border: 1px solid #7188e8; }"
        "QPushButton { border: none; border-radius: 7px;"
        " padding: 9px 15px; font-weight: 600; }"
        "QPushButton#primaryButton { background: #526de5; color: white; }"
        "QPushButton#primaryButton:hover { background: #405bd0; }"
        "QPushButton#secondaryButton { background: white; color: #34405b;"
        " border: 1px solid #dce1eb; }"
        "QPushButton#secondaryButton:hover { background: #f0f3fa; }"
        "QPushButton:disabled { color: #a6adbc; background: #edf0f5; }"
        "QTableWidget { background: white; border: 1px solid #e3e7ef;"
        " border-radius: 9px; gridline-color: #edf0f5;"
        " selection-background-color: #e9edff; selection-color: #20283b; }"
        "QHeaderView::section { background: #f5f7fb; color: #68738b;"
        " padding: 11px; border: none; border-bottom: 1px solid #e3e7ef;"
        " font-weight: 600; }"
    );

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 25, 28, 28);
    root->setSpacing(16);

    auto *title = new QLabel("Examinations");
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *subtitle = new QLabel(
        "Schedule examination sessions by subject, date, and time.");
    subtitle->setObjectName("pageSubtitle");
    root->addWidget(subtitle);

    auto *toolbar = new QHBoxLayout();
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search subject, date, or time...");
    m_searchInput->setClearButtonEnabled(true);
    toolbar->addWidget(m_searchInput, 1);

    m_editButton = new QPushButton("Edit examination");
    m_editButton->setObjectName("secondaryButton");
    toolbar->addWidget(m_editButton);

    m_deleteButton = new QPushButton("Delete examination");
    m_deleteButton->setObjectName("secondaryButton");
    toolbar->addWidget(m_deleteButton);

    auto *addButton = new QPushButton("+ Add examination");
    addButton->setObjectName("primaryButton");
    toolbar->addWidget(addButton);
    root->addLayout(toolbar);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"Subject", "Date", "Time"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(43);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    root->addWidget(m_table, 1);

    m_emptyLabel = new QLabel("No examinations scheduled yet.");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #758097; padding: 12px;");
    root->addWidget(m_emptyLabel);

    connect(addButton, &QPushButton::clicked, this,
            [this]() { showExaminationDialog(false); });
    connect(m_editButton, &QPushButton::clicked, this,
            [this]() { showExaminationDialog(true); });
    connect(m_deleteButton, &QPushButton::clicked, this,
            [this]() { deleteSelectedExamination(); });
    connect(m_searchInput, &QLineEdit::textChanged, this,
            [this]() { applySearch(); });
    connect(m_table, &QTableWidget::itemSelectionChanged, this,
            [this]() { updateActionButtons(); });
    connect(m_table, &QTableWidget::itemDoubleClicked, this,
            [this]() { showExaminationDialog(true); });
}

void ExaminationsPage::loadExaminations()
{
    const QVector<Examination> examinations =
        m_repository->getAllExaminations();

    m_table->setRowCount(0);

    for (const Examination &examination : examinations) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto *subjectItem = new QTableWidgetItem(examination.subject);
        subjectItem->setData(Qt::UserRole, examination.id);
        m_table->setItem(row, 0, subjectItem);
        m_table->setItem(row, 1, new QTableWidgetItem(examination.examDate));
        m_table->setItem(row, 2, new QTableWidgetItem(examination.examTime));
    }

    m_emptyLabel->setText(examinations.isEmpty()
        ? "No examinations scheduled yet. Add your first examination."
        : "No examinations match your search.");

    applySearch();
}

void ExaminationsPage::applySearch()
{
    const QString query = m_searchInput->text().trimmed();
    bool anyVisible = false;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool matches = false;

        for (int column = 0; column < m_table->columnCount(); ++column) {
            if (m_table->item(row, column)->text().contains(
                    query, Qt::CaseInsensitive)) {
                matches = true;
                break;
            }
        }

        m_table->setRowHidden(row, !matches);
        anyVisible = anyVisible || matches;
    }

    m_emptyLabel->setVisible(m_table->rowCount() == 0 || !anyVisible);
    updateActionButtons();
}

int ExaminationsPage::selectedExaminationId() const
{
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0) || m_table->isRowHidden(row))
        return 0;

    return m_table->item(row, 0)->data(Qt::UserRole).toInt();
}

void ExaminationsPage::updateActionButtons()
{
    const bool selected = selectedExaminationId() > 0;
    m_editButton->setEnabled(selected);
    m_deleteButton->setEnabled(selected);
}

void ExaminationsPage::showExaminationDialog(bool editing)
{
    const int examinationId = editing ? selectedExaminationId() : 0;
    if (editing && examinationId == 0)
        return;

    QString currentSubject;
    QDate currentDate = QDate::currentDate();
    QTime currentTime(9, 0);

    if (editing) {
        const int row = m_table->currentRow();
        currentSubject = m_table->item(row, 0)->text();
        currentDate = QDate::fromString(m_table->item(row, 1)->text(),
                                        "yyyy-MM-dd");
        currentTime = QTime::fromString(m_table->item(row, 2)->text(),
                                        "HH:mm");
        if (!currentDate.isValid())
            currentDate = QDate::currentDate();
        if (!currentTime.isValid())
            currentTime = QTime(9, 0);
    }

    QDialog dialog(this);
    dialog.setWindowTitle(editing ? "Edit examination" : "Add examination");
    dialog.setMinimumWidth(440);
    dialog.setStyleSheet(
        "QDialog { background: white; color: #20283b; }"
        "QLabel { color: #34405b; }"
        "QLineEdit, QDateEdit, QTimeEdit { background: white;"
        " border: 1px solid #dce1eb; border-radius: 6px; padding: 8px; }"
        "QPushButton { padding: 8px 14px; border-radius: 6px; }"
    );

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *subjectInput = new QLineEdit(currentSubject);
    subjectInput->setPlaceholderText("e.g. Data Structures");

    auto *dateInput = new QDateEdit(currentDate);
    dateInput->setCalendarPopup(true);
    dateInput->setDisplayFormat("yyyy-MM-dd");

    auto *timeInput = new QTimeEdit(currentTime);
    timeInput->setDisplayFormat("HH:mm");
    timeInput->setTimeRange(QTime(0, 0), QTime(23, 59));

    form->addRow("Subject", subjectInput);
    form->addRow("Examination date", dateInput);
    form->addRow("Start time", timeInput);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    Examination examination;
    examination.id = examinationId;
    examination.subject = subjectInput->text().trimmed();
    examination.examDate = dateInput->date().toString("yyyy-MM-dd");
    examination.examTime = timeInput->time().toString("HH:mm");

    if (examination.subject.isEmpty()) {
        QMessageBox::warning(&dialog, "Subject required",
                             "Enter a subject for this examination.");
        return;
    }

    const bool success = editing
        ? m_repository->updateExamination(examination)
        : m_repository->addExamination(examination);

    if (!success) {
        QMessageBox::warning(
            this, "Could not save examination",
            m_repository->lastError().isEmpty()
                ? "Check the examination details and try again."
                : m_repository->lastError());
        return;
    }

    loadExaminations();
}

void ExaminationsPage::deleteSelectedExamination()
{
    const int examinationId = selectedExaminationId();
    if (examinationId == 0)
        return;

    const int answer = QMessageBox::warning(
        this, "Delete examination",
        "Delete this examination? Its registrations and seat allocations may also be removed.",
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if (answer != QMessageBox::Yes)
        return;

    if (!m_repository->deleteExamination(examinationId)) {
        QMessageBox::warning(this, "Could not delete examination",
                             m_repository->lastError());
        return;
    }

    loadExaminations();
}
