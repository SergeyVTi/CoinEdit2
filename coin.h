#pragma once

#include <QColor>
#include <QGraphicsItem>

class Coin : public QGraphicsItem
{
public:
    Coin(const QColor &color, const int radius, const int step, const QString cellNum = "", const QVector<qreal> sectors = {});

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int radius;
    int step;
    QString cellNum;
    QColor color;
    QVector<QPointF> stuff;
    QVector<qreal> sectors;

};
