#pragma once

#include <QtWidgets/QtWidgets>

QT_BEGIN_NAMESPACE
class Coin;
class IconDialog;
class TextEditors;
QT_END_NAMESPACE

class CellDialog : public QDialog
{
    Q_OBJECT
public:
    CellDialog(QWidget *parent = nullptr);

    void setAllCells(const QHash<QString, Coin *> &newAllCells);
    void setRightTextEditor(TextEditors *textEditors);

public slots:
    void updateCellData();
    void updateDialog(Coin *cell);

private slots:
    void addMultipleCellsButtonClicked();
    void changeSubTypes(const QString &type);
    void editorTextChanged();
    void makeDataToRightTextEditor();

signals:
    void toRightTextEditor(Coin *tableCell, QVector<QString> text);
    void updateStats(QHash<QString, size_t> &typeCounter, QHash<QString, size_t> &subTypeCounter);

private:
    void cellData();
    void changeLoading();
    void colors();
    void stats();

    Coin *coin;
    Coin *currentCell;
    IconDialog *iconDialog;
    QCheckBox *visibleCheckBox;
    QColor color;
    QComboBox *comboLoadingSubType;
    QComboBox *comboLoadingType;
    QDialogButtonBox *buttonBox;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGroupBox *changeCellLoading;
    QHash<QString, Coin *> allCells;
    QHash<QString, QStringList> loadingTypes;
    QHash<QString, size_t> subTypeCounter;
    QHash<QString, size_t> typeCounter;
    QLabel *cellLoadingLabel;
    QLabel *cellNumLabel;
    QLabel *statsLabel01;
    QLabel *statsLabel11;
    QPushButton *addMultipleCellsButton;
    QPushButton *addToRightTextEditor;
    QPushButton *colorButton;
    QPushButton *iconButton;
    QPushButton *textButton;
    QStringList changeList;
    QTextEdit *editor;
    QVBoxLayout *mainLayout;
    TextEditors *textEditors;
};

class IconDialog : public QDialog
{
    Q_OBJECT
public:
    IconDialog(QWidget *parent = nullptr);

    void setColor(const QColor &newColor);

    void setText(const QString &newText);

public slots:

private slots:

signals:
    void updateDialog(Coin *icon);

private:
    QDialogButtonBox *buttonBox;
    QGraphicsScene *scene;
    QGraphicsView *view;
    QVBoxLayout *mainLayout;
    QVector<Coin *> iconTypes;
};
