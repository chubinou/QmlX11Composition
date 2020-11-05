#include <QDebug>
#include "RenderClient.hpp"
#include <xcb/composite.h>

#undef None


RenderClient::RenderClient(QWindow* window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_conn(QX11Info::connection())
    //, m_dpy (QX11Info::display())
    , m_wid(window->winId())
    , m_pixmap(m_conn)
    , m_picture(m_conn)
    , m_damage(m_conn)
{
    xcb_get_window_attributes_cookie_t attrCookie = xcb_get_window_attributes(m_conn, m_wid);
    auto attrReply = wrap_cptr(xcb_get_window_attributes_reply(m_conn, attrCookie, nullptr));
    xcb_visualid_t visual = attrReply->visual;
    //XGetWindowAttributes( m_dpy, m_wid, &m_attr );

    findVisualFormat(m_conn, visual, &m_format, nullptr);
    //m_format = XRenderFindVisualFormat( m_dpy, m_attr.visual );


    //reparent the window to the Composite overlay to hide it from other compositor?
    //Window cow = XCompositeGetOverlayWindow(m_dpy, m_wid);
    //XReparentWindow(m_dpy, m_wid, cow, 0,0);
    //XCompositeReleaseOverlayWindow(m_dpy, cow);
    //XUnmapWindow(m_dpy, cow);
    //XDestroyWindow(m_dpy, cow);

    //XCompositeRedirectWindow(m_dpy, m_wid, CompositeRedirectManual);
    xcb_composite_redirect_window(m_conn, m_wid, XCB_COMPOSITE_REDIRECT_MANUAL);

    //Window cow = XCompositeGetOverlayWindow(m_dpy, m_wid);
    //XShapeCombineRectangles(m_dpy,
    //                          cow,
    //                          ShapeInput,
    //                          0,     // x_off
    //                          0,     // y_off
    //                          NULL,  // rectangles
    //                          0,     // n_rects
    //                          ShapeSet,
    //                          Unsorted);
    //XReparentWindow(m_dpy, m_wid, cow, 0, 0);


    xcb_damage_damage_t dam = xcb_generate_id(m_conn);
    xcb_damage_create(m_conn, dam, m_wid, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
    //m_damage = XDamageCreate( m_dpy, m_wid, XDamageReportNonEmpty );

    // Make sure we get notified when the window shape is changed
    //XShapeSelectInput( m_dpy, m_wid, ShapeNotifyMask );

    show();

    connect(window, &QWindow::widthChanged, this, &RenderClient::geometryChanged);
    connect(window, &QWindow::heightChanged, this, &RenderClient::geometryChanged);
}

RenderClient::~RenderClient() {
    m_pixmap.reset();
    m_picture.reset();
    m_damage.reset();
}

void RenderClient::show()
{
    //XSelectInput( m_dpy, m_wid, PropertyChangeMask );
}

void RenderClient::createPicture()
{
    xcb_void_cookie_t voidCookie;
    auto err = wrap_cptr<xcb_generic_error_t>(nullptr);

    if ( !m_pixmap ) {
        m_pixmap = xcb_generate_id(m_conn);
        voidCookie = xcb_composite_name_window_pixmap_checked(m_conn, m_wid, m_pixmap.get());
        err.reset(xcb_request_check(m_conn, voidCookie));
        if (err)
        {
            qWarning() << "can't create pixmap";
            m_pixmap.reset();
            return;
        }
        //m_pixmap = XCompositeNameWindowPixmap( m_dpy, m_wid );
    }

    m_picture = xcb_generate_id(m_conn);
    voidCookie = xcb_render_create_picture_checked(m_conn, m_picture.get(), m_pixmap.get(), m_format, 0, 0);
    err.reset(xcb_request_check(m_conn, voidCookie));
    if (err)
    {
        qWarning() << "can't create picture";
        m_pixmap.reset();
        m_picture.reset();
    }
    //m_picture = XRenderCreatePicture( m_dpy, m_pixmap.get(), m_format, 0, 0 );
}

xcb_render_picture_t RenderClient::getPicture()
{
    if (!m_picture)
        createPicture();
    //if ( !m_sourceClipValid ) {
    //    XserverRegion clip = XFixesCreateRegionFromWindow( m_dpy, m_wid, WindowRegionBounding );
    //    XFixesSetPictureClipRegion( m_dpy, m_picture.get(), 0, 0, clip );
    //    XFixesDestroyRegion( m_dpy, clip );
    //    m_sourceClipValid = true;
    //}
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

    //mGeometry    = newGeometry;

    //if ( !m_window->isVisible() )
    //    return;
    //
    //// Create an empty region and initialize it with the current visible region

    //if ( m_visibleRegion )
    //    XFixesCopyRegion( m_dpy, damage.get(), m_visibleRegion.get() );
    //
    //// Create a region with the new geometry, and union it with the old
    //XserverRegionPtr region { m_dpy, XFixesCreateRegionFromWindow( m_dpy, m_wid, WindowRegionBounding ) };
    //XFixesUnionRegion( m_dpy, damage.get(),  damage.get(), region.get() );
    //
    //damage?;,,,,
}
