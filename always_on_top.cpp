#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QQuickView>
#include <QtQuickWidgets/QQuickWidget>
#include <QWidget>
#include <QStackedLayout>

#include "RenderWindow.hpp"
#include "RenderClient.hpp"

#include <vlc/vlc.h>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QMainWindow* mainWidget = new QMainWindow();
    mainWidget->resize(800, 600);
    mainWidget->show();

    //QWidget* videoWidget = new QWidget(mainWidget);
    //videoWidget->setAttribute(Qt::WA_NativeWindow);
    //videoWidget->move(100, 100);
    //videoWidget->resize(800, 600);
    //videoWidget->show();

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setAlphaBufferSize(8);

    QQuickWidget* view = new QQuickWidget(mainWidget);
    view->setAttribute(Qt::WA_AlwaysStackOnTop);
    //view->setAttribute(Qt::WA_NativeWindow);
    view->setClearColor(QColor(Qt::transparent));
    view->setFormat(format);
    view->setAttribute(Qt::WA_TranslucentBackground);
    view->setResizeMode( QQuickWidget::SizeRootObjectToView );
    view->move(0, 0);
    view->resize( 800, 600);
    view->setSource(QUrl(QStringLiteral("qrc:///main.qml")));
    view->raise();
    view->show();

    libvlc_instance_t* vlc = libvlc_new(0, nullptr);
    auto m = libvlc_media_new_location(vlc, "file:///home/pierre/Videos/Doctor.Who.2005.S08E06.720p.HDTV.x265.mp4");
    libvlc_media_player_t* mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release (m);
    libvlc_media_player_set_xwindow(mp, mainWidget->winId());
    libvlc_media_player_play (mp);

    app.exec();

    return 0;
}
