#include <QtEvents>
#include <QCoreApplication>
#include <QWindow>
#include <QQuickItem>
#include <QX11Info>
#include "RenderWindow.hpp"
#include "RenderClient.hpp"
#include "OffscreenQmlView.hpp"

#undef KeyPress
#undef KeyRelease
#undef None

RenderWindow::RenderWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    m_dpy = QX11Info::display();

    m_wid = winId();

    windowHandle()->
            installEventFilter(this);
}

void RenderWindow::refresh()
{
    Picture pic;

    XGrabServer(m_dpy); //avoids tearing by locking the server
    XFlush(m_dpy);

    Picture drawingarea = getBackTexture();

    int realW = width()  * devicePixelRatioF();
    int realH = height() * devicePixelRatioF();

    if (m_videoClient) {
        pic = m_videoClient->getPicture();
        XRenderComposite(m_dpy, PictOpOver, pic, 0, drawingarea,
                         0,0, 0, 0,
                         0, 0, realW, realH);
    }

    if (m_interfaceClient) {
        pic = m_interfaceClient->getPicture();
        XRenderComposite(m_dpy, PictOpOver, pic, 0, m_drawingarea,
                         0,0, 0, 0,
                         0, 0, realW, realH);
    }

    XClearArea(m_dpy, m_wid, 0, 0, realW, realH, 0);
    XUngrabServer(m_dpy);
}



static void remapInputMethodQueryEvent(QObject *object, QInputMethodQueryEvent *e)
{
    auto item = qobject_cast<QQuickItem *>(object);
    if (!item)
        return;
    // Remap all QRectF values.
    for (auto query : {Qt::ImCursorRectangle, Qt::ImAnchorRectangle, Qt::ImInputItemClipRectangle})
    {
        if (e->queries() & query)
        {
            auto value = e->value(query);
            if (value.canConvert<QRectF>())
                e->setValue(query, item->mapRectToScene(value.toRectF()));
        }
    }
    // Remap all QPointF values.
    if (e->queries() & Qt::ImCursorPosition)
    {
        auto value = e->value(Qt::ImCursorPosition);
        if (value.canConvert<QPointF>())
            e->setValue(Qt::ImCursorPosition, item->mapToScene(value.toPointF()));
    }   
}


bool RenderWindow::eventFilter(QObject*, QEvent* event)
{
    if (m_videoWindow) {
        switch (event->type()) {

        case QEvent::Move:
        case QEvent::Show:
        {
            m_videoWindow->setPosition(pos());
            refresh();
            break;
        }
        case QEvent::Resize:
        {
            QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
            m_videoWindow->resize(resizeEvent->size());
            m_videoClient->geometryChanged();
            break;
        }
        default: break;
        }
    }

    bool ret = false;
    if (m_interfaceWindow)
        ret =  m_interfaceWindow->handleWindowEvent(event);

    switch (event->type()) {

    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Resize:
        m_interfaceClient->geometryChanged();
        refresh();
        break;
    default: break;
    }

    return ret;
}

Picture RenderWindow::getBackTexture() {
    if (m_drawingarea)
        return m_drawingarea;

    int screen=DefaultScreen(m_dpy);
    Visual *visual=DefaultVisual(m_dpy, screen);
    XRenderPictFormat* fmt = XRenderFindVisualFormat(m_dpy, visual);
    int depth = DefaultDepth(m_dpy, screen);
    int width = DisplayWidth(m_dpy, screen);
    int height = DisplayHeight(m_dpy, screen);

    m_background = XCreatePixmap(m_dpy, m_wid, width, height, depth);
    //XSelectInput(m_dpy, m_wid, StructureNotifyMask);
    XSetWindowBackgroundPixmap(m_dpy, m_wid, m_background);
    //XMapWindow(m_dpy, m_wid);

    XRenderPictureAttributes pict_attr;
    pict_attr.poly_edge=PolyEdgeSmooth;
    pict_attr.poly_mode=PolyModeImprecise;
    m_drawingarea = XRenderCreatePicture(m_dpy, m_background, fmt, CPPolyEdge|CPPolyMode,
                                         &pict_attr);

    return m_drawingarea;
}

void RenderWindow::paintEvent(QPaintEvent* event)
{
    qDebug() << "paintEvent";
    QWidget::paintEvent(event);
    refresh();
}

void RenderWindow::keyPressEvent(QKeyEvent* ev)
{
    qDebug() << "You Pressed Key " << ev->text();
}

void RenderWindow::keyReleaseEvent(QKeyEvent* ev)
{
    qDebug() << "You Pressed Key " << ev->text();
}
