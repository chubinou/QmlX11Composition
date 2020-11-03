#include <QOpenGLContext>
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQuickItem>

#include "OffscreenQmlView.hpp"

OffscreenQmlView::OffscreenQmlView(QScreen* screen)
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

    m_uiRenderControl = new CompositorX11RenderControl(this);

    m_uiWindow = new QQuickWindow(m_uiRenderControl);
    m_uiWindow->setDefaultAlphaBuffer(true);
    m_uiWindow->setFormat(format);
    m_uiWindow->setClearBeforeRendering(true);


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
    qDebug() << "render";
    m_context->makeCurrent(this);

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
