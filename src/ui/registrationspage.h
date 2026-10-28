#ifndef REGISTRATIONSPAGE_H
#define REGISTRATIONSPAGE_H

#include <QWidget>
#include "../database/registrationrepository.h"

class QComboBox;
class QTableWidget;
class QLabel;
class QPushButton;

class RegistrationsPage : public QWidget {
    Q_OBJECT

public:
    explicit RegistrationsPage(RegistrationRepository *repository,
                               QWidget *parent = nullptr);

private:
    RegistrationRepository *m_repository;
    QComboBox *m_examinationCombo;
    QTableWidget *m_table;
    QLabel *m_countLabel;
    QLabel *m_emptyLabel;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;

    void setupUi();
    void loadExaminations();
    void loadRegistrations();
    void updateActionButtons();
    int selectedStudentId() const;
    int selectedExaminationId() const;
    void addRegistration();
    void removeRegistration();
};

#endif
