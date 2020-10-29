#include <QDebug>
#include "RenderClient.hpp"
#include <X11/extensions/Xcomposite.h>

#undef None

RenderClient::RenderClient(QWindow* window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_dpy (QX11Info::display())
    , m_wid(window->winId())
    , m_pixmap(m_dpy)
    , m_picture(m_dpy)
    , m_damage(m_dpy)
    , m_shapeRegion(m_dpy)
    , m_visibleRegion(m_dpy)
{

    //XSetWindowAttributes attr;
    //int baseEventMask = 0
    //        | ExposureMask
    //        | SubstructureNotifyMask
    //        | PropertyChangeMask
    //        | FocusChangeMask
    //        ;
    //
    //int defaultEventMask = baseEventMask
    //       | KeyPressMask | KeyReleaseMask
    //       | ButtonPressMask | ButtonReleaseMask
    //       | EnterWindowMask | LeaveWindowMask
    //       | PointerMotionMask | PointerMotionHintMask
    //       | ButtonMotionMask
    //       | Button1MotionMask | Button2MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask
    //       | KeymapStateMask
    //       | StructureNotifyMask
    //        ;
    //int transparentForInputEventMask = baseEventMask
    //        | VisibilityChangeMask
    //        | ResizeRedirectMask
    //        | SubstructureRedirectMask
    //        | ColormapChangeMask
    //        | OwnerGrabButtonMask
    //        ;
    //attr.event_mask = transparentForInputEventMask;
    //attr.override_redirect = true;
    //XChangeWindowAttributes(m_dpy, m_wid, CWEventMask|CWOverrideRedirect , &attr);

    XGetWindowAttributes( m_dpy, m_wid, &m_attr );

    XCompositeRedirectWindow(m_dpy, m_wid, CompositeRedirectManual);

    m_format = XRenderFindVisualFormat( m_dpy, m_attr.visual );
    m_damage = XDamageCreate( m_dpy, m_wid, XDamageReportNonEmpty );

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

    m_shapeRegion.reset();

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
    //XserverRegionPtr damage{ m_dpy, XFixesCreateRegion( m_dpy, 0, 0  ) };
    //if ( m_visibleRegion )
    //    XFixesCopyRegion( m_dpy, damage.get(), m_visibleRegion.get() );
    //
    //// Create a region with the new geometry, and union it with the old
    //XserverRegionPtr region { m_dpy, XFixesCreateRegionFromWindow( m_dpy, m_wid, WindowRegionBounding ) };
    //XFixesUnionRegion( m_dpy, damage.get(),  damage.get(), region.get() );
    //
    //damage?;,,,,
}
