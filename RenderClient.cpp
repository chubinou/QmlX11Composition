#include <QDebug>
#include "RenderClient.hpp"
#include <xcb/composite.h>

#undef None

#define _NET_WM_BYPASS_COMPOSITOR_NAME "_NET_WM_BYPASS_COMPOSITOR"
#define _NET_WM_STATE_NAME "_NET_WM_STATE"
#define _NET_WM_STATE_HIDDEN_NAME "_NET_WM_STATE_HIDDEN"


RenderClient::RenderClient(QWindow* window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_conn(QX11Info::connection())
    , m_wid(window->winId())
    , m_pixmap(m_conn)
    , m_picture(m_conn)
    , m_damage(m_conn)
{
    //xcb_atom_t _NET_WM_STATE = getInternAtom(m_conn, _NET_WM_STATE_NAME);
    //xcb_atom_t _NET_WM_STATE_HIDDEN = getInternAtom(m_conn, _NET_WM_STATE_HIDDEN_NAME);

    xcb_get_window_attributes_cookie_t attrCookie = xcb_get_window_attributes(m_conn, m_wid);
    auto attrReply = wrap_cptr(xcb_get_window_attributes_reply(m_conn, attrCookie, nullptr));


    xcb_visualid_t visual = attrReply->visual;

    findVisualFormat(m_conn, visual, &m_format, nullptr);


   //auto overlayCookie = xcb_composite_get_overlay_window(m_conn, m_wid);
   //auto overlayReply = wrap_cptr(xcb_composite_get_overlay_window_reply(m_conn, overlayCookie, nullptr));
   //xcb_window_t overlayWin = overlayReply->overlay_win;
   //xcb_reparent_window(m_conn, m_wid, 0, 0, 0);
    //xcb_unmap_window(m_conn, overlayWin);

    xcb_composite_redirect_window(m_conn, m_wid, XCB_COMPOSITE_REDIRECT_MANUAL);

    xcb_damage_damage_t dam = xcb_generate_id(m_conn);
    xcb_damage_create(m_conn, dam, m_wid, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);

    xcb_atom_t _NET_WM_BYPASS_COMPOSITOR = getInternAtom(m_conn, _NET_WM_BYPASS_COMPOSITOR_NAME);
    uint32_t val = 1;
    xcb_change_property(m_conn, XCB_PROP_MODE_REPLACE, m_wid,
                        _NET_WM_BYPASS_COMPOSITOR, XCB_ATOM_CARDINAL, 32, 1, &val);

    connect(window, &QWindow::widthChanged, this, &RenderClient::geometryChanged);
    connect(window, &QWindow::heightChanged, this, &RenderClient::geometryChanged);
}

RenderClient::~RenderClient() {
    m_pixmap.reset();
    m_picture.reset();
    m_damage.reset();
}

void RenderClient::createPicture()
{
    xcb_void_cookie_t voidCookie;
    auto err = wrap_cptr<xcb_generic_error_t>(nullptr);

    //if ( !m_pixmap ) {
        m_pixmap.generateId();
        voidCookie = xcb_composite_name_window_pixmap_checked(m_conn, m_wid, m_pixmap.get());
        err.reset(xcb_request_check(m_conn, voidCookie));
        if (err)
        {
            qWarning() << "can't create pixmap";
            m_pixmap.reset();
            return;
        }
    //}

    m_picture.generateId();
    voidCookie = xcb_render_create_picture_checked(m_conn, m_picture.get(), m_pixmap.get(), m_format, 0, 0);
    err.reset(xcb_request_check(m_conn, voidCookie));
    if (err)
    {
        qWarning() << "can't create picture";
        m_pixmap.reset();
        m_picture.reset();
    }
}

xcb_render_picture_t RenderClient::getPicture()
{
    //if (!m_picture)
    createPicture();
    return m_picture.get();
}

void RenderClient::geometryChanged()
{
    qDebug() << "resize";

    //if ( width() != newGeometry.width() || height() != newGeometry.height() )
    {
        m_pixmap.reset();
        m_picture.reset();
        m_sourceClipValid = false;
    }
}
