#include "graphics.h"
#include "coinedit.h"

GraphicsView::GraphicsView(CoinEdit *ce)
    : QGraphicsView()
    , coinedit(ce)
{
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(true);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() > 0)
        coinedit->zoomIn(8);
    else
        coinedit->zoomOut(8);
    e->accept();
}

void GraphicsView::mousePressEvent(QMouseEvent *e)
{
    QGraphicsView::mousePressEvent(e);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *e)
{
    QGraphicsView::mouseMoveEvent(e);
}

void GraphicsView::keyPressEvent(QKeyEvent *key)
{
    if (key->key() == Qt::Key_Control) {
        setInteractive(false);
    }
    QGraphicsView::keyPressEvent(key);
}

void GraphicsView::keyReleaseEvent(QKeyEvent *key)
{
    if (key->key() == Qt::Key_Control) {
        setInteractive(true);
    }
    QGraphicsView::keyReleaseEvent(key);
}
