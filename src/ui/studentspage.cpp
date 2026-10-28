#include "studentspage.h"

#include "../database/studentrepository.h"

#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalBlocker>

StudentsPage::StudentsPage(StudentRepository *repository, QWidget *parent)
    : QWidget(parent),
      m_repository(repository),
      m_table(nullptr),
      m_search(nullptr),
      m_emptyState(nullptr),
      m_editButton(nullptr),
      m_deleteButton(nullptr)
{
    setupUi();
    loadStudents();
}

void StudentsPage::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(16);

    auto *title = new QLabel("Students", this);
    title->setStyleSheet(
        "font-size: 26px; font-weight: 600; color: #202938;"
    );

    auto *description = new QLabel(
        "Manage student records and registration details.", this);
    description->setStyleSheet("font-size: 13px; color: #697386;");

    auto *toolbar = new QHBoxLayout;
    toolbar->setSpacing(8);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("Search students...");
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(220);
    m_search->setStyleSheet(
        "QLineEdit { background: white; color: #202938;"
        " border: 1px solid #D6DCE5; border-radius: 6px;"
        " padding: 9px 11px; }"
        "QLineEdit:focus { border: 1px solid #315EFB; }"
    );

    auto *editButton = new QPushButton("Edit", this);
    editButton->setObjectName("studentEditButton");
    editButton->setEnabled(false);
    m_editButton = editButton;

    auto *deleteButton = new QPushButton("Delete", this);
    deleteButton->setObjectName("studentDeleteButton");
    deleteButton->setEnabled(false);
    m_deleteButton = deleteButton;

    auto *addButton = new QPushButton("Add student", this);
    addButton->setObjectName("studentAddButton");

    toolbar->addWidget(m_search, 1);
    toolbar->addWidget(editButton);
    toolbar->addWidget(deleteButton);
    toolbar->addWidget(addButton);

    const QString buttonBase =
        "QPushButton { background: #FFFFFF; color: #273449;"
        " border: 1px solid #D6DCE5; border-radius: 6px;"
        " padding: 9px 13px; font-weight: 600; }"
        "QPushButton:hover:enabled { background: #F1F4F8; }"
        "QPushButton:disabled { color: #A0A7B4;"
        " background: #F8FAFC; border-color: #E3E7EE; }";

    editButton->setStyleSheet(buttonBase);

    deleteButton->setStyleSheet(
        "QPushButton { background: #FFFFFF; color: #B42318;"
        " border: 1px solid #F0C7C3; border-radius: 6px;"
        " padding: 9px 13px; font-weight: 600; }"
        "QPushButton:hover:enabled { background: #FFF1F0; }"
        "QPushButton:disabled { color: #A0A7B4;"
        " background: #F8FAFC; border-color: #E3E7EE; }"
    );

    addButton->setStyleSheet(
        "QPushButton { background: #315EFB; color: white;"
        " border: none; border-radius: 6px;"
        " padding: 10px 15px; font-weight: 600; }"
        "QPushButton:hover { background: #254AD6; }"
    );

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels(
        {"Registration number", "Name", "Department"}
    );
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget { background: white; color: #202938;"
        " border: 1px solid #E3E7EE; border-radius: 8px;"
        " alternate-background-color: #F8FAFC; }"
        "QHeaderView::section { background: #F4F6FA;"
        " color: #586174; padding: 10px; border: none;"
        " border-bottom: 1px solid #E3E7EE; font-weight: 600; }"
        "QTableWidget::item { padding: 8px; border: none; }"
        "QTableWidget::item:selected { background: #EAF0FF;"
        " color: #202938; }"
    );

    m_emptyState = new QLabel(this);
    m_emptyState->setAlignment(Qt::AlignCenter);
    m_emptyState->setStyleSheet(
        "color: #697386; padding: 18px; background: #FFFFFF;"
        " border: 1px solid #E3E7EE; border-radius: 8px;"
    );
    m_emptyState->hide();

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addLayout(toolbar);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_emptyState);

    connect(m_search, &QLineEdit::textChanged,
            this, &StudentsPage::applySearch);

    connect(addButton, &QPushButton::clicked, this, [this]() {
        showStudentDialog(false);
    });

    connect(editButton, &QPushButton::clicked, this, [this]() {
        showStudentDialog(true);
    });

    connect(deleteButton, &QPushButton::clicked,
            this, &StudentsPage::deleteSelectedStudent);

    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &StudentsPage::updateActionButtons);
}

void StudentsPage::loadStudents()
{
    m_table->setRowCount(0);

    if (!m_repository) {
        m_emptyState->setText("The student database is unavailable.");
        m_emptyState->show();
        return;
    }

    const auto students = m_repository->getAllStudents();

    for (const auto &student : students) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto *registrationItem =
            new QTableWidgetItem(student.registrationNumber);
        registrationItem->setData(Qt::UserRole, student.id);

        m_table->setItem(row, 0, registrationItem);
        m_table->setItem(row, 1, new QTableWidgetItem(student.name));
        m_table->setItem(row, 2, new QTableWidgetItem(student.department));
    }

    applySearch();
    updateActionButtons();
}

void StudentsPage::applySearch()
{
    const QString query = m_search->text().trimmed();
    int visibleRows = 0;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QStringList fields;

        for (int column = 0; column < m_table->columnCount(); ++column) {
            auto *item = m_table->item(row, column);
            fields.append(item ? item->text() : QString());
        }

        const bool matches =
            query.isEmpty()
            || fields.join(' ').contains(query, Qt::CaseInsensitive);

        m_table->setRowHidden(row, !matches);

        if (matches) {
            ++visibleRows;
        }
    }

    if (m_table->rowCount() == 0) {
        m_emptyState->setText(
            "No students yet. Select “Add student” to create the first record."
        );
        m_emptyState->show();
        m_table->hide();
    } else if (visibleRows == 0) {
        m_emptyState->setText(
            "No students match your search. Try another name or registration number."
        );
        m_emptyState->show();
        m_table->show();
    } else {
        m_emptyState->hide();
        m_table->show();
    }

    updateActionButtons();
}

void StudentsPage::updateActionButtons()
{
    const int row = m_table->currentRow();
    const bool validSelection =
        row >= 0 && !m_table->isRowHidden(row);

    if (m_editButton) {
        m_editButton->setEnabled(validSelection);
    }

    if (m_deleteButton) {
        m_deleteButton->setEnabled(validSelection);
    }
}

void StudentsPage::showStudentDialog(bool editing)
{
    int row = m_table->currentRow();

    if (editing && (row < 0 || m_table->isRowHidden(row))) {
        return;
    }

    Student student;

    if (editing) {
        auto *item = m_table->item(row, 0);
        if (!item) {
            return;
        }

        student.id = item->data(Qt::UserRole).toInt();
        student.registrationNumber = item->text();
        student.name = m_table->item(row, 1)->text();
        student.department = m_table->item(row, 2)->text();
    }

    QDialog dialog(this);
    dialog.setWindowTitle(editing ? "Edit student" : "Add student");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(R"(
        QDialog { background: #FFFFFF; color: #202938; }
        QLabel { color: #374151; background: transparent; }
        QLineEdit {
            background: #FFFFFF; color: #202938;
            border: 1px solid #D6DCE5; border-radius: 5px;
            padding: 8px;
        }
        QLineEdit:focus { border: 1px solid #315EFB; }
        QPushButton {
            background: #F3F5F9; color: #273449;
            border: 1px solid #D6DCE5; border-radius: 5px;
            padding: 7px 14px;
        }
        QPushButton:hover { background: #E8EDF5; }
    )");

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;

    auto *registrationInput = new QLineEdit(student.registrationNumber, &dialog);
    registrationInput->setPlaceholderText("e.g. CS2026001");

    auto *nameInput = new QLineEdit(student.name, &dialog);
    nameInput->setPlaceholderText("Full name");

    auto *departmentInput = new QLineEdit(student.department, &dialog);
    departmentInput->setPlaceholderText("e.g. Computer Science");

    form->addRow("Registration number", registrationInput);
    form->addRow("Name", nameInput);
    form->addRow("Department", departmentInput);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);

    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected,
            &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    student.registrationNumber = registrationInput->text().trimmed();
    student.name = nameInput->text().trimmed();
    student.department = departmentInput->text().trimmed();

    if (student.registrationNumber.isEmpty()
        || student.name.isEmpty()
        || student.department.isEmpty()) {
        QMessageBox::warning(
            this, "Missing information",
            "Enter the registration number, name, and department.");
        return;
    }

    const bool saved = editing
        ? m_repository && m_repository->updateStudent(student)
        : m_repository && m_repository->addStudent(student);

    if (!saved) {
        QMessageBox::warning(
            this,
            editing ? "Could not update student" : "Could not save student",
            m_repository
                ? m_repository->lastError()
                : "The student repository is not available.");
        return;
    }

    loadStudents();
}

void StudentsPage::deleteSelectedStudent()
{
    const int row = m_table->currentRow();

    if (row < 0 || m_table->isRowHidden(row)) {
        return;
    }

    auto *item = m_table->item(row, 0);
    if (!item) {
        return;
    }

    const int studentId = item->data(Qt::UserRole).toInt();
    const QString registrationNumber = item->text();

    QMessageBox confirmation(this);
    confirmation.setWindowTitle("Delete student");
    confirmation.setIcon(QMessageBox::Warning);
    confirmation.setText("Delete student " + registrationNumber + "?");
    confirmation.setInformativeText(
        "This may also remove associated examination registrations "
        "and seat allocations.");
    confirmation.setStandardButtons(
        QMessageBox::Yes | QMessageBox::Cancel);
    confirmation.setDefaultButton(QMessageBox::Cancel);
    confirmation.setStyleSheet(R"(
        QMessageBox { background: #FFFFFF; color: #202938; }
        QLabel { color: #202938; background: transparent; }
        QPushButton {
            background: #F3F5F9; color: #273449;
            border: 1px solid #D6DCE5; border-radius: 5px;
            padding: 7px 16px; min-width: 70px;
        }
        QPushButton:hover { background: #E8EDF5; }
    )");

    if (confirmation.exec() != QMessageBox::Yes) {
        return;
    }

    if (!m_repository || !m_repository->deleteStudent(studentId)) {
        QMessageBox::warning(
            this, "Could not delete student",
            m_repository
                ? m_repository->lastError()
                : "The student repository is not available.");
        return;
    }

    loadStudents();
}
