#include "coin.h"

Coin::Coin()
    : QGraphicsObject()
{
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
    visible = true;
    printing = false;
    cellDialog = nullptr;
}

QRectF Coin::boundingRect() const
{
    if (itemMode == ItemMode::IconDialogItem)
        return QRectF(0, 0, (radius + step) * 2.3, (radius + step) * 2.3);

    return QRectF(0, 0, (radius + step) * 2.1, (radius + step) * 2.2);
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

    if (itemMode == ItemMode::TextEditorRightItem) {
        paintTextEditorRightItem(painter);
    } else if (itemMode == ItemMode::BackGroundItem) {
        paintBackGroundItem(painter);
    } else if (itemMode == ItemMode::SectorsItem) {
        paintSectorsItem(painter);
    } else if (itemMode == ItemMode::CellDialogItem) {
        paintCellDialogItem(painter, option);
    } else if (itemMode == ItemMode::MainItem) {
        painMainItem(painter, option);
    } else if (itemMode == ItemMode::IconDialogItem) {
        paintIconDialogItem(painter, option);
    }
}

void Coin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && itemMode == ItemMode::MainItem) {
        color = color.darker(150);
    } else if (event->button() == Qt::LeftButton && itemMode == ItemMode::IconDialogItem) {
        //        color = color.darker(150);
        scene()->clearSelection();
        setSelected(true);
    }
    qDebug() << "mousePressEvent";

    QGraphicsItem::mousePressEvent(event);
}

void Coin::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && itemMode == ItemMode::MainItem) {
        color = color.lighter(150);
        update();
        emit updateDialog(this);
        emit mainState();
        cellDialog->show();
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void Coin::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (itemMode != ItemMode::TextEditorRightItem)
        return;

    scene()->clearSelection();
    setSelected(true);
    contextMenu->exec(event->screenPos());
}

void Coin::paintTextEditorRightItem(QPainter *painter)
{
    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(QBrush(color));

    paintIcon(iconType, painter);

    paintText(painter);
}

void Coin::paintBackGroundItem(QPainter *painter)
{
    painter->setBrush(QBrush(color));
    painter->setPen(Qt::NoPen);

    const int n = 6;
    QPointF a[n];

    for (int i = 0; i < n; ++i) {
        qreal fAngle = qDegreesToRadians(30.0 + 360.0 * i / n);
        qreal x = (radius + step) + qCos(fAngle) * radius;
        qreal y = (radius + step) + qSin(fAngle) * radius;
        a[i] = QPointF(x, y);
    }
    painter->drawPolygon(a, n);

    return;
}

void Coin::paintSectorsItem(QPainter *painter)
{
    painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    qreal keff = 0;
    const int n = 6;
    for (int i = 0; i < sectors.size() - 1; i += 2) {
        qreal fAngle1 = qDegreesToRadians(30.0 + 360.0 * sectors[i] / n);
        qreal x1 = 1.2 * (radius + step + keff) + qCos(fAngle1) * (radius + keff + step);
        qreal y1 = 1.2 * (radius + step + keff) + qSin(fAngle1) * (radius + keff + step);
        QPointF p1 = QPointF(x1, y1);

        qreal fAngle2 = qDegreesToRadians(30.0 + 360.0 * sectors[i + 1] / n);
        qreal x2 = 1.2 * (radius + step + keff) + qCos(fAngle2) * (radius + keff + step);
        qreal y2 = 1.2 * (radius + step + keff) + qSin(fAngle2) * (radius + keff + step);
        QPointF p2 = QPointF(x2, y2);

        painter->drawLine(p1, p2);
    }
    return;
}

void Coin::paintCellDialogItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    QColor fillColor = color;
    if (visible)
        fillColor = fillColor.lighter(170);

    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    paintHighlight(painter, option);

    paintIcon(iconType, painter);

    paintNumber(painter);

    paintText(painter);
}

void Coin::paintHighlight(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    QColor fillColor = color;

    if (visible)
        fillColor = fillColor.lighter(170);

    if (option->state & QStyle::State_MouseOver) {
        fillColor = fillColor.lighter(125);

        if (itemMode == ItemMode::MainItem)
            emit printCellNum(cellNum + " " + loadingType + " " + loadingSubType);

        if (itemMode == ItemMode::IconDialogItem && !isSelected()) {
            painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawRoundedRect(8, 5, 90, 96, 10, 10);
        }

        painter->setPen(QPen(QBrush(Qt::blue), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    }

    painter->setBrush(QBrush(fillColor));
}

void Coin::painMainItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    if (visible && printing) {
        painter->setBrush(Qt::white);
        paintIcon(Default, painter);
        paintNumber(painter);
        return;
    }

    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    paintHighlight(painter, option);

    paintIcon(iconType, painter);

    paintNumber(painter);

    paintText(painter);
}

void Coin::paintIconDialogItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    paintHighlight(painter, option);

    paintIcon(iconType, painter);

    paintText(painter);
}

void Coin::paintIcon(IconType iconType, QPainter *painter)
{
    painter->save();
    if (isSelected() && itemMode == ItemMode::IconDialogItem) {
        painter->setBrush(Qt::white);
        QPen pen = QPen(QBrush(Qt::blue), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setStyle(Qt::DotLine);
        painter->setPen(pen);
        painter->drawRoundedRect(8, 5, 90, 96, 10, 10);
    }
    painter->restore();

    paintHex(painter);

    painter->save();
    if (iconType == IconType::Default) {
    } else if (iconType == IconType::Exp) {
        painter->setBrush(Qt::white);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawEllipse(QPointF(1.2 * (radius + step), 1.2 * (radius + step)), 30, 30);
    } else if (iconType == IconType::Dot) {
        painter->setBrush(Qt::black);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawEllipse(QPointF(1.2 * (radius + step), 1.2 * (radius + step)), 10, 10);
    }
    painter->restore();
}

void Coin::paintHex(QPainter *painter)
{
    const int n = 6;
    QPointF a[n];
    for (int i = 0; i < n; ++i) {
        qreal fAngle = qDegreesToRadians(30.0 + 360.0 * i / n);
        qreal x = 1.2 * (radius + step) + qCos(fAngle) * radius;
        qreal y = 1.2 * (radius + step) + qSin(fAngle) * radius;
        a[i] = QPointF(x, y);
    }
    painter->drawPolygon(a, n);
}

Coin::IconType Coin::getIconType() const
{
    return iconType;
}

Coin::ItemMode Coin::getItemMode() const
{
    return itemMode;
}

void Coin::paintText(QPainter *painter)
{
    QFont font("Times New Roman");
    font.setPixelSize(18);

    QRectF textRect = QRectF(23, 23, 60, 60);
    painter->setFont(font);
    painter->drawText(textRect, Qt::AlignCenter, text);
}

void Coin::paintNumber(QPainter *painter)
{
    QFont font("Times New Roman");
    font.setPixelSize(12);

    QRectF rectangle = QRectF(-31, 21, 100, 50);
    painter->setFont(font);
    painter->rotate(-30);
    painter->drawText(rectangle, Qt::AlignCenter, cellNum);
    painter->rotate(30);
}

void Coin::setIconType(IconType newIconType)
{
    iconType = newIconType;
}

QString Coin::getText() const
{
    return text;
}

void Coin::setText(const QString &newText)
{
    text = newText;
}

void Coin::setItemMode(ItemMode newMode)
{
    itemMode = newMode;

    if (itemMode == ItemMode::TextEditorRightItem) {
        radius = 40;
        step = 4;
        setPos(0, 0);
        setFlag(ItemIsMovable);
    } else if (itemMode == ItemMode::CellDialogItem) {
        radius = 40;
        step = 4;
        setPos(0, 0);
        setFlag(ItemIsMovable, false);
        setFlag(ItemIsSelectable, false);
    } else if (itemMode == ItemMode::IconDialogItem) {
        radius = 40;
        step = 4;
        setPos(0, 0);
        setFlag(ItemIsMovable, false);
        setFlag(ItemIsSelectable, true);
        visible = false;
    } else if (itemMode == ItemMode::MainItem) {
        radius = 40;
        step = 4;
        setPos(0, 0);
        setFlag(ItemIsMovable, false);
        setFlag(ItemIsSelectable, true);
        iconType = IconType::Default;
    }
}

void Coin::setContextMenu(QMenu *newContextMenu)
{
    contextMenu = newContextMenu;
}

void Coin::setPrinting(bool newPrinting)
{
    printing = newPrinting;
}

bool Coin::getVisible() const
{
    return visible;
}

void Coin::setVisible(bool newVisible)
{
    visible = newVisible;
}

const QString &Coin::getLoadingSubType() const
{
    return loadingSubType;
}

void Coin::setLoadingSubType(const QString &newLoadingSubType)
{
    loadingSubType = newLoadingSubType;
}

void Coin::read(const QJsonObject &json)
{
    if (json.contains(cellNum) && json[cellNum].isArray()) {
        QJsonArray cellData = json[cellNum].toArray();
        loadingType = cellData.at(0).toString();
        loadingSubType = cellData.at(1).toString();
        color = cellData.at(2).toString();
        visible = cellData.at(3).toBool();
        text = cellData.at(4).toString();
        iconType = (Coin::IconType) cellData.at(5).toInt();
        return;
    }

    for (const auto &cell : json){
        QJsonArray cellData = cell.toArray();
        loadingType = cellData.at(0).toString();
        loadingSubType = cellData.at(1).toString();
        color = cellData.at(2).toString();
        setPos(cellData.at(3).toDouble(), cellData.at(4).toDouble());
        text = cellData.at(5).toString();
        iconType = (Coin::IconType) cellData.at(6).toInt();
    }
}

void Coin::write(QJsonObject &json)
{
    if (itemMode != ItemMode::TextEditorRightItem) {
        QJsonArray cellData = {loadingType, loadingSubType, color.name(), visible, text, iconType};
        json[cellNum] = cellData;
        return;
    }

    QJsonArray cellData
        = {loadingType, loadingSubType, color.name(), pos().x(), pos().y(), text, iconType};
    json[loadingType + " " + loadingSubType] = cellData;
}

const QString &Coin::getLoadingType() const
{
    return loadingType;
}

void Coin::setLoadingType(const QString &newLoadingType)
{
    loadingType = newLoadingType;
}

void Coin::setSectors(const QVector<qreal> &newSectors)
{
    sectors = newSectors;
}

void Coin::setCellDialog(CellDialog *newCellDialog)
{
    cellDialog = newCellDialog;
    connect(this, &Coin::updateDialog, cellDialog, &CellDialog::updateDialog);
}

const QString &Coin::getCellNum() const
{
    return cellNum;
}

void Coin::setCellNum(const QString &newCellNum)
{
    cellNum = newCellNum;
}

void Coin::setStep(int newStep)
{
    step = newStep;
}

void Coin::setRadius(int newRadius)
{
    radius = newRadius;
}

const QColor &Coin::getColor() const
{
    return color;
}

void Coin::setColor(const QColor &newColor)
{
    color = newColor;
}

GraphicsTextItem::GraphicsTextItem(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
{
    //    setFlag(QGraphicsItem::ItemIsMovable);
    //    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF GraphicsTextItem::boundingRect() const
{
    if (mode == TextEditorRight)
        return QRectF(0, 0, 930, 2647);
    else if (mode == TextEditorLeft)
        return QRectF(0, 0, 930, 2647);
    else if (mode == Header)
        return QRectF(0, 0, 3350, 100);

    return QRectF();
}

void GraphicsTextItem::setMode(Mode newMode)
{
    mode = newMode;

    if (mode == TextEditorLeft) {
        setPos(-490, -2590);
        setZValue(3);
        setTextWidth(918);
        setSelected(false);
        hide();
    } else if (mode == TextEditorRight) {
        setPos(-490 + 930 + 1933, -2589);
        setZValue(3);
        setTextWidth(918);
        setSelected(false);
        hide();
    } else if (mode == Header) {
        setPos(-460, -2520);
        setTextWidth(3350);
        setZValue(3);
        setSelected(false);
    }
}

void GraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *) {}
