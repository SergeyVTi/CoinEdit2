#include "graphics.h"
#include "coinedit.h"
#include <QWheelEvent>
#include <QDebug>

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
    if (e->button() == Qt::LeftButton){
        setInteractive(false);
        QGraphicsView::mousePressEvent(e);
        return;
    }

    setInteractive(true);
    QGraphicsView::mousePressEvent(e);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    QGraphicsView::mouseReleaseEvent(e);
    setInteractive(true);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *e)
{
    QGraphicsView::mouseMoveEvent(e);
}

//void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    GraphicsScene::mousePressEvent(event);
//}
