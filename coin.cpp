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
        paintTextEditorRightItem(painter, option);
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
        scene()->clearSelection();
        setSelected(true);
    } else if (event->button() == Qt::LeftButton && itemMode == ItemMode::CellDialogItem) {
        color = color.darker(150);
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

        simmMainHighlight = true;
    } else if (event->button() == Qt::LeftButton && itemMode == ItemMode::CellDialogItem) {
        color = color.lighter(150);
        update();
        cellDialog->showIconDialog();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void Coin::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (itemMode == ItemMode::MainItem || itemMode == ItemMode::TextEditorRightItem) {
        scene()->clearSelection();
        setSelected(true);
        contextMenu->exec(event->screenPos());
    }
}

void Coin::paintTextEditorRightItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(QBrush(color));

    paintIcon(iconType, painter, option);

    paintText(painter);
}

void Coin::setShowSrez(bool newShowSrez)
{
    showSrez = newShowSrez;
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

    paintIcon(iconType, painter, option);

    paintNumber(painter);

    paintText(painter);
}

void Coin::paintDT(QPainter *painter)
{
    QFont font("Times New Roman");
    font.setPixelSize(12);

    QRectF rectangle = QRectF(-30, 72, 100, 50);
    painter->setFont(font);
    painter->rotate(-30);
    painter->drawText(rectangle, Qt::AlignCenter, dT);
    painter->rotate(30);
}

void Coin::paintHighlight(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    QColor fillColor = color;

    if (visible)
        fillColor = fillColor.lighter(170);

    if (option->state & QStyle::State_MouseOver) {
        fillColor = fillColor.lighter(125);

        if (itemMode == ItemMode::MainItem) {
            emit printCellNum(cellNum + " " + loadingType + " " + loadingSubType + " " + dT + " "
                              + suzPos);
            emit cellPosChanged(mapToScene(QPointF(1.2 * (radius + step), 1.2 * (radius + step))));

            painter->setPen(QPen(QBrush(Qt::blue), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        } else if (itemMode == ItemMode::IconDialogItem && !isSelected()) {
            painter->save();
            painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawRoundedRect(8, 5, 90, 96, 10, 10);
            painter->restore();
        }
    } else {
        if (itemMode == ItemMode::MainItem && simmMainHighlight) {
            painter->setPen(QPen(QBrush(Qt::blue), 4, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
        } else if (itemMode == ItemMode::MainItem && simmHighlight) {
            painter->setPen(
                QPen(QBrush(Qt::darkGreen), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        }
    }

    painter->setBrush(QBrush(fillColor));
}

void Coin::painMainItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    if (visible && printing) {
        painter->setBrush(Qt::white);
        paintIcon(Default, painter, option);
        paintNumber(painter);
        return;
    }

    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    paintHighlight(painter, option);

    paintIcon(iconType, painter, option);

    paintNumber(painter);

    paintText(painter);

    if (showSrez) {
        paintSuzPos(painter);

        paintDT(painter);
    }
}

void Coin::paintIconDialogItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    painter->setPen(QPen(QBrush(Qt::black), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    paintHighlight(painter, option);

    paintIcon(iconType, painter, option);

    paintText(painter);
}

void Coin::paintIcon(IconType iconType, QPainter *painter, const QStyleOptionGraphicsItem *option)
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

    painter->save();
    if (iconType == IconType::Default) {
        paintHex(painter);
    } else if (iconType == IconType::Dot) {
        paintHex(painter);
        painter->setBrush(Qt::black);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawEllipse(QPointF(1.2 * (radius + step), 1.2 * (radius + step)), 5, 5);
    } else if (iconType == IconType::SmallDot) {
        paintHex(painter);
        painter->setBrush(Qt::black);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawEllipse(QPointF(1.2 * (radius + step), 1.2 * (radius + step) + radius / 2),
                             5,
                             5);
    } else if (iconType == IconType::PenSize2blue) {
        painter->setPen(QPen(QBrush(Qt::blue), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        paintHex(painter);
    } else if (iconType == IconType::PenSize2red) {
        painter->setPen(QPen(QBrush(Qt::red), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        paintHex(painter);
    } else if (iconType == IconType::RedCross) {
        paintHex(painter);
        QPointF center = QPointF(1.2 * (radius + step), 1.2 * (radius + step));
        painter->drawEllipse(QPointF(1.2 * (radius + step), 1.2 * (radius + step)), 5, 5);

        painter->setPen(QPen(QBrush(Qt::red), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::red);
        QPolygonF polygon = {
            QPointF(1.2 * (radius + step) - 2 * radius / 5 + 2,
                    1.2 * (radius + step) - radius / 5 - 2),
            QPointF(1.2 * (radius + step) - radius / 5 - 2,
                    1.2 * (radius + step) - 2 * radius / 5 + 2),
            QPointF(1.2 * (radius + step) + 2 * radius / 5 - 2,
                    1.2 * (radius + step) + radius / 5 + 2),
            QPointF(1.2 * (radius + step) + radius / 5 + 2,
                    1.2 * (radius + step) + 2 * radius / 5 - 2),
        };
        painter->drawPolygon(polygon);

        polygon = {
            QPointF(1.2 * (radius + step) + radius / 5 + 2,
                    1.2 * (radius + step) - 2 * radius / 5 + 2),
            QPointF(1.2 * (radius + step) + 2 * radius / 5 - 2,
                    1.2 * (radius + step) - radius / 5 - 2),
            QPointF(1.2 * (radius + step) - radius / 5 - 2,
                    1.2 * (radius + step) + 2 * radius / 5 - 2),
            QPointF(1.2 * (radius + step) - 2 * radius / 5 + 2,
                    1.2 * (radius + step) + radius / 5 + 2),
        };
        painter->drawPolygon(polygon);
    } else if (iconType == IconType::Dense6Pattern) {
        painter->setBrush(Qt::Dense6Pattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::Dense7Pattern) {
        painter->setBrush(Qt::Dense7Pattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::HorPattern) {
        painter->setBrush(Qt::HorPattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::VerPattern) {
        painter->setBrush(Qt::VerPattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::CrossPattern) {
        painter->setBrush(Qt::CrossPattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::BDiagPattern) {
        painter->setBrush(Qt::BDiagPattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::FDiagPattern) {
        painter->setBrush(Qt::FDiagPattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::DiagCrossPattern) {
        painter->setBrush(Qt::DiagCrossPattern);
        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (iconType == IconType::z23) {
        QPixmap pix(":/icons/z23.png");

        QBrush brush;
        brush.setTexture(pix.scaled(66, 72));
        painter->setBrush(brush);
        painter->setBrushOrigin(20, 17);

        paintHex(painter);
        painter->setPen(QPen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else {
        paintHex(painter);
    }

    painter->restore();

    if (option->state & QStyle::State_MouseOver && itemMode == ItemMode::MainItem) {
        painter->setBrush(Qt::NoBrush);
        paintHex(painter);
    } else if (simmHighlight || isSelected()) {
        painter->setBrush(Qt::NoBrush);
        paintHex(painter);
    }
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

void Coin::setDT(const QString &newDT)
{
    dT = newDT;
}

void Coin::setSuzPos(const QString &newSuzPos)
{
    suzPos = newSuzPos;
}

void Coin::setSimmMainHighlight(bool newSimmMainHighlight)
{
    simmMainHighlight = newSimmMainHighlight;
}

void Coin::setSimmHighlight(bool newSimmHighlight)
{
    simmHighlight = newSimmHighlight;
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

void Coin::paintSuzPos(QPainter *painter)
{
    if (suzPos.isEmpty())
        return;

    QFont font("Times New Roman");
    font.setPixelSize(12);

    QRectF rectangle = QRectF(22, 20, 100, 50);
    painter->setFont(font);
    painter->rotate(30);
    painter->drawText(rectangle, Qt::AlignCenter, suzPos);
    painter->rotate(-30);
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
        setFlag(ItemIsSelectable, true);
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
        visible = false;
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
}

QRectF GraphicsTextItem::boundingRect() const
{
    if (mode == TextEditorRight)
        return QRectF(0, 0, 930, 2647);
    else if (mode == TextEditorLeft)
        return QRectF(0, 0, 930, 2647);
    else if (mode == Header)
        return QRectF(0, 0, 3350, 100);
    else if (mode == dTLabel || mode == loadingLabel)
        return QRectF(0, 0, 700, 100);

    return QRectF();
}

void GraphicsTextItem::setMode(Mode newMode)
{
    mode = newMode;

    if (mode == TextEditorLeft) {
        setPos(-490, -2590);
        setZValue(3);
        setTextWidth(918);
        hide();
    } else if (mode == TextEditorRight) {
        setPos(-490 + 930 + 1933, -2589);
        setZValue(3);
        setTextWidth(918);
        hide();
    } else if (mode == Header) {
        setPos(-460, -2520);
        setTextWidth(3350);
        setZValue(1);
    } else if (mode == dTLabel) {
        setPos(1480, -2230);
        setTextWidth(600);
        setZValue(1);
    } else if (mode == loadingLabel) {
        setPos(1540, -2164);
        setTextWidth(600);
        setZValue(1);
    }
}

void GraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
    QGraphicsTextItem::mousePressEvent(e);
}
