#include "coinedit.h"
#include "coin.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QGuiApplication>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStatusBar>
#include <QToolBar>
#include <QtMath>
#include <QPrintDialog>
#include <QSlider>
#include <QWheelEvent>
#include <algorithm>
#include <QGraphicsObject>

void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->angleDelta().y() > 0)
        coinedit->zoomIn(8);
    else
        coinedit->zoomOut(8);
    e->accept();
}

//void GraphicsView::resizeEvent(QResizeEvent *event)
//{
//    QGraphicsView::resizeEvent(event);
//    fitInView(sceneRect(), Qt::KeepAspectRatio);
//}

CoinEdit::CoinEdit(QWidget *parent) : QMainWindow(parent),
    view(new GraphicsView(this)), zoomSlider(new QSlider), cellDialog(new CellDialog(this))
{
    //Set main layout
    QGroupBox *mainGroupBox = new QGroupBox;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setCentralWidget(mainGroupBox);
    mainGroupBox->setLayout(mainLayout);

//        resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    //Add central widget
    setSectorLines();

    setupCells();

    drawPitak();

    //    QImage image("://pics/niagara_falls.jpg");
    //    QLabel *imageLabel = new QLabel;
    //    imageLabel->setPixmap(QPixmap::fromImage(image));
    //    imageLabel->setBackgroundRole(QPalette::Base);
    //    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    //    imageLabel->setScaledContents(true);

    //Add scroll area
    //    scrollArea = new QScrollArea;
    //    scrollArea->setBackgroundRole(QPalette::Dark);
    //    scrollArea->setWidget(view);
    //    mainLayout->addWidget(scrollArea);
    //    view->setRenderHint(QPainter::Antialiasing, false);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    //    view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    mainLayout->addWidget(view);

    //Create menus
    createActions();

    //Add status bar
    statusBar()->showMessage("Привет!",4000);

    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &CoinEdit::setupMatrix);
}

bool CoinEdit::save()
{
    SaveFormat saveFormat = SaveFormat::Json;

    QFile saveFile(saveFormat == Json
          ? QStringLiteral("save.json")
          : QStringLiteral("save.dat"));

      if (!saveFile.open(QIODevice::WriteOnly)) {
          qWarning("Couldn't open save file.");
          return false;
      }

      QJsonObject dataObject;
      write(dataObject);
      saveFile.write(saveFormat == Json
          ? QJsonDocument(dataObject).toJson()
          : QCborValue::fromJsonValue(dataObject).toCbor());

      return true;
}

bool CoinEdit::load()
{
    SaveFormat saveFormat = SaveFormat::Json;

    QFile loadFile(saveFormat == Json
           ? QStringLiteral("save.json")
           : QStringLiteral("save.dat"));

       if (!loadFile.open(QIODevice::ReadOnly)) {
           qWarning("Couldn't open save file.");
           return false;
       }

       QByteArray saveData = loadFile.readAll();

       QJsonDocument loadDoc(saveFormat == Json
           ? QJsonDocument::fromJson(saveData)
           : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

       read(loadDoc.object());

       QTextStream(stdout) << "Loaded save for "
                           << loadDoc["Ячейки"].toString()
                           << " using "
                           << (saveFormat != Json ? "CBOR" : "JSON") << "...\n";
       return true;
}

void CoinEdit::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);
//    matrix.rotate(rotateSlider->value());

    view->setTransform(matrix);
    //    setResetButtonEnabled();
}

//void CoinEdit::wheelEvent(QWheelEvent *e)
//{
//    if (e->modifiers() & Qt::ControlModifier) {
//        if (e->angleDelta().y() > 0)
//            view->zoomIn(6);
//        else
//            view->zoomOut(6);
//        e->accept();
//    } else {
//        QGraphicsView::wheelEvent(e);
//    }
//}

void CoinEdit::newFile()
{
    TODO();
}

void CoinEdit::open()
{
    TODO();
}

//bool CoinEdit::save()
//{
//    TODO();
//}

bool CoinEdit::saveAs()
{
    TODO();
}

void CoinEdit::drawPitak()
{
    int indent = 100;
    qreal k = 1.2;
    //Add graphics
    QGraphicsScene *scene = new QGraphicsScene(this);
//    cellDialog = new CellDialog();

    int xStep = qSin(qDegreesToRadians(60.0)) * RADIUS + STEP;
    int yStep = RADIUS + qSin(qDegreesToRadians(30.0)) * RADIUS + STEP*1.5;

    int maxCoinInRow = static_cast<qreal>(*std::max_element(xSize.begin(), xSize.end()));

    for(int x = 1; x < maxCoinInRow*2; x += 1)
    {
        QGraphicsLineItem *lineY = new QGraphicsLineItem(x*xStep + (RADIUS + STEP)*k, indent - 3*k*(RADIUS + STEP),
                                 x*xStep + (RADIUS + STEP)*k,-indent - (5+xSize.size())*k*(RADIUS + STEP));

        QPen pen = (x % 2 == 0) ? QPen(Qt::black) : QPen(Qt::blue);
        pen.setWidth(1);

        lineY->setPen(pen);
        scene->addItem(lineY);

        QString xNum = (x + 6 < 10) ? "0" + QString::number(x + 6) : QString::number(x + 6);
        QGraphicsSimpleTextItem *lineNumberDown = new QGraphicsSimpleTextItem(xNum);
        QFont font = QFont("Times", 20, QFont::Normal, true);
        lineNumberDown->setFont(font);
        lineNumberDown->setPen(pen);
        lineNumberDown->setPos(x*xStep + (RADIUS + STEP)*k - 15, indent - 3*k*(RADIUS + STEP) - 3);

        scene->addItem(lineNumberDown);

        QGraphicsSimpleTextItem *lineNumberUp = new QGraphicsSimpleTextItem(xNum);
        lineNumberUp->setFont(font);
        lineNumberUp->setPen(pen);
        lineNumberUp->setPos(x*xStep + (RADIUS + STEP)*k - 15,-indent - (5+xSize.size())*k*(RADIUS + STEP) - 30);

        scene->addItem(lineNumberUp);
    }

    //Внутренний пятак
    for(int y = 1; y <= xSize.size(); y++)
    {
        if (xSize[y-1] == 0)
            continue;

        QGraphicsLineItem *lineX = new QGraphicsLineItem(-indent, -y*yStep + (RADIUS + STEP)*k,
                                 maxCoinInRow*xStep*2+xStep*3 - 3*STEP +indent, -y*yStep + (RADIUS + STEP)*k);

        QPen pen = (y % 2 == 0) ? QPen(Qt::black) : QPen(Qt::blue);
        pen.setWidth(1);
        lineX->setPen(pen);
        scene->addItem(lineX);

        QString yNum = (y < 10) ? "0" + QString::number(y) : QString::number(y);
        QGraphicsSimpleTextItem *lineNumberLeft = new QGraphicsSimpleTextItem(yNum);
        QFont font = QFont("Times", 20, QFont::Normal, true);
        lineNumberLeft->setFont(font);
        lineNumberLeft->setPen(pen);
        lineNumberLeft->setPos(-indent-30, -y*yStep + (RADIUS + STEP)*k - 17);

        scene->addItem(lineNumberLeft);

        QGraphicsSimpleTextItem *lineNumberRight = new QGraphicsSimpleTextItem(yNum);
        lineNumberRight->setFont(font);
        lineNumberRight->setPen(pen);
        lineNumberRight->setPos(static_cast<qreal>(*std::max_element(xSize.begin(), xSize.end()))*xStep*2+xStep*3 - 3*STEP +indent + 4, -y*yStep + (RADIUS + STEP)*k - 17);

        scene->addItem(lineNumberRight);

        for(int x = 1; x < xSize[y-1]*2; x += 2)
        {
            Coin *item = new Coin;

            QColor color(Qt::white);
            item->setColor(color);

            item->setRadius(k*RADIUS);

            item->setStep(STEP);

            item->setPos(QPointF(x*xStep + xStart[y-1]*xStep, -y*yStep));

            scene->addItem(item);
        }
    }

    for(int y = 1; y <= xSize.size(); y++)
    {
        if (xSize[y-1] == 0)
            continue;

        for(int x = 1; x < xSize[y-1]*2; x += 2)
        {
            Coin *item = new Coin;

            QColor color(Qt::white);
            item->setColor(color);

            item->setRadius(RADIUS);

            item->setStep(STEP);

            QString cellNum = (y < 10 ? ("0" + QString::number(y)) : QString::number(y))
                    + "-"
                    + ((x + 6  + xStart[y-1]) < 10 ? ("0" + QString::number(x + 6  + xStart[y-1])) : QString::number(x + 6  + xStart[y-1]));
            item->setCellNum(cellNum);

            item->setCellDialog(cellDialog);

            if (cells.contains(cellNum))
                item->setSectors(cells[cellNum]);

            item->setPos(QPointF(x*xStep + xStart[y-1]*xStep, -y*yStep));

            item->setLoadingType("Без загрузки");
            item->setLoadingSubType("Без загрузки");

        cellsVec.append(item);

            scene->addItem(item);
        }

    }

    //    scene->setSceneRect(0,0,1000,1000);

    //    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setScene(scene);
    //    mainLayout->addWidget(imageLabel);
}

void CoinEdit::setSectorLines()
{
    cells["34-25"]={2,3,3,4,4,5};cells["34-27"]={3,4,4,5};cells["34-29"]={3,4,4,5};cells["34-31"]={3,4,4,5};cells["34-33"]={3,4,4,5};cells["34-35"]={3,4,4,5};cells["34-37"]={3,4,4,5};cells["34-39"]={3,4,4,5};cells["34-41"]={3,4,4,5};cells["34-43"]={3,4,4,5,5,6};
    cells["33-22"]={2,3,3,4,4,5};cells["33-24"]={3,4};cells["33-44"]={4,5};cells["33-46"]={3,4,4,5,5,6};
    cells["32-19"]={2,3,3,4,4,5,5,6};cells["32-21"]={3,4};cells["32-27"]={2,3,3,4,4,5};cells["32-29"]={3,4,4,5};cells["32-31"]={3,4,4,5};cells["32-33"]={3,4,4,5};cells["32-35"]={3,4,4,5};cells["32-37"]={3,4,4,5};cells["32-39"]={3,4,4,5};cells["32-41"]={3,4,4,5,5,6};cells["32-47"]={2,3,3,4,4,5};cells["32-49"]={3,4,4,5,5,6};
    cells["31-16"]={2,3,3,4,4,5};cells["31-18"]={3,4};cells["31-20"]={4,5,5,6};cells["31-24"]={2,3,3,4,4,5};cells["31-26"]={3,4};cells["31-42"]={4,5};cells["31-44"]={3,4,4,5,5,6};cells["31-46"]={3,4};cells["31-50"]={4,5};cells["31-52"]={3,4,4,5,5,6};
    cells["30-15"]={2,3,3,4};cells["30-21"]={2,3,3,4,4,5,5,6};cells["30-23"]={3,4};cells["30-45"]={2,3,3,4,4,5};cells["30-47"]={3,4,4,5,5,6};cells["30-53"]={4,5,5,6};
    cells["29-14"]={2,3,3,4};cells["29-18"]={2,3,3,4,4,5};cells["29-20"]={3,4};cells["29-22"]={4,5,5,6};cells["29-44"]={2,3,3,4};cells["29-48"]={4,5};cells["29-50"]={3,4,4,5,5,6};cells["29-54"]={4,5,5,6};
    cells["28-13"]={2,3,3,4};cells["28-17"]={2,3,3,4};cells["28-23"]={4,5,5,6};cells["28-43"]={2,3,3,4};cells["28-51"]={4,5,5,6};cells["28-55"]={4,5,5,6};
    cells["27-12"]={2,3,3,4};cells["27-16"]={2,3,3,4};cells["27-24"]={4,5,5,6};cells["27-42"]={2,3,3,4};cells["27-52"]={4,5,5,6};cells["27-56"]={4,5,5,6};
    cells["26-11"]={2,3,3,4};cells["26-15"]={2,3,3,4};cells["26-25"]={4,5,5,6};cells["26-41"]={2,3,3,4};cells["26-53"]={4,5,5,6};cells["26-57"]={4,5,5,6};
    cells["25-10"]={2,3,3,4};cells["25-14"]={2,3,3,4};cells["25-26"]={4,5,5,6};cells["25-40"]={2,3,3,4};cells["25-54"]={4,5,5,6};cells["25-58"]={4,5,5,6};
    cells["24-09"]={2,3,3,4};cells["24-13"]={2,3,3,4};cells["24-27"]={4,5,5,6};cells["24-39"]={2,3,3,4};cells["24-55"]={4,5,5,6};cells["24-59"]={4,5,5,6};
    cells["23-08"]={2,3,3,4};cells["23-12"]={2,3,3,4};cells["23-28"]={4,5,5,6};cells["23-38"]={2,3,3,4};cells["23-56"]={4,5,5,6};cells["23-60"]={4,5,5,6};
    cells["22-07"]={1,2,2,3,3,4};cells["22-11"]={2,3,3,4};cells["22-29"]={4,5,5,6};cells["22-37"]={2,3,3,4};cells["22-57"]={4,5,5,6};cells["22-61"]={4,5,5,6,6,1};
    cells["21-08"]={2,3};cells["21-10"]={4,5,5,6};cells["21-30"]={4,5,5,6};cells["21-36"]={2,3,3,4};cells["21-58"]={2,3,3,4};cells["21-60"]={5,6};
    cells["20-07"]={1,2,2,3,3,4};cells["20-11"]={2,3,3,4};cells["20-31"]={4,5,5,6};cells["20-35"]={2,3,3,4};cells["20-57"]={4,5,5,6};cells["20-61"]={4,5,5,6,6,1};
    cells["19-08"]={2,3};cells["19-10"]={4,5,5,6};cells["19-32"]={4,5,5,6};cells["19-36"]={3,4,4,5};cells["19-38"]={3,4,4,5};cells["19-40"]={3,4,4,5};cells["19-42"]={3,4,4,5};cells["19-44"]={3,4,4,5};cells["19-46"]={3,4,4,5};cells["19-48"]={3,4,4,5};cells["19-50"]={3,4,4,5};cells["19-52"]={3,4,4,5};cells["19-54"]={3,4,4,5};cells["19-56"]={3,4,4,5};cells["19-58"]={2,3,3,4,4,5};cells["19-60"]={3,4,4,5,5,6};
    cells["19-34"] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6};
    cells["18-07"]={1,2,2,3,3,4,4,5};cells["18-09"]={3,4,4,5,5,6};cells["18-11"]={3,4,4,5};cells["18-13"]={3,4,4,5};cells["18-15"]={3,4,4,5};cells["18-17"]={3,4,4,5};cells["18-19"]={3,4,4,5};cells["18-21"]={3,4,4,5};cells["18-23"]={3,4,4,5};cells["18-25"]={3,4,4,5};cells["18-27"]={3,4,4,5};cells["18-29"]={3,4,4,5};cells["18-31"]={3,4,4,5};cells["18-33"]={3,4,4,5};cells["18-35"]={2,3,3,4,4,5,5,6};cells["18-57"]={4,5,5,6};cells["18-61"]={4,5,5,6,6,1};
    cells["17-08"]={2,3};cells["17-10"]={4,5,5,6};cells["17-34"]={2,3,3,4};cells["17-36"]={4,5,5,6};cells["17-58"]={2,3,3,4};cells["17-60"]={5,6};
    cells["16-07"]={1,2,2,3,3,4};cells["16-11"]={2,3,3,4};cells["16-33"]={2,3,3,4};cells["16-37"]={4,5,5,6};cells["16-57"]={4,5,5,6};cells["16-61"]={4,5,5,6,6,1};
    cells["15-08"]={1,2,2,3};cells["15-10"]={4,5,5,6};cells["15-32"]={2,3,3,4};cells["15-38"]={4,5,5,6};cells["15-58"]={2,3,3,4};cells["15-60"]={5,6,6,1};
    cells["14-09"]={1,2,2,3};cells["14-11"]={4,5,5,6};cells["14-31"]={2,3,3,4};cells["14-39"]={4,5,5,6};cells["14-57"]={2,3,3,4};cells["14-59"]={5,6,6,1};
    cells["13-10"]={1,2,2,3};cells["13-12"]={4,5,5,6};cells["13-30"]={2,3,3,4};cells["13-40"]={4,5,5,6};cells["13-56"]={2,3,3,4};cells["13-58"]={5,6,6,1};
    cells["12-11"]={1,2,2,3};cells["12-13"]={4,5,5,6};cells["12-29"]={2,3,3,4};cells["12-41"]={4,5,5,6};cells["12-55"]={2,3,3,4};cells["12-57"]={5,6,6,1};
    cells["11-12"]={1,2,2,3};cells["11-14"]={4,5,5,6};cells["11-28"]={2,3,3,4};cells["11-42"]={4,5,5,6};cells["11-54"]={2,3,3,4};cells["11-56"]={5,6,6,1};
    cells["10-13"]={1,2,2,3};cells["10-15"]={4,5,5,6};cells["10-27"]={2,3,3,4};cells["10-43"]={4,5,5,6};cells["10-53"]={2,3,3,4};cells["10-55"]={5,6,6,1};
    cells["09-14"]={1,2,2,3};cells["09-16"]={4,5,5,6};cells["09-26"]={2,3,3,4};cells["09-44"]={4,5,5,6};cells["09-52"]={2,3,3,4};cells["09-54"]={5,6,6,1};
    cells["08-15"]={1,2,2,3};cells["08-17"]={4,5};cells["08-19"]={3,4,4,5,5,6};cells["08-25"]={2,3,3,4};cells["08-45"]={4,5,5,6};cells["08-49"]={2,3,3,4,4,5};cells["08-51"]={3,4};cells["08-53"]={5,6,6,1};
    cells["07-16"]={6,1,1,2,2,3};cells["07-18"]={1,2};cells["07-20"]={4,5};cells["07-22"]={3,4,4,5,5,6};cells["07-24"]={3,4};cells["07-46"]={2,3,3,4,4,5,5,6};cells["07-48"]={3,4};cells["07-50"]={6,1};cells["07-52"]={1,2,5,6,6,1};
    cells["06-19"]={6,1,1,2,2,3};cells["06-21"]={1,2};cells["06-23"]={2,3,3,4,4,5};cells["06-25"]={3,4,4,5,5,6};cells["06-43"]={2,3,3,4,4,5};cells["06-45"]={3,4};cells["06-47"]={4,5,5,6,6,1};cells["06-49"]={5,6,6,1,1,2};
    cells["05-22"]={6,1,1,2,2,3,3,4};cells["05-24"]={1,2};cells["05-26"]={4,5};cells["05-28"]={3,4,4,5};cells["05-30"]={3,4,4,5};cells["05-32"]={3,4,4,5};cells["05-34"]={3,4,4,5};cells["05-36"]={3,4,4,5};cells["05-38"]={3,4,4,5};cells["05-40"]={3,4,4,5};cells["05-42"]={3,4};cells["05-44"]={6,1};cells["05-46"]={5,6,6,1,1,2};
    cells["04-25"]={6,1,1,2,2,3};cells["04-27"]={6,1,1,2};cells["04-29"]={6,1,1,2};cells["04-31"]={6,1,1,2};cells["04-33"]={6,1,1,2};cells["04-35"]={6,1,1,2};cells["04-37"]={6,1,1,2};cells["04-39"]={6,1,1,2};cells["04-41"]={6,1,1,2};cells["04-43"]={5,6,6,1,1,2};
}

void CoinEdit::setupCells()
{
    //Количество ячеек в ряду
    xSize =  {0, 0, 0, 10, 13, 16, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 27, 28, 27, 28, 27, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 16, 13, 10, 0, 0, 0};
    //Отступ ряда
    xStart = {0, 0, 0, 18, 15, 12,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,  1,  0,  1,  0,  1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 12, 15, 18, 0, 0, 0};
}

void CoinEdit::read(const QJsonObject &json)
{
    if (json.contains("Ячейки") && json["Ячейки"].isArray()){
        QJsonArray cellsArray = json["Ячейки"].toArray();

        for(int i = 0; i < cellsVec.size(); i++){
          QJsonObject cellObject = cellsArray[i].toObject();
          cellsVec[i]->read(cellObject);
        }
//        QHash<QString, Coin*>::iterator i = cellsMap.begin();
//        size_t ind = 0;
//        while (i != cellsMap.end()){
//            QJsonObject cellObject = cellsArray[ind].toObject();
//            i.value()->read(cellObject);
//            ind++;
//        }
    }
}

void CoinEdit::write(QJsonObject &json)
{
    QJsonArray cellsArray;

    for(auto& coin : cellsVec){
      QJsonObject cellObject;
      coin->write(cellObject);
      cellsArray.append(cellObject);
    }
//    QHash<QString, Coin*>::const_iterator i = cellsMap.constBegin();
//    while (i != cellsMap.constEnd()){
//        QJsonObject cellObject;
//        i.value()->write(cellObject);
//        cellsArray.append(cellObject);
//    }

    json["Ячейки"] = cellsArray;
}

//bool CoinEdit::loadFile(const QString &fileName)
//{
//    QImageReader reader(fileName);
//    reader.setAutoTransform(true);
//    const QImage newImage = reader.read();
//    if (newImage.isNull()) {
//        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
//                                 tr("Cannot load %1: %2")
//                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
//        return false;
//    }

//    setImage(newImage);

//    setWindowFilePath(fileName);

//    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
//        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
//    statusBar()->showMessage(message);
//    return true;
//}

//bool CoinEdit::saveFile(const QString &fileName)
//{
//    QImageWriter writer(fileName);

//    if (!writer.write(image)) {
//        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
//                                 tr("Cannot write %1: %2")
//                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
//        return false;
//    }
//    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
//    statusBar()->showMessage(message);
//    return true;
//}

//static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
//{
//    static bool firstDialog = true;

//    if (firstDialog) {
//        firstDialog = false;
//        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
//        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
//    }

//    QStringList mimeTypeFilters;
//    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
//        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
//    for (const QByteArray &mimeTypeName : supportedMimeTypes)
//        mimeTypeFilters.append(mimeTypeName);
//    mimeTypeFilters.sort();
//    dialog.setMimeTypeFilters(mimeTypeFilters);
//    dialog.selectMimeTypeFilter("image/jpeg");
//    if (acceptMode == QFileDialog::AcceptSave)
//        dialog.setDefaultSuffix("jpg");
//}

//void CoinEdit::open()
//{
//    QFileDialog dialog(this, tr("Open File"));
//    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

//    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
//}

//void CoinEdit::saveAs()
//{
//    QFileDialog dialog(this, tr("Save File As"));
//    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

//    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
//}

void CoinEdit::print()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        view->render(&painter);
    }
}

void CoinEdit::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void CoinEdit::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}

//void CoinEdit::fitToWindow()
//{
//    bool fitToWindow = fitToWindowAct->isChecked();
//    scrollArea->setWidgetResizable(fitToWindow);
//    if (!fitToWindow)
//        normalSize();
//    updateActions();
//}

void CoinEdit::about()
{
    QMessageBox::about(this, "CoinEdit 2",
               tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
                  "and QScrollArea to display an image. QLabel is typically used "
                  "for displaying a text, but it can also display an image. "
                  "QScrollArea provides a scrolling view around another widget. "
                  "If the child widget exceeds the size of the frame, QScrollArea "
                  "automatically provides scroll bars. </p><p>The example "
                  "demonstrates how QLabel's ability to scale its contents "
                  "(QLabel::scaledContents), and QScrollArea's ability to "
                  "automatically resize its contents "
                  "(QScrollArea::widgetResizable), can be used to implement "
                  "zooming and scaling features. </p><p>In addition the example "
                  "shows how to use QPainter to print an image.</p>"));
}

void CoinEdit::TODO()
{
    QMessageBox message;
    message.setWindowTitle("Сорян");
    message.setText("Функционал не готов");
    //    message.setInformativeText("?");
    message.setIcon(QMessageBox::Critical);
    message.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    message.setDefaultButton(QMessageBox::Ok);

    message.exec();
}

//void CoinEdit::createCellDialog()
////void CoinEdit::createCellDialog(const QString &cellNum)
//{
////   cellDialog = new CellDialog(this);
//}

void CoinEdit::createActions()
{
    //File
    QMenu *fileMenu = menuBar()->addMenu("&Файл");
    QToolBar *fileToolBar = addToolBar("File");
    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/icons/new.png"));
    QAction *newAct = new QAction(newIcon, "&Создать", this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip("Создать новую картограмму");
    connect(newAct, &QAction::triggered, this, &CoinEdit::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    //Open
    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/icons/open.png"));
    QAction *openAct = new QAction(openIcon, "&Открыть", this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip("Открыть картограмму");
    connect(openAct, &QAction::triggered, this, &CoinEdit::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    //Save
    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/icons/save.png"));
    QAction *saveAct = new QAction(saveIcon, "Со&хранить", this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip("Сохранить картограмму");
    connect(saveAct, &QAction::triggered, this, &CoinEdit::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    //Save as
    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, "Сохранить к&ак...", this, &CoinEdit::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip("Сохранить картограмму под новым именем");

    fileMenu->addSeparator();

    //Print
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/icons/print.png"));
    QAction *printAct = fileMenu->addAction(printIcon, "&Печать...", this, &CoinEdit::print);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip("Распечатать картограмму");
    QToolBar *printToolBar = addToolBar("Печать");
    fileMenu->addAction(printAct);
    printToolBar->addAction(printAct);

    fileMenu->addSeparator();

    //Zoom
    QToolBar *zoomToolBar = addToolBar("Масштаб");
    const QIcon zoomOutIcon = QIcon::fromTheme("document-zoom-out", QIcon(":/icons/zoomout.png"));
    QAction *zoomOutAct = zoomToolBar->addAction(zoomOutIcon, "Отдалить", this, &CoinEdit::zoomOut);
    zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
    zoomOutAct->setStatusTip("Отдалить");
    zoomToolBar->addAction(zoomOutAct);

    zoomSlider->setMinimum(0);
    zoomSlider->setMaximum(500);
    zoomSlider->setValue(250);
    zoomSlider->setTickPosition(QSlider::TicksRight);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomToolBar->addWidget(zoomSlider);

    const QIcon zoomInIcon = QIcon::fromTheme("document-zoom-in", QIcon(":/icons/zoomin.png"));
    QAction *zoomInAct = zoomToolBar->addAction(zoomInIcon, "Приблизить", this, &CoinEdit::zoomIn);
    zoomInAct->setShortcuts(QKeySequence::ZoomIn);
    zoomInAct->setStatusTip("Приблизить");
    zoomToolBar->addAction(zoomInAct);

    //Exit
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, "&Выход", this, &QApplication::exit);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip("Завершить работу");

    QMenu *helpMenu = menuBar()->addMenu("&Помощь");

    helpMenu->addAction("&О программе", this, &CoinEdit::about);
    helpMenu->addAction("About &Qt", &QApplication::aboutQt);

}

void CoinEdit::createVertGroupBox()
{
    vertGroupBox = new QGroupBox("Vertical layout");
    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *buttons[5];

    for (int i = 0; i < 5; ++i) {
        buttons[i] = new QPushButton(QString("Button %1").arg(i + 1));
        layout->addWidget(buttons[i]);
    }

    vertGroupBox->setLayout(layout);
}


//void CoinEdit::updateActions()
//{
//    saveAsAct->setEnabled(!image.isNull());
//    copyAct->setEnabled(!image.isNull());
//    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
//    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
//    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
//}

void CoinEdit::scaleImage(double factor)
{
//    scaleFactor *= factor;
//    view->resize(scaleFactor * imageLabel->pixmap(Qt::ReturnByValue).size());

//    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
//    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

//    zoomInAct->setEnabled(scaleFactor < 3.0);
//    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

//void CoinEdit::adjustScrollBar(QScrollBar *scrollBar, double factor)
//{
//    scrollBar->setValue(int(factor * scrollBar->value()
//                            + ((factor - 1) * scrollBar->pageStep()/2)));
//}
