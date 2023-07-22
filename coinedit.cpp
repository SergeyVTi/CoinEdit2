#include "coinedit.h"
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QSignalTransition>
#include <QState>
#include <QStateMachine>
#include "coin.h"
#include "graphics.h"

CoinEdit::CoinEdit(QWidget *parent)
    : QMainWindow(parent)
{
    // Set main layout
    QGroupBox *mainGroupBox = new QGroupBox;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setCentralWidget(mainGroupBox);
    mainGroupBox->setLayout(mainLayout);

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    //    resize(QGuiApplication::primaryScreen()->availableSize());
    //    setWindowState(Qt::WindowFullScreen);

    // TextEditor
    textEditors = new TextEditors(this);
    connect(textEditors, &TextEditors::addTableCell, this, &CoinEdit::addTableCell);

    textEditorLeftPrintItem = new GraphicsTextItem;
    textEditorLeftPrintItem->setMode(GraphicsTextItem::TextEditorLeft);

    textEditorRightPrintItem = new GraphicsTextItem;
    textEditorRightPrintItem->setMode(GraphicsTextItem::TextEditorRight);

    // cellDialog
    cellDialog = new CellDialog(this);
    cellDialog->setRightTextEditor(textEditors);
    connect(this, &CoinEdit::addCellNumToDialog, cellDialog, &CellDialog::addCellNumToDialog);
    connect(this,
            &CoinEdit::updateComboLoadingTypes,
            cellDialog,
            &CellDialog::updateComboLoadingTypes);

    // Header
    header = new GraphicsTextItem;
    header->setMode(GraphicsTextItem::Header);

    setHeader("Новая комплектация");

    dTItem = new GraphicsTextItem;
    dTItem->setMode(GraphicsTextItem::dTLabel);

    loadingItem = new GraphicsTextItem;
    loadingItem->setMode(GraphicsTextItem::loadingLabel);

    // Add status bar
    statusBar()->showMessage("Привет!", 4000);

    zoomSlider = new QSlider;
    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &CoinEdit::setupMatrix);

    view = new GraphicsView(this);
    view->horizontalScrollBar()->setValue(1876);
    view->verticalScrollBar()->setValue(-2397);
    mainLayout->addWidget(view);
    emit zoomSlider->valueChanged(206);

    scene = new GraphicsScene(view);
    scene->setSceneRect(QRect(-1000, -2950, 4800, 4800 / 1.42));
    scene->addItem(header);
    scene->addItem(textEditorLeftPrintItem);
    scene->addItem(textEditorRightPrintItem);
    scene->addItem(dTItem);
    scene->addItem(loadingItem);

    QGraphicsProxyWidget *proxy = scene->addWidget(textEditors);
    proxy->setPos(-500, -2600);
    // Тормозит
    //    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect();
    //    effect->setColor(Qt::black);
    //    effect->setOffset(10);
    //    effect->setBlurRadius(10);
    //    proxy->setGraphicsEffect(effect);

    // Create menus
    createActions();

    // Create states
    rootState = new QState;
    mainState = new QState(rootState);
    randomState = new QState(rootState);
    centeredState = new QState(rootState);
    mainStateAct = new QAction;

    // Add central widget
    setSectorLines();

    setupCells();

    drawPitak();

    setupSimmLines();

    setupStateMachine();

    //    open(); //!
    //    addTArch();
    //    addLoadingArch();

    emit onStartUp();

    showTimer = new QTimer;
    showTimer->start(2000);
    showTimer->callOnTimeout([&] {
        for (auto cellNum : cellNumVec)
            cellNum->show();
    });
}

void CoinEdit::save()
{
    if (fileName.isEmpty())
        return saveAs();
    if (fileName.fileName().startsWith(QStringLiteral(":/")))
        return saveAs();

    SaveFormat saveFormat = fileName.fileName().contains(QRegularExpression(".json$"))
                                ? SaveFormat::Json
                                : SaveFormat::Binary;

    QFile saveFile(fileName.toDisplayString(QUrl::None));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, "Не могу открыть файл", saveFile.errorString());
        return;
    }

    QJsonObject dataObject;

    write(dataObject);

    saveFile.write(saveFormat == Json ? QJsonDocument(dataObject).toJson()
                                      : QCborValue::fromJsonValue(dataObject).toCbor());

    setHeader(fileName.fileName());
    statusBar()->showMessage("Комплектация сохранена в " + fileName.toDisplayString(QUrl::None),
                             10000);
}

void CoinEdit::open()
{
    QFileDialog fileDialog(this, "Открыть комплектация");

    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    QDir path(QDir::currentPath());
    fileDialog.setDirectory(path);
    //    fileDialog.setDefaultSuffix("json");
    //    fileDialog.setNameFilter("JSON (*.json);;binary (*.dat)");
    fileDialog.setDefaultSuffix("dat");
    fileDialog.setNameFilter("Binary (*.dat)");

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QString fn = fileDialog.selectedFiles().first();
    if (fn.isEmpty())
        return;

    path = fileDialog.directory();

    fileName = path.path() + "/" + QFileInfo(fn).fileName();
    //    qDebug() << "fileName" << fileName.toDisplayString(QUrl::None);
    //    fileName = "test.dat";

    setHeader(fileName.fileName());

    SaveFormat saveFormat = fileName.fileName().contains(QRegularExpression(".json$"))
                                ? SaveFormat::Json
                                : SaveFormat::Binary;

    QFile loadFile(fileName.toDisplayString(QUrl::None));

    if (!loadFile.open(QIODevice::ReadWrite)) {
        QMessageBox::information(this, "Не могу открыть файл", loadFile.errorString());
        return;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(
        saveFormat == Json ? QJsonDocument::fromJson(saveData)
                           : QJsonDocument(QCborValue::fromCbor(saveData).toMap().toJsonObject()));

    read(loadDoc.object());

    statusBar()->showMessage("Открыта комплектация " + fileName.toDisplayString(QUrl::None), 10000);

    QTimer *timer = new QTimer(this);
    timer->start(300);
    timer->setSingleShot(true);
    timer->callOnTimeout([&]{
        qDebug()<<"timeout";
        emit mainStateAct->triggered();
    });
}

void CoinEdit::setupMatrix()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

    QTransform matrix;
    matrix.scale(scale, scale);

    view->setTransform(matrix);
}

void CoinEdit::setupStateMachine()
{
    // Values
    for (int i = 0; i < cellsVec.size(); ++i) {
        Coin *item = cellsVec.at(i);

        // Random
        randomState->assignProperty(item,
                                    "pos",
                                    QPointF(-500 + QRandomGenerator::global()->bounded(3650),
                                            -2600
                                                + QRandomGenerator::global()->bounded(3650 / 1.42)));

        // Centered
        centeredState->assignProperty(item, "pos", center);
    }

    states = new QStateMachine;
    states->addState(rootState);
    states->setInitialState(rootState);
    rootState->setInitialState(centeredState);

    group = new QParallelAnimationGroup;
    for (int i = 0; i < cellsVec.size(); ++i) {
        QPropertyAnimation *anim = new QPropertyAnimation(cellsVec[i], "pos");
        anim->setDuration(1000 + i);
        anim->setEasingCurve(QEasingCurve::InOutBack);
        group->addAnimation(anim);
    }

    trans = rootState->addTransition(openAct, &QAction::hovered, randomState);
    trans->addAnimation(group);

    trans = rootState->addTransition(openAct, &QAction::triggered, randomState);
    trans->addAnimation(group);

    trans = rootState->addTransition(saveAct, &QAction::hovered, mainState);
    trans->addAnimation(group);

    trans = rootState->addTransition(saveAct, &QAction::triggered, mainState);
    trans->addAnimation(group);

    trans = rootState->addTransition(mainStateAct, &QAction::triggered, mainState);
    trans->addAnimation(group);

    timer = new QTimer;
    timer->start(125);
    timer->setSingleShot(true);
    trans = rootState->addTransition(timer, &QTimer::timeout, mainState);
    trans->addAnimation(group);

    states->start();
}

void CoinEdit::setupSimmLines()
{
    QPen pen(QBrush(Qt::black), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    for (int i = 0; i < 6; i++) {
        QGraphicsLineItem *simmLine = new QGraphicsLineItem;
        simmLine->setZValue(6);
        simmLine->setPen(pen);
        //        scene->addItem(simmLine);
        simmLinesVec.append(simmLine);
    }
}

void CoinEdit::saveAs()
{
    QFileDialog fileDialog(this, "Сохранить комплектацию");
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QDir path(QDir::currentPath());
    fileDialog.setDirectory(path);
    //    fileDialog.setDefaultSuffix("json");
    //    fileDialog.setNameFilter("JSON (*.json);;binary (*.dat)");
    fileDialog.setDefaultSuffix("dat");
    fileDialog.setNameFilter("Binary (*.dat)");

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QString fn = fileDialog.selectedFiles().first();

    path = fileDialog.directory();
    fileName = path.path() + "/" + QFileInfo(fn).fileName();
    qDebug() << "fileName" << fileName;

    save();
}

void CoinEdit::highlightSimmCells(QPointF cellPos)
{
    center = centerCoinPtr->mapToScene(QPointF(1.2 * (RADIUS + STEP), 1.2 * (RADIUS + STEP)));
    QLineF line(center, cellPos);

    for (auto &cell : simmMap) {
        cell->setSimmHighlight(false);
        cell->update();
    }
    simmMap.clear();

    for (int stepAngle = 60, i = 1; stepAngle < 360; stepAngle += 60, i++) {
        simmLinesVec[i]->setLine(line);
        simmLinesVec[i]->setTransformOriginPoint(center);
        simmLinesVec[i]->setRotation(stepAngle);
        simmLinesVec[i]->update();

        QPointF simmPoint = simmLinesVec[i]->mapToScene(simmLinesVec[i]->line().p2());

        QList<QGraphicsItem *> list = scene->items(simmPoint);
        for (QGraphicsItem *item : qAsConst(list)) {
            Coin *cell = qgraphicsitem_cast<Coin *>(item);
            if (cell && cell->getItemMode() == Coin::ItemMode::MainItem) {
                simmMap[cell->getCellNum()] = cell;
            }
        }
    }

    for (auto &cell : simmMap) {
        cell->setSimmHighlight(true);
        cell->update();
    }
}

void CoinEdit::deleteTableItem()
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    for (QGraphicsItem *item : qAsConst(selectedItems)) {
        for (auto it = tableCells.begin(); it != tableCells.end();) {
            if (*it == qgraphicsitem_cast<Coin *>(item)) {
                tableCells.erase(it);
                qDebug() << "deleted";
                break;
            }
            it++;
        }
        scene->removeItem(item);
        delete item;
    }
}

void CoinEdit::addCelltoDialog()
{
    qDebug() << "addCelltoDialog";
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    for (QGraphicsItem *item : qAsConst(selectedItems)) {
        qDebug() << qgraphicsitem_cast<Coin *>(item)->getCellNum();
        emit addCellNumToDialog(qgraphicsitem_cast<Coin *>(item)->getCellNum());
    }
}

void CoinEdit::addTArch()
{
    QFileDialog fileDialog(this, "Открыть архив");

    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    QDir path(QDir::currentPath());
    fileDialog.setDirectory(path);
    fileDialog.setDefaultSuffix("csv");
    fileDialog.setNameFilter("csv (*.csv)");

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QString fn = fileDialog.selectedFiles().first();
    if (fn.isEmpty())
        return;

    path = fileDialog.directory();

    archTFileName = path.path() + "/" + QFileInfo(fn).fileName();
    //    archTFileName = "2023_05_28.csv";

    QFile loadFile(archTFileName.toDisplayString(QUrl::None));

    if (!loadFile.open(QIODevice::ReadWrite)) {
        QMessageBox::information(this, "Не могу открыть файл", loadFile.errorString());
        return;
    }

    QTextStream in(&loadFile);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.contains("A4G4L")) {
            QString date = line.section(';', 0, 0);
            QString time = line.section(';', 1, 1);

            dTItem->setHtml("<p align=\"center\"><span style=\"font-family:'Times New "
                            "Roman'; font-size:20pt\">"
                            + QString("Архив подогревов от ") + date + " "
                            + time.remove(time.size() - 7, 7) + "</span></p>");
            break;
        }
    }

    QRegularExpression regExSUZnum("([^0-9]|^)([(])(?<cell>\\d\\d-\\d\\d)([)])([^0-9]|$)");
    QRegularExpression regExdT("([^0-9]|^)(?<cell>\\d\\d-\\d\\d)([^0-9]|$)");

    in.seek(0);
    while (!in.atEnd()) {
        QString line = in.readLine();

        QRegularExpressionMatchIterator i = regExSUZnum.globalMatch(line);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString suzNum = match.captured("cell");
            QString suzPos = line.section(';', 4, 4);
            allCells[suzNum]->setSuzPos(suzPos);
            allCells[suzNum]->setIconType(Coin::IconType::PenSize2red);
        }

        if (line.contains("A4D")) {
            QRegularExpressionMatchIterator i = regExdT.globalMatch(line);
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString dTCellNum = match.captured("cell");
                QString dT = line.section(';', 4, 4);
                (dT.size() == 7) ? allCells[dTCellNum]->setDT(dT.remove(dT.size() - 3, 3))
                                 : allCells[dTCellNum]->setDT(dT.remove(dT.size() - 2, 2));
            }
        }
    }
}

void CoinEdit::addLoadingArch()
{
    QUrl typesFileName("ТИПЫ.txt");

    QFile typesFile(typesFileName.toDisplayString(QUrl::None));

    if (!typesFile.open(QIODevice::ReadWrite)) {
        QMessageBox::information(this,
                                 "Не могу открыть файл: "
                                     + typesFileName.toDisplayString(QUrl::None),
                                 typesFile.errorString());
        return;
    }

    QHash<QString, QString> typesMap;
    QHash<QString, QString> csMap;
    bool isType = true;

    QTextStream typesIn(&typesFile);
    typesIn.setEncoding(QStringConverter::System);
    QString typesInStr = typesIn.readAll();
    if (!typesInStr.contains("ТИПЫ:") || !typesInStr.contains("ЦС:")) {
        qDebug() << typesFileName << "is empty";
        typesIn << "Задайте типы кассет, цс в формате (\"название\" номер)\n"
                   "ТИПЫ:\n\n"
                   "ЦС:";
        return;
    }

    typesIn.seek(0);
    while (!typesIn.atEnd()) {
        QString line = typesIn.readLine();
        if (line.isEmpty())
            continue;

        if (line.contains("ТИПЫ:")) {
            isType = true;
        } else if (line.contains("ЦС:")) {
            isType = false;
        }

        QString type = line.section(' ', 0, 0);
        type.remove('"');
        QString typeNum = line.section(' ', 1, 1);

        isType ? typesMap[typeNum] = type : csMap[typeNum] = type;
    }

    QFileDialog fileDialog(this, "Открыть архив");

    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    QDir path(QDir::currentPath());
    fileDialog.setDirectory(path);
    fileDialog.setDefaultSuffix("txt");
    fileDialog.setNameFilter("txt (*.txt)");

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QString fn = fileDialog.selectedFiles().first();
    if (fn.isEmpty())
        return;

    path = fileDialog.directory();

    archLoadingFileName = path.path() + "/" + QFileInfo(fn).fileName();
    //    archLoadingFileName = "20230601.txt";

    QFile loadingFile(archLoadingFileName.toDisplayString(QUrl::None));

    if (!loadingFile.open(QIODevice::ReadWrite)) {
        QMessageBox::information(this, "Не могу открыть файл", loadingFile.errorString());
        return;
    }

    QRegularExpression regExNum("([^0-9]|^)(?<cell>\\d\\d-\\d\\d)([^0-9]|$)");
    QHash<QString, QStringList> loadingTypes;

    QTextStream loadingsIn(&loadingFile);
    bool isCS;
    while (!loadingsIn.atEnd()) {
        QString line = loadingsIn.readLine();
        //        qDebug() << line;

        QRegularExpressionMatchIterator i = regExNum.globalMatch(line);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString cellNum = match.captured("cell");
            QString loadingNum = line.section('\t', 2, 2);
            line.contains("ЦС") ? isCS = true : isCS = false;
            //            qDebug() << "loading" << cellNum << loadingNum << isCS;
            if (allCells[cellNum]) {
                isCS ? allCells[cellNum]->setLoadingSubType(csMap[loadingNum])
                     : allCells[cellNum]->setLoadingType(typesMap[loadingNum]);
            }
        }
    }

    loadingItem->setHtml(
        "<p align=\"center\"><span style=\"font-family:'Times New "
        "Roman'; font-size:20pt\">"
        + QString("Архив загрузки ")
        //                         + archLoadingFileName.toDisplayString(QUrl::None) + +"</span></p>");
        + archLoadingFileName.fileName() + "</span></p>");

    for (auto &cell : cellsVec) {
        loadingTypes[cell->getLoadingType()].append(cell->getLoadingSubType());
        cell->setText(cell->getLoadingSubType());
    }

    for (auto &list : loadingTypes) {
        list.removeDuplicates();
    }

    showSrezAct->setChecked(true);
    emit updateComboLoadingTypes(loadingTypes);
}

void CoinEdit::drawPitak()
{
    scene->setBackgroundBrush(QBrush(Qt::darkGray));

    const int xStep = qSin(qDegreesToRadians(60.0)) * RADIUS + STEP;
    const int yStep = RADIUS + qSin(qDegreesToRadians(30.0)) * RADIUS + STEP * 1.5;
    const int maxCoinInRow = static_cast<qreal>(*std::max_element(xSize.begin(), xSize.end()));

    for (int x = 1; x < maxCoinInRow * 2; x += 1) {
        QGraphicsLineItem *lineY
            = new QGraphicsLineItem(x * xStep + (RADIUS + STEP) * k,
                                    indent - 3 * k * (RADIUS + STEP) + vertIndent,
                                    x * xStep + (RADIUS + STEP) * k,
                                    -indent - (5 + xSize.size()) * k * (RADIUS + STEP) - vertIndent);

        QPen pen = (x % 2 == 0) ? QPen(Qt::black) : QPen(Qt::blue);
        pen.setWidth(1);

        lineY->setPen(pen);
        scene->addItem(lineY);

        QString xNum = (x + 6 < 10) ? "0" + QString::number(x + 6) : QString::number(x + 6);
        QGraphicsSimpleTextItem *lineNumberDown = new QGraphicsSimpleTextItem(xNum);
        QFont font = QFont("Times", 20, QFont::Normal, true);
        lineNumberDown->setFont(font);
        lineNumberDown->setPen(pen);
        lineNumberDown->setPos(x * xStep + (RADIUS + STEP) * k - 17,
                               indent - 3 * k * (RADIUS + STEP) + 1 + vertIndent);

        scene->addItem(lineNumberDown);

        QGraphicsSimpleTextItem *lineNumberUp = new QGraphicsSimpleTextItem(xNum);
        lineNumberUp->setFont(font);
        lineNumberUp->setPen(pen);
        lineNumberUp->setPos(x * xStep + (RADIUS + STEP) * k - 15,
                             -indent - (5 + xSize.size()) * k * (RADIUS + STEP) - 30 - vertIndent);

        scene->addItem(lineNumberUp);
    }

    // Внутренний пятак
    for (int y = 1; y <= xSize.size(); y++) {
        if (xSize[y - 1] == 0)
            continue;

        QGraphicsLineItem *lineX = new QGraphicsLineItem(-indent,
                                                         -y * yStep + (RADIUS + STEP) * k,
                                                         maxCoinInRow * xStep * 2 + xStep * 3
                                                             - 3 * STEP + indent,
                                                         -y * yStep + (RADIUS + STEP) * k);

        QPen pen = (y % 2 == 0) ? QPen(Qt::black) : QPen(Qt::blue);
        pen.setWidth(1);
        lineX->setPen(pen);
        scene->addItem(lineX);

        QString yNum = (y < 10) ? "0" + QString::number(y) : QString::number(y);
        QGraphicsSimpleTextItem *lineNumberLeft = new QGraphicsSimpleTextItem(yNum);
        QFont font = QFont("Times", 20, QFont::Normal, true);
        lineNumberLeft->setFont(font);
        lineNumberLeft->setPen(pen);
        lineNumberLeft->setPos(-indent - 39, -y * yStep + (RADIUS + STEP) * k - 15);

        scene->addItem(lineNumberLeft);

        QGraphicsSimpleTextItem *lineNumberRight = new QGraphicsSimpleTextItem(yNum);
        lineNumberRight->setFont(font);
        lineNumberRight->setPen(pen);
        lineNumberRight->setPos(static_cast<qreal>(*std::max_element(xSize.begin(), xSize.end()))
                                        * xStep * 2
                                    + xStep * 3 - 3 * STEP + indent + 4,
                                -y * yStep + (RADIUS + STEP) * k - 15);

        scene->addItem(lineNumberRight);

        for (int x = 1; x < xSize[y - 1] * 2; x += 2) {
            Coin *backGroundItem = new Coin;

            backGroundItem->setItemMode(Coin::BackGroundItem);
            QColor color(Qt::white);
            backGroundItem->setColor(color);
            backGroundItem->setRadius(k * RADIUS);
            backGroundItem->setStep(STEP);
            backGroundItem->setPos(QPointF(x * xStep + xStart[y - 1] * xStep, -y * yStep));
            backGroundItem->setZValue(1);

            scene->addItem(backGroundItem);
        }
    }

    for (int y = 1; y <= xSize.size(); y++) {
        if (xSize[y - 1] == 0)
            continue;

        for (int x = 1; x < xSize[y - 1] * 2; x += 2) {
            Coin *sectorsItem = new Coin;

            sectorsItem->setItemMode(Coin::ItemMode::SectorsItem);
            sectorsItem->setColor(Qt::white);
            sectorsItem->setRadius(RADIUS);
            sectorsItem->setStep(STEP);
            sectorsItem->setPos(QPointF(x * xStep + xStart[y - 1] * xStep, -y * yStep));
            sectorsItem->setZValue(2);

            Coin *item = new Coin;

            item->setItemMode(Coin::ItemMode::MainItem);
            item->setColor(Qt::white);
            item->setRadius(RADIUS);
            item->setStep(STEP);
            item->setZValue(3);
            item->setContextMenu(contexMenuMainItem);

            QString cellNum = (y < 10 ? ("0" + QString::number(y)) : QString::number(y)) + "-"
                              + ((x + 6 + xStart[y - 1]) < 10
                                     ? ("0" + QString::number(x + 6 + xStart[y - 1]))
                                     : QString::number(x + 6 + xStart[y - 1]));

            item->setCellNum(cellNum);
            item->setCellDialog(cellDialog);

            if (cells.contains(cellNum))
                sectorsItem->setSectors(cells[cellNum]);

            scene->addItem(sectorsItem);

            item->setPos(QPointF(x * xStep + xStart[y - 1] * xStep, -y * yStep));
            item->setLoadingType("Без загрузки");
            item->setLoadingSubType("");

            if (cellNum == "19-34") {
                center = item->pos();
                centerCoinPtr = item;
            }

            cellsVec.append(item);
            allCells[cellNum] = item;

            scene->addItem(item);
            connect(item, &Coin::printCellNum, [&](const QString &cellNum) {
                statusBar()->showMessage(cellNum, 5000);
            });
            connect(item, &Coin::mainState, [&] { mainStateAct->trigger(); });
            connect(item, &Coin::cellPosChanged, this, &CoinEdit::highlightSimmCells);

            mainState->assignProperty(item, "pos", item->pos());
        }
    }

    cellDialog->setAllCells(allCells);

    view->setScene(scene);
}

void CoinEdit::setSectorLines()
{
    cells["34-25"] = {2, 3, 3, 4, 4, 5};
    cells["34-27"] = {3, 4, 4, 5};
    cells["34-29"] = {3, 4, 4, 5};
    cells["34-31"] = {3, 4, 4, 5};
    cells["34-33"] = {3, 4, 4, 5};
    cells["34-35"] = {3, 4, 4, 5};
    cells["34-37"] = {3, 4, 4, 5};
    cells["34-39"] = {3, 4, 4, 5};
    cells["34-41"] = {3, 4, 4, 5};
    cells["34-43"] = {3, 4, 4, 5, 5, 6};
    cells["33-22"] = {2, 3, 3, 4, 4, 5};
    cells["33-24"] = {3, 4};
    cells["33-44"] = {4, 5};
    cells["33-46"] = {3, 4, 4, 5, 5, 6};
    cells["32-19"] = {2, 3, 3, 4, 4, 5, 5, 6};
    cells["32-21"] = {3, 4};
    cells["32-27"] = {2, 3, 3, 4, 4, 5};
    cells["32-29"] = {3, 4, 4, 5};
    cells["32-31"] = {3, 4, 4, 5};
    cells["32-33"] = {3, 4, 4, 5};
    cells["32-35"] = {3, 4, 4, 5};
    cells["32-37"] = {3, 4, 4, 5};
    cells["32-39"] = {3, 4, 4, 5};
    cells["32-41"] = {3, 4, 4, 5, 5, 6};
    cells["32-47"] = {2, 3, 3, 4, 4, 5};
    cells["32-49"] = {3, 4, 4, 5, 5, 6};
    cells["31-16"] = {2, 3, 3, 4, 4, 5};
    cells["31-18"] = {3, 4};
    cells["31-20"] = {4, 5, 5, 6};
    cells["31-24"] = {2, 3, 3, 4, 4, 5};
    cells["31-26"] = {3, 4};
    cells["31-42"] = {4, 5};
    cells["31-44"] = {3, 4, 4, 5, 5, 6};
    cells["31-46"] = {3, 4};
    cells["31-50"] = {4, 5};
    cells["31-52"] = {3, 4, 4, 5, 5, 6};
    cells["30-15"] = {2, 3, 3, 4};
    cells["30-21"] = {2, 3, 3, 4, 4, 5, 5, 6};
    cells["30-23"] = {3, 4};
    cells["30-45"] = {2, 3, 3, 4, 4, 5};
    cells["30-47"] = {3, 4, 4, 5, 5, 6};
    cells["30-53"] = {4, 5, 5, 6};
    cells["29-14"] = {2, 3, 3, 4};
    cells["29-18"] = {2, 3, 3, 4, 4, 5};
    cells["29-20"] = {3, 4};
    cells["29-22"] = {4, 5, 5, 6};
    cells["29-44"] = {2, 3, 3, 4};
    cells["29-48"] = {4, 5};
    cells["29-50"] = {3, 4, 4, 5, 5, 6};
    cells["29-54"] = {4, 5, 5, 6};
    cells["28-13"] = {2, 3, 3, 4};
    cells["28-17"] = {2, 3, 3, 4};
    cells["28-23"] = {4, 5, 5, 6};
    cells["28-43"] = {2, 3, 3, 4};
    cells["28-51"] = {4, 5, 5, 6};
    cells["28-55"] = {4, 5, 5, 6};
    cells["27-12"] = {2, 3, 3, 4};
    cells["27-16"] = {2, 3, 3, 4};
    cells["27-24"] = {4, 5, 5, 6};
    cells["27-42"] = {2, 3, 3, 4};
    cells["27-52"] = {4, 5, 5, 6};
    cells["27-56"] = {4, 5, 5, 6};
    cells["26-11"] = {2, 3, 3, 4};
    cells["26-15"] = {2, 3, 3, 4};
    cells["26-25"] = {4, 5, 5, 6};
    cells["26-41"] = {2, 3, 3, 4};
    cells["26-53"] = {4, 5, 5, 6};
    cells["26-57"] = {4, 5, 5, 6};
    cells["25-10"] = {2, 3, 3, 4};
    cells["25-14"] = {2, 3, 3, 4};
    cells["25-26"] = {4, 5, 5, 6};
    cells["25-40"] = {2, 3, 3, 4};
    cells["25-54"] = {4, 5, 5, 6};
    cells["25-58"] = {4, 5, 5, 6};
    cells["24-09"] = {2, 3, 3, 4};
    cells["24-13"] = {2, 3, 3, 4};
    cells["24-27"] = {4, 5, 5, 6};
    cells["24-39"] = {2, 3, 3, 4};
    cells["24-55"] = {4, 5, 5, 6};
    cells["24-59"] = {4, 5, 5, 6};
    cells["23-08"] = {2, 3, 3, 4};
    cells["23-12"] = {2, 3, 3, 4};
    cells["23-28"] = {4, 5, 5, 6};
    cells["23-38"] = {2, 3, 3, 4};
    cells["23-56"] = {4, 5, 5, 6};
    cells["23-60"] = {4, 5, 5, 6};
    cells["22-07"] = {1, 2, 2, 3, 3, 4};
    cells["22-11"] = {2, 3, 3, 4};
    cells["22-29"] = {4, 5, 5, 6};
    cells["22-37"] = {2, 3, 3, 4};
    cells["22-57"] = {4, 5, 5, 6};
    cells["22-61"] = {4, 5, 5, 6, 6, 1};
    cells["21-08"] = {2, 3};
    cells["21-10"] = {4, 5, 5, 6};
    cells["21-30"] = {4, 5, 5, 6};
    cells["21-36"] = {2, 3, 3, 4};
    cells["21-58"] = {2, 3, 3, 4};
    cells["21-60"] = {5, 6};
    cells["20-07"] = {1, 2, 2, 3, 3, 4};
    cells["20-11"] = {2, 3, 3, 4};
    cells["20-31"] = {4, 5, 5, 6};
    cells["20-35"] = {2, 3, 3, 4};
    cells["20-57"] = {4, 5, 5, 6};
    cells["20-61"] = {4, 5, 5, 6, 6, 1};
    cells["19-08"] = {2, 3};
    cells["19-10"] = {4, 5, 5, 6};
    cells["19-32"] = {4, 5, 5, 6};
    cells["19-36"] = {3, 4, 4, 5};
    cells["19-38"] = {3, 4, 4, 5};
    cells["19-40"] = {3, 4, 4, 5};
    cells["19-42"] = {3, 4, 4, 5};
    cells["19-44"] = {3, 4, 4, 5};
    cells["19-46"] = {3, 4, 4, 5};
    cells["19-48"] = {3, 4, 4, 5};
    cells["19-50"] = {3, 4, 4, 5};
    cells["19-52"] = {3, 4, 4, 5};
    cells["19-54"] = {3, 4, 4, 5};
    cells["19-56"] = {3, 4, 4, 5};
    cells["19-58"] = {2, 3, 3, 4, 4, 5};
    cells["19-60"] = {3, 4, 4, 5, 5, 6};
    cells["19-34"] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6};
    cells["18-07"] = {1, 2, 2, 3, 3, 4, 4, 5};
    cells["18-09"] = {3, 4, 4, 5, 5, 6};
    cells["18-11"] = {3, 4, 4, 5};
    cells["18-13"] = {3, 4, 4, 5};
    cells["18-15"] = {3, 4, 4, 5};
    cells["18-17"] = {3, 4, 4, 5};
    cells["18-19"] = {3, 4, 4, 5};
    cells["18-21"] = {3, 4, 4, 5};
    cells["18-23"] = {3, 4, 4, 5};
    cells["18-25"] = {3, 4, 4, 5};
    cells["18-27"] = {3, 4, 4, 5};
    cells["18-29"] = {3, 4, 4, 5};
    cells["18-31"] = {3, 4, 4, 5};
    cells["18-33"] = {3, 4, 4, 5};
    cells["18-35"] = {2, 3, 3, 4, 4, 5, 5, 6};
    cells["18-57"] = {4, 5, 5, 6};
    cells["18-61"] = {4, 5, 5, 6, 6, 1};
    cells["17-08"] = {2, 3};
    cells["17-10"] = {4, 5, 5, 6};
    cells["17-34"] = {2, 3, 3, 4};
    cells["17-36"] = {4, 5, 5, 6};
    cells["17-58"] = {2, 3, 3, 4};
    cells["17-60"] = {5, 6};
    cells["16-07"] = {1, 2, 2, 3, 3, 4};
    cells["16-11"] = {2, 3, 3, 4};
    cells["16-33"] = {2, 3, 3, 4};
    cells["16-37"] = {4, 5, 5, 6};
    cells["16-57"] = {4, 5, 5, 6};
    cells["16-61"] = {4, 5, 5, 6, 6, 1};
    cells["15-08"] = {1, 2, 2, 3};
    cells["15-10"] = {4, 5, 5, 6};
    cells["15-32"] = {2, 3, 3, 4};
    cells["15-38"] = {4, 5, 5, 6};
    cells["15-58"] = {2, 3, 3, 4};
    cells["15-60"] = {5, 6, 6, 1};
    cells["14-09"] = {1, 2, 2, 3};
    cells["14-11"] = {4, 5, 5, 6};
    cells["14-31"] = {2, 3, 3, 4};
    cells["14-39"] = {4, 5, 5, 6};
    cells["14-57"] = {2, 3, 3, 4};
    cells["14-59"] = {5, 6, 6, 1};
    cells["13-10"] = {1, 2, 2, 3};
    cells["13-12"] = {4, 5, 5, 6};
    cells["13-30"] = {2, 3, 3, 4};
    cells["13-40"] = {4, 5, 5, 6};
    cells["13-56"] = {2, 3, 3, 4};
    cells["13-58"] = {5, 6, 6, 1};
    cells["12-11"] = {1, 2, 2, 3};
    cells["12-13"] = {4, 5, 5, 6};
    cells["12-29"] = {2, 3, 3, 4};
    cells["12-41"] = {4, 5, 5, 6};
    cells["12-55"] = {2, 3, 3, 4};
    cells["12-57"] = {5, 6, 6, 1};
    cells["11-12"] = {1, 2, 2, 3};
    cells["11-14"] = {4, 5, 5, 6};
    cells["11-28"] = {2, 3, 3, 4};
    cells["11-42"] = {4, 5, 5, 6};
    cells["11-54"] = {2, 3, 3, 4};
    cells["11-56"] = {5, 6, 6, 1};
    cells["10-13"] = {1, 2, 2, 3};
    cells["10-15"] = {4, 5, 5, 6};
    cells["10-27"] = {2, 3, 3, 4};
    cells["10-43"] = {4, 5, 5, 6};
    cells["10-53"] = {2, 3, 3, 4};
    cells["10-55"] = {5, 6, 6, 1};
    cells["09-14"] = {1, 2, 2, 3};
    cells["09-16"] = {4, 5, 5, 6};
    cells["09-26"] = {2, 3, 3, 4};
    cells["09-44"] = {4, 5, 5, 6};
    cells["09-52"] = {2, 3, 3, 4};
    cells["09-54"] = {5, 6, 6, 1};
    cells["08-15"] = {1, 2, 2, 3};
    cells["08-17"] = {4, 5};
    cells["08-19"] = {3, 4, 4, 5, 5, 6};
    cells["08-25"] = {2, 3, 3, 4};
    cells["08-45"] = {4, 5, 5, 6};
    cells["08-49"] = {2, 3, 3, 4, 4, 5};
    cells["08-51"] = {3, 4};
    cells["08-53"] = {5, 6, 6, 1};
    cells["07-16"] = {6, 1, 1, 2, 2, 3};
    cells["07-18"] = {1, 2};
    cells["07-20"] = {4, 5};
    cells["07-22"] = {3, 4, 4, 5, 5, 6};
    cells["07-24"] = {3, 4};
    cells["07-46"] = {2, 3, 3, 4, 4, 5, 5, 6};
    cells["07-48"] = {3, 4};
    cells["07-50"] = {6, 1};
    cells["07-52"] = {1, 2, 5, 6, 6, 1};
    cells["06-19"] = {6, 1, 1, 2, 2, 3};
    cells["06-21"] = {1, 2};
    cells["06-23"] = {2, 3, 3, 4, 4, 5};
    cells["06-25"] = {3, 4, 4, 5, 5, 6};
    cells["06-43"] = {2, 3, 3, 4, 4, 5};
    cells["06-45"] = {3, 4};
    cells["06-47"] = {4, 5, 5, 6, 6, 1};
    cells["06-49"] = {5, 6, 6, 1, 1, 2};
    cells["05-22"] = {6, 1, 1, 2, 2, 3, 3, 4};
    cells["05-24"] = {1, 2};
    cells["05-26"] = {4, 5};
    cells["05-28"] = {3, 4, 4, 5};
    cells["05-30"] = {3, 4, 4, 5};
    cells["05-32"] = {3, 4, 4, 5};
    cells["05-34"] = {3, 4, 4, 5};
    cells["05-36"] = {3, 4, 4, 5};
    cells["05-38"] = {3, 4, 4, 5};
    cells["05-40"] = {3, 4, 4, 5};
    cells["05-42"] = {3, 4};
    cells["05-44"] = {6, 1};
    cells["05-46"] = {5, 6, 6, 1, 1, 2};
    cells["04-25"] = {6, 1, 1, 2, 2, 3};
    cells["04-27"] = {6, 1, 1, 2};
    cells["04-29"] = {6, 1, 1, 2};
    cells["04-31"] = {6, 1, 1, 2};
    cells["04-33"] = {6, 1, 1, 2};
    cells["04-35"] = {6, 1, 1, 2};
    cells["04-37"] = {6, 1, 1, 2};
    cells["04-39"] = {6, 1, 1, 2};
    cells["04-41"] = {6, 1, 1, 2};
    cells["04-43"] = {5, 6, 6, 1, 1, 2};
}

void CoinEdit::setupCells()
{
    // Количество ячеек в ряду
    xSize = {0,  0,  0,  10, 13, 16, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 27, 28, 27,
             28, 27, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 16, 13, 10, 0,  0,  0};
    // Отступ ряда
    xStart = {0, 0, 0, 18, 15, 12, 9, 8, 7, 6, 5, 4, 3,  2,  1,  0, 1, 0, 1,
              0, 1, 0, 1,  2,  3,  4, 5, 6, 7, 8, 9, 12, 15, 18, 0, 0, 0};
}

void CoinEdit::setHeader(const QString &fileName)
{
    QString headerText = fileName;
    headerText.remove(QRegularExpression(".json$"));
    headerText.remove(QRegularExpression(".dat$"));
    header->setHtml("<p align=\"center\"><span style=\"font-family:'Times New "
                    "Roman'; font-size:60pt\">"
                    + headerText + "</span></p>");
}

void CoinEdit::read(const QJsonObject &json)
{
    if (json.contains("Ячейки") && json["Ячейки"].isArray()) {
        QJsonArray cellsArray = json["Ячейки"].toArray();

        QHash<QString, QStringList> loadingTypes;
        for (int i = 0; i < cellsVec.size(); i++) {
            QJsonObject cellObject = cellsArray[i].toObject();
            cellsVec[i]->read(cellObject);

            loadingTypes[cellsVec[i]->getLoadingType()].append(cellsVec[i]->getLoadingSubType());
        }

        for (auto &list : loadingTypes) {
            list.removeDuplicates();
        }

        emit updateComboLoadingTypes(loadingTypes);
    }

    if (json.contains("Ячейки на боковушке") && json["Ячейки на боковушке"].isArray()) {
        QJsonArray cellsArray = json["Ячейки на боковушке"].toArray();

        for (auto &cell : tableCells) {
            scene->removeItem(cell);
            delete cell;
        }
        tableCells.clear();

        for (auto cell : cellsArray) {
            QJsonObject cellObject = cell.toObject();
            Coin *coin = new Coin;
            coin->setItemMode(Coin::ItemMode::TextEditorRightItem);
            coin->setZValue(3);
            coin->setContextMenu(contexMenuEditorItem);
            coin->read(cellObject);
            scene->addItem(coin);
            tableCells.append(coin);
        }
    }

    if (json.contains("Текст") && json["Текст"].isObject()) {
        QJsonObject textEditorDataObject = json["Текст"].toObject();
        textEditors->read(textEditorDataObject);
    }

    view->setInteractive(true); //hz
}

void CoinEdit::write(QJsonObject &json)
{
    QJsonArray tableCellsArray;
    for (auto coin : qAsConst(tableCells)) {
        QJsonObject cellObject;
        coin->write(cellObject);
        tableCellsArray.append(cellObject);
    }
    json["Ячейки на боковушке"] = tableCellsArray;

    QJsonArray cellsArray;

    for (auto &coin : cellsVec) {
        QJsonObject cellObject;
        coin->write(cellObject);
        cellsArray.append(cellObject);
    }
    json["Ячейки"] = cellsArray;

    QJsonObject textEditorObject;
    textEditors->write(textEditorObject);
    json["Текст"] = textEditorObject;
}

void CoinEdit::render(QPrinter *printer)
{
    QPainter painter(printer);

    QRect sceneRect = scene->itemsBoundingRect().toRect();
    qDebug() << "sceneRect" << sceneRect;

    QRect mappedRect = view->mapFromScene(sceneRect).boundingRect();
    qDebug() << "mappedRect" << mappedRect;

    QRectF target = QRectF(0, 0, printer->width(), printer->height());
    qDebug() << "target" << target;

    // do before printing
    scene->setBackgroundBrush(Qt::white);
    QString cellNum = cellDialog->getCurrentCell()->getCellNum();
    for (auto cell : cellsVec) {
        cell->setPrinting(true);
        if (cell->getCellNum() == cellNum)
            cell->setSimmMainHighlight(false);
    }
    textEditors->hide();
    textEditorLeftPrintItem->show();
    textEditorLeftPrintItem->setHtml(textEditors->getLeftTextHtml());
    textEditorRightPrintItem->show();
    textEditorRightPrintItem->setHtml(textEditors->getRightTextHtml());

    for (auto &cell : simmMap) {
        cell->setSimmHighlight(false);
    }

    view->render(&painter,
                 target,
                 mappedRect.marginsRemoved(QMargins(0, 0, 0, 0)),
                 Qt::KeepAspectRatio);

    // do after printing
    scene->setBackgroundBrush(QBrush(Qt::darkGray));
    for (auto cell : cellsVec) {
        cell->setPrinting(false);
        if (cell->getCellNum() == cellNum)
            cell->setSimmMainHighlight(true);
    }
    textEditors->show();
    textEditorLeftPrintItem->hide();
    textEditorRightPrintItem->hide();
}

void CoinEdit::printPdf()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);

    if (fileName.isEmpty())
        save();
    if (fileName.isEmpty())
        return;

    QString pdfFileName = fileName.fileName().remove(QRegularExpression(".dat$")) + ".pdf";
    //    pdfFileName = fileName.fileName().remove(QRegularExpression(".json$")) + ".pdf";
    printer.setOutputFileName(pdfFileName);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setFullPage(true);

    render(&printer);

    statusBar()->showMessage(tr("Комплектация сохранена в \"%1\"")
                                 .arg(QDir::toNativeSeparators(pdfFileName)),
                             10000);
}

void CoinEdit::printPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setFullPage(true);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &CoinEdit::render);
    preview.exec();
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
    QMessageBox::about(this,
                       "О программе",
                       "<p>Программа <b>CoinEdit 2</b> предназначна для составления"
                       " комплектаций загрузки реактора Р.</p>"
                       "<p>Разработал Тихонов С.</p>"
                       "<p>stikhon88@gmail.com</p>");
}

void CoinEdit::addTableCell(Coin *tableCell, QPoint topLeft)
{
    tableCell->setZValue(3);
    tableCell->setContextMenu(contexMenuEditorItem);
    topLeft.rx() += 2369;
    topLeft.ry() += -2482;
    tableCell->setPos(topLeft);
    tableCells.append(tableCell);
    scene->addItem(tableCell);
}

void CoinEdit::createActions()
{
    // File
    QMenu *fileMenu = menuBar()->addMenu("&Файл");
    QToolBar *fileToolBar = addToolBar("File");

    // Open
    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/icons/open.png"));
    openAct = new QAction(openIcon, "&Открыть", this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip("Открыть комплектацию");
    connect(openAct, &QAction::triggered, this, &CoinEdit::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    // Save
    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/icons/save.png"));
    saveAct = new QAction(saveIcon, "Со&хранить", this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip("Сохранить комплектацию");
    connect(saveAct, &QAction::triggered, this, &CoinEdit::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    // Save as
    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon,
                                             "Сохранить к&ак...",
                                             this,
                                             &CoinEdit::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip("Сохранить комплектацию под новым именем");

    // Print
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/icons/print.png"));
    QAction *printAct = fileMenu->addAction(printIcon, "&Печать...", this, &CoinEdit::printPreview);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip("Распечатать комплектацию");
    fileToolBar->addAction(printAct);

    // Pdf Print
    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(":/icons/exportpdf.png"));
    QAction *exportPdfAct = fileMenu->addAction(exportPdfIcon,
                                                "Печат&ь в PDF",
                                                this,
                                                &CoinEdit::printPdf);
    exportPdfAct->setShortcut(Qt::CTRL | Qt::Key_F);
    exportPdfAct->setStatusTip("Распечатать комплектацию в PDF");
    fileMenu->addAction(exportPdfAct);
    fileToolBar->addAction(exportPdfAct);

    fileMenu->addSeparator();

    // Zoom
    QToolBar *zoomToolBar = addToolBar("Масштаб");
    const QIcon zoomOutIcon = QIcon::fromTheme("document-zoom-out", QIcon(":/icons/zoomout.png"));
    QAction *zoomOutAct = zoomToolBar->addAction(zoomOutIcon, "Отдалить", this, [&] { zoomOut(8); });
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
    QAction *zoomInAct = zoomToolBar->addAction(zoomInIcon, "Приблизить", this, [&] { zoomIn(8); });
    zoomInAct->setShortcuts(QKeySequence::ZoomIn);
    zoomInAct->setStatusTip("Приблизить");
    zoomToolBar->addAction(zoomInAct);

    //Srez
    QMenu *srezMenu = menuBar()->addMenu("Срез");
    QAction *addTArchAct = new QAction("Добавить срез dT");
    srezMenu->addAction(addTArchAct);
    addTArchAct->setShortcut(Qt::CTRL | Qt::Key_W);
    connect(addTArchAct, &QAction::triggered, this, &CoinEdit::addTArch);

    QAction *addLoadingArchAct = new QAction("Добавить срез загрузки");
    srezMenu->addAction(addLoadingArchAct);
    addLoadingArchAct->setShortcut(Qt::CTRL | Qt::Key_E);
    connect(addLoadingArchAct, &QAction::triggered, this, &CoinEdit::addLoadingArch);

    srezMenu->addSeparator();

    showSrezAct = new QAction("Показать срез");
    showSrezAct->setCheckable(true);
    showSrezAct->setChecked(false);
    srezMenu->addAction(showSrezAct);
    connect(showSrezAct, &QAction::toggled, [&] {
        for (auto &cell : cellsVec) {
            showSrezAct->isChecked() ? cell->setShowSrez(true) : cell->setShowSrez(false);
            cell->update();
        }

        if (showSrezAct->isChecked()) {
            dTItem->show();
            loadingItem->show();
        } else {
            dTItem->hide();
            loadingItem->hide();
        }
    });

    // Боковушка
    textEditors->setupEditActions();
    //
    textEditors->setupTextActions();
    //
    textEditors->doOnStartUp();

    // Exit
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, "Выход", this, &QApplication::exit);
    exitAct->setShortcut(QKeySequence("Ctrl+Q"));
    exitAct->setStatusTip("Завершить работу");

    QMenu *helpMenu = menuBar()->addMenu("П&омощь");

    helpMenu->addAction("&О программе", this, &CoinEdit::about);
    helpMenu->addAction("About &Qt", &QApplication::aboutQt);

    // Context menus
    contexMenuEditorItem = new QMenu;
    const QIcon deleteTableIcon = QIcon(":/icons/delete.png");
    contexMenuEditorItem->addAction(deleteTableIcon, "Удалить", this, &CoinEdit::deleteTableItem);

    contexMenuMainItem = new QMenu;
    const QIcon addCelltoDialogIcon = QIcon(":/icons/plus-sign.png");
    contexMenuMainItem->addAction(addCelltoDialogIcon,
                                  "Добавить в список",
                                  this,
                                  &CoinEdit::addCelltoDialog);
    contexMenuMainItem->addAction(showSrezAct);
}
