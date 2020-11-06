#ifndef RENDERWINDOW_HPP
#define RENDERWINDOW_HPP

#include <QObject>
#include <QWidget>
#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>
#include <xcb/render.h>


class RenderClient;
class OffscreenQmlView;
class RenderWindow : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit RenderWindow(QWidget *parent = nullptr);

    bool init();

    void refresh(unsigned short requestId);

    void requestRefresh();

    bool eventFilter(QObject *, QEvent *event) override;


    inline xcb_window_t getWindow() {
        return m_wid;
    }

    xcb_render_picture_t getBackTexture();

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
    xcb_window_t m_wid = 0;
    xcb_pixmap_t m_background = 0;
    xcb_render_picture_t m_drawingarea = 0;

    int m_xdamageBaseEvent;

    unsigned short m_refreshRequestId = 0;

};

#endif // RENDERWINDOW_HPP
