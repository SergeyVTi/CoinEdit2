
#include "celldialog.h"
#include "coin.h"

CellDialog::CellDialog(QWidget *parent) : QDialog(parent)
{
//    gridLayout = new QGridLayout;
    vLayout = new QVBoxLayout;
    setLayout(vLayout);

    cellData();
    changeLoading();
    stats();
    colors();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);

//    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CellDialog::updateCellData);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    vLayout->addWidget(buttonBox,0,Qt::AlignHCenter);

    setWindowTitle("Свойства ячейки");

    move(QGuiApplication::primaryScreen()->availableSize().width()*3/5,
         QGuiApplication::primaryScreen()->availableSize().height()*2/5);

}

void CellDialog::cellData()
{
    QHBoxLayout *cellLayout = new QHBoxLayout;

    QGroupBox *cellNum = new QGroupBox("Номер ячейки");
    QHBoxLayout *cellNumLayout = new QHBoxLayout;
    cellNum->setLayout(cellNumLayout);
    cellNumLabel = new QLabel("Номер ячейки");
    cellNumLayout->addWidget(cellNumLabel);

    cellLayout->addWidget(cellNum);

    QGroupBox *cellLoading = new QGroupBox("Загрузка");
    QHBoxLayout *cellLoadingLayout = new QHBoxLayout;
    cellLoading->setLayout(cellLoadingLayout);
    cellLoadingLabel = new QLabel("Загрузка ячейки");
    cellLoadingLayout->addWidget(cellLoadingLabel);

    cellLayout->addWidget(cellLoading);

    vLayout->addLayout(cellLayout);
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
    QHBoxLayout *changeCellLoadingLayout = new QHBoxLayout;
    changeCellLoading->setLayout(changeCellLoadingLayout);

    QLabel *type1 = new QLabel("Тип кассеты");
    changeCellLoadingLayout->addWidget(type1);

    comboLoadingType = new QComboBox;
//    comboLoadingType->insertItems(0,loadingTypes_);
    comboLoadingType->insertItems(0, loadingTypes_.keys());
    changeCellLoadingLayout->addWidget(comboLoadingType);

    connect(comboLoadingType, &QComboBox::currentTextChanged, this, &CellDialog::changeSubTypes);

    QLabel *type2 = new QLabel("Тип ЦС");
    changeCellLoadingLayout->addWidget(type2);

    comboLoadingSubType = new QComboBox;
    changeCellLoadingLayout->addWidget(comboLoadingSubType);
    comboLoadingSubType->insertItems(0, loadingTypes_[comboLoadingType->currentText()]);

    vLayout->addWidget(changeCellLoading);

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

    vLayout->addWidget(stats);
}

void CellDialog::colors()
{
    QGroupBox *col = new QGroupBox("Внешний вид");
    QGridLayout *colLayout = new QGridLayout;
    col->setLayout(colLayout);

    QLabel *colLabel00 = new QLabel("Вид");
    colLayout->addWidget(colLabel00,0,0);

    vLayout->addWidget(col);
}

void CellDialog::updateDialog(Coin *cell)
{
    currentCell_ = cell;

    cellNumLabel->setText(cell->cellNum());
    cellLoadingLabel->setText(cell->loadingSubType());

    statsLabel01->setText(QString::number(typeCounter_[cell->loadingType()]));
    statsLabel11->setText(QString::number(subTypeCounter_[cell->loadingSubType()]));
}

void CellDialog::loadingTypeChanged(const QString &newLoadingType, const QString &oldLoadingType)
{
    typeCounter_[newLoadingType]++;

    if (!oldLoadingType.isEmpty())
        typeCounter_[oldLoadingType]--;
}

void CellDialog::loadingSubTypeChanged(const QString &newLoadingSubType, const QString &oldLoadingSubType)
{
    subTypeCounter_[newLoadingSubType]++;

    if (!oldLoadingSubType.isEmpty())
        subTypeCounter_[oldLoadingSubType]--;
}

void CellDialog::updateCellData()
{
    QString loadingType = comboLoadingType->currentText();
    QString loadingSubType = comboLoadingType->currentText() + " " + comboLoadingSubType->currentText();

    currentCell_->setLoadingType(loadingType);
    currentCell_->setLoadingSubType(loadingSubType);

    updateDialog(currentCell_);
}

void CellDialog::changeSubTypes(const QString &type)
{
    comboLoadingSubType->clear();

    comboLoadingSubType->insertItems(0, loadingTypes_[type]);
}

