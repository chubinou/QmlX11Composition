#include <QtEvents>
#include <QCoreApplication>
#include <QWindow>
#include <QQuickItem>
#include <QX11Info>
#include <QScreen>

#include <memory>

#include <xcb/render.h>
#include <xcb/composite.h>
#include <xcb/damage.h>

#include "RenderWindow.hpp"
#include "RenderClient.hpp"
#include "OffscreenQmlView.hpp"

static bool queryExtension(xcb_connection_t* conn, const QString& name, int* first_event_out, int* first_error_out)
{
    xcb_query_extension_cookie_t cookie = xcb_query_extension(conn, (uint16_t)name.size(), name.toUtf8().constData());
    xcb_generic_error_t* error = NULL;
    auto reply = wrap_cptr(xcb_query_extension_reply(conn, cookie, &error));
    auto errorPtr = wrap_cptr(error);
    if (errorPtr || !reply) {
        qWarning() << "Querying extension " << name << " failed";
        return false;
    }
    if (!reply->present) {
        qWarning() << "Extension " << name << " is not present";
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
        qWarning() << #extension  << " extension missing"; \
        return false; \
    } \
    qDebug() << #extension<< " version " << reply->major_version << "." <<  reply->minor_version; \
} while(0)


RenderWindow::RenderWindow(QWidget *parent)
    : QWidget(parent)
    , m_conn(QX11Info::connection())
    , m_drawingarea(m_conn)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    m_wid = winId();
}

RenderWindow::~RenderWindow()
{
    if (m_videoClient)
        m_videoClient.reset();
    if (m_interfaceClient)
        m_interfaceClient.reset();
}


bool RenderWindow::init()
{
    if (!QX11Info::isPlatformX11()) {
        qWarning() << "this program only runs on X11 plateforms, if you are running wayland you can try to run it with XWayland using:";
        qWarning() << "    export QT_QPA_PLATFORM=xcb";
        return false;
    }

    bool ret;

    ret = queryExtension(m_conn, "DAMAGE", &m_xdamageBaseEvent, nullptr);
    if (! ret)
        return false;
    ret = queryExtension(m_conn,"Composite", nullptr, nullptr);
    if (! ret)
        return false;
    ret = queryExtension(m_conn,"RENDER", nullptr, nullptr);
    if (! ret)
        return false;
    CHECK_EXTENSION_VERSION(m_conn, damage, 1, 1);
    CHECK_EXTENSION_VERSION(m_conn, composite, 0, 4);
    CHECK_EXTENSION_VERSION(m_conn, render, XCB_RENDER_MAJOR_VERSION, XCB_RENDER_MINOR_VERSION);

    windowHandle()->installEventFilter(this);

    qApp->installNativeEventFilter(this);

    return true;
}


void RenderWindow::refresh(unsigned short requestId)
{
    if (requestId != m_refreshRequestId) {
        return;
    }
    xcb_render_picture_t pic;

    //xcb_grab_server(m_conn);
    xcb_flush(m_conn);
    xcb_render_picture_t drawingarea = getBackTexture();

    int realW = width()  * devicePixelRatioF();
    int realH = height() * devicePixelRatioF();

    if (m_videoClient) {
        pic = m_videoClient->getPicture();
        if (pic) {
            xcb_render_composite(m_conn, XCB_RENDER_PICT_OP_OVER,
                                 pic, 0, drawingarea,
                                 0,0,0,0,
                                 0,0, realW, realH);
        }
    }

    if (m_interfaceClient) {
        pic = m_interfaceClient->getPicture();
        if (pic) {
            xcb_render_composite(m_conn, XCB_RENDER_PICT_OP_OVER,
                                 pic, 0, drawingarea,
                                 0,0,0,0,
                                 0,0, realW, realH);
        }
    }

    xcb_render_color_t color1 = { 0x0000, 0xFFFF, 0x0000, 0xFFFF };
    xcb_render_color_t color2 = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
    xcb_rectangle_t rect = {10, 100, 20, 20};
    xcb_render_fill_rectangles(m_conn, XCB_RENDER_PICT_OP_OVER, drawingarea,
                               m_refreshRequestId %2 ? color1 : color2, 1, &rect);

    xcb_clear_area(m_conn, 0, m_wid, 0, 0, 0, 0);

    m_refreshRequestId++;
}

void RenderWindow::requestRefresh()
{
    size_t requestId = m_refreshRequestId;
    QMetaObject::invokeMethod(this, [this, requestId]() {
        refresh(requestId);
    }, Qt::QueuedConnection);
}


bool RenderWindow::eventFilter(QObject*, QEvent* event)
{
    if (m_videoWindow) {
        switch (event->type()) {

        case QEvent::Move:
        case QEvent::Show:
        {
            //m_videoWindow->setPosition(pos());
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
        return false;
        break;
    case QEvent::Show:
    case QEvent::Resize:
        m_interfaceClient->geometryChanged();
        requestRefresh();
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
                requestRefresh();
            }

            return true;  // filter out this event, stop its processing
        }
    }
    return false;
}

xcb_render_picture_t RenderWindow::getBackTexture() {
    if (m_drawingarea)
        return m_drawingarea.get();

    xcb_void_cookie_t voidCookie;
    auto err = wrap_cptr<xcb_generic_error_t>(nullptr);

    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(m_conn)).data;
    int width = screen->width_in_pixels;
    int height = screen->height_in_pixels;

    xcb_get_window_attributes_cookie_t attrCookie = xcb_get_window_attributes(m_conn, m_wid);
    auto attrReply = wrap_cptr(xcb_get_window_attributes_reply(m_conn, attrCookie, nullptr));
    xcb_visualid_t visual = attrReply->visual;

    uint8_t depth;
    xcb_render_pictformat_t fmt;
    findVisualFormat(m_conn, visual, &fmt, &depth);

    qDebug() << "FORMAT is " << fmt
             << " height " << height
             << " height " << width
             << " depth " << depth;

    //TODO dynamically alloc backTexture size
    PixmapPtr  background{ m_conn};
    background.generateId();
    voidCookie =  xcb_create_pixmap_checked(m_conn, depth, background.get(), m_wid, width, height);
    err.reset(xcb_request_check(m_conn, voidCookie));
    if (err) {
        qWarning() << " error: xcb_create_pixmap " << err->error_code;
        return 0;
    }

    uint32_t attributeList[] = {background.get()};
    voidCookie = xcb_change_window_attributes_checked(m_conn, m_wid, XCB_CW_BACK_PIXMAP, attributeList);
    err.reset(xcb_request_check(m_conn, voidCookie));
    if (err) {
        qWarning() << "error: xcb_change_window_attributes_checked" << err->error_code;
        return 0;
    }

    m_drawingarea.generateId();
    xcb_render_create_picture_checked(m_conn, m_drawingarea.get(), background.get(), fmt, 0, nullptr);
    err.reset(xcb_request_check(m_conn, voidCookie));
    if (err) {
        qWarning() << "error: xcb_change_window_attributes_checked" << err->error_code;
        return 0;
    }

    return m_drawingarea.get();
}

void RenderWindow::setVideoWindow( QWindow* window) {
    window->setParent(windowHandle());
    m_videoClient.reset(new RenderClient(window));
    m_videoWindow = window;
}

void RenderWindow::setInterfaceWindow(OffscreenQmlView* window) {
    //window->setParent(windowHandle());
    xcb_reparent_window(m_conn, window->winId(), windowHandle()->winId(), 0, 0);
    m_interfaceClient.reset(new RenderClient(window));
    m_interfaceWindow = window;
}

void RenderWindow::paintEvent(QPaintEvent* event)
{
    qDebug() << "paintEvent";
    QWidget::paintEvent(event);
    requestRefresh();
}
