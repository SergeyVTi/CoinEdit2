#include "coin.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

Coin::Coin(const QColor &color, const int radius, const int step, const QString cellNum, const QVector<qreal> sectors)
{
    this->radius = radius;
    this->color = color;
    this->cellNum = cellNum;
    this->step = step;
    this->sectors = sectors;
//    setZValue((x + y) % 2);

    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}

QRectF Coin::boundingRect() const
{
//    return QRectF(QPointF(-1*(radius+step),(radius+step)),QPointF((radius+step),-1*(radius+step)));
    return QRectF(0, 0, (radius+step)*2.1, (radius+step)*2.1);
}

QPainterPath Coin::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void Coin::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing, true);
//    this->painter = painter;

    if (cellNum.size() == 0){
//        QPen oldPen = painter->pen();
//        QPen pen = painter->pen();
//        pen.setBrush(QBrush(Qt::red));

//        QBrush b = painter->brush();
        painter->setBrush(QBrush(color));
//        painter->setPen(QPen(QBrush(Qt::red), step, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setPen(Qt::NoPen);

        const int n = 6;
        QPointF a[n];

        //Draw item
        for (int i = 0; i < n; ++i) {
            qreal fAngle = qDegreesToRadians(30.0 + 360.0 * i / n);
            qreal x = (radius+step) + qCos(fAngle) * radius;
            qreal y = (radius+step) + qSin(fAngle) * radius;
            a[i] = QPointF(x, y);
        }
        painter->drawPolygon(a, n);

        return;
    }

    //Highlighter
    QColor fillColor = (option->state & QStyle::State_Selected) ? color.darker(150) : color;
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.lighter(125);

    QPen oldPen = painter->pen();
    QPen pen = oldPen;
    int width = 0;
    if (option->state & QStyle::State_Selected)
        width += 2;

    pen.setWidth(width);
    QBrush b = painter->brush();
    painter->setBrush(QBrush(fillColor.darker(option->state & QStyle::State_Sunken ? 120 : 100)));

    const int n = 6;
    QPointF a[n];

    //Draw item
    for (int i = 0; i < n; ++i)
    {
        qreal fAngle = qDegreesToRadians(30.0 + 360.0 * i / n);
        qreal x = 1.2*(radius+step) + qCos(fAngle) * radius;
        qreal y = 1.2*(radius+step) + qSin(fAngle) * radius;
        a[i] = QPointF(x, y);
    }
    painter->drawPolygon(a, n);

    //Draw sector lines
    if (sectors.size() != 0){
        pen.setWidth(2);
        pen.setColor(Qt::black);
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        qreal keff = 0;
        for (int i = 0; i < sectors.size() - 1; i+=2)
        {
            qreal fAngle1 = qDegreesToRadians(30.0 + 360.0 * sectors[i] / n);
            qreal x1 = 1.2*(radius+step + keff) + qCos(fAngle1) * (radius + keff + step);
//            qreal x1 = 1.2*(radius+step) + qCos(fAngle1) * ((1.2*radius - radius)/2 + radius + 1);
            qreal y1 = 1.2*(radius+step + keff) + qSin(fAngle1) * (radius + keff + step);
            QPointF p1 = QPointF(x1, y1);

            qreal fAngle2 = qDegreesToRadians(30.0 + 360.0 * sectors[i+1] / n);
            qreal x2 = 1.2*(radius+step + keff) + qCos(fAngle2) * (radius + keff + step);
            qreal y2 = 1.2*(radius+step + keff) + qSin(fAngle2) * (radius + keff + step);
            QPointF p2 = QPointF(x2, y2);

            painter->drawLine(p1, p2);
        }
    }

    QFont font("Times", 10, QFont::Normal, false);
    pen.setColor(Qt::black);
    painter->setPen(pen);
    painter->setFont(font);
    QRectF rectangle = QRectF(0, 0, (1.2*radius+step)*2, (1.2*radius+step)*2);
    painter->drawText(QRectF(0, 0, (1.2*radius+step)*2, (1.2*radius+step)*2), Qt::AlignCenter, cellNum, &rectangle);
}

void Coin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void Coin::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        stuff << event->pos();
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void Coin::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}
