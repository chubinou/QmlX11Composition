#ifndef RENDERCLIENT_HPP
#define RENDERCLIENT_HPP

#include <QObject>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/shape.h>

class RenderClient : public QObject
{
    Q_OBJECT
public:
    RenderClient(Window _wid, QObject* parent = nullptr);

    ~RenderClient();

    void show();

    void createPicture();

    void geometryChanged();

    Picture getPicture();


private:
    Display* m_dpy = 0;
    Pixmap m_pixmap = 0;
    Picture m_picture = 0;
    Window m_wid = 0;
    Damage m_damage = 0;
    XRenderPictFormat* m_format;
    XWindowAttributes m_attr;

    bool m_sourceClipValid = false;
};

#endif // RENDERCLIENT_HPP
