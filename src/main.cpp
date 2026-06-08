#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QStyle>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Sistem Manajemen Restoran");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("UAS Terintegrasi");

    // ── Paksa style Fusion agar tidak ikut tema Windows ──────
    // Fusion adalah style bawaan Qt yang konsisten di semua OS/tema
    app.setStyle(QStyleFactory::create("Fusion"));

    // ── Set palette warna tetap (tidak terpengaruh light/dark) ─
    QPalette palette;

    // Warna dasar aplikasi
    palette.setColor(QPalette::Window,          QColor(245,245,245)); // background utama
    palette.setColor(QPalette::WindowText,      QColor( 33, 33, 33)); // teks utama
    palette.setColor(QPalette::Base,            QColor(255,255,255)); // background input/table
    palette.setColor(QPalette::AlternateBase,   QColor(240,244,255)); // baris selang-seling tabel
    palette.setColor(QPalette::Text,            QColor( 33, 33, 33)); // teks dalam input/table
    palette.setColor(QPalette::Button,          QColor(224,224,224)); // background button
    palette.setColor(QPalette::ButtonText,      QColor( 33, 33, 33)); // teks button
    palette.setColor(QPalette::BrightText,      QColor(255,255,255)); // teks kontras tinggi
    palette.setColor(QPalette::Highlight,       QColor( 25,118,210)); // warna seleksi (#1976D2)
    palette.setColor(QPalette::HighlightedText, QColor(255,255,255)); // teks saat diseleksi
    palette.setColor(QPalette::Link,            QColor( 25,118,210)); // hyperlink
    palette.setColor(QPalette::ToolTipBase,     QColor(255,253,231)); // tooltip background
    palette.setColor(QPalette::ToolTipText,     QColor( 33, 33, 33)); // tooltip teks
    palette.setColor(QPalette::PlaceholderText, QColor(158,158,158)); // placeholder input

    // Disabled state (abu-abu)
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(158,158,158));
    palette.setColor(QPalette::Disabled, QPalette::Text,       QColor(158,158,158));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(158,158,158));
    palette.setColor(QPalette::Disabled, QPalette::Base,       QColor(238,238,238));

    app.setPalette(palette);

    MainWindow window;
    window.show();

    return app.exec();
}
