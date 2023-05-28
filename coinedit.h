#pragma once

#include <QtWidgets/QtWidgets>
#include "celldialog.h"
#include "texteditor.h"

QT_BEGIN_NAMESPACE
class GraphicsScene;
class GraphicsTextItem;
class QAbstractTransition;
class QParallelAnimationGroup;
class QPrinter;
class QSignalTransition;
class QState;
class QStateMachine;
QT_END_NAMESPACE

class CoinEdit : public QMainWindow
{
    Q_OBJECT
public:
    explicit CoinEdit(QWidget *parent = nullptr);

    enum SaveFormat { Json, Binary };

public slots:
    void zoomIn(int level = 8);
    void zoomOut(int level = 8);

private slots:
    void about();
    void addTableCell(Coin *tableCell, QPoint topLeft);
    void open();
    void print(QPrinter *printer);
    void printPdf();
    void printPreview();
    void save();
    void saveAs();
    void deleteTableItem();

signals:
    void onStartUp();

private:
    void createActions();
    void drawPitak();
    void read(const QJsonObject &json);
    void setHeader(const QString &fileName);
    void setSectorLines();
    void setupCells();
    void setupMatrix();
    void setupStateMachine();
    void write(QJsonObject &json);

    CellDialog *cellDialog;
    GraphicsScene *scene;
    GraphicsTextItem *header;
    GraphicsTextItem *textEditorLeftPrintItem;
    GraphicsTextItem *textEditorRightPrintItem;
    GraphicsView *view;
    QAction *mainStateAct;
    QAction *openAct;
    QAction *saveAct;
    QGraphicsRectItem *pdfRect1;
    QGraphicsRectItem *pdfRect2;
    QGroupBox *vertGroupBox;
    QHash<QString, QVector<qreal>> cells;
    QMenu *contextMenu;
    QParallelAnimationGroup *group;
    QPointF center;
    QRect workingArea;
    QSignalTransition *trans;
    QSlider *zoomSlider;
    QState *centeredState;
    QState *mainState;
    QState *randomState;
    QState *rootState;
    QStateMachine *states;
    QUrl fileName;
    QTimer *showTimer;
    QTimer *timer;
    //    QUrl pdfFileName;
    QVector<Coin *> cellsVec;
    QVector<Coin *> tableCells;
    QVector<QGraphicsTextItem *> cellNumVec;
    QVector<int> xSize;
    QVector<int> xStart;
    SaveFormat saveFormat;
    TextEditors *textEditors;
    qreal scaleFactor = 1;
};
