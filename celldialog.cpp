#include "celldialog.h"
#include "coin.h"
#include "texteditor.h"

CellDialog::CellDialog(QWidget *parent)
    : QDialog(parent)
    , currentCell(new Coin)
    , scene(new QGraphicsScene(this))
    , view(new QGraphicsView(this))
    , iconDialog(new IconDialog(this))
{
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);

    cellData();
    changeLoading();
    stats();
    colors();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CellDialog::updateCellData);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox, 0, Qt::AlignHCenter);

    setWindowTitle("Свойства ячейки");

    move(QGuiApplication::primaryScreen()->availableSize().width() * 3 / 5,
         QGuiApplication::primaryScreen()->availableSize().height() * 0.75 / 5);

    connect(iconDialog, &IconDialog::updateDialog, this, &CellDialog::updateDialog);
    connect(this, &CellDialog::updateIconDialog, iconDialog, &IconDialog::updateIconDialog);
}

void CellDialog::cellData()
{
    QGridLayout *cellLayout = new QGridLayout;

    QGroupBox *cellNum = new QGroupBox("Номер ячейки");
    QGridLayout *cellNumLayout = new QGridLayout;
    cellNum->setLayout(cellNumLayout);
    cellNumLabel = new QLabel("Номер ячейки");
    cellNumLayout->addWidget(cellNumLabel, 0, 0);

    addMultipleCellsButton = new QPushButton;
    addMultipleCellsButton->setIcon(QIcon(":/icons/plus-sign.png"));
    addMultipleCellsButton->setFixedSize(20, 20);
    addMultipleCellsButton->setFlat(true);
    cellNumLayout->addWidget(addMultipleCellsButton, 0, 1, Qt::AlignRight);

    editor = new QTextEdit();
    editor->hide();
    editor->setFixedSize(260, 150);
    editor->setFont(QFont("Times New Roman", 14));
    cellNumLayout->addWidget(editor, 1, 0, 1, 2, Qt::AlignCenter);

    connect(addMultipleCellsButton,
            &QPushButton::clicked,
            this,
            &CellDialog::addMultipleCellsButtonClicked);
    connect(editor, &QTextEdit::textChanged, this, &CellDialog::editorTextChanged);

    cellLayout->addWidget(cellNum, 0, 0, 1, 1);

    QGroupBox *cellLoading = new QGroupBox("Загрузка");
    QVBoxLayout *cellLoadingLayout = new QVBoxLayout;
    cellLoading->setLayout(cellLoadingLayout);
    cellLoadingLabel = new QLabel("Загрузка ячейки");
    cellLoadingLayout->addWidget(cellLoadingLabel);

    addToRightTextEditor = new QPushButton("Добавить на боковушку");
    connect(addToRightTextEditor,
            &QPushButton::clicked,
            this,
            &CellDialog::makeDataToRightTextEditor);
    cellLoadingLayout->addWidget(addToRightTextEditor);

    cellLayout->addWidget(cellLoading, 1, 0, 1, 1);

    mainLayout->addLayout(cellLayout);
}

void CellDialog::changeLoading()
{
    changeCellLoading = new QGroupBox("Изменить загрузку");
    QGridLayout *changeCellLoadingLayout = new QGridLayout;
    changeCellLoading->setLayout(changeCellLoadingLayout);

    QLabel *type1 = new QLabel("Тип кассеты");
    changeCellLoadingLayout->addWidget(type1, 0, 0, 1, 1);

    comboLoadingType = new QComboBox;
    comboLoadingType->setEditable(true);
    changeCellLoadingLayout->addWidget(comboLoadingType, 0, 1, 1, 1);

    connect(comboLoadingType, &QComboBox::currentTextChanged, this, &CellDialog::changeSubTypes);

    QLabel *type2 = new QLabel("Тип ЦС");
    changeCellLoadingLayout->addWidget(type2, 1, 0, 1, 1);

    comboLoadingSubType = new QComboBox;
    changeCellLoadingLayout->addWidget(comboLoadingSubType, 1, 1, 1, 1);
    comboLoadingSubType->setEditable(true);
    comboLoadingSubType->setIconSize(QSize(30, 30));
    comboLoadingSubType->setFixedSize(150, 32);
    connect(comboLoadingSubType, &QComboBox::currentTextChanged, [&](QString type) {
        for (auto &cell : allCells) {
            if (cell->getLoadingType() == comboLoadingType->currentText()
                && cell->getLoadingSubType() == type) {
                currentCell->setColor(cell->getColor());
                currentCell->setVisible(cell->getVisible());
                currentCell->setText(cell->getText());
                currentCell->setIconType(cell->getIconType());
                currentCell->update();
                visibleCheckBox->setChecked(cell->getVisible());

                statsLabel01->setText(QString::number(typeCounter[cell->getLoadingType()]));
                statsLabel11->setText(QString::number(
                    subTypeCounter[cell->getLoadingType() + " " + cell->getLoadingSubType()]));
                return;
            }
        }
    });

    mainLayout->addWidget(changeCellLoading);

    updateComboLoadingTypes(loadingTypes);
}

void CellDialog::stats()
{
    QGroupBox *stats = new QGroupBox("Количество");
    QGridLayout *statsLayout = new QGridLayout;
    stats->setLayout(statsLayout);

    QLabel *statsLabel00 = new QLabel("По типу кассеты");
    statsLayout->addWidget(statsLabel00, 0, 0);

    statsLabel01 = new QLabel("Кол-во");
    statsLayout->addWidget(statsLabel01, 0, 1);

    QLabel *statsLabel10 = new QLabel("По типу ЦС");
    statsLayout->addWidget(statsLabel10, 1, 0);

    statsLabel11 = new QLabel("Кол-во");
    statsLayout->addWidget(statsLabel11, 1, 1);

    showStatsButton = new QPushButton;
    showStatsButton->setIcon(QIcon(":/icons/plus-sign.png"));
    showStatsButton->setFixedSize(20, 20);
    showStatsButton->setFlat(true);
    showStatsButton->setToolTip("Показать статистику");
    statsLayout->addWidget(showStatsButton, 1, 2, Qt::AlignRight);

    loadingStats = new QGroupBox("Статистика");
    QGridLayout *loadingStatsLayout = new QGridLayout;
    loadingStats->setLayout(loadingStatsLayout);
    loadingStats->hide();

    connect(showStatsButton, &QPushButton::clicked, [&] {
        if (loadingStats->isHidden()) {
            loadingStats->show();
            showStatsButton->setIcon(QIcon(":/icons/minus-sign.png"));
            showStatsButton->setToolTip("Скрыть");
        } else {
            loadingStats->hide();
            showStatsButton->setIcon(QIcon(":/icons/plus-sign.png"));
            showStatsButton->setToolTip("Показать статистику");
        }
    });
    statsLayout->addWidget(loadingStats, 2, 0, 1, 3);

    mainLayout->addWidget(stats);
}

Coin *CellDialog::getCurrentCell()
{
    return currentCell;
}

void CellDialog::colors()
{
    QGroupBox *col = new QGroupBox("Внешний вид");
    QGridLayout *colLayout = new QGridLayout;
    col->setLayout(colLayout);

    currentCell->setItemMode(Coin::ItemMode::CellDialogItem);
    currentCell->setCellDialog(this);
    scene->addItem(currentCell);

    view->setScene(scene);
    view->setSceneRect(5, 5, 95, 95);
    colLayout->addWidget(view, 0, 0, 4, 1, Qt::AlignLeft);

    colorButton = new QPushButton("Цвет");
    connect(colorButton, &QPushButton::clicked, [&] {
        color = QColorDialog::getColor();
        if (color.isValid())
            currentCell->setColor(color);
    });
    colLayout->addWidget(colorButton, 0, 1);

    iconButton = new QPushButton("Иконка");
    connect(iconButton, &QPushButton::clicked, [&] {
        iconDialog->setColor(color);
        iconDialog->setText(currentCell->getText());
        iconDialog->show();
    });
    colLayout->addWidget(iconButton, 1, 1);

    textButton = new QPushButton("Текст");
    connect(textButton, &QPushButton::clicked, [&] {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             "Введите текст",
                                             "Текст (не более 6 символов): ",
                                             QLineEdit::Normal,
                                             "",
                                             &ok);
        if (ok) {
            currentCell->setText(text);
            currentCell->update();
        }
    });
    colLayout->addWidget(textButton, 2, 1);

    visibleCheckBox = new QCheckBox("Невидимый");
    connect(visibleCheckBox, &QCheckBox::stateChanged, [&] {
        currentCell->setVisible(visibleCheckBox->checkState() & Qt::Checked);
        currentCell->update();
    });
    colLayout->addWidget(visibleCheckBox, 3, 1);

    mainLayout->addWidget(col);
}

void CellDialog::setAllCells(const QHash<QString, Coin *> &newAllCells)
{
    allCells = newAllCells;
}

void CellDialog::setRightTextEditor(TextEditors *textEditors)
{
    this->textEditors = textEditors;
    connect(this, &CellDialog::toRightTextEditor, textEditors, &TextEditors::addTable);
}

void CellDialog::showIconDialog()
{
    //    qDebug() << "showIconDialog";
    iconButton->click();
}

void CellDialog::updateDialog(Coin *cell)
{
    //    qDebug() << "updateDialog";

    if (cell->getItemMode() == Coin::ItemMode::IconDialogItem) {
        currentCell->setIconType(cell->getIconType());
        color = cell->getColor();
        currentCell->setColor(cell->getColor());
        currentCell->update();
        return;
    }

    //
    color = cell->getColor();

    cellNumLabel->setText(cell->getCellNum());
    cellLoadingLabel->setText(cell->getLoadingType() + " " + cell->getLoadingSubType());

    typeCounter.clear();
    subTypeCounter.clear();

    for (auto &cellPtr : allCells) {
        typeCounter[cellPtr->getLoadingType()]++;
        subTypeCounter[cellPtr->getLoadingType() + " " + cellPtr->getLoadingSubType()]++;

        if (cell->getCellNum() != cellPtr->getCellNum()) {
            cellPtr->setSimmMainHighlight(false);
            cellPtr->update();
        }
    }

    statsLabel01->setText(QString::number(typeCounter[cell->getLoadingType()]));
    statsLabel11->setText(
        QString::number(subTypeCounter[cell->getLoadingType() + " " + cell->getLoadingSubType()]));

    visibleCheckBox->setChecked(cell->getVisible());

    currentCell->setColor(cell->getColor());
    currentCell->setCellNum(cell->getCellNum());
    currentCell->setLoadingType(cell->getLoadingType());
    currentCell->setLoadingSubType(cell->getLoadingSubType());
    currentCell->setVisible(cell->getVisible());
    currentCell->setText(cell->getText());
    currentCell->setIconType(cell->getIconType());
    currentCell->update();

    comboLoadingType->setCurrentText(cell->getLoadingType());
    changeSubTypes(cell->getLoadingType());

    emit updateIconDialog(currentCell);
}

void CellDialog::addCellNumToDialog(const QString cellNum)
{
    this->show();
    cellNumLabel->hide();
    if (editor->toPlainText().isEmpty())
        editor->setText(cellNum);
    else
        editor->setText(QString(editor->toPlainText() + ", " + cellNum));
    editor->show();
    addMultipleCellsButton->setIcon(QIcon(":/icons/minus-sign.png"));
    addMultipleCellsButton->setToolTip("Скрыть");
    editorTextChanged();
}

void CellDialog::updateComboLoadingTypes(QHash<QString, QStringList> types)
{
    //    loadingTypes.clear();
    //    loadingTypes["РК"] = QStringList({"64.5",
    //                                      "64.2",
    //                                      "60.2",
    //                                      "55.2",
    //                                      "64.05",
    //                                      "Л2-2",
    //                                      "50.2",
    //                                      "60.05",
    //                                      "55.05",
    //                                      "55.05m",
    //                                      "50.05",
    //                                      "Co",
    //                                      "Л2-5",
    //                                      "50.05m",
    //                                      "Com",
    //                                      "60.0"});
    //    loadingTypes["КС"] = QStringList({"III", "IV"});
    //    loadingTypes["ТВС-П"] = QStringList({"64.5",
    //                                         "64.2",
    //                                         "60.2",
    //                                         "55.2",
    //                                         "64.05",
    //                                         "50.2",
    //                                         "60.05",
    //                                         "55.05",
    //                                         "55.05m",
    //                                         "50.05",
    //                                         "Co",
    //                                         "50.05m",
    //                                         "Com",
    //                                         "60.0"});
    //    loadingTypes["КРЭ"] = QStringList({"Co", "Л2-5"});
    //    loadingTypes["КИРЭ-У"] = QStringList({"Co", "Л2-5"});
    //    loadingTypes["КИС"] = QStringList({"Ir", "Co"});
    //    loadingTypes["Без загрузки"] = QStringList({});

    //    comboLoadingType->insertItems(0, loadingTypes.keys());
    //    comboLoadingSubType->insertItems(0, loadingTypes[comboLoadingType->currentText()]);

    loadingTypes.clear();
    loadingTypes = types;
    comboLoadingType->clear();
    comboLoadingSubType->clear();

    comboIcons.clear();

    QGraphicsView *comboView = new QGraphicsView();
    QGraphicsScene *comboScene = new QGraphicsScene();
    comboView->setScene(comboScene);
    comboView->setSceneRect(5, 5, 95, 95);

    Coin *comboCell = new Coin;
    comboCell->setItemMode(Coin::ItemMode::IconDialogItem);
    comboScene->addItem(comboCell);
    comboScene->setBackgroundBrush(Qt::white);
    //    comboScene->setBackgroundBrush(Qt::black);

    for (auto loadingType = loadingTypes.begin(); loadingType != loadingTypes.end(); loadingType++) {
        for (auto &loadingSubType : loadingType.value()) {
            if (!comboIcons[loadingType.key() + " " + loadingSubType]) {
                for (auto &cell : allCells) {
                    if (cell->getLoadingType() == loadingType.key()
                        && cell->getLoadingSubType() == loadingSubType) {
                        comboCell->setIconType(cell->getIconType());
                        comboCell->setColor(cell->getColor());
                        break;
                    }
                }

                //                qDebug() << loadingType.key() + "+" + loadingSubType;
                QPixmap pixmap(100, 100);
                QPainter painter(&pixmap);
                //                painter.setBrush(Qt::NoBrush);
                comboScene->render(&painter);
                //                pixmap.fill(Qt::black);
                //                pixmap.setMask(pixmap.createMaskFromColor(Qt::black, Qt::MaskOutColor));
                //                pixmap.setMask(pixmap.createHeuristicMask());
                //                QBitmap bitmap = pixmap.mask();
                //                bitmap.save(loadingType.key() + ".bmp");
                //                pixmap.save(loadingType.key() + " " + loadingSubType + ".png");

                QIcon *icon = new QIcon(pixmap);
                comboIcons[loadingType.key() + " " + loadingSubType] = icon;
            }
        }
    }

    comboLoadingType->insertItems(0, loadingTypes.keys());

    delete comboView;
    delete comboScene;
}

void CellDialog::updateCellData()
{
    //    qDebug() << "updateCellData";

    QString loadingType = comboLoadingType->currentText();
    QString loadingSubType = comboLoadingSubType->currentText();

    if (changeList.isEmpty()){
        changeList.append(currentCell->getCellNum());
    }

    for (auto &cell : allCells) {
        if (cell->getLoadingType() == loadingType && cell->getLoadingSubType() == loadingSubType) {
            changeList.append(cell->getCellNum());
        }
    }

    for (auto &cell : changeList) {
        allCells[cell]->setLoadingType(loadingType);
        allCells[cell]->setLoadingSubType(loadingSubType);
        allCells[cell]->setColor(currentCell->getColor());
        allCells[cell]->setVisible(visibleCheckBox->checkState() & Qt::Checked);
        allCells[cell]->setText(currentCell->getText());
        allCells[cell]->setIconType(currentCell->getIconType());
        allCells[cell]->update();
    }

    //
    QHash<QString, QStringList> loadingTypesNew;
    loadingTypesNew[loadingType] << loadingSubType;
    for (auto &cell : allCells) {
        loadingTypesNew[cell->getLoadingType()] << cell->getLoadingSubType();
    }

    for (auto &list : loadingTypesNew) {
        list.removeDuplicates();
    }

    updateComboLoadingTypes(loadingTypesNew);

    //
    updateDialog(allCells[*changeList.begin()]);

    changeList.clear();
}

void CellDialog::changeSubTypes(const QString &type)
{
    comboLoadingSubType->clear();

    for (int i = 0; i < loadingTypes[type].size(); i++) {
        //        qDebug() << loadingTypes[type].size();
        //        qDebug() << type + "+" + loadingTypes[type].at(i);

        if (comboIcons[type + " " + loadingTypes[type].at(i)])
            comboLoadingSubType->insertItem(i,
                                            *comboIcons[type + " " + loadingTypes[type][i]],
                                            loadingTypes[type][i]);

        if (currentCell->getLoadingSubType() == loadingTypes[type].at(i)) {
            //            qDebug() << "setCurrentIndex" << i;
            comboLoadingSubType->setCurrentIndex(i);
        }
    }
}

void CellDialog::editorTextChanged()
{
    //    qDebug() << "editorTextChanged";

    QRegularExpression regEx("([^0-9]|^)(?<cell>\\d\\d-\\d\\d)([^0-9]|$)");
    QString text = editor->toPlainText();

    changeList.clear();
    QRegularExpressionMatchIterator i = regEx.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString word = match.captured("cell");
        if (changeList.contains(word)) {
            cellLoadingLabel->setText("Ошибка: " + word);
            changeList.clear();
            return;
        }
        changeList << word;
    }

    QHash<QString, size_t> typeCounter;

    for (auto &cell : changeList) {
        //        qDebug() << cell;
        if (!allCells[cell]) {
            cellLoadingLabel->setText("Ошибка: " + cell);
            changeList.clear();
            return;
        }
        typeCounter[allCells[cell]->getLoadingType() + " " + allCells[cell]->getLoadingSubType()]++;
    }

    QString statsMessage;
    QHash<QString, size_t>::iterator it = typeCounter.begin();
    size_t total = 0;
    while (it != typeCounter.end()) {
        statsMessage += it.key() + " - " + QString::number(it.value()) + " шт;\n";
        total += it.value();
        ++it;
    }
    statsMessage += "Всего: " + QString::number(total) + " шт.";

    cellLoadingLabel->setText(statsMessage);
}

void CellDialog::addMultipleCellsButtonClicked()
{
    changeList.clear();

    if (editor->isHidden()) {
        cellNumLabel->hide();
        editor->show();
        addMultipleCellsButton->setIcon(QIcon(":/icons/minus-sign.png"));
        addMultipleCellsButton->setToolTip("Скрыть");
        editorTextChanged();
    } else {
        cellNumLabel->show();
        editor->hide();
        editor->clear();
        addMultipleCellsButton->setIcon(QIcon(":/icons/plus-sign.png"));
        addMultipleCellsButton->setToolTip("Выбрать несколько ячеек");
        cellLoadingLabel->setText(currentCell->getLoadingType() + " "
                                  + currentCell->getLoadingSubType());
    }
}

void CellDialog::makeDataToRightTextEditor()
{
    //    qDebug() << "makeDataToRightTextEditor";

    QVector<QString> data;
    for (auto cellPtr : allCells) {
        if (cellPtr->getLoadingType() == currentCell->getLoadingType()
            && cellPtr->getLoadingSubType() == currentCell->getLoadingSubType()) {
            data.append(cellPtr->getCellNum());
        }
    }

    if (data.isEmpty())
        return;

    std::sort(data.begin(), data.end());

    Coin *tableCell = new Coin;
    tableCell->setLoadingType(currentCell->getLoadingType());
    tableCell->setLoadingSubType(currentCell->getLoadingSubType());
    tableCell->setColor(currentCell->getColor());
    tableCell->setText(currentCell->getText());
    tableCell->setIconType(currentCell->getIconType());
    tableCell->setItemMode(Coin::ItemMode::TextEditorRightItem);

    emit toRightTextEditor(tableCell, data);
}

IconDialog::IconDialog(QWidget *parent)
    : QDialog(parent)
{
    //
    mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);

    //
    view = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(350, 270);
    view->setScene(scene);

    mainLayout->addWidget(view, 0, 0, 1, 4, Qt::AlignHCenter);

    //
    int typesSize = 15;
    int xStep = 100;
    int yStep = 100;
    int x = 0;
    int y = 0;
    for (auto i = 0; i < typesSize; i++, x += xStep) {
        if (i % 3 == 0 && i != 0) {
            y += yStep;
            x = 0;
        }
        Coin *coin = new Coin;
        coin->setItemMode(Coin::ItemMode::IconDialogItem);
        coin->setPos(x, y);
        iconTypes.append(coin);
        scene->addItem(coin);
    }
    iconTypes[0]->setIconType(Coin::IconType::Default);
    iconTypes[1]->setIconType(Coin::IconType::Dot);
    iconTypes[2]->setIconType(Coin::IconType::SmallDot);
    iconTypes[3]->setIconType(Coin::IconType::PenSize2blue);
    iconTypes[4]->setIconType(Coin::IconType::PenSize2red);
    iconTypes[5]->setIconType(Coin::IconType::RedCross);
    iconTypes[6]->setIconType(Coin::IconType::Dense6Pattern);
    iconTypes[7]->setIconType(Coin::IconType::Dense7Pattern);
    iconTypes[8]->setIconType(Coin::IconType::HorPattern);
    iconTypes[9]->setIconType(Coin::IconType::VerPattern);
    iconTypes[10]->setIconType(Coin::IconType::CrossPattern);
    iconTypes[11]->setIconType(Coin::IconType::BDiagPattern);
    iconTypes[12]->setIconType(Coin::IconType::FDiagPattern);
    iconTypes[13]->setIconType(Coin::IconType::DiagCrossPattern);
    iconTypes[14]->setIconType(Coin::IconType::z23);

    color1 = iconTypes[0]->getColor();

    //
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [&] {
        for (auto icon : iconTypes) {
            if (icon->isSelected()) {
                emit updateDialog(icon);
                accept();
            }
        }
        reject();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox, 4, 1, 1, 2, Qt::AlignHCenter);

    setWindowTitle("Выбор иконки");
}

void IconDialog::setColor(const QColor &newColor)
{
    color1 = newColor;
    for (auto coin : iconTypes) {
        coin->setColor(color1);
        coin->update();
    }
}

void IconDialog::setText(const QString &newText)
{
    for (auto coin : iconTypes) {
        coin->setText(newText);
        coin->update();
    }
}

void IconDialog::updateIconDialog(Coin *cell)
{
    //    qDebug() << "updateIconDialog";

    for (auto coin : iconTypes) {
        coin->setSelected(false);
    }

    setColor(cell->getColor());
    setText(cell->getText());

    for (auto coin : iconTypes) {
        if (cell->getIconType() == coin->getIconType()) {
            coin->setSelected(true);
            return;
        }
    }
}
