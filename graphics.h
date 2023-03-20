#pragma once

#include <QGraphicsView>
QT_BEGIN_NAMESPACE
class CoinEdit;
QT_END_NAMESPACE

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(CoinEdit *ce);

protected:
    void wheelEvent(QWheelEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
//    void keyPressEvent(QKeyEvent *) override;
//    void keyReleaseEvent(QKeyEvent *) override;
//    void resizeEvent(QResizeEvent *event) override;

private:
    CoinEdit *coinedit;
};

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    GraphicsScene(GraphicsView *view) : QGraphicsScene(), view(view) { }
//    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    GraphicsView *view;

protected:
//    void wheelEvent(QWheelEvent *) override;
//    void mousePressEvent(QMouseEvent *) override;
//    void mouseReleaseEvent(QMouseEvent *) override;
//    void mouseMoveEvent(QMouseEvent *) override;
//    void resizeEvent(QResizeEvent *event) override;

private:
//};
//*event) override;

private:
};
