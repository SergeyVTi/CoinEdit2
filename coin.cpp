#include "coin.h"
#include "graphics.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QGraphicsScene>

Coin::Coin() : QGraphicsObject()
{
    setFlags(ItemIsSelectable);// | ItemIsMovable
    setAcceptHoverEvents(true);

}

QRectF Coin::boundingRect() const
{
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

    int width = 1;
    QPen pen(QBrush(Qt::black), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush brush(color_);
    QColor fillColor = color_;

    if (cellNum_.size() == 0){
        painter->setBrush(brush);
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
//    QColor fillColor = (option->state & QStyle::State_Selected) ? color_.darker(150) : color_;
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.lighter(125);

    if (option->state & QStyle::State_MouseOver){
        pen.setColor(Qt::blue);
        pen.setWidth(width+1);
        painter->setPen(pen);
    }

//    pen.setColor(Qt::black);
//    pen.setWidth(width);
//    pen.setJoinStyle(Qt::RoundJoin);

//    painter->setPen(QPen(QBrush(Qt::black), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setPen(pen);
//    painter->setBrush(QBrush(fillColor.darker(option->state & QStyle::State_Sunken ? 120 : 100)));
    brush.setColor(fillColor);
    painter->setBrush(brush);

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
    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if (sectors.size() != 0){
        qreal keff = 0;
        for (int i = 0; i < sectors.size() - 1; i+=2)
        {
            qreal fAngle1 = qDegreesToRadians(30.0 + 360.0 * sectors[i] / n);
            qreal x1 = 1.2*(radius+step + keff) + qCos(fAngle1) * (radius + keff + step);
            qreal y1 = 1.2*(radius+step + keff) + qSin(fAngle1) * (radius + keff + step);
            QPointF p1 = QPointF(x1, y1);

            qreal fAngle2 = qDegreesToRadians(30.0 + 360.0 * sectors[i+1] / n);
            qreal x2 = 1.2*(radius+step + keff) + qCos(fAngle2) * (radius + keff + step);
            qreal y2 = 1.2*(radius+step + keff) + qSin(fAngle2) * (radius + keff + step);
            QPointF p2 = QPointF(x2, y2);

            painter->drawLine(p1, p2);
        }
    }

}

void Coin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
//    QGraphicsItem::mousePressEvent(event);
//    qDebug() << "coin mousePressEvent";
//    qDebug() << cellNum();

    if (event->button() == Qt::RightButton && cellDialog){
        color_ = color_.darker(150);
        update();
    }
//    setSelected(true);
//    scene_->mousePressEvent(event);
//    if (cellDialog){
//        emit updateDialog(this);
//        cellDialog->show();
//        update();
//    }
}

void Coin::mouseMoveEvent(QGraphicsSceneMouseEvent *)
{
//    qDebug() << cellNum_;
    emit printCellNum(cellNum_);
}

void Coin::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
//    qDebug() << "coin mouseReleaseEvent";
//    qDebug() << cellNum();
//    QGraphicsItem::mouseReleaseEvent(event);

    if (event->button() == Qt::RightButton && cellDialog){
        color_ = color_.lighter(150);
        emit updateDialog(this);
        cellDialog->show();
        update();
    }
}

void Coin::setScene(GraphicsScene *newScene)
{
    scene_ = newScene;
}

const QString &Coin::loadingSubType() const
{
    return loadingSubType_;
}

void Coin::setLoadingSubType(const QString &newLoadingSubType)
{
//    emit loadingSubTypeChanged(newLoadingSubType, loadingSubType_);

    loadingSubType_ = newLoadingSubType;
}

void Coin::read(const QJsonObject &json)
{
    if (json.contains(cellNum_) && json[cellNum_].isArray()){
        QJsonArray cellData = json[cellNum_].toArray();
        loadingType_ = cellData.at(0).toString();
        loadingSubType_ = cellData.at(1).toString();
        color_ = cellData.at(2).toString();
    }
}

void Coin::write(QJsonObject &json)
{
    QJsonArray cellData = {
        loadingType_,
        loadingSubType_,
        color_.name()
    };

    json[cellNum_] = cellData;
}

const QString &Coin::loadingType() const
{
    return loadingType_;
}

void Coin::setLoadingType(const QString &newLoadingType)
{
//    emit loadingTypeChanged(newLoadingType, loadingType_);

    loadingType_ = newLoadingType;
}

void Coin::setSectors(const QVector<qreal> &newSectors)
{
    sectors = newSectors;
}

void Coin::setCellDialog(CellDialog *newCellDialog)
{
    cellDialog = newCellDialog;

    connect(this, &Coin::updateDialog, cellDialog, &CellDialog::updateDialog);
//    connect(this, &Coin::loadingTypeChanged, cellDialog, &CellDialog::loadingTypeChanged);
//    connect(this, &Coin::loadingSubTypeChanged, cellDialog, &CellDialog::loadingSubTypeChanged);
}

const QString &Coin::cellNum() const
{
    return cellNum_;
}

void Coin::setCellNum(const QString &newCellNum)
{
    cellNum_ = newCellNum;
}

void Coin::setStep(int newStep)
{
    step = newStep;
}

void Coin::setRadius(int newRadius)
{
    radius = newRadius;
}

const QColor &Coin::color() const
{
    return color_;
}

void Coin::setColor(const QColor &newColor)
{
    color_ = newColor;
}
