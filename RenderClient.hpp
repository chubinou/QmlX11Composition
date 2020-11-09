#ifndef RENDERCLIENT_HPP
#define RENDERCLIENT_HPP


#include <QObject>
#include <QX11Info>
#include <QWindow>

#include "X11Utils.hpp"

class RenderClient : public QObject
{
    Q_OBJECT
public:
    RenderClient(QWindow* window, QObject* parent = nullptr);

    ~RenderClient();

    void createPicture();

    xcb_render_picture_t getPicture();

public slots:
    void geometryChanged();

private:
    QWindow* m_window = nullptr;

    xcb_connection_t* m_conn = 0;
    xcb_window_t m_wid = 0;
    PixmapPtr m_pixmap;
    PicturePtr m_picture;
    DamagePtr m_damage;

    xcb_render_pictformat_t m_format;

    bool m_sourceClipValid = false;
};

#endif // RENDERCLIENT_HPP
