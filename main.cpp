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

#define panic(x...) do {fprintf(stderr, x); abort();} while(0)

Display *dpy;
XRenderPictFormat *fmt;
int depth;

void init(void)
{
    dpy = QX11Info::display();

    int render_event_base, render_error_base;
    int render_present=XRenderQueryExtension(dpy, &render_event_base, &render_error_base);
    if (!render_present) panic("RENDER extension missing\n");

    int screen=DefaultScreen(dpy);
    Visual *visual=DefaultVisual(dpy, screen);
    depth=DefaultDepth(dpy, screen);

    fmt=XRenderFindVisualFormat(dpy, visual);
}

class Server
{
public:
    Server()
    {
        m_widget = new QWidget();
        m_widget->resize(800, 600);
        m_widget->setAttribute(Qt::WA_NativeWindow);
        m_widget->setAttribute(Qt::WA_OpaquePaintEvent);
        m_widget->setAttribute(Qt::WA_NoSystemBackground);
        //QPalette palette = m_widget->palette();
        //m_widget->setAttribute(Qt::WA_TranslucentBackground);
        //palette.setColor(QPalette::Background, Qt::transparent);
        //m_widget->setPalette(palette);
        m_wid = m_widget->winId();

        m_background = XCreatePixmap(dpy, m_wid, 800, 600, depth);
        XSelectInput(dpy, m_wid, StructureNotifyMask);
        XSetWindowBackgroundPixmap(dpy, m_wid, m_background);
        XMapWindow(dpy, m_wid);

        m_widget->show();

        XRenderPictureAttributes pict_attr;
        pict_attr.poly_edge=PolyEdgeSmooth;
        pict_attr.poly_mode=PolyModeImprecise;
        m_drawingarea = XRenderCreatePicture(dpy, m_background, fmt, CPPolyEdge|CPPolyMode,
            &pict_attr);
    }

    Window getWindow() {
        return m_wid;
    }

    Picture getBackTexture()
    {
        return m_drawingarea;
    }

private:
    QWidget* m_widget = nullptr;
    Window m_wid = 0;
    Pixmap m_background = 0;
    Picture m_drawingarea = 0;
};


class Client
{
public:
    Client(Window _wid)
        : m_wid(_wid)
    {
        XCompositeRedirectWindow(dpy, m_wid, CompositeRedirectManual);
        XGetWindowAttributes( dpy, m_wid, &m_attr );

        m_format = XRenderFindVisualFormat( dpy, m_attr.visual );
        m_damage = XDamageCreate( dpy, m_wid, XDamageReportNonEmpty );

        // Make sure we get notified when the window shape is changed
        XShapeSelectInput( dpy, m_wid, ShapeNotifyMask );

        show();
    }

    ~Client() {
        if (m_pixmap)
            XFreePixmap( dpy, m_pixmap );
        if (m_picture)
            XRenderFreePicture( dpy, m_picture );
        if ( m_damage )
            XDamageDestroy( dpy, m_damage );

    }

    void show()
    {
        XSelectInput( dpy, m_wid, PropertyChangeMask );
    }

    void createPicture()
    {
        if ( !m_pixmap )
                m_pixmap = XCompositeNameWindowPixmap( dpy, m_wid );
        m_picture = XRenderCreatePicture( dpy, m_pixmap, m_format, 0, 0 );
    }


    Picture getPicture()
    {
        if (!m_picture)
            createPicture();
        if ( !m_sourceClipValid ) {
            XserverRegion clip = XFixesCreateRegionFromWindow( dpy, m_wid, WindowRegionBounding );
            XFixesSetPictureClipRegion( dpy, m_picture, 0, 0, clip );
            XFixesDestroyRegion( dpy, clip );
            m_sourceClipValid = true;
        }
        return m_picture;
    }

private:
    Pixmap m_pixmap = 0;
    Picture m_picture = 0;
    Window m_wid = 0;
    Damage m_damage = 0;
    XRenderPictFormat* m_format;
    XWindowAttributes m_attr;

    bool m_sourceClipValid = false;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);


    assert(QX11Info::isPlatformX11());
    init();

    Server server;

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


    Client client((Window) view.winId());
    client.show();


    auto videoWidget = new QWidget();

    videoWidget->setAttribute(Qt::WA_NativeWindow);
    videoWidget->setWindowFlag(Qt::WindowType::BypassWindowManagerHint);
    videoWidget->setWindowFlag(Qt::WindowType::WindowTransparentForInput);
    videoWidget->resize(800, 600);

    videoWidget->show();
    Client client2((Window) videoWidget->winId());
    client2.show();


    libvlc_instance_t* vlc = libvlc_new(0, nullptr);
    auto m = libvlc_media_new_location(vlc, "file:///home/pierre/Videos/Doctor.Who.2005.S08E06.720p.HDTV.x265.mp4");
    libvlc_media_player_t* mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release (m);
    libvlc_media_player_set_xwindow(mp, videoWidget->winId());

    libvlc_media_player_play (mp);

    QObject::connect(&view, &QQuickView::afterRendering, [&]() {
        Picture pic;

        XFlush(dpy);

        auto  drawing =  server.getBackTexture();

        int realW = 800 * view.devicePixelRatio();
        int realH = 600 * view.devicePixelRatio();

        pic = client2.getPicture();
        XRenderComposite(dpy, PictOpOver, pic, 0, drawing,
                         0,0, 0, 0,
                         0, 0, realW, realH);

        pic = client.getPicture();
        XRenderComposite(dpy, PictOpOver, pic, 0, drawing,
                         0,0, 0, 0,
                         0, 0, realW, realH);

        XClearArea(dpy, server.getWindow(), 0, 0, realW, realH, 0);
    });

    app.exec();

    return 0;
}
