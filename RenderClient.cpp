#include <QDebug>
#include "RenderClient.hpp"
#include <X11/extensions/Xcomposite.h>

#undef None

RenderClient::RenderClient(QWindow* window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_conn(QX11Info::connection())
    , m_dpy (QX11Info::display())
    , m_wid(window->winId())
    , m_pixmap(m_dpy)
    , m_picture(m_dpy)
    , m_damage(m_dpy)
{
    XGetWindowAttributes( m_dpy, m_wid, &m_attr );

    //reparent the window to the Composite overlay to hide it from other compositor?
    Window cow = XCompositeGetOverlayWindow(m_dpy, DefaultRootWindow(m_dpy));
    XReparentWindow(m_dpy, m_wid, cow, 0,0);

    XCompositeRedirectWindow(m_dpy, m_wid, CompositeRedirectManual);

    m_format = XRenderFindVisualFormat( m_dpy, m_attr.visual );
    xcb_damage_damage_t dam = xcb_generate_id(m_conn);
    xcb_damage_create(m_conn, dam, m_wid, XCB_DAMAGE_REPORT_LEVEL_RAW_RECTANGLES);
    //m_damage = XDamageCreate( m_dpy, m_wid, XDamageReportNonEmpty );

    // Make sure we get notified when the window shape is changed
    XShapeSelectInput( m_dpy, m_wid, ShapeNotifyMask );

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
    XSelectInput( m_dpy, m_wid, PropertyChangeMask );
}

void RenderClient::createPicture()
{
    if ( !m_pixmap )
        m_pixmap = XCompositeNameWindowPixmap( m_dpy, m_wid );
    m_picture = XRenderCreatePicture( m_dpy, m_pixmap.get(), m_format, 0, 0 );
}

Picture RenderClient::getPicture()
{
    if (!m_picture)
        createPicture();
    if ( !m_sourceClipValid ) {
        XserverRegion clip = XFixesCreateRegionFromWindow( m_dpy, m_wid, WindowRegionBounding );
        XFixesSetPictureClipRegion( m_dpy, m_picture.get(), 0, 0, clip );
        XFixesDestroyRegion( m_dpy, clip );
        m_sourceClipValid = true;
    }
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
