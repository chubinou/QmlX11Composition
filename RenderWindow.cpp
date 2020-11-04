#include <QtEvents>
#include <QCoreApplication>
#include <QWindow>
#include <QQuickItem>
#include <QX11Info>
#include <memory>

#include <xcb/render.h>
#include <xcb/composite.h>
#include <xcb/damage.h>

#include "RenderWindow.hpp"
#include "RenderClient.hpp"
#include "OffscreenQmlView.hpp"

#undef KeyPress
#undef KeyRelease
#undef None


template <typename T, typename Releaser>
inline auto wrap_cptr( T* ptr, Releaser&& r ) noexcept
    -> std::unique_ptr<T, typename std::decay<decltype( r )>::type>
{
    return std::unique_ptr<T, typename std::decay<decltype( r )>::type>{
                ptr, std::forward<Releaser>( r )
    };
}

template <typename T>
inline std::unique_ptr<T, void (*)(void*)> wrap_cptr( T* ptr ) noexcept
{
    return wrap_cptr( ptr, &free );
}

bool queryExtension(const QString& name, int* first_event_out, int* first_error_out)
{
    xcb_connection_t* c = QX11Info::connection();

    xcb_query_extension_cookie_t cookie = xcb_query_extension(c, (uint16_t)name.size(), name.toUtf8().constData());
    xcb_generic_error_t* error = NULL;
    auto reply = wrap_cptr(xcb_query_extension_reply(c, cookie, &error));
    auto errorPtr = wrap_cptr(error);
    if (errorPtr || !reply) {
        qDebug() << "Querying extension " << name << " failed";
        return false;
    }
    if (!reply->present) {
        qDebug() << "Extension " << name << " is not present";
        return false;
    }

    if (first_event_out)
      *first_event_out = reply->first_event;
    if (first_error_out)
      *first_error_out = reply->first_error;
    return true;
}

#define CHECK_EXTENSION_VERSION(c, extension, minor, major) \
do { \
    xcb_ ## extension  ##_query_version_cookie_t cookie = xcb_## extension  ##_query_version(c, minor, major); \
    auto reply = wrap_cptr(xcb_## extension  ##_query_version_reply(c, cookie, 0)); \
    if (!reply) { \
        printf("%s extension missing\n", #extension); \
        return false; \
    } \
    printf("%s version %i.%i\n", #extension,  reply->major_version, reply->minor_version); \
} while(0)


RenderWindow::RenderWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    m_dpy = QX11Info::display();

    m_wid = winId();

}


bool RenderWindow::init()
{
    xcb_connection_t* c = QX11Info::connection();

    bool ret;

    ret = queryExtension("DAMAGE", &m_xdamageBaseEvent, nullptr);
    if (! ret)
        return false;
    ret = queryExtension("Composite", nullptr, nullptr);
    if (! ret)
        return false;
    ret = queryExtension("RENDER", nullptr, nullptr);
    if (! ret)
        return false;
    CHECK_EXTENSION_VERSION(c, damage, 1, 1);
    CHECK_EXTENSION_VERSION(c, composite, 0, 4);
    CHECK_EXTENSION_VERSION(c, render, XCB_RENDER_MAJOR_VERSION, XCB_RENDER_MINOR_VERSION);

    windowHandle()->
            installEventFilter(this);

    qApp->installNativeEventFilter(this);

    return true;
}


void RenderWindow::refresh(size_t requestId)
{
    if (requestId != m_refresh_request) {
        return;
    }
    //xcb_render_picture_t pic;
    Picture pic;

    //xcb_grab_server(m_conn);
    //xcb_flush(m_conn);
    //XGrabServer(m_dpy); //avoids tearing by locking the server
    XFlush(m_dpy);

    //xcb_render_picture_t drawingarea = getBackTexture();
    Picture drawingarea = getBackTexture();

    int realW = width()  * devicePixelRatioF();
    int realH = height() * devicePixelRatioF();

    if (m_videoClient) {
        pic = m_videoClient->getPicture();
        if (pic) {
            //xcb_render_composite(m_conn, XCB_RENDER_PICT_OP_OVER,
            //                     pic, 0, drawingarea,
            //                     0,0,0,0,
            //                     0,0, realW, realH);
            XRenderComposite(m_dpy, PictOpOver, pic, 0, drawingarea,
                         0,0, 0, 0,
                         0, 0, realW, realH);
        }
    }

    if (m_interfaceClient) {
        pic = m_interfaceClient->getPicture();
        if (pic) {
            //xcb_render_composite(m_conn, XCB_RENDER_PICT_OP_OVER,
            //                     pic, 0, drawingarea,
            //                     0,0,0,0,
            //                     0,0, realW, realH);
            XRenderComposite(m_dpy, PictOpOver, pic, 0, m_drawingarea,
                         0,0, 0, 0,
                         0, 0, realW, realH);
        }
    }


    //xcb_clear_area(m_conn, 0 /* exposure ??? */, m_wid, 0, 0, realW, realH);
    //xcb_ungrab_server(m_conn);
    XClearArea(m_dpy, m_wid, 0, 0, realW, realH, 0);
    //XUngrabServer(m_dpy);

    m_refresh_request++;
}


bool RenderWindow::eventFilter(QObject*, QEvent* event)
{
    if (m_videoWindow) {
        switch (event->type()) {

        case QEvent::Move:
        case QEvent::Show:
        {
            //m_videoWindow->setPosition(pos());
            refresh(m_refresh_request);
            break;
        }
        case QEvent::Resize:
        {
            QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
            m_videoWindow->resize(resizeEvent->size());
            m_videoClient->geometryChanged();
            break;
        }
        default: break;
        }
    }

    bool ret = false;
    if (m_interfaceWindow)
        ret =  m_interfaceWindow->handleWindowEvent(event);

    switch (event->type()) {

    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Resize:
        m_interfaceClient->geometryChanged();
        refresh(m_refresh_request);
        break;
    default: break;
    }

    return ret;
}

bool RenderWindow::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);

        if ((ev->response_type & 0x7F) == (m_xdamageBaseEvent + XCB_DAMAGE_NOTIFY)) {
            xcb_damage_notify_event_t* damageEvent = static_cast<xcb_damage_notify_event_t*>(message);
            if (damageEvent->drawable == m_interfaceWindow->winId()
                || damageEvent->drawable == m_videoWindow->winId())
            {
                size_t requestId = m_refresh_request;
                QMetaObject::invokeMethod(this, [this, requestId]() {
                    refresh(requestId);
                }, Qt::QueuedConnection);
            }

            return false;  // filter out this event, stop its processing
        }
    }
    return false;
}

Picture RenderWindow::getBackTexture() {
    if (m_drawingarea)
        return m_drawingarea;

    int screen=DefaultScreen(m_dpy);
    Visual *visual=DefaultVisual(m_dpy, screen);
    XRenderPictFormat* fmt = XRenderFindVisualFormat(m_dpy, visual);
    int depth = DefaultDepth(m_dpy, screen);
    int width = DisplayWidth(m_dpy, screen);
    int height = DisplayHeight(m_dpy, screen);


    //m_background = xcb_generate_id(m_conn);
    //xcb_create_pixmap(m_conn, depth, m_background, m_wid, width, height);
    //xcb_change_window_attributes(m_conn, m_wid, XCB_CW_BACK_PIXMAP, &m_background);

    m_background = XCreatePixmap(m_dpy, m_wid, width, height, depth);
    XSetWindowBackgroundPixmap(m_dpy, m_wid, m_background);

    //xcb_render_pictformat_t fmt;
    //pict_attr.poly_edge=PolyEdgeSmooth;
    //pict_attr.poly_mode=PolyModeImprecise;
    //m_drawingarea = xcb_generate_id(m_conn);
    //xcb_render_create_picture(m_conn, m_drawingarea, m_background, fmt , CPPolyEdge|CPPolyMode, &pict_attr)

    XRenderPictureAttributes pict_attr;
    pict_attr.poly_edge=PolyEdgeSmooth;
    pict_attr.poly_mode=PolyModeImprecise;
    m_drawingarea = XRenderCreatePicture(m_dpy, m_background, fmt, CPPolyEdge|CPPolyMode,
                                         &pict_attr);

    return m_drawingarea;
}

void RenderWindow::paintEvent(QPaintEvent* event)
{
    qDebug() << "paintEvent";
    QWidget::paintEvent(event);
    refresh(m_refresh_request);
}
