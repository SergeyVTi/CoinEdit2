#pragma once

#include <QDialog>
#include <QRegularExpression>

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
class QGraphicsView;
class QGraphicsScene;
QT_END_NAMESPACE

class CellDialog : public QDialog
{
    Q_OBJECT
public:
    CellDialog(QWidget *parent = nullptr);

    void setAllCells(const QHash<QString, Coin *> &newAllCells);

public slots:
    void updateDialog(Coin *cell);
    void updateCellData();
private slots:
    void changeSubTypes(const QString &type);
    void editorTextChanged();
    void addMultipleCellsButtonClicked();

signals:
    void updateStats(QHash<QString, size_t> &typeCounter, QHash<QString, size_t> &subTypeCounter);

private:
    void cellData();
    void changeLoading();
    void stats();
    void colors();

    QVBoxLayout *mainLayout;
    QGroupBox *changeCellLoading;
    QLabel *cellNumLabel;
    QLabel *cellLoadingLabel;
    QLabel *statsLabel01;
    QLabel *statsLabel11;
    QComboBox *comboLoadingType;
    QComboBox *comboLoadingSubType;
    QDialogButtonBox *buttonBox;
    QPushButton *addMultipleCellsButton;
    QTextEdit *editor;
    QGraphicsView *view;
    QGraphicsScene *scene;
    Coin *coin;
    QPushButton *colorButton;
    QColor color;

    QHash<QString, size_t> typeCounter_;
    QHash<QString, size_t> subTypeCounter_;
    QHash<QString, QStringList> loadingTypes_;
    QHash<QString, Coin*> allCells_;
    QStringList changeList;
    Coin *currentCell_;
};

