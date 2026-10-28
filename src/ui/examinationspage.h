#ifndef EXAMINATIONSPAGE_H
#define EXAMINATIONSPAGE_H

#include "../database/examinationrepository.h"
#include <QWidget>

class QLineEdit;
class QTableWidget;
class QLabel;
class QPushButton;

class ExaminationsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExaminationsPage(ExaminationRepository *repository,
                              QWidget *parent = nullptr);

private:
    ExaminationRepository *m_repository;
    QLineEdit *m_searchInput;
    QTableWidget *m_table;
    QLabel *m_emptyLabel;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;

    void setupUi();
    void loadExaminations();
    void applySearch();
    void updateActionButtons();
    int selectedExaminationId() const;
    void showExaminationDialog(bool editing);
    void deleteSelectedExamination();
};

#endif
