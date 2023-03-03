#include "coin.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <QGraphicsScene>

Coin::Coin() : QGraphicsObject()
{
    setFlags(ItemIsSelectable | ItemIsMovable);
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
//    this->painter = painter;

    if (cellNum_.size() == 0){
//        QPen oldPen = painter->pen();
//        QPen pen = painter->pen();
//        pen.setBrush(QBrush(Qt::red));

//        QBrush b = painter->brush();
        painter->setBrush(QBrush(color_));
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
    //
    //if (option->state & QStyle::State_Selected)

    //Highlighter
    QColor fillColor = (option->state & QStyle::State_Selected) ? color_.darker(150) : color_;
    if (option->state & QStyle::State_MouseOver)
        fillColor = fillColor.lighter(125);

    QPen oldPen = painter->pen();
    QPen pen = oldPen;
    int width = 0;
    if (option->state & QStyle::State_Selected)
        width += 2;//

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
    painter->drawText(QRectF(0, 0, (1.2*radius+step)*2, (1.2*radius+step)*2), Qt::AlignCenter, cellNum_, &rectangle);
}

void Coin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
//    scene()->clearSelection();
//    setSelected(true);
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

//    cellDialog->setCellNumLabel(cellNum_);
//    cellDialog->setCellNumLabel(this);

    emit updateDialog(this);
    cellDialog->show();
    update();
}

const QString &Coin::loadingSubType() const
{
    return loadingSubType_;
}

void Coin::setLoadingSubType(const QString &newLoadingSubType)
{
    emit loadingSubTypeChanged(newLoadingSubType, loadingSubType_);

    loadingSubType_ = newLoadingSubType;
}

void Coin::read(const QJsonObject &json)
{
    if (json.contains(cellNum_) && json[cellNum_].isArray()){
        QJsonArray cellData = json[cellNum_].toArray();
        loadingType_ = cellData.at(0).toString();
        loadingSubType_ = cellData.at(1).toString();
    }
}

void Coin::write(QJsonObject &json)
{
    QJsonArray cellData = {
        loadingType_,
        loadingSubType_
    };

    json[cellNum_] = cellData;
}

const QString &Coin::loadingType() const
{
    return loadingType_;
}

void Coin::setLoadingType(const QString &newLoadingType)
{
    emit loadingTypeChanged(newLoadingType, loadingType_);

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
    connect(this, &Coin::loadingTypeChanged, cellDialog, &CellDialog::loadingTypeChanged);
    connect(this, &Coin::loadingSubTypeChanged, cellDialog, &CellDialog::loadingSubTypeChanged);
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
