#include "texteditor.h"

#include <QTextEdit>
#include <QDebug>
#include <QStyleOptionFrame>
#include <QMenu>
#include "graphics.h"
#include "coinedit.h"
#include <QToolBar>
#include <QMenuBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextList>
#include <QColorDialog>
#include <QColor>
#include <QComboBox>
#include <QFontComboBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollBar>

TextEditor::TextEditor(CoinEdit *coinEdit) : coinEdit(coinEdit)
{
    setGeometry(QRect(2374, -2347, 807, 2292));
//    setContentsMargins(0, 0, 500, 500);
//    setMaximumSize(300,300);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QStyleOptionFrame::None);


}

void TextEditor::doOnStartUp()
{
    connect(this, &QTextEdit::currentCharFormatChanged,
            this, &TextEditor::currentCharFormatChanged);
    connect(this, &QTextEdit::cursorPositionChanged,
            this, &TextEditor::cursorPositionChanged);

    QFont textFont("Times");
    textFont.setStyleHint(QFont::SansSerif);
    QTextEdit::setFont(textFont);
    fontChanged(QTextEdit::font());
    colorChanged(QTextEdit::textColor());
    alignmentChanged(this->alignment());

//        connect(this->document(), &QTextDocument::modificationChanged,
//                actionSave, &QAction::setEnabled);
        connect(document(), &QTextDocument::modificationChanged,
                this, &QWidget::setWindowModified);
        connect(document(), &QTextDocument::undoAvailable,
                actionUndo, &QAction::setEnabled);
        connect(document(), &QTextDocument::redoAvailable,
                actionRedo, &QAction::setEnabled);

        setWindowModified(this->document()->isModified());
    //    actionSave->setEnabled(textEdit->document()->isModified());
        actionUndo->setEnabled(this->document()->isUndoAvailable());
        actionRedo->setEnabled(this->document()->isRedoAvailable());

        actionCut->setEnabled(false);
        connect(this, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
        actionCopy->setEnabled(false);
        connect(this, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

        connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEditor::clipboardDataChanged);

    //    textEdit->setFocus();
        //    setCurrentFileName(QString());
}

void TextEditor::read(const QJsonObject &json)
{
    if (json.contains("Текст") && json["Текст"].isString()){
        QJsonValue textEditorData = json["Текст"].toString();
        QTextDocument *textDoc = new QTextDocument();
//        qDebug()<<textEditorData.toString();
        textDoc->setHtml(textEditorData.toString());
        setDocument(textDoc);
    }
}

void TextEditor::write(QJsonObject &json)
{
    QJsonValue textEditorData = {
        this->document()->toHtml()
    };
    json["Текст"] = textEditorData;
}

//void TextEditor::mouseMoveEvent(QMouseEvent *event)
//{
//    //   qDebug() << "Event";
//    view->setInteractive(true);
//}

void TextEditor::clipboardDataChanged()
{
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
}

void TextEditor::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void TextEditor::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

void TextEditor::setupEditActions()
{
    //TextEditor
    QToolBar *editToolBar = coinEdit->addToolBar("Edit Actions");
//    coinEdit->addToolBar(Qt::RightToolBarArea, editToolBar);
    QMenu *editMenu = coinEdit->menuBar()->addMenu("&Правка");

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/icons/editundo.png"));
    actionUndo = editMenu->addAction(undoIcon, "&Отменить", this, &TextEditor::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    editToolBar->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(":/icons/editredo.png"));
    actionRedo = editMenu->addAction(redoIcon, "&Вернуть", this, &TextEditor::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    editToolBar->addAction(actionRedo);

    editMenu->addSeparator();

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/icons/editcut.png"));
    actionCut = editMenu->addAction(cutIcon, "В&ырезать", this, &TextEditor::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    editToolBar->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/icons/editcopy.png"));
    actionCopy = editMenu->addAction(copyIcon, "&Копировать", this, &TextEditor::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    editToolBar->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/icons/editpaste.png"));
    actionPaste = editMenu->addAction(pasteIcon, "В&ставить", this, &TextEditor::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    editToolBar->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());

    editMenu->addSeparator();

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(":/icons/exportpdf.png"));
    actionExportPdfFromEditor = editMenu->addAction(exportPdfIcon, "&Сохранить боковушку в PDF...", this, &TextEditor::filePrintPdf);
    actionExportPdfFromEditor->setPriority(QAction::LowPriority);
    actionExportPdfFromEditor->setShortcut(Qt::CTRL + Qt::Key_D);
    editToolBar->addAction(actionExportPdfFromEditor);
}

void TextEditor::setupTextActions()
{
    QToolBar *formattingToolBar = coinEdit->addToolBar("Format Actions");
    QMenu *formattingMenu = coinEdit->menuBar()->addMenu("Формат");

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(":/icons/textbold.png"));
    actionTextBold = formattingMenu->addAction(boldIcon, "&Жирный", this, &TextEditor::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    formattingToolBar->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(":/icons/textitalic.png"));
    actionTextItalic = formattingMenu->addAction(italicIcon, "&Курсив", this, &TextEditor::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    formattingToolBar->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(":/icons/textunder.png"));
    actionTextUnderline = formattingMenu->addAction(underlineIcon, "&Подчёркнутый", this, &TextEditor::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    formattingToolBar->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    formattingMenu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(":/icons/textleft.png"));
    actionAlignLeft = new QAction(leftIcon, "&Лево", this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(":/icons/textcenter.png"));
    actionAlignCenter = new QAction(centerIcon, "&Центр", this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(":/icons/textright.png"));
    actionAlignRight = new QAction(rightIcon, "&Право", this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(":/icons/textjustify.png"));
    actionAlignJustify = new QAction(fillIcon, "По &ширене", this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);
    const QIcon indentMoreIcon = QIcon::fromTheme("format-indent-more", QIcon(":/icons/format-indent-more.png"));
    actionIndentMore = formattingToolBar->addAction(indentMoreIcon, "Отступ&+", this, &TextEditor::indent);
    actionIndentMore->setShortcut(Qt::CTRL + Qt::Key_BracketRight);
    actionIndentMore->setPriority(QAction::LowPriority);
    const QIcon indentLessIcon = QIcon::fromTheme("format-indent-less", QIcon(":/icons/format-indent-less.png"));
    actionIndentLess = formattingToolBar->addAction(indentLessIcon, "Отступ&-", this, &TextEditor::unindent);
    actionIndentLess->setShortcut(Qt::CTRL + Qt::Key_BracketLeft);
    actionIndentLess->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &TextEditor::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    formattingToolBar->addActions(alignGroup->actions());
    formattingMenu->addActions(alignGroup->actions());
    formattingToolBar->addAction(actionIndentMore);
    formattingToolBar->addAction(actionIndentLess);
    formattingMenu->addAction(actionIndentMore);
    formattingMenu->addAction(actionIndentLess);

    formattingMenu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = formattingMenu->addAction(pix, "&Цвет...", this, &TextEditor::textColor);
    formattingToolBar->addAction(actionTextColor);

    formattingMenu->addSeparator();

//    const QIcon checkboxIcon = QIcon::fromTheme("status-checkbox-checked", QIcon(":/icons/checkbox-checked.png"));
//    actionToggleCheckState = formattingMenu->addAction(checkboxIcon, "Chec&ked", this, &TextEditor::setChecked);
//    actionToggleCheckState->setShortcut(Qt::CTRL + Qt::Key_K);
//    actionToggleCheckState->setCheckable(true);
//    actionToggleCheckState->setPriority(QAction::LowPriority);
//    formattingToolBar->addAction(actionToggleCheckState);

//    tb = addToolBar(tr("Format Actions"));
//    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
//    addToolBarBreak(Qt::TopToolBarArea);
//    addToolBar(tb);

    comboStyle = new QComboBox(formattingToolBar);
    formattingToolBar->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Task List (Unchecked)");
    comboStyle->addItem("Task List (Checked)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    comboStyle->addItem("Heading 1");
    comboStyle->addItem("Heading 2");
    comboStyle->addItem("Heading 3");
    comboStyle->addItem("Heading 4");
    comboStyle->addItem("Heading 5");
    comboStyle->addItem("Heading 6");

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &TextEditor::textStyle);

    comboFont = new QFontComboBox(formattingToolBar);
    comboFont->setCurrentFont(QFont("Times"));
    formattingToolBar->addWidget(comboFont);
    connect(comboFont, &QComboBox::textActivated, this, &TextEditor::textFamily);

    comboSize = new QComboBox(formattingToolBar);
    comboSize->setObjectName("comboSize");
    formattingToolBar->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    for (int size : standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, &QComboBox::textActivated, this, &TextEditor::textSize);
}

void TextEditor::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEditor::textStyle(int styleIndex)
{
    QTextCursor cursor = this->textCursor();
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;
    QTextBlockFormat::MarkerType marker = QTextBlockFormat::MarkerType::NoMarker;

    switch (styleIndex) {
    case 1:
        style = QTextListFormat::ListDisc;
        break;
    case 2:
        style = QTextListFormat::ListCircle;
        break;
    case 3:
        style = QTextListFormat::ListSquare;
        break;
    case 4:
        if (cursor.currentList())
            style = cursor.currentList()->format().style();
        else
            style = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::MarkerType::Unchecked;
        break;
    case 5:
        if (cursor.currentList())
            style = cursor.currentList()->format().style();
        else
            style = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::MarkerType::Checked;
        break;
    case 6:
        style = QTextListFormat::ListDecimal;
        break;
    case 7:
        style = QTextListFormat::ListLowerAlpha;
        break;
    case 8:
        style = QTextListFormat::ListUpperAlpha;
        break;
    case 9:
        style = QTextListFormat::ListLowerRoman;
        break;
    case 10:
        style = QTextListFormat::ListUpperRoman;
        break;
    default:
        break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    if (style == QTextListFormat::ListStyleUndefined) {
        blockFmt.setObjectIndex(-1);
        int headingLevel = styleIndex >= 11 ? styleIndex - 11 + 1 : 0; // H1 to H6, or Standard
        blockFmt.setHeadingLevel(headingLevel);
        cursor.setBlockFormat(blockFmt);

        int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
        QTextCharFormat fmt;
        fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
        fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.mergeCharFormat(fmt);
        this->mergeCurrentCharFormat(fmt);
    } else {
        blockFmt.setMarker(marker);
        cursor.setBlockFormat(blockFmt);
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
}

void TextEditor::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        this->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        this->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        this->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        this->setAlignment(Qt::AlignJustify);
}

void TextEditor::setView(GraphicsView *newView)
{
    view = newView;
}

void TextEditor::filePrintPdf()
{
//    QFileDialog fileDialog(this, tr("Export PDF"));
//    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
//    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
//    fileDialog.setDefaultSuffix("pdf");
//    if (fileDialog.exec() != QDialog::Accepted)
//        return;
//    QString fileName = fileDialog.selectedFiles().first();
//    QPrinter printer(QPrinter::HighResolution);
//    printer.setOutputFormat(QPrinter::PdfFormat);
//    printer.setOutputFileName(fileName);
//    textEdit->document()->print(&printer);
//    statusBar()->showMessage(tr("Exported \"%1\"")
//                             .arg(QDir::toNativeSeparators(fileName)));
}

void TextEditor::indent()
{
    modifyIndentation(1);
}

void TextEditor::unindent()
{
    modifyIndentation(-1);
}

void TextEditor::modifyIndentation(int amount)
{
    QTextCursor cursor = this->textCursor();
    cursor.beginEditBlock();
    if (cursor.currentList()) {
        QTextListFormat listFmt = cursor.currentList()->format();
        // See whether the line above is the list we want to move this item into,
        // or whether we need a new list.
        QTextCursor above(cursor);
        above.movePosition(QTextCursor::Up);
        if (above.currentList() && listFmt.indent() + amount == above.currentList()->format().indent()) {
            above.currentList()->add(cursor.block());
        } else {
            listFmt.setIndent(listFmt.indent() + amount);
            cursor.createList(listFmt);
        }
    } else {
        QTextBlockFormat blockFmt = cursor.blockFormat();
        blockFmt.setIndent(blockFmt.indent() + amount);
        cursor.setBlockFormat(blockFmt);
    }
    cursor.endEditBlock();
}

void TextEditor::currentCharFormatChanged(const QTextCharFormat &format)
{
//    fontChanged(format.font());
//    colorChanged(format.foreground().color());
}

void TextEditor::textColor()
{
    QColor col = QColorDialog::getColor(QTextEdit::textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void TextEditor::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEditor::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditor::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = this->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    this->mergeCurrentCharFormat(format);
}

void TextEditor::cursorPositionChanged()
{
//    alignmentChanged(textEdit->alignment());
//    QTextList *list = textEdit->textCursor().currentList();
//    if (list) {
//        switch (list->format().style()) {
//        case QTextListFormat::ListDisc:
//            comboStyle->setCurrentIndex(1);
//            break;
//        case QTextListFormat::ListCircle:
//            comboStyle->setCurrentIndex(2);
//            break;
//        case QTextListFormat::ListSquare:
//            comboStyle->setCurrentIndex(3);
//            break;
//        case QTextListFormat::ListDecimal:
//            comboStyle->setCurrentIndex(6);
//            break;
//        case QTextListFormat::ListLowerAlpha:
//            comboStyle->setCurrentIndex(7);
//            break;
//        case QTextListFormat::ListUpperAlpha:
//            comboStyle->setCurrentIndex(8);
//            break;
//        case QTextListFormat::ListLowerRoman:
//            comboStyle->setCurrentIndex(9);
//            break;
//        case QTextListFormat::ListUpperRoman:
//            comboStyle->setCurrentIndex(10);
//            break;
//        default:
//            comboStyle->setCurrentIndex(-1);
//            break;
//        }
//        switch (textEdit->textCursor().block().blockFormat().marker()) {
//        case QTextBlockFormat::MarkerType::NoMarker:
//            actionToggleCheckState->setChecked(false);
//            break;
//        case QTextBlockFormat::MarkerType::Unchecked:
//            comboStyle->setCurrentIndex(4);
//            actionToggleCheckState->setChecked(false);
//            break;
//        case QTextBlockFormat::MarkerType::Checked:
//            comboStyle->setCurrentIndex(5);
//            actionToggleCheckState->setChecked(true);
//            break;
//        }
//    } else {
//        int headingLevel = textEdit->textCursor().blockFormat().headingLevel();
//        comboStyle->setCurrentIndex(headingLevel ? headingLevel + 10 : 0);
//    }
}
