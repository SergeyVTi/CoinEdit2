#pragma once

#include <QColor>
#include <QtWidgets/QtWidgets>
#include "celldialog.h"

QT_BEGIN_NAMESPACE
class GraphicsScene;
QT_END_NAMESPACE


class Coin : public QGraphicsObject
{
    Q_OBJECT
public:
    Coin();

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

signals:
    void updateDialog(Coin *thisCell);
    void mousePressEv(QMouseEvent *);
    void printCellNum(const QString &cellNum);
//    void loadingTypeChanged(const QString &newLoadingType, const QString &oldLoadingType = "");
//    void loadingSubTypeChanged(const QString &newLoadingSubType, const QString &oldLoadingSubType = "");

public:
    const QColor &color() const;
    void setColor(const QColor &newColor);
    void setRadius(int newRadius);
    void setStep(int newStep);
    const QString &cellNum() const;
    void setCellNum(const QString &newCellNum);
    void setCellDialog(CellDialog *newCellDialog);
    void setSectors(const QVector<qreal> &newSectors);

    const QString &loadingType() const;
    void setLoadingType(const QString &newLoadingType);

    const QString &loadingSubType() const;
    void setLoadingSubType(const QString &newLoadingSubType);

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    void setScene(GraphicsScene *newScene);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int radius;
    int step;
    QColor color_;
    QVector<qreal> sectors;
    CellDialog *cellDialog;
    QVector<QPointF> stuff;
    GraphicsScene *scene_;

    QString cellNum_;
    QString loadingType_;
    QString loadingSubType_;

};

