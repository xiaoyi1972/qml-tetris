#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include<tetris.h>
#include<QQuickStyle>
//#include<vld.h>
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QQuickStyle::setStyle("Basic");
    qmlRegisterType<Tetris>("Tetris", 1, 0, "Tetris");
      engine.rootContext()->setContextProperty("TetrisConfig", &Tetris::keyconfig);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
    &app, [url](QObject * obj, const QUrl & objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
