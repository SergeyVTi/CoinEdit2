#pragma once

#include <QGraphicsProxyWidget>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
class GraphicsView;
class QTextEdit;
class CoinEdit;
class QComboBox;
class QFontComboBox;
QT_END_NAMESPACE

class TextEditor : public QTextEdit
{
Q_OBJECT
public:
    TextEditor(CoinEdit *coinEdit);

    void setProxy(QGraphicsProxyWidget *newProxy);

    void setView(GraphicsView *newView);
    void setupEditActions();
    void setupTextActions();

    void doOnStartUp();
    void read(const QJsonObject &json);
    void write(QJsonObject &json);

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void modifyIndentation(int amount);
    void colorChanged(const QColor &c);
    void fontChanged(const QFont &f);
    void alignmentChanged(Qt::Alignment a);

public slots:

protected:
//    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void filePrintPdf();
    void textBold();
    void textItalic();
    void textUnderline();

    void indent();
    void unindent();
    void textAlign(QAction *a);
    void textColor();
    void textStyle(int styleIndex);
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void clipboardDataChanged();
private:
    GraphicsView *view;
    CoinEdit *coinEdit;
    //    QGraphicsProxyWidget* proxy;

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionIndentLess;
    QAction *actionIndentMore;
    QAction *actionToggleCheckState;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionExportPdfFromEditor;

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

};

