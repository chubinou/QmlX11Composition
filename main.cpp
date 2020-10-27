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

//xcb_render_pictformat_t
//find_visual_format(xcb_connection_t* conn,  xcb_visualid_t visual)
//{
//    auto cookie = xcb_render_query_pict_formats(conn);
//    auto reply =  xcb_render_query_pict_formats_reply(conn, cookie, nullptr);
//
//    if (!reply) return 0;
//
//    for (auto screens = xcb_render_query_pict_formats_screens_iterator(reply);
//         screens.rem; xcb_render_pictscreen_next(&screens))
//    {
//        for (auto depths = xcb_render_pictscreen_depths_iterator(screens.data);
//             depths.rem; xcb_render_pictdepth_next(&depths))
//        {
//            for (auto visuals = xcb_render_pictdepth_visuals_iterator(depths.data);
//                 visuals.rem; xcb_render_pictvisual_next(&visuals))
//            {
//                xcb_render_pictvisual_t* data = visuals.data;
//                if (data->visual == visual) {
//                    xcb_render_pictformat_t ret = data->format;
//                    free(reply);
//                    return ret;
//                }
//            }
//        }
//    }
//
//    free(reply);
//    return 0;
//}
//
//static void fill_rect(Display* dpy, Picture p, uint8_t op,
//                      int x, int y, int w, int h,
//                      uint8_t s_red, uint8_t s_green, uint8_t s_blue, uint8_t s_alpha)
//{
//    XRenderColor render_color;
//
//    render_color.red   = s_red * s_alpha;
//    render_color.green = s_green * s_alpha;
//    render_color.blue  = s_blue * s_alpha;
//    render_color.alpha = s_alpha << 8 | s_alpha;
//    Picture source = XRenderCreateSolidFill(dpy, &render_color);
//    XRenderComposite(dpy, op, source, None, p, 0, 0, 0, 0, x, y, w,h);
//    XRenderFreePicture(dpy, source);
//}
//
//int main( int argc, char *argv[] )
//{
//
//    //QApplication app( argc, argv );
//    //
//    //assert(QX11Info::isPlatformX11());
//    ////assert(QX11Info::isCompositingManagerRunning());
//    //
//    //Display *dpy = QX11Info::display();
//    //
//    //QQuickView view;
//    //view.setDefaultAlphaBuffer(true);
//    //view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));
//    //view.setClearBeforeRendering(true);
//    //view.setColor(QColor(Qt::transparent));
//    //view.setResizeMode( QQuickView::SizeRootObjectToView );
//    //view.setGeometry( 100, 100, 500, 500 );
//    //view.show();
//    //
//    //Window wId = (Window)view.winId();
//    //
//    //XCompositeRedirectWindow(dpy, wId, CompositeRedirectAutomatic);
//    //
//    //XWindowAttributes attr;
//    //XGetWindowAttributes( dpy, wId, &attr );
//
//    //auto mFormat = XRenderFindVisualFormat( dpy, attr.visual );
//    //auto mDamage = XDamageCreate( dpy, wId, XDamageReportNonEmpty );
//    ////XShapeSelectInput( dpy, wId, ShapeNotifyMask );
//    //QRect mGeometry = QRect( attr.x, attr.y, attr.width, attr.height );
//
//    //XRenderPictFormat *format = XRenderFindVisualFormat(dpy, attr.visual);
//    //
//    //
//    //Pixmap xc_pixmap = XCompositeNameWindowPixmap(dpy, wId);
//    //if (!xc_pixmap) {
//    //    printf("xc_pixmap not found\n");
//    //}
//    //
//    //XRenderPictureAttributes pa = {};
//    //pa.subwindow_mode = IncludeInferiors;
//
//
//    //QWidget widget;
//    //widget.setGeometry( 50, 50, 500, 500 );
//    //widget.setAttribute( Qt::WA_NativeWindow, true );
//    //widget.show();
//    //widget.winId();
//
//
//    Display* dpy = XOpenDisplay( 0 );
//    auto win = XCreateSimpleWindow(
//       dpy, DefaultRootWindow(dpy),
//       0, 0, 500, 500,
//       0, 0, 0
//    );
//    XMapWindow(dpy, win);
//
//    Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
//    XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);
//
//    XWindowAttributes outAttr;
//    XGetWindowAttributes( dpy, win, &outAttr );
//    XRenderPictFormat *outFormat = XRenderFindVisualFormat(dpy, outAttr.visual);
//
//    Pixmap pixmap = XCreatePixmap( dpy, win, 500, 500, outFormat->depth );
//    Picture outPic = XRenderCreatePicture( dpy, pixmap, outFormat, 0, 0 );
//    XFreePixmap( dpy, pixmap ); // The picture owns the pixmap now
//
//    int file_counter = 0;
//
//    //QObject::connect(&view, &QQuickView::frameSwapped, [&]() {
//    while( true) {
//        XEvent ev;
//        XNextEvent (dpy, &ev);
//
//
//        XSynchronize(dpy, True);
//
//        XRenderColor col;
//        col.red = 0xFF00; col.blue= 0x0FF0; col.green = 0xF00F; col.alpha = 0xFFFF;
//        XRenderFillRectangle(dpy, PictOpSrc, outPic, &col, 20, 20, 100, 100);
//        //XRenderComposite( dpy, PictOpSrc, wId, None, outPic,
//        //                      0, 0, 0, 0, attr.x, attr.y, attr.width, attr.height );
//
//
//        //char buffer[50];
//        //sprintf(buffer, "/tmp/%ld_test%d_xc.xpm", pixmap, file_counter);
//        //XWriteBitmapFile(dpy, buffer, xc_pixmap, 500, 500, -1, -1);
//        //sprintf(buffer, "/tmp/%ld_test%d_wid.xpm", wId, file_counter);
//        //XWriteBitmapFile(dpy, buffer, wId, 500, 500, -1, -1);
//        //XSynchronize(dpy, false);
//
//        //file_counter++;
//        if (ev.type == KeyPress)
//        {
//          char buf[128] = {0};
//          KeySym keysym;
//          int len = XLookupString(&ev.xkey, buf, sizeof buf, &keysym, NULL);
//          if (keysym == XK_Escape)
//            break;
//        }
//
//        if ((ev.type == ClientMessage) &&
//            (static_cast<unsigned int>(ev.xclient.data.l[0]) == WM_DELETE_WINDOW))
//        {
//          break;
//        }
//    }
//
//    XDestroyWindow(dpy, win);
//    XCloseDisplay(dpy);
//    //});
//
//
//
//    //return app.exec();
//}


#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#define panic(x...) do {fprintf(stderr, x); abort();} while(0)

Display *dpy;
XRenderPictFormat *fmt;
int depth;

Window window;
Pixmap background;
Picture drawingarea;

void init(void)
{
    dpy = QX11Info::display();

    int render_event_base, render_error_base;
    int render_present=XRenderQueryExtension(dpy, &render_event_base, &render_error_base);
    if (!render_present) panic("RENDER extension missing\n");

    int screen=DefaultScreen(dpy);
    Window root=DefaultRootWindow(dpy);
    Visual *visual=DefaultVisual(dpy, screen);
    depth=DefaultDepth(dpy, screen);

    fmt=XRenderFindVisualFormat(dpy, visual);

    window=XCreateWindow(dpy, root, 0, 0, 640, 480, 0,
        depth, InputOutput, visual, 0, NULL);
    background=XCreatePixmap(dpy, window, 640, 480, depth);
    XSelectInput(dpy, window, StructureNotifyMask);
    XSetWindowBackgroundPixmap(dpy, window, background);
    XMapWindow(dpy, window);

    XRenderPictureAttributes pict_attr;
    pict_attr.poly_edge=PolyEdgeSmooth;
    pict_attr.poly_mode=PolyModeImprecise;
    drawingarea=XRenderCreatePicture(dpy, background, fmt, CPPolyEdge|CPPolyMode,
        &pict_attr);

}

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

    QQuickView view;
    view.setDefaultAlphaBuffer(true);
    view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));
    view.setClearBeforeRendering(true);
    //view.setFlag(Qt::WindowType::X11BypassWindowManagerHint);
    view.setColor(QColor(Qt::transparent));
    view.setResizeMode( QQuickView::SizeRootObjectToView );
    view.setGeometry( 100, 100, 500, 500 );
    view.show();


    Client client((Window) view.winId());
    client.show();

    XRenderColor green      ={0x0FF0,   0xF00F, 0x00FF,0xFFFF};
    XRenderColor blackAplha ={0,        0,      0,      0x8888};


    //unsigned short alpha  = 0;
    //QTimer t;
    //t.setSingleShot(false);
    //
    //
    //QObject::connect(&t, &QTimer::timeout, [&]() {
    QObject::connect(&view, &QQuickView::afterRendering, [&]() {
        XFlush(dpy);

        XRenderFillRectangle(dpy, PictOpOver, drawingarea, &green, 0, 0, 640, 480);
        blackAplha.red += 0x800;
        qDebug() << "draw" << blackAplha.red;

        Picture pic = client.getPicture();
        //XRenderFillRectangle(dpy, PictOpOver, drawingarea, &blackAplha, 30, 30, 200, 200);
        XRenderComposite(dpy, PictOpOver, pic, 0, drawingarea,
                         0,0, 0, 0,
                         0, 0, 500, 500);

        XClearArea(dpy, window, 0, 0, 640, 480, 0);
    });
    //t.start(100);

    app.exec();

    return 0;
}
