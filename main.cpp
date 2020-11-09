#include <memory>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickView>
#include <QOpenGLContext>
#include <QQmlComponent>
#include <QWidget>
#include <QOffscreenSurface>
#include <QtGlobal>

#include <xcb/xcbext.h>
#include <xcb/render.h>
#include <xcb/damage.h>
#include <xcb/composite.h>

#include "RenderWindow.hpp"
#include "RenderClient.hpp"
#include "OffscreenQmlView.hpp"

#include <vlc/vlc.h>

//const char* defaultFilePath = "file:///home/pierre/Videos/Doctor.Who.2005.S08E06.720p.HDTV.x265.mp4";
const char* defaultFilePath = "https://streams.videolan.org/streams/mkv/Dexter.s04e12.720p.hdtv.x264-red.mkv";

void errorHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "%s\n", msg.toUtf8().constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "\033[1;33mWarning\033[0m: %s\n", msg.toUtf8().constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "\033[31mCritical\033[0m: %s\n", msg.toUtf8().constData());
            break;
        case QtFatalMsg:
            fprintf(stderr, "\033[31mFatal\033[0m: %s\n", msg.toUtf8().constData());
            abort();
        break;
        default: break;
    }
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    qInstallMessageHandler(errorHandler);

    RenderWindow server;
    if (!server.init()) {
        return -1;
    }

    OffscreenQmlView* qmlview = new OffscreenQmlView(server.windowHandle());
    QQmlEngine* engine = qmlview->engine();
    QQmlComponent* component = new QQmlComponent(engine, QStringLiteral("qrc:///main.qml"), QQmlComponent::PreferSynchronous, engine);
    QObject* rootObject = component->create();
    qmlview->setContent(component, (QQuickItem*)rootObject);
    qmlview->resize(800, 600);
    qmlview->setOpacity(0);
    qmlview->winId();
    qmlview->show();


    QWidget* videoWidget = new QWidget();
    videoWidget->setAttribute(Qt::WA_NativeWindow);
    videoWidget->setWindowFlag(Qt::WindowType::BypassWindowManagerHint);
    videoWidget->setWindowFlag(Qt::WindowType::WindowTransparentForInput);
    videoWidget->windowHandle()->setOpacity(0);
    videoWidget->resize(800, 600);
    videoWidget->winId();
    videoWidget->show();

    const char* path = defaultFilePath;
    if (argc > 1)
        path = argv[1];
    libvlc_instance_t* vlc = libvlc_new(0, nullptr);
    auto m = libvlc_media_new_location(vlc, path);
    libvlc_media_player_t* mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release (m);
    libvlc_media_player_set_xwindow(mp, videoWidget->winId());
    libvlc_media_player_play (mp);

    server.setInterfaceWindow(qmlview);
    server.setVideoWindow(videoWidget->windowHandle());
    server.show();

    app.exec();

    return 0;
}
