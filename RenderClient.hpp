#ifndef RENDERCLIENT_HPP
#define RENDERCLIENT_HPP

#include <QObject>
#include <QX11Info>
#include <QWindow>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/shape.h>

template<typename T, typename R, R RELEASE>
class X11Resource {
public:
    X11Resource() = delete;

    explicit X11Resource(Display* _dpy, T _xid = 0)
        : dpy(_dpy)
        , xid(_xid)
    {}

    X11Resource(const X11Resource &other) = delete;
    X11Resource(X11Resource &&other)
        : dpy (other.dpy)
        , xid (other.xid)
    {
        other.xid = 0;
    }

    ~X11Resource()
    {
        if (xid)
            RELEASE(dpy, xid);
    }

    X11Resource &operator=(const X11Resource &other) = delete;
    X11Resource &operator=(X11Resource &&other) noexcept
    {
        reset(other.xid);
        other.xid = 0;
        return *this;
    }
    X11Resource &operator=(T value) noexcept
    {
        reset(value);
        return *this;
    }

    void reset(T newval = 0) {
        if (xid)
            RELEASE(dpy, xid);
        xid = newval;
    }

    operator bool() noexcept
    {
        return xid != 0;
    }

    T get() const { return xid; }
    //operator T() const { return xid; }

    Display* dpy;
    T xid = 0;
};

//using  XcbDamagePtr = X11Resource<xcb_damage_damage_t, decltype(&xcb_damage_destroy), xcb_damage_destroy>;
//using  XcbPixmapPtr = X11Resource<xcb_pixmap_t, decltype(&xcb_free_pixmap), xcb_free_pixmap>;
//using  XcbPicturePtr = X11Resource<xcb_render_picture_t, decltype(&xcb_render_free_picture), xcb_render_free_picture>;
//using  XcbRegionPtr = X11Resource<xcb_xfixes_region_t, decltype(&xcb_xfixes_destroy_region), xcb_xfixes_destroy_region>;


using  PixmapPtr = X11Resource<Pixmap, decltype(&XFreePixmap), XFreePixmap>;
using  DamagePtr = X11Resource<Damage, decltype(&XDamageDestroy), XDamageDestroy>;
using  PicturePtr = X11Resource<Picture, decltype(&XRenderFreePicture), XRenderFreePicture>;
using  XserverRegionPtr = X11Resource<XserverRegion, decltype(&XFixesDestroyRegion), XFixesDestroyRegion>;

class RenderClient : public QObject
{
    Q_OBJECT
public:
    RenderClient(QWindow* window, QObject* parent = nullptr);

    ~RenderClient();

    void show();

    void createPicture();


    Picture getPicture();

public slots:
    void geometryChanged();

private:
    QWindow* m_window = nullptr;

    Display* m_dpy = 0;
    Window m_wid = 0;
    PixmapPtr m_pixmap;
    PicturePtr m_picture;
    DamagePtr m_damage;
    XserverRegionPtr m_shapeRegion;
    XserverRegionPtr m_visibleRegion;
    XRenderPictFormat* m_format;
    XWindowAttributes m_attr;

    bool m_sourceClipValid = false;
};

#endif // RENDERCLIENT_HPP
