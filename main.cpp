#include "src/database/database.h"
#include "src/database/studentrepository.h"
#include "src/database/roomrepository.h"
#include "src/database/examinationrepository.h"
#include "src/database/registrationrepository.h"
#include "src/ui/registrationspage.h"
#include "src/database/seatingplanrepository.h"
#include "src/ui/seatingplanspage.h"
#include "src/ui/examinationspage.h"
#include "src/ui/roomspage.h"
#include "src/ui/studentspage.h"
#include <QApplication>
#include <QColor>
#include <QStyleFactory>
#include <QPalette>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QWidget>

QWidget *createPage(const QString &title, const QString &description)
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 32);
    layout->setSpacing(12);

    auto *heading = new QLabel(title);
    heading->setObjectName("pageHeading");

    auto *subtitle = new QLabel(description);
    subtitle->setObjectName("pageDescription");
    subtitle->setWordWrap(true);

    layout->addWidget(heading);
    layout->addWidget(subtitle);
    layout->addSpacing(12);

    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("separator");
    layout->addWidget(separator);
    layout->addStretch();

    return page;
}

QWidget *createDashboardPage(Database &database)
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 32);
    layout->setSpacing(20);

    auto *heading = new QLabel("Dashboard");
    heading->setObjectName("pageHeading");

    auto *subtitle = new QLabel(
        "A central workspace for examination seating operations.");
    subtitle->setObjectName("pageDescription");

    layout->addWidget(heading);
    layout->addWidget(subtitle);

    auto *summaryLayout = new QHBoxLayout;
    summaryLayout->setSpacing(16);

    const QStringList summaryTitles = {
        "Students", "Rooms", "Examinations"
    };

    const QStringList summaryDescriptions = {
        "Registered university students",
        "Available examination rooms",
        "Scheduled examination sessions"
    };

    const QStringList summaryValues = {
        QString::number(database.countRows("students")),
        QString::number(database.countRows("rooms")),
        QString::number(database.countRows("examinations"))
    };

    for (int i = 0; i < summaryTitles.size(); ++i) {
        auto *section = new QFrame;
        section->setObjectName("summarySection");

        auto *sectionLayout = new QVBoxLayout(section);
        sectionLayout->setContentsMargins(18, 18, 18, 18);
        sectionLayout->setSpacing(8);

        auto *title = new QLabel(summaryTitles[i]);
        title->setObjectName("summaryTitle");

        auto *value = new QLabel(summaryValues[i]);
        value->setObjectName("summaryValue");
        value->setProperty(
            "tableName",
            QStringList{"students", "rooms", "examinations"}[i]);

        auto *description = new QLabel(summaryDescriptions[i]);
        description->setObjectName("summaryDescription");
        description->setWordWrap(true);

        sectionLayout->addWidget(title);
        sectionLayout->addWidget(value);
        sectionLayout->addWidget(description);
        sectionLayout->addStretch();

        summaryLayout->addWidget(section);
    }

    layout->addLayout(summaryLayout);

    auto *sectionHeading = new QLabel("Getting started");
    sectionHeading->setObjectName("sectionHeading");
    layout->addWidget(sectionHeading);

    auto *content = new QFrame;
    content->setObjectName("contentSection");

    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(20, 18, 20, 18);
    contentLayout->setSpacing(10);

    const QStringList tasks = {
        "01   Add student records",
        "02   Define examination rooms and capacities",
        "03   Create examinations and register students",
        "04   Generate and review a seating plan"
    };

    for (const QString &task : tasks) {
        auto *item = new QLabel(task);
        item->setObjectName("taskItem");
        contentLayout->addWidget(item);
    }

    layout->addWidget(content);
    layout->addStretch();

    return page;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Use a consistent light palette for Qt dialogs and standard controls.
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor("#FFFFFF"));
    lightPalette.setColor(QPalette::WindowText, QColor("#202938"));
    lightPalette.setColor(QPalette::Base, QColor("#FFFFFF"));
    lightPalette.setColor(QPalette::AlternateBase, QColor("#F5F7FA"));
    lightPalette.setColor(QPalette::Text, QColor("#202938"));
    lightPalette.setColor(QPalette::Button, QColor("#F3F5F9"));
    lightPalette.setColor(QPalette::ButtonText, QColor("#273449"));
    lightPalette.setColor(QPalette::Highlight, QColor("#DCE7FF"));
    lightPalette.setColor(QPalette::HighlightedText, QColor("#172554"));
    lightPalette.setColor(QPalette::Disabled, QPalette::Text,
                          QColor("#929AAA"));
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                          QColor("#929AAA"));
    app.setPalette(lightPalette);

    app.setApplicationName("University Seat Matrix Scheduler");
    app.setOrganizationName("UniversitySeatScheduler");

    const QString dataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (dataDirectory.isEmpty() || !QDir().mkpath(dataDirectory)) {
        QMessageBox::critical(
            nullptr,
            "Application startup failed",
            "The application data directory could not be created.");
        return 1;
    }

    Database database;

    const QString databasePath =
        dataDirectory + "/university.db";

    if (!database.open(databasePath)) {
        QMessageBox::critical(
            nullptr,
            "Database error",
            "Could not open the database:\n" + database.lastError());
        return 1;
    }

    if (!database.initializeSchema()) {
        QMessageBox::critical(
            nullptr,
            "Database error",
            "Could not initialize the database:\n" + database.lastError());
        return 1;
    }

    StudentRepository studentRepository(database.handle());
    RoomRepository roomRepository(database.handle());
    ExaminationRepository examinationRepository(database.handle());
    RegistrationRepository registrationRepository(database.handle());
    SeatingPlanRepository seatingPlanRepository(database.handle());

    QWidget window;
    window.setObjectName("mainWindow");
    window.setWindowTitle("University Seat Matrix Scheduler");
    window.resize(1100, 720);
    window.setMinimumSize(700, 480);

    auto *root = new QHBoxLayout(&window);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(190);

    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(20, 24, 20, 20);
    sidebarLayout->setSpacing(8);

    auto *brand = new QLabel("SEAT MATRIX");
    brand->setObjectName("brand");

    auto *brandSubtitle = new QLabel("EXAMINATION OFFICE");
    brandSubtitle->setObjectName("brandSubtitle");

    sidebarLayout->addWidget(brand);
    sidebarLayout->addWidget(brandSubtitle);
    sidebarLayout->addSpacing(28);

    auto *navLabel = new QLabel("WORKSPACE");
    navLabel->setObjectName("sectionLabel");
    sidebarLayout->addWidget(navLabel);
    sidebarLayout->addSpacing(4);

    auto *pages = new QStackedWidget;

    const QStringList pageTitles = {
        "Dashboard", "Students", "Rooms", "Examinations", "Registrations", "Seating Plans"
    };

    const QStringList pageDescriptions = {
        "Overview of university examination seating.",
        "Manage student records and registration numbers.",
        "Manage examination rooms and seating capacities.",
        "Create and manage scheduled examinations.",
        "Register students for examination sessions.",
        "Generate and inspect room-wise seating allocations."
    };

    auto *header = new QWidget;
    header->setObjectName("topHeader");
    header->setFixedHeight(64);

    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(32, 0, 32, 0);

    auto *headerTitle = new QLabel("University Administration");
    headerTitle->setObjectName("headerTitle");

    auto *headerContext = new QLabel("SEATING MANAGEMENT");
    headerContext->setObjectName("headerContext");

    headerLayout->addWidget(headerTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(headerContext);

    auto *workspace = new QWidget;
    auto *workspaceLayout = new QVBoxLayout(workspace);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(0);
    workspaceLayout->addWidget(header);
    workspaceLayout->addWidget(pages, 1);

    QList<QPushButton *> navigationButtons;

    for (int i = 0; i < pageTitles.size(); ++i) {
        auto *button = new QPushButton(pageTitles[i]);
        button->setObjectName("navButton");
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);

        sidebarLayout->addWidget(button);
        navigationButtons.append(button);

        if (i == 0) {
            pages->addWidget(createDashboardPage(database));
        } else if (i == 1) {
            pages->addWidget(new StudentsPage(&studentRepository));
        } else if (i == 2) {
            pages->addWidget(new RoomsPage(&roomRepository));
        } else if (i == 3) {
            pages->addWidget(new ExaminationsPage(&examinationRepository));
        } else if (i == 4) {
            pages->addWidget(new RegistrationsPage(&registrationRepository));
        } else if (i == 5) {
            pages->addWidget(new SeatingPlansPage(&seatingPlanRepository));
        } else {
            pages->addWidget(createPage(pageTitles[i], pageDescriptions[i]));
        }

        QObject::connect(button, &QPushButton::clicked, &window,
                         [pages, i]() {
            pages->setCurrentIndex(i);
        });
    }

    QObject::connect(pages, &QStackedWidget::currentChanged, &window,
                     [&database, pages](int index) {
        if (index != 0)
            return;

        const auto values = pages->widget(0)->findChildren<QLabel *>(
            "summaryValue");
        for (QLabel *value : values) {
            const QString tableName = value->property("tableName").toString();
            if (!tableName.isEmpty())
                value->setText(QString::number(database.countRows(tableName)));
        }
    });

    QObject::connect(pages, &QStackedWidget::currentChanged, &window,
                     [pages](int index) {
        if (index == 4) {
            auto *registrationPage =
                qobject_cast<RegistrationsPage *>(pages->widget(4));
            if (registrationPage)
                registrationPage->refresh();
        }
    });
QObject::connect(pages, &QStackedWidget::currentChanged, &window,
                 [pages](int index) {
    if (index == 5) {
        auto *seatingPage =
            qobject_cast<SeatingPlansPage *>(pages->widget(5));
        if (seatingPage)
            seatingPage->refresh();
    }
});

    sidebarLayout->addStretch();

    auto *footer = new QLabel("UNIVERSITY SEAT SCHEDULER");
    footer->setObjectName("sidebarFooter");
    sidebarLayout->addWidget(footer);

    for (int i = 0; i < navigationButtons.size(); ++i) {
        navigationButtons[i]->setChecked(i == 0);
    }

    root->addWidget(sidebar);
    root->addWidget(workspace, 1);

    window.setStyleSheet(R"(
        QWidget {
            font-family: "Noto Sans", "DejaVu Sans", sans-serif;
            font-size: 13px;
            color: #202938;
        }

        QWidget#mainWindow { background: #F5F7FA; }

        QWidget#sidebar {
            background: #FFFFFF;
            border-right: 1px solid #E2E7EE;
        }

        QLabel#brand {
            font-size: 17px;
            font-weight: 700;
            letter-spacing: 1px;
            color: #1F3558;
        }

        QLabel#brandSubtitle {
            font-size: 10px;
            font-weight: 600;
            color: #7B8798;
            letter-spacing: 1px;
        }

        QLabel#sectionLabel {
            font-size: 10px;
            font-weight: 700;
            color: #8994A5;
            letter-spacing: 1px;
        }

        QPushButton#navButton {
            text-align: left;
            padding: 11px 12px;
            border: none;
            border-radius: 4px;
            background: transparent;
            color: #596579;
            font-weight: 500;
        }

        QPushButton#navButton:hover:!checked {
            background: #F1F4F8;
            color: #1F3558;
        }

        QPushButton#navButton:checked,
        QPushButton#navButton:checked:hover {
            background: #EAF0F8;
            color: #234875;
            font-weight: 700;
        }

        QPushButton#navButton:focus {
            outline: none;
            border: none;
        }

        QWidget#topHeader {
            background: #FFFFFF;
            border-bottom: 1px solid #E2E7EE;
        }

        QLabel#headerTitle {
            font-size: 13px;
            font-weight: 600;
            color: #273448;
        }

        QLabel#headerContext {
            font-size: 10px;
            font-weight: 600;
            color: #8994A5;
            letter-spacing: 1px;
        }

        QLabel#pageHeading {
            font-size: 26px;
            font-weight: 700;
            color: #1F2D40;
        }

        QLabel#pageDescription {
            font-size: 13px;
            color: #687589;
        }

        QLabel#summaryTitle,
        QLabel#sectionHeading {
            font-size: 14px;
            font-weight: 700;
            color: #26364B;
        }

        QLabel#summaryValue {
            font-size: 16px;
            font-weight: 600;
            color: #52657D;
        }

        QLabel#summaryDescription {
            font-size: 11px;
            color: #778397;
        }

        QFrame#summarySection,
        QFrame#contentSection {
            background: #FFFFFF;
            border: 1px solid #E2E7EE;
            border-radius: 5px;
        }

        QLabel#taskItem {
            padding: 7px 0;
            color: #4C5B70;
        }

        QFrame#separator { color: #E2E7EE; }

        QLabel#sidebarFooter {
            font-size: 9px;
            color: #9AA4B2;
            letter-spacing: 0.5px;
        }

    )");

    window.show();
    return app.exec();
}
