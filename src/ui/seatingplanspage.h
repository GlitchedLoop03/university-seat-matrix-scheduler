#ifndef SEATINGPLANSPAGE_H
#define SEATINGPLANSPAGE_H

#include <QWidget>
#include "../database/seatingplanrepository.h"

class QComboBox;
class QTableWidget;
class QLabel;
class QPushButton;

class SeatingPlansPage : public QWidget
{
    Q_OBJECT

public:
    explicit SeatingPlansPage(SeatingPlanRepository *repository,
                              QWidget *parent = nullptr);

private:
    SeatingPlanRepository *m_repository;
    QComboBox *m_examinationCombo;
    QTableWidget *m_table;
    QLabel *m_summaryLabel;
    QLabel *m_emptyLabel;
    QPushButton *m_generateButton;

    int selectedExaminationId() const;
    void loadExaminations();
    void loadAllocations();
    void generatePlan();
};

#endif
