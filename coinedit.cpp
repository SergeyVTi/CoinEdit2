#include "coinedit.h"
#include "coin.h"
#include "graphics.h"

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
#include <QGraphicsObject>
#include <QPrintPreviewDialog>
//#include <QPdfDocument>

CoinEdit::CoinEdit(QWidget *parent) : QMainWindow(parent),
    view(new GraphicsView(this)), zoomSlider(new QSlider), cellDialog(new CellDialog(this)),
    scene(new GraphicsScene(view))
{
    //Set main layout
    QGroupBox *mainGroupBox = new QGroupBox;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setCentralWidget(mainGroupBox);
    mainGroupBox->setLayout(mainLayout);

    //resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);


    //Add central widget
    setSectorLines();

    setupCells();

    drawPitak();

    //Add scroll area
    //    scrollArea = new QScrollArea;
    //    scrollArea->setBackgroundRole(QPalette::Dark);
    //    scrollArea->setWidget(view);
    //    mainLayout->addWidget(scrollArea);
    //    view->setRenderHint(QPainter::Antialiasing, false);

    mainLayout->addWidget(view);

    //TextEditor
//    QTextEdit *textEdit = new QTextEdit();
//    textEditor = new TextEditor(scene->addWidget(textEdit));
    textEditor = new TextEditor(this);
    QGraphicsProxyWidget *proxy = scene->addWidget(textEditor);
//    textEditor->setProxy(proxy);
    textEditor->setView(view);

    //Create menus
    createActions();

    //Add status bar
    statusBar()->showMessage("Привет!",4000);

    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &CoinEdit::setupMatrix);
    connect(cellDialog, &CellDialog::updateStats, this, &CoinEdit::updateStats);


//    scene->setSceneRect(QRect(1000, -2600, 3800, 2676));
    view->horizontalScrollBar()->setValue(1876);
    view->verticalScrollBar()->setValue(-2397);
    emit zoomSlider->valueChanged(206);

//    view->horizontalScrollBar()->setMinimum(-3200);
//    view->horizontalScrollBar()->setMaximum(5000);
//    view->horizontalScrollBar()->setRange(-5000,5000);
    scene->setSceneRect(QRect(-500-500, -2600-500, 4800, 4800/1.42));

    open();
}

void CoinEdit::save()
{
    if (fileName.isEmpty()){
        saveAs();
        return;
    }

    SaveFormat saveFormat = SaveFormat::Json;

    if (!fileName.endsWith(".json"))
        fileName += ".json";

    QFile saveFile(saveFormat == Json
          ? fileName : QStringLiteral("save.dat"));

//     QFile saveFile(fileName);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, "Не могу открыть файл",
                     saveFile.errorString());
        return;
    }

      QJsonObject dataObject;
      write(dataObject);
      saveFile.write(QJsonDocument(dataObject).toJson());
//      saveFile.write(saveFormat == Json
//          ? QJsonDocument(dataObject).toJson()
//          : QCborValue::fromJsonValue(dataObject).toCbor());

//      return true;
    statusBar()->showMessage("Картограмма сохранена в " + fileName,4000);
}

void CoinEdit::open()
{
    SaveFormat saveFormat = SaveFormat::Json;

//    fileName = QFileDialog::getOpenFileName(this,
//            "Открыть картограмму", QDir::currentPath(),
//            "JSON (*.json);;binary (*.dat)");

   fileName = "save.json";

    if (fileName.isEmpty())
        return;

    QFile loadFile(saveFormat == Json ? fileName : QStringLiteral("save.dat"));

    if (!loadFile.open(QIODevice::ReadWrite)) {
        QMessageBox::information(this, tr("Unable to open file"),
            loadFile.errorString());
        return;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(saveFormat == Json
                  ? QJsonDocument::fromJson(saveData)
                  : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

    read(loadDoc.object());

    statusBar()->showMessage("Открыта картограмма " + fileName,4000);
}

void CoinEdit::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));
//    qDebug() << zoomSlider->value();

    QTransform matrix;
    matrix.scale(scale, scale);
//    matrix.rotate(rotateSlider->value());

    view->setTransform(matrix);
    //    setResetButtonEnabled();
}

void CoinEdit::saveAs()
{
    fileName = QFileDialog::getSaveFileName(this,
            "Сохранить картограмму", QDir::currentPath(),
            "JSON (*.json);;binary (*.dat)");

    if (fileName.isEmpty())
          return;

    save();
}

void CoinEdit::drawPitak()
{
    int x = 3800;
    workingArea = QRect(-500, -2600, x, x/1.42);
    QGraphicsRectItem *workingRect = scene->addRect(workingArea, QPen(Qt::NoPen), QBrush(Qt::white));
//    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
//    effect->setColor(Qt::black);
//    effect->setOffset(10);
//    effect->setBlurRadius(10);

//    workingRect->setGraphicsEffect(effect);

    scene->setBackgroundBrush(QBrush(Qt::darkGray));

    int indent = 100;
    qreal k = 1.2;
    //Add graphics

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

    QHash<QString, Coin*> allCells;

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
            allCells[cellNum] = item;

            scene->addItem(item);
            connect(item, &Coin::printCellNum, [&](const QString& cellNum){
                statusBar()->showMessage(cellNum, 4000);
            });

            QFont font("Times", 8, QFont::Normal, false);

            QGraphicsTextItem *text = scene->addText(cellNum, font);
            text->setRotation(-30);
            text->setPos(QPointF(x*xStep + xStart[y-1]*xStep + RADIUS*0.45, -y*yStep + RADIUS*0.7));
            text->setZValue(1);

        }
    }

//    QRect(-500, -2600, x, x/1.42);
    int yup = -indent - (5+xSize.size())*k*(RADIUS + STEP) - 30;
//    qDebug()<<"y up: "<< yup;
//    int yd = indent - 3*k*(RADIUS + STEP) - 3;
//    qDebug()<<"y down: "<< yd;

    int xr = static_cast<qreal>(*std::max_element(xSize.begin(), xSize.end()))*xStep*2+xStep*3 - 3*STEP +indent + 4;
//    qDebug()<<"x right: "<< xr;

    int pdfSize = 807;
//    pdfRect1 = scene->addRect(xr + 40, yup, pdfSize, pdfSize*1.42);
//    qDebug()<<xr + 40 << " "<< yup << " " <<  pdfSize << " " << pdfSize*1.42;

//    pdfRect2 = scene->addRect(xr + 40, yup+pdfSize*1.42, pdfSize, pdfSize*1.42);
//    qDebug()<<xr + 40 << " "<<  yup+pdfSize*1.42 << " " <<  pdfSize << " " << pdfSize*1.42;

    cellDialog->setAllCells(allCells);

    //TextEdit
    QRect textEditRect = QRect(xr + 40, yup, pdfSize, pdfSize*1.42*2);
    qDebug()<<xr + 40 << " "<<  yup << " " <<  pdfSize << " " << pdfSize*1.42*2;
//    proxyTextEdit->setGeometry(textEditRect);

    view->setScene(scene);
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
    }

    if (json.contains("Боковушка") && json["Боковушка"].isObject()){//
//        QJsonValue textEditorData = json["Боковушка"].toString();
//        qDebug()<<textEditorData;
//        QJsonObject textEditorDataObject = textEditorData.toObject();
        QJsonObject textEditorDataObject = json["Боковушка"].toObject();
        textEditor->read(textEditorDataObject);
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
    json["Ячейки"] = cellsArray;

//    QJsonArray textEditorArray;
    QJsonObject textEditorObject;
    textEditor->write(textEditorObject);
//    textEditorArray.append(textEditorObject);
    json["Боковушка"] = textEditorObject;
}

void CoinEdit::print(QPrinter *printer)
{
//    QPrinter printer(QPrinter::HighResolution);
//    QPrintDialog dialog(printer, this);

//    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(printer);
        QRect viewport = view->viewport()->rect();
        qDebug() << "viewport " << viewport;
//        qDebug() << "window " << view->window()->rect();

        QRect sceneRect = scene->itemsBoundingRect().toRect();
//        scene->setSceneRect(sceneRect);
        qDebug() << "scene " << sceneRect;

        QRect viewRect = view->contentsRect();
        qDebug() << "view " << viewRect;

        QRect mappedRect = view->mapFromScene(sceneRect).boundingRect();
        qDebug() << "mappedRect " << mappedRect;

//        painter.setViewport(sceneRect);
//        painter.setWindow(sceneRect);

//        painter.setWorldMatrixEnabled(false);

//        painter.setViewport(mappedRect);
//        painter.setWindow(QRect(0,0, 13699,9583));

        QRectF target = QRectF(0, 0, printer->width(), printer->height());
        qDebug() << "target " << target;

        view->render(&painter,
                     target,
                     mappedRect.adjusted(0,0,-10,-10),
//                     workingArea.adjusted(0, workingArea.height()/2, 0, workingArea.height()/2), //);
//                     viewport.adjusted(-viewport.width(), -viewport.height(), viewport.width(), viewport.height()));
                   Qt::KeepAspectRatio);
//        view->render(&painter);
//    }
}

void CoinEdit::printPreview()
{
    textEditor->hide();
    textEditorText = new QGraphicsTextItem();
    scene->addItem(textEditorText);
    textEditorText->setPos(2374, -2347);
//    textEditorText->setTextWidth()
    textEditorText->setHtml(textEditor->document()->toHtml());

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
//    QPainter painter(&printer);
//    view->render(&painter);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &CoinEdit::print);
    preview.exec();

    delete textEditorText;
    textEditor->show();
}

void CoinEdit::zoomIn(int level)
{
    zoomSlider->setValue(zoomSlider->value() + level);
}

void CoinEdit::zoomOut(int level)
{
    zoomSlider->setValue(zoomSlider->value() - level);
}

void CoinEdit::about()
{
    QMessageBox::about(this, "О программе",
               "<p>Программа <b>CoinEdit 2</b> предназначна для составления"
               " картограмм перегрузки реактра Р.</p>"
               "<p>Разработал Тихонов С.</p>"
               "<p>stikhon88@gmail.com</p>");
}

void CoinEdit::TODO()
{
    QMessageBox message;
//    message.setWindowTitle("Сорян");
    message.setText("Функционал не готов");
    //    message.setInformativeText("?");
    message.setIcon(QMessageBox::Critical);
    message.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    message.setDefaultButton(QMessageBox::Ok);

    message.exec();
}

void CoinEdit::updateStats(QHash<QString, size_t> &typeCounter, QHash<QString, size_t> &subTypeCounter)
{
    typeCounter.clear();
    subTypeCounter.clear();

    for(auto& cell : cellsVec){
        typeCounter[cell->loadingType()]++;
        subTypeCounter[cell->loadingSubType()]++;
    }
}

void CoinEdit::addPdf()
{
    qDebug()<<view->horizontalScrollBar()->value();
    qDebug()<<view->verticalScrollBar()->value();
    qDebug()<<zoomSlider->value();

#if   QT_VERSION >= 0x060402
//    pdfFileName = QFileDialog::getOpenFileName(this,
//                           "Открыть pdf файл", QDir::currentPath(),
//                           "pdf (*.pdf)");

//    QUrl pdfFileName = QFileDialog::getOpenFileUrl(this, tr("Choose a PDF"), QUrl(), "Portable Documents (*.pdf)");
//         if (pdfFileName.isValid())
//             open(pdfFileName);
//    QPdfDocument *pdfDoc;

//    if (docLocation.isLocalFile()) {
//             m_document->load(docLocation.toLocalFile());
//             const auto documentTitle = m_document->metaData(QPdfDocument::Title).toString();
//             setWindowTitle(!documentTitle.isEmpty() ? documentTitle : QStringLiteral("PDF Viewer"));
//         } else {
//             qCDebug(lcExample) << docLocation << "is not a valid local file";
//             QMessageBox::critical(this, tr("Failed to open"), tr("%1 is not a valid local file").arg(docLocation.toString()));
//         }
//         qCDebug(lcExample) << docLocation
#endif
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
//    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/icons/new.png"));
//    QAction *newAct = new QAction(newIcon, "&Создать", this);
//    newAct->setShortcuts(QKeySequence::New);
//    newAct->setStatusTip("Создать новую картограмму");
//    connect(newAct, &QAction::triggered, this, &CoinEdit::newFile);
//    fileMenu->addAction(newAct);
//    fileToolBar->addAction(newAct);

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
    QAction *printAct = fileMenu->addAction(printIcon, "&Печать...", this, &CoinEdit::printPreview);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip("Распечатать картограмму");
    QToolBar *printToolBar = addToolBar("Печать");
    fileMenu->addAction(printAct);
    printToolBar->addAction(printAct);

    fileMenu->addSeparator();

    //Zoom
    QToolBar *zoomToolBar = addToolBar("Масштаб");
    const QIcon zoomOutIcon = QIcon::fromTheme("document-zoom-out", QIcon(":/icons/zoomout.png"));
    QAction *zoomOutAct = zoomToolBar->addAction(zoomOutIcon, "Отдалить", this, [&]{
        zoomOut(8);
    });
    zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
    zoomOutAct->setStatusTip("Отдалить");
    zoomToolBar->addAction(zoomOutAct);

    zoomSlider->setMinimum(146);
    zoomSlider->setMaximum(282);
    zoomSlider->setValue(206);
    zoomSlider->setTickPosition(QSlider::TicksRight);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomToolBar->addWidget(zoomSlider);

    const QIcon zoomInIcon = QIcon::fromTheme("document-zoom-in", QIcon(":/icons/zoomin.png"));
    QAction *zoomInAct = zoomToolBar->addAction(zoomInIcon, "Приблизить", this, [&]{
        zoomIn(8);
    });
    zoomInAct->setShortcuts(QKeySequence::ZoomIn);
    zoomInAct->setStatusTip("Приблизить");
    zoomToolBar->addAction(zoomInAct);


    //Боковушка
//    QToolBar *addPdfToolBar = addToolBar("Добавить боковушку");
//    QAction *addPdfAct = addPdfToolBar->addAction("+pdf", this, &CoinEdit::addPdf);
//    addPdfAct->setStatusTip("Добавить боковушку");

    textEditor->setupEditActions();
    //
    textEditor->setupTextActions();
    //
    textEditor->doOnStartUp();

    //Exit
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, "Выход", this, &QApplication::exit);
    exitAct->setShortcut(QKeySequence("Ctrl+Q"));
    exitAct->setStatusTip("Завершить работу");

    QMenu *helpMenu = menuBar()->addMenu("П&омощь");

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

//void CoinEdit::adjustScrollBar(QScrollBar *scrollBar, double factor)
//{
//    scrollBar->setValue(int(factor * scrollBar->value()
//                            + ((factor - 1) * scrollBar->pageStep()/2)));
//}
