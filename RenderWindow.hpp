#ifndef RENDERWINDOW_HPP
#define RENDERWINDOW_HPP

#include <QObject>
#include <QWidget>
#include <QAbstractNativeEventFilter>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <xcb/xcb.h>


class RenderClient;
class OffscreenQmlView;
class RenderWindow : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit RenderWindow(QWidget *parent = nullptr);

    bool init();

    void refresh(size_t requestId);

    bool eventFilter(QObject *, QEvent *event) override;


    inline Window getWindow() {
        return m_wid;
    }

    Picture getBackTexture();

    inline void setVideoClient(RenderClient* client, QWindow* window) {
        m_videoClient = client;
        m_videoWindow = window;
    }

    inline void setInterfaceClient(RenderClient* client, OffscreenQmlView* window) {
        m_interfaceClient = client;
        m_interfaceWindow = window;
    }



protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void paintEvent(QPaintEvent *event) override;


private:
    QWindow* m_videoWindow = nullptr;
    RenderClient* m_videoClient = nullptr;
    OffscreenQmlView* m_interfaceWindow = nullptr;
    RenderClient* m_interfaceClient = nullptr;

    //Display* m_dpy = 0;
    xcb_connection_t* m_conn = nullptr;
    Window m_wid = 0;
    xcb_pixmap_t m_background = 0;
    Picture m_drawingarea = 0;

    int m_xdamageBaseEvent;

    size_t m_refresh_request = 0;

};

#endif // RENDERWINDOW_HPP
