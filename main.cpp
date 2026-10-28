#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
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

QWidget *createDashboardPage()
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
        "Students",
        "Rooms",
        "Examinations"
    };

    const QStringList summaryDescriptions = {
        "Registered university students",
        "Available examination rooms",
        "Scheduled examination sessions"
    };

    for (int i = 0; i < summaryTitles.size(); ++i) {
        auto *section = new QFrame;
        section->setObjectName("summarySection");

        auto *sectionLayout = new QVBoxLayout(section);
        sectionLayout->setContentsMargins(18, 18, 18, 18);
        sectionLayout->setSpacing(8);

        auto *title = new QLabel(summaryTitles[i]);
        title->setObjectName("summaryTitle");

        auto *value = new QLabel("Not connected");
        value->setObjectName("summaryValue");

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

    auto *activityTitle = new QLabel("Getting started");
    activityTitle->setObjectName("sectionHeading");
    layout->addWidget(activityTitle);

    auto *activity = new QFrame;
    activity->setObjectName("contentSection");

    auto *activityLayout = new QVBoxLayout(activity);
    activityLayout->setContentsMargins(20, 18, 20, 18);
    activityLayout->setSpacing(10);

    const QStringList tasks = {
        "01   Add student records",
        "02   Define examination rooms and capacities",
        "03   Create examinations and register students",
        "04   Generate and review a seating plan"
    };

    for (const QString &task : tasks) {
        auto *item = new QLabel(task);
        item->setObjectName("taskItem");
        activityLayout->addWidget(item);
    }

    layout->addWidget(activity);
    layout->addStretch();

    return page;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("University Seat Matrix Scheduler");

    QWidget window;
    window.setObjectName("mainWindow");
    window.setWindowTitle("University Seat Matrix Scheduler");
    window.resize(1100, 720);
    window.setMinimumSize(850, 560);

    auto *root = new QHBoxLayout(&window);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(220);

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
        "Dashboard",
        "Students",
        "Rooms",
        "Examinations",
        "Seating Plans"
    };

    const QStringList pageDescriptions = {
        "Overview of university examination seating.",
        "Manage student records and registration numbers.",
        "Manage examination rooms and seating capacities.",
        "Create and manage scheduled examinations.",
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
        button->setCursor(Qt::PointingHandCursor);

        sidebarLayout->addWidget(button);
        navigationButtons.append(button);

        if (i == 0) {
            pages->addWidget(createDashboardPage());
        } else {
            pages->addWidget(createPage(
                pageTitles[i], pageDescriptions[i]));
        }

        QObject::connect(button, &QPushButton::clicked, &window,
                         [pages, navigationButtons, i]() {
            pages->setCurrentIndex(i);

            for (int j = 0; j < navigationButtons.size(); ++j) {
                navigationButtons[j]->setChecked(j == i);
            }
        });
    }

    sidebarLayout->addStretch();

    auto *footer = new QLabel("UNIVERSITY SEAT SCHEDULER");
    footer->setObjectName("sidebarFooter");
    sidebarLayout->addWidget(footer);

    navigationButtons.first()->setChecked(true);

    root->addWidget(sidebar);
    root->addWidget(workspace, 1);

    window.setStyleSheet(R"(
        QWidget {
            font-family: "Noto Sans", "DejaVu Sans", sans-serif;
            font-size: 13px;
            color: #202938;
        }

        QWidget#mainWindow {
            background: #F5F7FA;
        }

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

        QPushButton#navButton:hover {
            background: #F1F4F8;
            color: #1F3558;
        }

        QPushButton#navButton:checked {
            background: #EAF0F8;
            color: #234875;
            font-weight: 700;
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

        QFrame#separator {
            color: #E2E7EE;
        }

        QLabel#sidebarFooter {
            font-size: 9px;
            color: #9AA4B2;
            letter-spacing: 0.5px;
        }
    )");

    window.show();
    return app.exec();
}
