#include <QApplication>
#include <QLabel>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("University Seat Matrix Scheduler");
    window.resize(800, 500);

    QLabel message("University Seat Matrix Scheduler", &window);
    message.setAlignment(Qt::AlignCenter);
    message.setGeometry(0, 0, 800, 500);

    window.show();

    return app.exec();
}
