#ifndef RENDERWINDOW_HPP
#define RENDERWINDOW_HPP

#include <QObject>
#include <QWidget>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include "RenderClient.hpp"

class RenderWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RenderWindow(QWidget *parent = nullptr);

    void refresh();

    bool eventFilter(QObject *watched, QEvent *event) override;


    inline Window getWindow() {
        return m_wid;
    }

    inline Picture getBackTexture() {
        return m_drawingarea;
    }

    inline void setVideoClient(RenderClient* client) {
        m_videoClient = client;
    }

    inline void setInterfaceClient(RenderClient* client, QWindow* window) {
        m_interfaceClient = client;
        m_interfaceWindow = window;
    }


private:
    RenderClient* m_videoClient = nullptr;
    QWindow* m_interfaceWindow = nullptr;
    RenderClient* m_interfaceClient = nullptr;
    Display* m_dpy = 0;
    Window m_wid = 0;
    Pixmap m_background = 0;
    Picture m_drawingarea = 0;

};

#endif // RENDERWINDOW_HPP