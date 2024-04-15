#include <iostream>
#include <new>

#include <QApplication>
#include <QWindow>
#include <QString>
#include <QLabel>

#include <QQmlApplicationEngine>

#include <Backend.h>

int main (int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    Backend back(app);
    back.resize(1000, 600);
    back.show();
    return app.exec();
}