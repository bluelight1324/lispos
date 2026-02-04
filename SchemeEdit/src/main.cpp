#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("SchemeEdit");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LispOS");

    MainWindow window;
    window.show();

    return app.exec();
}
