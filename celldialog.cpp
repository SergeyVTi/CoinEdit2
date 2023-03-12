
#include "celldialog.h"
#include "coin.h"

CellDialog::CellDialog(QWidget *parent) : QDialog(parent), view(new QGraphicsView(this)),
                                scene(new QGraphicsScene(this)), coin(new Coin)
{
//    gridLayout = new QGridLayout;
    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);

    cellData();
    changeLoading();
    stats();
    colors();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);

//    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CellDialog::updateCellData);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox,0,Qt::AlignHCenter);

    setWindowTitle("Свойства ячейки");

    move(QGuiApplication::primaryScreen()->availableSize().width()*3/5,
         QGuiApplication::primaryScreen()->availableSize().height()*2/5);

}

void CellDialog::cellData()
{
    QGridLayout *cellLayout = new QGridLayout;

    QGroupBox *cellNum = new QGroupBox("Номер ячейки");
    QGridLayout *cellNumLayout = new QGridLayout;
    cellNum->setLayout(cellNumLayout);
    cellNumLabel = new QLabel("Номер ячейки");
    cellNumLayout->addWidget(cellNumLabel,0,0);

    addMultipleCellsButton = new QPushButton("+");
    addMultipleCellsButton->setFixedSize(20,20);
    cellNumLayout->addWidget(addMultipleCellsButton,0,1,Qt::AlignRight);

    editor = new QTextEdit();
    editor->hide();
    cellNumLayout->addWidget(editor,1,0,1,2);

    connect(addMultipleCellsButton, &QPushButton::clicked, this, &CellDialog::addMultipleCellsButtonClicked);
    connect(editor, &QTextEdit::textChanged, this, &CellDialog::editorTextChanged);

    cellLayout->addWidget(cellNum,0,0,1,1);

    QGroupBox *cellLoading = new QGroupBox("Загрузка");
    QHBoxLayout *cellLoadingLayout = new QHBoxLayout;
    cellLoading->setLayout(cellLoadingLayout);
    cellLoadingLabel = new QLabel("Загрузка ячейки");
    cellLoadingLayout->addWidget(cellLoadingLabel);

    cellLayout->addWidget(cellLoading,1,0,1,1);

    mainLayout->addLayout(cellLayout);
}

void CellDialog::changeLoading()
{
    loadingTypes_["РК"] = QStringList({"64.2", "60.02", "55.05", "50.2"});
    loadingTypes_["КС"] = QStringList({"III", "IV"});
    loadingTypes_["ТВС-П"] = QStringList({"64.2", "60.02", "55.05", "50.2"});
    loadingTypes_["КРЭ"] = QStringList({"Co", "Л2-5"});
    loadingTypes_["КИРЭ-У"] = QStringList({"Co", "Л2-5"});
    loadingTypes_["КИС"] = QStringList({"Ir", "Co"});
    loadingTypes_["Без загрузки"] = QStringList({});

    changeCellLoading = new QGroupBox("Изменить загрузку");
//    QHBoxLayout *changeCellLoadingLayout = new QHBoxLayout;
    QGridLayout *changeCellLoadingLayout = new QGridLayout;
    changeCellLoading->setLayout(changeCellLoadingLayout);

    QLabel *type1 = new QLabel("Тип кассеты");
    changeCellLoadingLayout->addWidget(type1,0,0,1,1);

    comboLoadingType = new QComboBox;
//    comboLoadingType->insertItems(0,loadingTypes_);
    comboLoadingType->insertItems(0, loadingTypes_.keys());
    changeCellLoadingLayout->addWidget(comboLoadingType,0,1,1,1);

    connect(comboLoadingType, &QComboBox::currentTextChanged, this, &CellDialog::changeSubTypes);

    QLabel *type2 = new QLabel("Тип ЦС");
    changeCellLoadingLayout->addWidget(type2,1,0,1,1);

    comboLoadingSubType = new QComboBox;
    changeCellLoadingLayout->addWidget(comboLoadingSubType,1,1,1,1);
    comboLoadingSubType->insertItems(0, loadingTypes_[comboLoadingType->currentText()]);

    mainLayout->addWidget(changeCellLoading);

}

void CellDialog::stats()
{
    QGroupBox *stats = new QGroupBox("Статистика");
    QGridLayout *statsLayout = new QGridLayout;
    stats->setLayout(statsLayout);

    QLabel *statsLabel00 = new QLabel("По типу кассеты");
    statsLayout->addWidget(statsLabel00,0,0);

    statsLabel01 = new QLabel("Кол-во");
    statsLayout->addWidget(statsLabel01,0,1);

    QLabel *statsLabel10 = new QLabel("По типу ЦС");
    statsLayout->addWidget(statsLabel10,1,0);

    statsLabel11 = new QLabel("Кол-во");
    statsLayout->addWidget(statsLabel11,1,1);

    mainLayout->addWidget(stats);
}

void CellDialog::colors()
{
    QGroupBox *col = new QGroupBox("Внешний вид");
    QGridLayout *colLayout = new QGridLayout;
    col->setLayout(colLayout);

//    GraphicsScene *scene = new QGraphicsScene(this);
    colLayout->addWidget(view,0,0);
//    if(currentCell_){
//        Coin *coin = currentCell_;
//        coin->setPos(0,0);
//    }

    coin->setRadius(40);
    coin->setStep(0);
    coin->setPos(0, 0);
    coin->setFlag(QGraphicsItem::ItemIsSelectable, false);
    scene->addItem(coin);

//    scene->setSceneRect(0,0,90,100);

    view->setScene(scene);
    view->setSceneRect(0,0,95,95);
//    view->centerOn(coin);

//    QLabel *colLabel00 = new QLabel("Вид");
//    colLayout->addWidget(colLabel00,0,0);
    colorButton = new QPushButton("Цвет");
    connect(colorButton, &QPushButton::clicked, [&]{
        color = QColorDialog::getColor();
        if (color.isValid())
            coin->setColor(color);
    });

    colLayout->addWidget(colorButton,0,1);

    mainLayout->addWidget(col);
}

void CellDialog::setAllCells(const QHash<QString, Coin *> &newAllCells)
{
    allCells_ = newAllCells;
}

void CellDialog::updateDialog(Coin *cell)
{
    qDebug() << "updateDialog";

    currentCell_ = cell;
    color = cell->color();

    cellNumLabel->setText(cell->cellNum());
    cellLoadingLabel->setText(cell->loadingSubType());

    emit updateStats(typeCounter_, subTypeCounter_);

    statsLabel01->setText(QString::number(typeCounter_[cell->loadingType()]));
    statsLabel11->setText(QString::number(subTypeCounter_[cell->loadingSubType()]));

    coin->setColor(cell->color());
    coin->setCellNum(cell->cellNum());
    coin->update();
}

void CellDialog::updateCellData()
{
    qDebug() << "updateCellData";

    QString loadingType = comboLoadingType->currentText();
    QString loadingSubType = comboLoadingType->currentText() + " " + comboLoadingSubType->currentText();

    if (changeList.isEmpty())
        changeList.append(currentCell_->cellNum());

    for(auto& cell : changeList){
       allCells_[cell]->setLoadingType(loadingType);
       allCells_[cell]->setLoadingSubType(loadingSubType);
       allCells_[cell]->setColor(color);
       allCells_[cell]->update();
    }

    updateDialog(allCells_[*changeList.begin()]);
}

void CellDialog::changeSubTypes(const QString &type)
{
    comboLoadingSubType->clear();

    comboLoadingSubType->insertItems(0, loadingTypes_[type]);
}

void CellDialog::editorTextChanged()
{//11-13, 13-43, 55-55, 13-42 123-23, 231423-123123, 9-44, 44-44, 23-123
    //19-34, 19-36, 19-38
    qDebug() << "editorTextChanged";

    QRegularExpression regEx("([^0-9]|^)(?<cell>\\d\\d-\\d\\d)([^0-9]|$)");
    QString text = editor->toPlainText();

    changeList.clear();
    QRegularExpressionMatchIterator i = regEx.globalMatch(text);
    while (i.hasNext()) {
         QRegularExpressionMatch match = i.next();
         QString word = match.captured("cell");
         if (changeList.contains(word)){
             cellLoadingLabel->setText("Ошибка: " + word);
             changeList.clear();
             return;
         }
         changeList << word;
     }

    QHash<QString, size_t> typeCounter;

    for(auto& cell : changeList){
        qDebug() << cell;
        if (!allCells_[cell]){
            cellLoadingLabel->setText("Ошибка: " + cell);
            changeList.clear();
            return;
        }
        typeCounter[allCells_[cell]->loadingSubType()]++;
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

    if (editor->isHidden()){
        cellNumLabel->hide();
        editor->show();
        addMultipleCellsButton->setText("-");
        addMultipleCellsButton->setToolTip("Скрыть");
        editorTextChanged();
    }else{
        cellNumLabel->show();
        editor->hide();
        addMultipleCellsButton->setText("+");
        addMultipleCellsButton->setToolTip("Выбрать несколько ячеек");
    }
}

