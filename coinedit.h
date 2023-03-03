#pragma once

#include <QMainWindow>
#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)
#    include <QPrinter>
#  endif
#endif

#include <QGraphicsView>
#include "celldialog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QGraphicsView;
class QGroupBox;
class QImage;
class QLabel;
class QMenu;
class QPushButton;
class QScrollArea;
class QScrollBar;
class QScrollArea;
class QSlider;
class Coin;
QT_END_NAMESPACE

constexpr int RADIUS = 40;
constexpr int STEP = 4;

class CoinEdit;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(CoinEdit *ce) : QGraphicsView(), coinedit(ce) { }

protected:
    void wheelEvent(QWheelEvent *) override;
//    void resizeEvent(QResizeEvent *event) override;

private:
    CoinEdit *coinedit;
};

class CoinEdit : public QMainWindow
{
    Q_OBJECT
public:
    explicit CoinEdit(QWidget *parent = nullptr);
//    bool loadFile(const QString &);
    enum SaveFormat {
        Json, Binary
    };

    bool save();
//    bool save(CoinEdit::SaveFormat saveFormat);
    bool load();
//    bool load(CoinEdit::SaveFormat saveFormat);

public slots:
    void zoomIn(int level = 1);
    void zoomOut(int level = 1);

private slots:
    void newFile();
    void open();
//    bool save();
    bool saveAs();
    void print();
    void about();
    void TODO();
//    void createCellDialog();
//    void createCellDialog(const QString& cellNum);

private:
    void createActions();
    void createVertGroupBox();
    void drawPitak();
    void setSectorLines();
    void scaleImage(double factor);
    void setupMatrix();
    void setupCells();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QGroupBox *vertGroupBox;
    GraphicsView *view;
    QSlider *zoomSlider;
    QHash<QString, QVector<qreal>> cells;
    QVector<Coin*> cellsVec;
    qreal scaleFactor = 1;
    QVector<int> xSize;
    QVector<int> xStart;
    CellDialog *cellDialog;

//    QAction *saveAsAct;
//    QAction *printAct;
//    QAction *zoomInAct;
//    QAction *zoomOutAct;
//    QAction *normalSizeAct;
//    QAction *fitToWindowAct;
};

