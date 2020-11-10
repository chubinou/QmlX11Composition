#ifndef RENDERWINDOW_HPP
#define RENDERWINDOW_HPP

#include <QObject>
#include <QWidget>
#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>
#include <xcb/render.h>
#include "X11Utils.hpp"


class RenderClient;
class OffscreenQmlView;
class RenderWindow : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit RenderWindow(QWidget *parent = nullptr);
    ~RenderWindow();

    bool init();

    void refresh(unsigned short requestId);

    void requestRefresh();

    bool eventFilter(QObject *, QEvent *event) override;


    inline xcb_window_t getWindow() {
        return m_wid;
    }

    xcb_render_picture_t getBackTexture();

    void setVideoWindow(QWindow* window);
    void setInterfaceWindow(OffscreenQmlView* window);

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void paintEvent(QPaintEvent *event) override;


private:
    QWindow* m_videoWindow = nullptr;
    std::unique_ptr<RenderClient> m_videoClient;
    OffscreenQmlView* m_interfaceWindow = nullptr;
    std::unique_ptr<RenderClient> m_interfaceClient;

    xcb_connection_t* m_conn = nullptr;
    xcb_window_t m_wid = 0;
    PicturePtr m_drawingarea;

    int m_xdamageBaseEvent;

    unsigned short m_refreshRequestId = 0;

};

#endif // RENDERWINDOW_HPP
