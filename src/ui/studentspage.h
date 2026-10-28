#ifndef STUDENTSPAGE_H
#define STUDENTSPAGE_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QTableWidget;
class StudentRepository;

class StudentsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StudentsPage(
        StudentRepository *repository,
        QWidget *parent = nullptr
    );

private:
    void setupUi();
    void loadStudents();
    void applySearch();
    void showStudentDialog(bool editing);
    void deleteSelectedStudent();
    void updateActionButtons();

    StudentRepository *m_repository;
    QTableWidget *m_table;
    QLineEdit *m_search;
    QLabel *m_emptyState;
    QWidget *m_editButton;
    QWidget *m_deleteButton;
};

#endif // STUDENTSPAGE_H
