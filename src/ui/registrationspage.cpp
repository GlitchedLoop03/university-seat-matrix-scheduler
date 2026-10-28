#include "registrationspage.h"

#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QTableWidgetItem>

RegistrationsPage::RegistrationsPage(
    RegistrationRepository *repository, QWidget *parent)
    : QWidget(parent), m_repository(repository)
{
    setupUi();
    loadExaminations();
}

void RegistrationsPage::setupUi()
{
    setStyleSheet(R"(
        QWidget { background: #f5f7fb; color: #202938; font-size: 13px; }
        QLabel#pageTitle { font-size: 24px; font-weight: 700; }
        QLabel#muted { color: #687386; }
        QComboBox, QLineEdit {
            background: white; border: 1px solid #d7deea;
            border-radius: 7px; padding: 9px;
        }
        QPushButton {
            background: white; border: 1px solid #d7deea;
            border-radius: 7px; padding: 9px 14px; font-weight: 600;
        }
        QPushButton:hover { background: #edf2f9; }
        QPushButton#primary { background: #315efb; color: white; border: none; }
        QPushButton#primary:hover { background: #244bd4; }
        QPushButton#danger { color: #c53030; }
        QTableWidget {
            alternate-background-color: #f7f9fc;
            color: #202938;
            background: white; border: 1px solid #e1e6ef;
            border-radius: 9px; gridline-color: #edf0f5;
            selection-background-color: #e5edff;
            selection-color: #172554;
        }
        QHeaderView::section {
            background: #f8faff; color: #596579;
            border: none; border-bottom: 1px solid #e1e6ef;
            padding: 11px; font-weight: 700;
        }
    )");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(30, 26, 30, 30);
    root->setSpacing(18);

    auto *title = new QLabel("Registration Management");
    title->setObjectName("pageTitle");
    root->addWidget(title);

    auto *subtitle = new QLabel(
        "Register existing students for examinations and manage their enrolments.");
    subtitle->setObjectName("muted");
    root->addWidget(subtitle);

    auto *examRow = new QHBoxLayout();
    auto *examLabel = new QLabel("Examination");
    m_examinationCombo = new QComboBox();
    m_examinationCombo->setMinimumWidth(280);
    examRow->addWidget(examLabel);
    examRow->addWidget(m_examinationCombo, 1);
    root->addLayout(examRow);

    auto *actions = new QHBoxLayout();
    m_countLabel = new QLabel("0 registered students");
    m_countLabel->setObjectName("muted");

    m_addButton = new QPushButton("Register Student");
    m_addButton->setObjectName("primary");

    m_removeButton = new QPushButton("Remove Registration");
    m_removeButton->setObjectName("danger");
    m_removeButton->setEnabled(false);

    actions->addWidget(m_countLabel);
    actions->addStretch();
    actions->addWidget(m_removeButton);
    actions->addWidget(m_addButton);
    root->addLayout(actions);

    m_table = new QTableWidget();
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels(
        {"Registration Number", "Student Name", "Department"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet("QTableWidget { background: white; color: #202938; }"" QTableWidget::item { background: white; color: #202938; padding: 5px; }"" QTableWidget::item:alternate { background: #f7f9fc; color: #202938; }"" QTableWidget::item:selected { background: #dce7ff; color: #172554; }");
    root->addWidget(m_table, 1);

    m_emptyLabel = new QLabel("No students are registered for this examination yet.");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setObjectName("muted");
    root->addWidget(m_emptyLabel);

    connect(m_examinationCombo, &QComboBox::currentIndexChanged,
            this, [this](int) { loadRegistrations(); });
    connect(m_addButton, &QPushButton::clicked,
            this, &RegistrationsPage::addRegistration);
    connect(m_removeButton, &QPushButton::clicked,
            this, &RegistrationsPage::removeRegistration);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &RegistrationsPage::updateActionButtons);
}

void RegistrationsPage::loadExaminations()
{
    m_examinationCombo->clear();

    const auto examinations = m_repository->getExaminations();
    for (const auto &exam : examinations)
        m_examinationCombo->addItem(exam.label, exam.id);

    const bool hasExaminations = m_examinationCombo->count() > 0;
    m_examinationCombo->setEnabled(hasExaminations);
    m_addButton->setEnabled(hasExaminations);

    if (!hasExaminations) {
        m_countLabel->setText("No examinations available");
        m_emptyLabel->setText("Create an examination before registering students.");
    }

    loadRegistrations();
}

void RegistrationsPage::loadRegistrations()
{
    m_table->setRowCount(0);

    const int examinationId = selectedExaminationId();
    if (examinationId <= 0) {
        m_countLabel->setText("0 registered students");
        m_emptyLabel->show();
        updateActionButtons();
        return;
    }

    const auto students = m_repository->getRegisteredStudents(examinationId);

    for (const auto &student : students) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto *registration = new QTableWidgetItem(student.registrationNumber);
        registration->setData(Qt::UserRole, student.id);
        m_table->setItem(row, 0, registration);
        m_table->setItem(row, 1, new QTableWidgetItem(student.name));
        m_table->setItem(row, 2, new QTableWidgetItem(student.department));
    }

    m_countLabel->setText(
        QString("%1 registered student%2")
            .arg(students.size())
            .arg(students.size() == 1 ? "" : "s"));

    m_emptyLabel->setVisible(students.isEmpty());
    updateActionButtons();
}

void RegistrationsPage::updateActionButtons()
{
    m_removeButton->setEnabled(
        selectedStudentId() > 0 && selectedExaminationId() > 0);
}

int RegistrationsPage::selectedStudentId() const
{
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0))
        return -1;

    return m_table->item(row, 0)->data(Qt::UserRole).toInt();
}

int RegistrationsPage::selectedExaminationId() const
{
    return m_examinationCombo->currentData().toInt();
}

void RegistrationsPage::addRegistration()
{
    const int examinationId = selectedExaminationId();
    if (examinationId <= 0)
        return;

    const auto available = m_repository->getAvailableStudents(examinationId);
    if (available.isEmpty()) {
        QMessageBox::information(
            this, "No Available Students",
            "All students are already registered for this examination, "
            "or no students have been added yet.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Register Student");
    dialog.setMinimumWidth(440);
    dialog.setStyleSheet(
        "QDialog { background: #f5f7fb; }"
        "QComboBox { background: white; padding: 8px; border: 1px solid #d7deea; }"
        "QPushButton { padding: 8px 14px; }");

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    auto *studentCombo = new QComboBox();

    for (const auto &student : available) {
        studentCombo->addItem(
            student.registrationNumber + " — " + student.name
            + " (" + student.department + ")", student.id);
    }

    form->addRow("Select student", studentCombo);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttons->button(QDialogButtonBox::Ok)->setText("Register");
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const int studentId = studentCombo->currentData().toInt();
    if (!m_repository->registerStudent(examinationId, studentId)) {
        QMessageBox::critical(this, "Registration Failed",
                              m_repository->lastError());
        return;
    }

    loadRegistrations();
}

void RegistrationsPage::removeRegistration()
{
    const int examinationId = selectedExaminationId();
    const int studentId = selectedStudentId();

    if (examinationId <= 0 || studentId <= 0)
        return;

    const auto answer = QMessageBox::question(
        this, "Remove Registration",
        "Remove this student's registration for the selected examination?\n\n"
        "The student record itself will not be deleted.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    if (!m_repository->unregisterStudent(examinationId, studentId)) {
        QMessageBox::critical(this, "Removal Failed",
                              m_repository->lastError());
        return;
    }

    loadRegistrations();
}
