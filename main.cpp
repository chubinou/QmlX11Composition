#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickView>
#include <QOpenGLContext>
#include <QQmlComponent>
#include <QWidget>
#include <QTimer>
#include <QX11Info>
#include <QOffscreenSurface>

#include "RenderWindow.hpp"
#include "RenderClient.hpp"
#include "OffscreenQmlView.hpp"

#include <vlc/vlc.h>


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



    //QQuickView* qmlview = new QQuickView();
    //qmlview->setDefaultAlphaBuffer(true);
    //qmlview->setSource(QUrl(QStringLiteral("qrc:///main.qml")));
    //qmlview->setClearBeforeRendering(false);
    //qmlview->setFlag(Qt::WindowType::BypassWindowManagerHint);
    //qmlview->setFlag(Qt::WindowType::WindowTransparentForInput);
    //qmlview->setColor(QColor(Qt::transparent));
    //qmlview->setResizeMode( QQuickView::SizeRootObjectToView );
    //qmlview->setGeometry( 100, 100, 800, 600);
    //qmlview->show();

    OffscreenQmlView* qmlview = new OffscreenQmlView();
    qmlview->setFlag(Qt::WindowType::BypassWindowManagerHint);
    qmlview->setFlag(Qt::WindowType::WindowTransparentForInput);
    QQmlEngine* engine = qmlview->engine();
    QQmlComponent* component = new QQmlComponent(engine, QStringLiteral("qrc:///main.qml"), QQmlComponent::PreferSynchronous, engine);
    QObject* rootObject = component->create();
    qmlview->setContent(component, (QQuickItem*)rootObject);
    qmlview->resize(800, 600);
    qmlview->setPosition(800, 600);
    qmlview->show();
    qmlview->winId();

    RenderClient interfaceClient(qmlview);
    interfaceClient.show();

    RenderWindow server;
    server.setInterfaceClient(&interfaceClient, qmlview);
    server.show();

   QWidget* videoWidget = new QWidget();

   videoWidget->setAttribute(Qt::WA_NativeWindow);
   videoWidget->setWindowFlag(Qt::WindowType::BypassWindowManagerHint);
   videoWidget->setWindowFlag(Qt::WindowType::WindowTransparentForInput);
   videoWidget->resize(800, 600);
   videoWidget->show();

   RenderClient videoClient(videoWidget->windowHandle());
   videoClient.show();

   server.setVideoClient(&videoClient, videoWidget->windowHandle());

   libvlc_instance_t* vlc = libvlc_new(0, nullptr);
   auto m = libvlc_media_new_location(vlc, "file:///home/pierre/Videos/Doctor.Who.2005.S08E06.720p.HDTV.x265.mp4");
   libvlc_media_player_t* mp = libvlc_media_player_new_from_media(m);
   libvlc_media_release (m);
   libvlc_media_player_set_xwindow(mp, videoWidget->winId());
   libvlc_media_player_play (mp);

   QObject::connect(qmlview, &OffscreenQmlView::afterRendering, [&]() {
       server.refresh();
   });

   //QTimer t;
   //t.singleShot(3000, [&](){
   //    qmlview->resize(300, 200);
   //});

    app.exec();

    return 0;
}
