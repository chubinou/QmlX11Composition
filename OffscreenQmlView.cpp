#include <QOpenGLContext>
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQuickItem>

#include "OffscreenQmlView.hpp"

OffscreenQmlView::OffscreenQmlView(QWindow* window, QScreen* screen)
    : QWindow(screen)
{
    bool ret;

    setSurfaceType(QWindow::OpenGLSurface);


    QSurfaceFormat format;
    // Qt Quick may need a depth and stencil buffer. Always make sure these are available.
    format.setDepthBufferSize(8);
    format.setStencilBufferSize(8);
    format.setAlphaBufferSize(8);

    setFormat(format);

    m_context = new QOpenGLContext();
    m_context->setScreen(this->screen());
    m_context->setFormat(format);
    ret = m_context->create();


    m_uiRenderControl = new CompositorX11RenderControl(window);

    m_uiWindow = new QQuickWindow(m_uiRenderControl);
    m_uiWindow->setDefaultAlphaBuffer(true);
    m_uiWindow->setFormat(format);
    m_uiWindow->setClearBeforeRendering(false);


    m_qmlEngine = new QQmlEngine();
    if (!m_qmlEngine->incubationController())
        m_qmlEngine->setIncubationController(m_uiWindow->incubationController());

    connect(m_uiWindow, &QQuickWindow::sceneGraphInitialized, this, &OffscreenQmlView::createFbo);
    connect(m_uiWindow, &QQuickWindow::sceneGraphInvalidated, this, &OffscreenQmlView::destroyFbo);
    connect(m_uiWindow, &QQuickWindow::beforeRendering, this, &OffscreenQmlView::beforeRendering);
    connect(m_uiWindow, &QQuickWindow::afterRendering, this, &OffscreenQmlView::afterRendering);

    connect(m_uiRenderControl, &QQuickRenderControl::renderRequested, this, &OffscreenQmlView::requestUpdate);
    connect(m_uiRenderControl, &QQuickRenderControl::sceneChanged, this, &OffscreenQmlView::requestUpdate);
}

OffscreenQmlView::~OffscreenQmlView()
{
    delete m_device;
}


void OffscreenQmlView::setContent(QQmlComponent*,  QQuickItem* rootItem)
{
    qDebug() << "setContent" << rootItem;
    m_rootItem = rootItem;

    QQuickItem* contentItem  = m_uiWindow->contentItem();

    m_rootItem->setParentItem(contentItem);

    updateSizes();

}

void OffscreenQmlView::createFbo()
{
    qDebug() << "createFbo";
    //write to the immediate context
    m_uiWindow->setRenderTarget(0, size() * devicePixelRatio());
}

void OffscreenQmlView::destroyFbo()
{
    qDebug() << "destroyFbo";
}

void OffscreenQmlView::render()
{
    if (!isExposed())
        return;
    QSize realSize = size() * devicePixelRatio();

    m_context->makeCurrent(this);

    m_context->functions()->glViewport(0, 0, realSize.width(), realSize.height());
    m_context->functions()->glScissor( 0, 0, realSize.width(), realSize.height());
    m_context->functions()->glEnable(GL_SCISSOR_TEST);
    m_context->functions()->glClearColor(0.,0.,0.,0.);
    m_context->functions()->glClear(GL_COLOR_BUFFER_BIT);

    m_uiRenderControl->polishItems();
    m_uiRenderControl->sync();
    m_uiRenderControl->render();

    m_uiWindow->resetOpenGLState();

    m_context->functions()->glFlush(); //glFinish?
    m_context->doneCurrent();
    m_context->swapBuffers(this);

}


void OffscreenQmlView::updateSizes()
{
    qDebug() << "updateSizes";
    qreal dpr = devicePixelRatio();
    QSize windowSize = size();

    m_onscreenSize = windowSize * dpr;
    //resizeSwapchain(windowSize.width() * dpr, windowSize.height() * dpr);
    //updateSharedTexture(windowSize.width() * dpr, windowSize.height() * dpr);

    // Behave like SizeRootObjectToView.
    m_rootItem->setSize(windowSize);
    m_uiWindow->resize(windowSize);
}

bool OffscreenQmlView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        render();
        return true;
    default:
        return QWindow::event(event);
    }
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

bool OffscreenQmlView::handleWindowEvent(QEvent *event)
{
    switch (event->type()) {

    case QEvent::Move:
    case QEvent::Show:
    {
        QPoint windowPosition = mapToGlobal(QPoint(0,0));
        if (m_uiWindow->position() != windowPosition)
            m_uiWindow->setPosition(windowPosition);
        break;
    }

    case QEvent::Resize:
    {
        QResizeEvent* resizeEvent = static_cast<QResizeEvent*>(event);
        m_uiWindow->resize(resizeEvent->size());
        resize( resizeEvent->size() );
        resizeFbo();
        break;
    }

    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::Leave:
    {
        qDebug()  << event;
        return QCoreApplication::sendEvent(m_uiWindow, event);
    }

    case QEvent::Enter:
    {
        qDebug()  << event;
        QEnterEvent *enterEvent = static_cast<QEnterEvent *>(event);
        QEnterEvent mappedEvent(enterEvent->localPos(), enterEvent->windowPos(),
                                enterEvent->screenPos());
        bool ret = QCoreApplication::sendEvent(m_uiWindow, &mappedEvent);
        event->setAccepted(mappedEvent.isAccepted());
        return ret;
    }

    case QEvent::InputMethod:
        return QCoreApplication::sendEvent(m_uiWindow->focusObject(), event);

    case QEvent::InputMethodQuery:
    {
        bool eventResult = QCoreApplication::sendEvent(m_uiWindow->focusObject(), event);
        // The result in focusObject are based on offscreenWindow. But
        // the inputMethodTransform won't get updated because the focus
        // is on QQuickWidget. We need to remap the value based on the
        // widget.
        remapInputMethodQueryEvent(m_uiWindow->focusObject(), static_cast<QInputMethodQueryEvent *>(event));
        return eventResult;
    }

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QMouseEvent mappedEvent(mouseEvent->type(), mouseEvent->localPos(),
                                mouseEvent->localPos(), mouseEvent->screenPos(),
                                mouseEvent->button(), mouseEvent->buttons(),
                                mouseEvent->modifiers(), mouseEvent->source());
        QCoreApplication::sendEvent(m_uiWindow, &mappedEvent);
        return true;
    }

    case QEvent::Wheel:
    case QEvent::HoverEnter:
    case QEvent::HoverLeave:
    case QEvent::HoverMove:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
    case QEvent::DragResponse:
    case QEvent::Drop:
        return QCoreApplication::sendEvent(m_uiWindow, event);

    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        return QCoreApplication::sendEvent(m_uiWindow, event);
    }

    case QEvent::ScreenChangeInternal:
        m_uiWindow->setScreen(screen());
        break;

    default:
        break;
    }
    return false;
}

void OffscreenQmlView::resizeFbo()
{
    qDebug() << "resizeFbo";
    if (m_rootItem && m_context->makeCurrent(this)) {
        createFbo();
        m_context->doneCurrent();
        updateSizes();
        //render();
    }
}

void OffscreenQmlView::resizeEvent(QResizeEvent *)
{
    qDebug() << "resizeEvent";
    if (m_onscreenSize != size() * devicePixelRatio()) {
        resizeFbo();
    }
}

void OffscreenQmlView::exposeEvent(QExposeEvent *)
{
    qDebug() << "exposeEvent";
    if (isExposed()) {

        m_context->makeCurrent(this);
        m_uiRenderControl->initialize(m_context);
        m_context->doneCurrent();

        requestUpdate();
    }
}


void OffscreenQmlView::handleScreenChange()
{
    qDebug() << "handleScreenChange";
    m_uiWindow->setGeometry(0, 0, width(), height());;
    requestUpdate();
}
