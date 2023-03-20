#pragma once

#include <QMainWindow>
#if defined(QT_PRINTSUPPORT_LIB)
#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)
#    include <QPrinter>
#  endif
#endif

#include <QGraphicsView>
#include <QUrl>
#include "celldialog.h"
#include "texteditor.h"

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
class GraphicsView;
class GraphicsScene;
QT_END_NAMESPACE

constexpr int RADIUS = 40;
constexpr int STEP = 4;

class CoinEdit : public QMainWindow
{
    Q_OBJECT
public:
    explicit CoinEdit(QWidget *parent = nullptr);
//    bool loadFile(const QString &);
    enum SaveFormat {
        Json, Binary
    };

//    bool save(CoinEdit::SaveFormat saveFormat);
//    bool load(CoinEdit::SaveFormat saveFormat);

public slots:
    void zoomIn(int level = 8);
    void zoomOut(int level = 8);

private slots:
//    void newFile();
    void save();
    void saveAs();
    void open();
    void print(QPrinter *printer);
    void printPreview();
    void about();
    void TODO();
    void updateStats(QHash<QString, size_t> &typeCounter, QHash<QString, size_t> &subTypeCounter);
    void addPdf();
//    void createCellDialog();
//    void createCellDialog(const QString& cellNum);

private:
    void createActions();
    void createVertGroupBox();
    void drawPitak();
    void setSectorLines();
//    void scaleImage(double factor);
    void setupMatrix();
    void setupCells();

    void read(const QJsonObject &json);
    void write(QJsonObject &json);


    GraphicsView *view;
    QSlider *zoomSlider;
    CellDialog *cellDialog;
    QGroupBox *vertGroupBox;
    GraphicsScene *scene;
    QHash<QString, QVector<qreal>> cells;
    QVector<Coin*> cellsVec;
//    QHash<QString, Coin*> allCells;
    qreal scaleFactor = 1;
    QVector<int> xSize;
    QVector<int> xStart;
    QString fileName;
    SaveFormat saveFormat;
    QRect workingArea;
    QGraphicsRectItem *pdfRect1;
    QGraphicsRectItem *pdfRect2;
    QUrl pdfFileName;
    TextEditor *textEditor;
    QGraphicsTextItem *textEditorText;
//    QTextEdit *textEdit;
//    QGraphicsProxyWidget *proxyTextEdit;

//    QAction *saveAsAct;
//    QAction *printAct;
//    QAction *zoomInAct;
//    QAction *zoomOutAct;
//    QAction *normalSizeAct;
//    QAction *fitToWindowAct;
};

