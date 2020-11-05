#include <memory>

#include <xcb/damage.h>
#include <xcb/xfixes.h>
#include <xcb/render.h>

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



template<typename T, typename R, R RELEASE>
class X11Resource {
public:
    X11Resource() = delete;

    explicit X11Resource(xcb_connection_t* conn, T _xid = 0)
        : m_conn(conn)
        , xid(_xid)
    {}

    X11Resource(const X11Resource &other) = delete;
    X11Resource(X11Resource &&other)
        : m_conn (other.m_conn)
        , xid (other.xid)
    {
        other.xid = 0;
    }

    ~X11Resource()
    {
        if (xid)
            RELEASE(m_conn, xid);
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
            RELEASE(m_conn, xid);
        xid = newval;
    }

    operator bool() noexcept
    {
        return xid != 0;
    }

    T get() const { return xid; }
    //operator T() const { return xid; }

    xcb_connection_t* m_conn;
    T xid = 0;
};

using  DamagePtr = X11Resource<xcb_damage_damage_t, decltype(&xcb_damage_destroy), xcb_damage_destroy>;
using  PixmapPtr = X11Resource<xcb_pixmap_t, decltype(&xcb_free_pixmap), xcb_free_pixmap>;
using  PicturePtr = X11Resource<xcb_render_picture_t, decltype(&xcb_render_free_picture), xcb_render_free_picture>;
using  RegionPtr = X11Resource<xcb_xfixes_region_t, decltype(&xcb_xfixes_destroy_region), xcb_xfixes_destroy_region>;

//using  PixmapPtr = X11Resource<Pixmap, decltype(&XFreePixmap), XFreePixmap>;
//using  DamagePtr = X11Resource<Damage, decltype(&XDamageDestroy), XDamageDestroy>;
//using  PicturePtr = X11Resource<Picture, decltype(&XRenderFreePicture), XRenderFreePicture>;
//using  RegionPtr = X11Resource<XserverRegion, decltype(&XFixesDestroyRegion), XFixesDestroyRegion>;

bool findVisualFormat(xcb_connection_t*  conn, xcb_visualid_t visual, xcb_render_pictformat_t* fmtOut, uint8_t* depthOut);
