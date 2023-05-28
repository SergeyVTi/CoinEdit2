#pragma once

#include <QtWidgets/QtWidgets>
#include "coin.h"

QT_BEGIN_NAMESPACE
class GraphicsView;
class CoinEdit;
class Coin;
QT_END_NAMESPACE

class TextEditors : public QWidget
{
    Q_OBJECT
public:
    TextEditors(CoinEdit *coinEdit);

    QString getLeftTextHtml();
    QString getRightTextHtml();
    void doOnStartUp();
    void read(const QJsonObject &json);
    //    void setDirPath(const QString &newDirPath);
    void setProxy(QGraphicsProxyWidget *newProxy);
    void setupEditActions();
    void setupTextActions();
    void write(QJsonObject &json);

signals:
    //    void needPath();
    void addTableCell(Coin *tableCell, QPoint topLeft);

private:
    //    void fontChanged(const QFont &f = QFont("Times New Roman", 14));
    void alignmentChanged(Qt::Alignment a);
    void colorChanged(const QColor &c);
    void fontChanged(const QFont &f);
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void modifyIndentation(int amount);

public slots:
    void addTable(Coin *tableCell, QVector<QString>);

protected:
    //    void mousePressEvent(QMouseEvent *event) override;
    //    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void clipboardDataChanged();
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void filePrintPdf();
    void indent();
    void textAlign(QAction *a);
    void textBold();
    void textColor();
    void textFamily(const QString &f);
    void textItalic();
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textUnderline();
    void unindent();

private:
    CoinEdit *coinEdit;
    QAction *actionAlignCenter;
    QAction *actionAlignJustify;
    QAction *actionAlignLeft;
    QAction *actionAlignRight;
    QAction *actionCopy;
    QAction *actionCut;
    QAction *actionExportPdfFromEditor;
    QAction *actionIndentLess;
    QAction *actionIndentMore;
    QAction *actionPaste;
    QAction *actionRedo;
    //    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextColor;
    QAction *actionTextItalic;
    QAction *actionTextUnderline;
    QAction *actionToggleCheckState;
    QAction *actionUndo;
    QComboBox *comboSize;
    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    //    QString dirPath;
    QString placeholder;
    QTextCursor currentCursor;
    QTextEdit *currentEditor;
    QTextEdit *textEditorLeft;
    QTextEdit *textEditorRight;
    //    QVector<QImage> imgVector;
};
