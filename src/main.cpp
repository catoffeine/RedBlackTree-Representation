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

//    QQmlApplicationEngine engine;
//
//    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
//
//
//    QLabel label;
//    label.setText("TestLabel"); 
//    label.setAlignment(Qt::AlignBottom);
    
//    QObject *item = engine.rootObjects().first();//->findChild<QObject*>("mainForm");
//
//    if (!item) {
//        std::cout << "item not found" << std::endl;
//    } else {
//        std::cout << "Ok" << std::endl;
//
//        Backend *back = new(std::nothrow) Backend(item);
//        if (!back) {
//            return 0;
//        }
//        QObject::connect(item, SIGNAL(button1_signal(QString)), back, SLOT(ButtonOnClick(QString)));
//    }
//

    Backend back(app);
    back.resize(1000, 600);
    back.show();
    return app.exec();
}
