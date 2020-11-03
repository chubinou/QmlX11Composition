#ifndef OFFSCREENQMLVIEW_HPP
#define OFFSCREENQMLVIEW_HPP

#include <QObject>
#include <QWindow>
#include <QOpenGLFunctions>
#include <QQuickRenderControl>

class CompositorX11RenderControl : public QQuickRenderControl {
    Q_OBJECT
public:
    CompositorX11RenderControl(QWindow* window, QObject* parent = nullptr)
        : QQuickRenderControl(parent)
        , m_window(window)
    {}

    QWindow *renderWindow(QPoint * offset) override
    {
        if (offset)
            *offset = QPoint(0, 0);
        return m_window;
    }

private:
    QWindow* m_window = nullptr;
};

class QOpenGLPaintDevice;
class QQuickWindow;
class QQmlEngine;
class QQmlComponent;
class QQuickItem;
class OffscreenQmlView : public QWindow //, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OffscreenQmlView(QWindow* window, QScreen *screen = nullptr);
    ~OffscreenQmlView();

    virtual void render();

    bool handleWindowEvent(QEvent *event);


    void setContent(QQmlComponent*,  QQuickItem* rootItem);

    QQmlEngine* engine() const { return m_qmlEngine; }

signals:
    void beforeRendering();
    void afterRendering();

protected:
    bool event(QEvent *event) override;

    void resizeEvent(QResizeEvent *) override;
    void exposeEvent(QExposeEvent *) override;
    void handleScreenChange();


    void updateSizes();

    void createFbo();
    void destroyFbo();
    void resizeFbo();

private:
    QQuickItem* m_rootItem = nullptr;
    QOpenGLContext *m_context = nullptr;
    QOpenGLPaintDevice *m_device = nullptr;
    QQuickWindow* m_uiWindow = nullptr;
    QQmlEngine* m_qmlEngine = nullptr;
    CompositorX11RenderControl* m_uiRenderControl = nullptr;

    QSize m_onscreenSize;
};

#endif // OFFSCREENQMLVIEW_HPP
