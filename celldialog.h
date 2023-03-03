#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QComboBox;
class Coin;
QT_END_NAMESPACE

class CellDialog : public QDialog
{
    Q_OBJECT
public:
    CellDialog(QWidget *parent = nullptr);

public slots:
    void updateDialog(Coin *cell);
    void loadingTypeChanged(const QString &newLoadingType, const QString &oldLoadingType = "");
    void loadingSubTypeChanged(const QString &newLoadingSubType, const QString &oldLoadingSubType = "");
    void updateCellData();
//private slots:
    void changeSubTypes(const QString &type);

private:
    void cellData();
    void changeLoading();
    void stats();
    void colors();

    QVBoxLayout *vLayout;
    QGroupBox *changeCellLoading;
    QLabel *cellNumLabel;
    QLabel *cellLoadingLabel;
    QLabel *statsLabel01;
    QLabel *statsLabel11;
    QComboBox *comboLoadingType;
    QComboBox *comboLoadingSubType;
    QDialogButtonBox *buttonBox;

    QHash<QString, size_t> typeCounter_;
    QHash<QString, size_t> subTypeCounter_;
    QHash<QString, QStringList> loadingTypes_;
    Coin *currentCell_;
};

