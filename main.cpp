#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickView>
#include <QWidget>
#include <QTimer>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/shape.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include <vlc/vlc.h>

#include "RenderWindow.hpp"
#include "RenderClient.hpp"

void init(void)
{
    Display *dpy = QX11Info::display();

    int render_event_base, render_error_base;
    int render_present=XRenderQueryExtension(dpy, &render_event_base, &render_error_base);
    if (!render_present)
        printf("RENDER extension missing\n");
}


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    assert(QX11Info::isPlatformX11());
    init();

    RenderWindow server;

    QQuickView view;
    view.setDefaultAlphaBuffer(true);
    view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));
    view.setClearBeforeRendering(true);
    view.setFlag(Qt::WindowType::BypassWindowManagerHint);
    view.setFlag(Qt::WindowType::WindowTransparentForInput);
    view.setColor(QColor(Qt::transparent));
    view.setResizeMode( QQuickView::SizeRootObjectToView );
    view.setGeometry( 100, 100, 800, 600);
    view.show();

    RenderClient interfaceClient((Window) view.winId());
    interfaceClient.show();

    server.setInterfaceClient(&interfaceClient, &view);


    QWidget* videoWidget = new QWidget();

    videoWidget->setAttribute(Qt::WA_NativeWindow);
    videoWidget->setWindowFlag(Qt::WindowType::BypassWindowManagerHint);
    videoWidget->setWindowFlag(Qt::WindowType::WindowTransparentForInput);
    videoWidget->resize(800, 600);
    videoWidget->show();

    RenderClient videoClient((Window) videoWidget->winId());
    videoClient.show();

    server.setVideoClient(&videoClient);

    libvlc_instance_t* vlc = libvlc_new(0, nullptr);
    auto m = libvlc_media_new_location(vlc, "file:///home/pierre/Videos/Doctor.Who.2005.S08E06.720p.HDTV.x265.mp4");
    libvlc_media_player_t* mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release (m);
    libvlc_media_player_set_xwindow(mp, videoWidget->winId());
    libvlc_media_player_play (mp);

    QObject::connect(&view, &QQuickView::afterRendering, [&]() {
        server.refresh();
    });

    app.exec();

    return 0;
}
