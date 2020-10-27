#include "RenderClient.hpp"
#include <QX11Info>
#include <X11/extensions/Xcomposite.h>

RenderClient::RenderClient(Window _wid, QObject *parent)
    : QObject(parent)
    , m_wid(_wid)
{
    m_dpy = QX11Info::display();

    XCompositeRedirectWindow(m_dpy, m_wid, CompositeRedirectManual);
    XGetWindowAttributes( m_dpy, m_wid, &m_attr );

    m_format = XRenderFindVisualFormat( m_dpy, m_attr.visual );
    m_damage = XDamageCreate( m_dpy, m_wid, XDamageReportNonEmpty );

    // Make sure we get notified when the window shape is changed
    XShapeSelectInput( m_dpy, m_wid, ShapeNotifyMask );

    show();
}

RenderClient::~RenderClient() {
    if (m_pixmap)
        XFreePixmap( m_dpy, m_pixmap );
    if (m_picture)
        XRenderFreePicture( m_dpy, m_picture );
    if ( m_damage )
        XDamageDestroy( m_dpy, m_damage );

}

void RenderClient::show()
{
    XSelectInput( m_dpy, m_wid, PropertyChangeMask );
}

void RenderClient::createPicture()
{
    if ( !m_pixmap )
        m_pixmap = XCompositeNameWindowPixmap( m_dpy, m_wid );
    m_picture = XRenderCreatePicture( m_dpy, m_pixmap, m_format, 0, 0 );
}

Picture RenderClient::getPicture()
{
    if (!m_picture)
        createPicture();
    if ( !m_sourceClipValid ) {
        XserverRegion clip = XFixesCreateRegionFromWindow( m_dpy, m_wid, WindowRegionBounding );
        XFixesSetPictureClipRegion( m_dpy, m_picture, 0, 0, clip );
        XFixesDestroyRegion( m_dpy, clip );
        m_sourceClipValid = true;
    }
    return m_picture;
}
