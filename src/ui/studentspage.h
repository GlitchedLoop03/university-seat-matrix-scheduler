#ifndef STUDENTSPAGE_H
#define STUDENTSPAGE_H

#include <QWidget>

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
    void showAddStudentDialog();
    void deleteSelectedStudent();

    StudentRepository *m_repository;
    QTableWidget *m_table;
};

#endif // STUDENTSPAGE_H
