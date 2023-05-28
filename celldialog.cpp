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
         QGuiApplication::primaryScreen()->availableSize().height() * 2 / 5);

    connect(iconDialog, &IconDialog::updateDialog, this, &CellDialog::updateDialog);
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
    editor->setFixedSize(185, 150);
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
    loadingTypes["РК"] = QStringList({"64.5",
                                      "64.2",
                                      "60.2",
                                      "55.2",
                                      "64.05",
                                      "Л2-2",
                                      "50.2",
                                      "60.05",
                                      "55.05",
                                      "55.05m",
                                      "50.05",
                                      "Co",
                                      "Л2-5",
                                      "50.05m",
                                      "Com",
                                      "60.0"});
    loadingTypes["КС"] = QStringList({"III", "IV"});
    loadingTypes["ТВС-П"] = QStringList({"64.5",
                                         "64.2",
                                         "60.2",
                                         "55.2",
                                         "64.05",
                                         "50.2",
                                         "60.05",
                                         "55.05",
                                         "55.05m",
                                         "50.05",
                                         "Co",
                                         "50.05m",
                                         "Com",
                                         "60.0"});
    loadingTypes["КРЭ"] = QStringList({"Co", "Л2-5"});
    loadingTypes["КИРЭ-У"] = QStringList({"Co", "Л2-5"});
    loadingTypes["КИС"] = QStringList({"Ir", "Co"});
    loadingTypes["Без загрузки"] = QStringList({});

    changeCellLoading = new QGroupBox("Изменить загрузку");
    QGridLayout *changeCellLoadingLayout = new QGridLayout;
    changeCellLoading->setLayout(changeCellLoadingLayout);

    QLabel *type1 = new QLabel("Тип кассеты");
    changeCellLoadingLayout->addWidget(type1, 0, 0, 1, 1);

    comboLoadingType = new QComboBox;
    comboLoadingType->insertItems(0, loadingTypes.keys());
    comboLoadingType->setEditable(true);
    changeCellLoadingLayout->addWidget(comboLoadingType, 0, 1, 1, 1);

    connect(comboLoadingType, &QComboBox::currentTextChanged, this, &CellDialog::changeSubTypes);

    QLabel *type2 = new QLabel("Тип ЦС");
    changeCellLoadingLayout->addWidget(type2, 1, 0, 1, 1);

    comboLoadingSubType = new QComboBox;
    changeCellLoadingLayout->addWidget(comboLoadingSubType, 1, 1, 1, 1);
    comboLoadingSubType->insertItems(0, loadingTypes[comboLoadingType->currentText()]);
    comboLoadingSubType->setEditable(true);

    mainLayout->addWidget(changeCellLoading);
}

void CellDialog::stats()
{
    QGroupBox *stats = new QGroupBox("Статистика");
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

    mainLayout->addWidget(stats);
}

void CellDialog::colors()
{
    QGroupBox *col = new QGroupBox("Внешний вид");
    QGridLayout *colLayout = new QGridLayout;
    col->setLayout(colLayout);

    currentCell->setItemMode(Coin::ItemMode::CellDialogItem);
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

void CellDialog::updateDialog(Coin *cell)
{
    qDebug() << "updateDialog";

    if (cell->getItemMode() == Coin::ItemMode::IconDialogItem) {
        currentCell->setIconType(cell->getIconType());
        //        currentCell->setColor(cell->getColor());
        currentCell->update();
        return;
    }

    color = cell->getColor();

    cellNumLabel->setText(cell->getCellNum());
    cellLoadingLabel->setText(cell->getLoadingType() + " " + cell->getLoadingSubType());

    typeCounter.clear();
    subTypeCounter.clear();

    for (auto cell : allCells) {
        typeCounter[cell->getLoadingType()]++;
        subTypeCounter[cell->getLoadingType() + " " + cell->getLoadingSubType()]++;
    }

    statsLabel01->setText(QString::number(typeCounter[cell->getLoadingType()]));
    statsLabel11->setText(
        QString::number(subTypeCounter[cell->getLoadingType() + " " + cell->getLoadingSubType()]));

    comboLoadingType->setCurrentText(cell->getLoadingType());
    comboLoadingSubType->setCurrentText(cell->getLoadingSubType());

    visibleCheckBox->setChecked(cell->getVisible());

    currentCell->setColor(cell->getColor());
    currentCell->setCellNum(cell->getCellNum());
    currentCell->setLoadingType(cell->getLoadingType());
    currentCell->setLoadingSubType(cell->getLoadingSubType());
    currentCell->setVisible(cell->getVisible());
    currentCell->setText(cell->getText());
    currentCell->setIconType(cell->getIconType());
    currentCell->update();
}

void CellDialog::updateCellData()
{
    qDebug() << "updateCellData";

    QString loadingType = comboLoadingType->currentText();
    QString loadingSubType = comboLoadingSubType->currentText();

    if (changeList.isEmpty()){
        changeList.append(currentCell->getCellNum());

        for (auto cell : allCells) {
            if (cell->getLoadingType() == loadingType && cell->getLoadingSubType() == loadingSubType)
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

    updateDialog(allCells[*changeList.begin()]);

    changeList.clear();
}

void CellDialog::changeSubTypes(const QString &type)
{
    comboLoadingSubType->clear();
    comboLoadingSubType->insertItems(0, loadingTypes[type]);
}

void CellDialog::editorTextChanged()
{
    qDebug() << "editorTextChanged";

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
        qDebug() << cell;
        if (!allCells[cell]) {
            cellLoadingLabel->setText("Ошибка: " + cell);
            changeList.clear();
            return;
        }
        typeCounter[allCells[cell]->getLoadingSubType()]++;
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
    qDebug() << "makeDataToRightTextEditor";

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
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);

    //
    view = new QGraphicsView(this);
    scene = new QGraphicsScene(this);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(350, 270);
    view->setScene(scene);

    mainLayout->addWidget(view, 0, Qt::AlignHCenter);

    //
    int typesSize = 9;
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
    iconTypes[1]->setIconType(Coin::IconType::Exp);
    iconTypes[2]->setIconType(Coin::IconType::Dot);
    iconTypes[3]->setIconType(Coin::IconType::Default);
    iconTypes[4]->setIconType(Coin::IconType::Default);
    iconTypes[5]->setIconType(Coin::IconType::Default);
    iconTypes[6]->setIconType(Coin::IconType::Default);
    iconTypes[7]->setIconType(Coin::IconType::Default);
    iconTypes[8]->setIconType(Coin::IconType::Default);

    //
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [&] {
        for (auto icon : iconTypes) {
            if (icon->isSelected()) {
                emit updateDialog(icon);
                accept();
            }
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox, 0, Qt::AlignHCenter);

    setWindowTitle("Выбор иконки");
}

void IconDialog::setColor(const QColor &newColor)
{
    for (auto coin : iconTypes) {
        coin->setColor(newColor);
    }
}

void IconDialog::setText(const QString &newText)
{
    for (auto coin : iconTypes) {
        coin->setText(newText);
    }
}
