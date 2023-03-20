#include "graphics.h"
#include "coinedit.h"
#include "coin.h"
#include <QWheelEvent>
#include <QDebug>

GraphicsView::GraphicsView(CoinEdit *ce) : QGraphicsView(), coinedit(ce) {
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(true);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
//    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
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
//    qDebug() << e->globalPos();
    setInteractive(false);
    QGraphicsView::mousePressEvent(e);
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
//    qDebug() << items(e->pos());
//    for(auto& item : items(e->pos())){
//        qDebug()<<item->childItems();
//    }
//    Coin *coin = qobject_cast<Coin *>(items(e->pos()).at(0));
//    if (coin)
//        qDebug() << coin->cellNum();
    QGraphicsView::mouseMoveEvent(e);
}

//void GraphicsView::keyPressEvent(QKeyEvent *key)
//{
//     if (key->key() == Qt::Key_Control){
//         setInteractive(false);
//     }
//     QGraphicsView::keyPressEvent(key);
//}

//void GraphicsView::keyReleaseEvent(QKeyEvent *key)
//{
//    if (key->key() == Qt::Key_Control){
//        setInteractive(true);
//    }
//    QGraphicsView::keyReleaseEvent(key);
//}

//void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    GraphicsScene::mousePressEvent(event);
//}
