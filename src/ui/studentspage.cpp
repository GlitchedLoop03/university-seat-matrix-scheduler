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

StudentsPage::StudentsPage(
    StudentRepository *repository,
    QWidget *parent
)
    : QWidget(parent),
      m_repository(repository),
      m_table(nullptr)
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
        "font-size: 26px;"
        "font-weight: 600;"
        "color: #202938;"
    );

    auto *description = new QLabel(
        "Manage student records and registration details.",
        this
    );
    description->setStyleSheet(
        "font-size: 13px;"
        "color: #697386;"
    );

    auto *topRow = new QHBoxLayout();

    auto *addButton = new QPushButton("Add student", this);
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setStyleSheet(
        "QPushButton {"
        " background-color: #315EFB;"
        " color: white;"
        " border: none;"
        " border-radius: 6px;"
        " padding: 10px 16px;"
        " font-weight: 600;"
        "}"
        "QPushButton:hover {"
        " background-color: #254AD6;"
        "}"
    );

    topRow->addStretch();
    topRow->addWidget(addButton);

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
    m_table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
    );
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget {"
        " background: white;"
        " border: 1px solid #E3E7EE;"
        " border-radius: 8px;"
        " alternate-background-color: #F8FAFC;"
        "}"
        "QHeaderView::section {"
        " background: #F4F6FA;"
        " color: #586174;"
        " padding: 10px;"
        " border: none;"
        " border-bottom: 1px solid #E3E7EE;"
        " font-weight: 600;"
        "}"
        "QTableWidget::item {"
        " padding: 8px;"
        " border: none;"
        "}"
    );

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addLayout(topRow);
    layout->addWidget(m_table, 1);

    connect(
        addButton,
        &QPushButton::clicked,
        this,
        &StudentsPage::showAddStudentDialog
    );
}

void StudentsPage::loadStudents()
{
    m_table->setRowCount(0);

    if (!m_repository) {
        return;
    }

    const auto students = m_repository->getAllStudents();

    for (const auto &student : students) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(
            row, 0, new QTableWidgetItem(student.registrationNumber)
        );
        m_table->setItem(
            row, 1, new QTableWidgetItem(student.name)
        );
        m_table->setItem(
            row, 2, new QTableWidgetItem(student.department)
        );
    }

    if (!m_repository->lastError().isEmpty()) {
        QMessageBox::warning(
            this,
            "Could not load students",
            m_repository->lastError()
        );
    }
}

void StudentsPage::showAddStudentDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add student");
    dialog.setMinimumWidth(380);

    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
            color: #202938;
        }
        QLabel {
            color: #374151;
            background: transparent;
        }
        QLineEdit {
            background-color: #FFFFFF;
            color: #202938;
            border: 1px solid #D6DCE5;
            border-radius: 5px;
            padding: 8px;
            selection-background-color: #315EFB;
        }
        QLineEdit:focus {
            border: 1px solid #315EFB;
        }
        QPushButton {
            background-color: #F3F5F9;
            color: #273449;
            border: 1px solid #D6DCE5;
            border-radius: 5px;
            padding: 7px 14px;
        }
        QPushButton:hover {
            background-color: #E8EDF5;
        }
    )");


    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *registrationInput = new QLineEdit(&dialog);
    registrationInput->setPlaceholderText("e.g. CS2026001");

    auto *nameInput = new QLineEdit(&dialog);
    nameInput->setPlaceholderText("Full name");

    auto *departmentInput = new QLineEdit(&dialog);
    departmentInput->setPlaceholderText("e.g. Computer Science");

    form->addRow("Registration number", registrationInput);
    form->addRow("Name", nameInput);
    form->addRow("Department", departmentInput);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel,
        &dialog
    );

    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(
        buttons,
        &QDialogButtonBox::accepted,
        &dialog,
        &QDialog::accept
    );

    connect(
        buttons,
        &QDialogButtonBox::rejected,
        &dialog,
        &QDialog::reject
    );

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Student student;
    student.registrationNumber = registrationInput->text().trimmed();
    student.name = nameInput->text().trimmed();
    student.department = departmentInput->text().trimmed();

    if (student.registrationNumber.isEmpty()
        || student.name.isEmpty()
        || student.department.isEmpty()) {
        QMessageBox::warning(
            this,
            "Missing information",
            "Enter the registration number, name, and department."
        );
        return;
    }

    if (!m_repository || !m_repository->addStudent(student)) {
        QMessageBox::warning(
            this,
            "Could not save student",
            m_repository
                ? m_repository->lastError()
                : "The student repository is not available."
        );
        return;
    }

    loadStudents();
}
