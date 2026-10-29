#ifndef SEATINGPLANSPAGE_H
#define SEATINGPLANSPAGE_H
#include <QWidget>
#include "../database/seatingplanrepository.h"
class QComboBox;
class QTableWidget;
class QLabel;
class QPushButton;
class SeatingPlansPage : public QWidget {
    Q_OBJECT
public:
    explicit SeatingPlansPage(SeatingPlanRepository *repository, QWidget *parent = nullptr);
    void refresh();
private:
    SeatingPlanRepository *m_repository;
    QComboBox *m_examinationCombo;
    QComboBox *m_roomCombo;
    QTableWidget *m_table;
    QTableWidget *m_grid;
    QLabel *m_summaryLabel;
    QLabel *m_emptyLabel;
    QPushButton *m_generateButton;
    int selectedExaminationId() const;
    void loadExaminations();
    void loadAllocations();
    void renderRoomGrid();
    void generatePlan();
};
#endif
