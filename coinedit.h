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
    void addCelltoDialog();
    void addTArch();
    void addLoadingArch();
    void addTableCell(Coin *tableCell, QPoint topLeft);
    void deleteTableItem();
    void highlightSimmCells(QPointF cellPos);
    void open();
    void printPdf();
    void printPreview();
    void render(QPrinter *printer);
    void save();
    void saveAs();

signals:
    void onStartUp();
    void addCellNumToDialog(const QString cellNum);
    void updateComboLoadingTypes(QHash<QString, QStringList> types);

private:
    void createActions();
    void drawPitak();
    void read(const QJsonObject &json);
    void setHeader(const QString &fileName);
    void setSectorLines();
    void setupCells();
    void setupMatrix();
    void setupStateMachine();
    void setupSimmLines();
    void write(QJsonObject &json);

    //    QUrl pdfFileName;
    CellDialog *cellDialog;
    Coin *centerCoinPtr;
    GraphicsScene *scene;
    GraphicsTextItem *dTItem;
    GraphicsTextItem *header;
    GraphicsTextItem *loadingItem;
    GraphicsTextItem *textEditorLeftPrintItem;
    GraphicsTextItem *textEditorRightPrintItem;
    GraphicsView *view;
    QAction *mainStateAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *showSrezAct;
    QGraphicsRectItem *pdfRect1;
    QGraphicsRectItem *pdfRect2;
    QGroupBox *vertGroupBox;
    QHash<QString, Coin *> allCells;
    QHash<QString, Coin *> simmMap;
    QHash<QString, QVector<qreal>> cells;
    QMenu *contexMenuEditorItem;
    QMenu *contexMenuMainItem;
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
    QTimer *showTimer;
    QTimer *timer;
    QUrl fileName;
    QUrl archTFileName;
    QUrl archLoadingFileName;
    QVector<Coin *> cellsVec;
    QVector<Coin *> tableCells;
    QVector<QGraphicsLineItem *> simmLinesVec;
    QVector<QGraphicsTextItem *> cellNumVec;
    QVector<int> xSize;
    QVector<int> xStart;
    SaveFormat saveFormat;
    TextEditors *textEditors;
    qreal scaleFactor = 1;

    const int RADIUS = 40;
    const int STEP = 4;
    const int indent = 70;
    const int vertIndent = 30;
    const qreal k = 1.2;
};
