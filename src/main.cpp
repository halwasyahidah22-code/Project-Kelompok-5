#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Sistem Manajemen Restoran");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("UAS Terintegrasi");

    MainWindow window;
    window.show();

    return app.exec();
}
