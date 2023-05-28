#pragma once

#include <QtWidgets/QtWidgets>
#include "celldialog.h"

QT_BEGIN_NAMESPACE
class GraphicsScene;
QT_END_NAMESPACE

class Coin : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    Coin();

    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    enum ItemMode {
        MainItem,
        TextEditorRightItem,
        BackGroundItem,
        SectorsItem,
        CellDialogItem,
        IconDialogItem
    };

    enum IconType { Default, Dot, Exp };

signals:
    void mousePressEv(QMouseEvent *);
    void printCellNum(const QString &cellNum);
    void updateDialog(Coin *thisCell);
    void mainState();

public:
    QString getText() const;
    bool getVisible() const;
    const QColor &getColor() const;
    const QString &getCellNum() const;
    const QString &getLoadingSubType() const;
    const QString &getLoadingType() const;
    void read(const QJsonObject &json);
    void setCellDialog(CellDialog *newCellDialog);
    void setCellNum(const QString &newCellNum);
    void setColor(const QColor &newColor);
    void setContextMenu(QMenu *newContextMenu);
    void setItemMode(ItemMode newMode);
    void setLoadingSubType(const QString &newLoadingSubType);
    void setLoadingType(const QString &newLoadingType);
    void setPrinting(bool newPrinting);
    void setRadius(int newRadius);
    void setSectors(const QVector<qreal> &newSectors);
    void setStep(int newStep);
    void setText(const QString &newText);
    void setVisible(bool newVisible);
    void write(QJsonObject &json);

    void setIconType(IconType newIconType);

    ItemMode getItemMode() const;

    IconType getIconType() const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    void painMainItem(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintBackGroundItem(QPainter *painter);
    void paintCellDialogItem(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintHighlight(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintIcon(IconType iconType, QPainter *painter);
    void paintIconDialogItem(QPainter *painter, const QStyleOptionGraphicsItem *option);
    void paintNumber(QPainter *painter);
    void paintSectorsItem(QPainter *painter);
    void paintText(QPainter *painter);
    void paintTextEditorRightItem(QPainter *painter);
    void paintHex(QPainter *painter);

private:
    CellDialog *cellDialog;
    IconType iconType;
    ItemMode itemMode;
    QColor color;
    QMenu *contextMenu;
    QString cellNum;
    QString loadingSubType;
    QString loadingType;
    QString text;
    QVector<qreal> sectors;
    bool printing;
    bool visible;
    int radius;
    int step;
};

class GraphicsTextItem : public QGraphicsTextItem
{
public:
    //    enum { Type = UserType + 3 };
    enum Mode { TextEditorRight, TextEditorLeft, Header };

    GraphicsTextItem(QGraphicsItem *parent = nullptr);
    //    int type() const override { return Type; }
    virtual QRectF boundingRect() const override;
    void setMode(Mode newMode);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Mode mode;
};

