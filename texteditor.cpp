#include "texteditor.h"
#include <QPrinter>
#include "coinedit.h"

TextEditors::TextEditors(CoinEdit *coinEdit)
    : coinEdit(coinEdit)
{
    setGeometry(0, 0, 3800, 3800 / 1.42);
    setPalette(Qt::white);

    QHBoxLayout *layout = new QHBoxLayout;
    textEditorRight = new QTextEdit(this);
    textEditorRight->setFrameStyle(QStyleOptionFrame::None);
    textEditorLeft = new QTextEdit(this);
    textEditorLeft->setFrameStyle(QStyleOptionFrame::None);
    layout->addWidget(textEditorLeft);
    layout->addSpacing(1940);
    layout->addWidget(textEditorRight);

    setLayout(layout);

    QFont textFont("Times New Roman");
    textFont.setPointSize(14);
    textFont.setStyleHint(QFont::Serif);

    textEditorRight->setFont(textFont);
    textEditorRight->setGeometry(QRect(0, 0, 930, 2641));
    textEditorRight->setPlaceholderText("\n\n\n\n\n\n\n\n\n\n\nБоковушка");

    textEditorLeft->setFont(textFont);
    textEditorLeft->setGeometry(QRect(0, 0, 930, 2641));
    textEditorLeft->setPlaceholderText("\n\n\n\n\n\n\n\n\n\n\nСогласование");

    currentEditor = textEditorRight;
}

void TextEditors::doOnStartUp()
{
    connect(textEditorRight,
            &QTextEdit::currentCharFormatChanged,
            this,
            &TextEditors::currentCharFormatChanged);
    connect(textEditorLeft,
            &QTextEdit::currentCharFormatChanged,
            this,
            &TextEditors::currentCharFormatChanged);

    connect(textEditorRight, &QTextEdit::cursorPositionChanged, [&] {
        currentEditor = textEditorRight;
        cursorPositionChanged();
    });
    connect(textEditorLeft, &QTextEdit::cursorPositionChanged, [&] {
        currentEditor = textEditorLeft;
        cursorPositionChanged();
    });

    connect(textEditorRight->document(),
            &QTextDocument::modificationChanged,
            this,
            &QWidget::setWindowModified);

    connect(textEditorRight, &QTextEdit::undoAvailable, actionUndo, &QAction::setEnabled);

    connect(textEditorRight, &QTextEdit::redoAvailable, actionRedo, &QAction::setEnabled);

    setWindowModified(textEditorRight->document()->isModified());
    actionUndo->setEnabled(textEditorRight->document()->isUndoAvailable());
    actionRedo->setEnabled(textEditorRight->document()->isRedoAvailable());

    actionCut->setEnabled(false);
    connect(textEditorRight, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEditorRight, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(),
            &QClipboard::dataChanged,
            this,
            &TextEditors::clipboardDataChanged);

    QTextCharFormat format;
    format.setFont(QFont("Times New Roman", 14));
    textEditorRight->mergeCurrentCharFormat(format);
    textEditorLeft->mergeCurrentCharFormat(format);
    qDebug() << "on startup";
}

void TextEditors::read(const QJsonObject &json)
{
    if (json.contains("Боковушка") && json["Боковушка"].isString()) {
        QJsonValue textEditorData = json["Боковушка"].toString();
        QTextDocument *textDoc = new QTextDocument();
        textDoc->setHtml(textEditorData.toString());
        textDoc->setDefaultFont(QFont("Times New Roman", 14));
        textEditorRight->setDocument(textDoc);
    }

    if (json.contains("Иконостас") && json["Иконостас"].isString()) {
        QJsonValue textEditorData = json["Иконостас"].toString();
        QTextDocument *textDoc = new QTextDocument();
        textDoc->setHtml(textEditorData.toString());
        textDoc->setDefaultFont(QFont("Times New Roman", 14));
        textEditorLeft->setDocument(textDoc);
    }
}

void TextEditors::write(QJsonObject &json)
{
    QJsonValue textEditorRightData = {textEditorRight->document()->toHtml()};
    json["Боковушка"] = textEditorRightData;

    QJsonValue textEditorLeftData = {textEditorLeft->document()->toHtml()};
    json["Иконостас"] = textEditorLeftData;
}

QString TextEditors::getRightTextHtml()
{
    return textEditorRight->toHtml();
}

QString TextEditors::getLeftTextHtml()
{
    return textEditorLeft->toHtml();
}

void TextEditors::clipboardDataChanged()
{
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
}

void TextEditors::fontChanged(const QFont &f)
{
    qDebug() << "fontChanged " << f.family();
    //    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    //    !!!
    comboFont->setCurrentIndex(comboFont->findText(f.family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void TextEditors::alignmentChanged(Qt::Alignment a)
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

void TextEditors::addTable(Coin *tableCell, QVector<QString> text)
{
    QTextCursor cursor = textEditorRight->textCursor();
    QRect rect = textEditorRight->cursorRect(cursor);

    cursor.beginEditBlock();

    cursor.insertHtml("<br /><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                      "margin-right:px; -qt-block-indent:0; text-indent:0px;\">"
                      "<span style=\" font-family:'Times New Roman'; "
                      "font-size:14pt;\">Извлечь:</span></p>\n"
                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
                      "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                      "<span style=\" font-family:'Times New Roman'; font-size:14pt;\">"
                      "Поставить: "
                      + tableCell->getLoadingType() + " " + tableCell->getLoadingSubType()
                      + "</span></p>\n");

    QTextTable *table = cursor.insertTable(2, 2);

    cursor = table->cellAt(0, 1).firstCursorPosition();

    QTextBlockFormat blockFormat;
    blockFormat.setAlignment(Qt::AlignHCenter);
    cursor.setBlockFormat(blockFormat);

    QTextCharFormat charFormat;
    charFormat.setFont(QFont("Times New Roman", 14));
    cursor.setBlockCharFormat(charFormat);

    cursor.insertText("Ячейки");

    cursor = table->cellAt(1, 0).firstCursorPosition();
    cursor.insertText("\t  \n\n\n");
    emit addTableCell(tableCell, rect.topLeft());

    cursor = table->cellAt(1, 1).firstCursorPosition();
    blockFormat.setAlignment(Qt::AlignLeft);
    cursor.setBlockFormat(blockFormat);
    cursor.setBlockCharFormat(charFormat);
    for (int i = 0; i < text.size(); i++) {
        if (i == text.size() - 1) {
            cursor.insertText(text[i] + "\n");
            break;
        }
        ((i + 1) % 10 != 0) ? cursor.insertText(text[i] + ", ")
                            : cursor.insertText(text[i] + ", \n");
    }

    int num = text.size();
    if (num < 10)
        cursor.insertText("\t\t\t\t\t             = " + QString::number(num) + " шт.");
    else if (num < 100)
        cursor.insertText("\t\t\t\t\t           = " + QString::number(num) + " шт.");
    else
        cursor.insertText("\t\t\t\t\t         = " + QString::number(num) + " шт.");

    cursor.endEditBlock();
}

void TextEditors::setupEditActions()
{
    // TextEditor
    QToolBar *editToolBar = coinEdit->addToolBar("Edit Actions");
    QMenu *editMenu = coinEdit->menuBar()->addMenu("&Правка");

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/icons/editundo.png"));
    actionUndo = editMenu->addAction(undoIcon, "&Отменить", textEditorRight, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    editToolBar->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(":/icons/editredo.png"));
    actionRedo = editMenu->addAction(redoIcon, "&Вернуть", textEditorRight, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    editToolBar->addAction(actionRedo);

    editMenu->addSeparator();

    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/icons/editcut.png"));
    actionCut = editMenu->addAction(cutIcon, "В&ырезать", textEditorRight, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    editToolBar->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/icons/editcopy.png"));
    actionCopy = editMenu->addAction(copyIcon, "&Копировать", textEditorRight, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    editToolBar->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/icons/editpaste.png"));
    actionPaste = editMenu->addAction(pasteIcon, "В&ставить", textEditorRight, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    editToolBar->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());

    editMenu->addSeparator();

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(":/icons/exportpdf.png"));
    actionExportPdfFromEditor = editMenu->addAction(exportPdfIcon,
                                                    "&Сохранить боковушку в PDF...",
                                                    this,
                                                    &TextEditors::filePrintPdf);
    actionExportPdfFromEditor->setPriority(QAction::LowPriority);
    actionExportPdfFromEditor->setShortcut(Qt::CTRL | Qt::Key_D);
    editToolBar->addAction(actionExportPdfFromEditor);
}

void TextEditors::setupTextActions()
{
    QToolBar *formattingToolBar = coinEdit->addToolBar("Format Actions");
    QMenu *formattingMenu = coinEdit->menuBar()->addMenu("Формат");

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(":/icons/textbold.png"));
    actionTextBold = formattingMenu->addAction(boldIcon, "&Жирный", this, &TextEditors::textBold);
    actionTextBold->setShortcut(Qt::CTRL | Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    formattingToolBar->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(":/icons/textitalic.png"));
    actionTextItalic = formattingMenu->addAction(italicIcon,
                                                 "&Курсив",
                                                 this,
                                                 &TextEditors::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL | Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    formattingToolBar->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline",
                                                 QIcon(":/icons/textunder.png"));
    actionTextUnderline = formattingMenu->addAction(underlineIcon,
                                                    "&Подчёркнутый",
                                                    this,
                                                    &TextEditors::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL | Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    formattingToolBar->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    formattingMenu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(":/icons/textleft.png"));
    actionAlignLeft = new QAction(leftIcon, "&Лево", this);
    actionAlignLeft->setShortcut(Qt::CTRL | Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center",
                                              QIcon(":/icons/textcenter.png"));
    actionAlignCenter = new QAction(centerIcon, "&Центр", this);
    actionAlignCenter->setShortcut(Qt::CTRL | Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(":/icons/textright.png"));
    actionAlignRight = new QAction(rightIcon, "&Право", this);
    actionAlignRight->setShortcut(Qt::CTRL | Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(":/icons/textjustify.png"));
    actionAlignJustify = new QAction(fillIcon, "По &ширене", this);
    actionAlignJustify->setShortcut(Qt::CTRL | Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);
    const QIcon indentMoreIcon = QIcon::fromTheme("format-indent-more",
                                                  QIcon(":/icons/format-indent-more.png"));
    actionIndentMore = formattingToolBar->addAction(indentMoreIcon,
                                                    "Отступ&+",
                                                    this,
                                                    &TextEditors::indent);
    actionIndentMore->setShortcut(Qt::CTRL | Qt::Key_BracketRight);
    actionIndentMore->setPriority(QAction::LowPriority);
    const QIcon indentLessIcon = QIcon::fromTheme("format-indent-less",
                                                  QIcon(":/icons/format-indent-less.png"));
    actionIndentLess = formattingToolBar->addAction(indentLessIcon,
                                                    "Отступ&-",
                                                    this,
                                                    &TextEditors::unindent);
    actionIndentLess->setShortcut(Qt::CTRL | Qt::Key_BracketLeft);
    actionIndentLess->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &TextEditors::textAlign);

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
    actionTextColor = formattingMenu->addAction(pix, "&Цвет...", this, &TextEditors::textColor);
    formattingToolBar->addAction(actionTextColor);

    formattingMenu->addSeparator();

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

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &TextEditors::textStyle);

    comboFont = new QFontComboBox(formattingToolBar);
    formattingToolBar->addWidget(comboFont);
    connect(comboFont, &QComboBox::textActivated, this, &TextEditors::textFamily);

    comboSize = new QComboBox(formattingToolBar);
    comboSize->setObjectName("comboSize");
    formattingToolBar->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    for (int size : standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(14));

    connect(comboSize, &QComboBox::textActivated, this, &TextEditors::textSize);
}

void TextEditors::textFamily(const QString &f)
{
    qDebug() << "textFamily " << f;

    QTextCharFormat fmt;
#if QT_VERSION < 0x060500
    fmt.setFontFamily(f);
#else
    fmt.setFontFamilies({f});
#endif
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditors::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEditors::textStyle(int styleIndex)
{
    qDebug() << "textStyle " << currentEditor;
    QTextCursor cursor = currentEditor->textCursor();

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

        currentEditor->mergeCurrentCharFormat(fmt);
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

void TextEditors::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        currentEditor->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        currentEditor->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        currentEditor->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        currentEditor->setAlignment(Qt::AlignJustify);
}

void TextEditors::filePrintPdf()
{
    QFileDialog fileDialog(this, "Сохранить боковушку в PDF");
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setFullPage(true);
    printer.setPageMargins(QMargins(0, 0, 0, 0));
    printer.setOutputFileName(fileName);
    textEditorRight->document()->print(&printer);
    coinEdit->statusBar()
        ->showMessage(tr("Боковушка сохранена в \"%1\"").arg(QDir::toNativeSeparators(fileName)),
                      4000);
}

void TextEditors::indent()
{
    modifyIndentation(1);
}

void TextEditors::unindent()
{
    modifyIndentation(-1);
}

void TextEditors::modifyIndentation(int amount)
{
    QTextCursor cursor = currentEditor->textCursor();

    cursor.beginEditBlock();
    if (cursor.currentList()) {
        QTextListFormat listFmt = cursor.currentList()->format();
        QTextCursor above(cursor);
        above.movePosition(QTextCursor::Up);
        if (above.currentList()
            && listFmt.indent() + amount == above.currentList()->format().indent()) {
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

void TextEditors::currentCharFormatChanged(const QTextCharFormat &format)
{
    qDebug() << "currentCharFormatChanged " << format.fontFamilies();

    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void TextEditors::textColor()
{
    QColor col = QColorDialog::getColor();
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void TextEditors::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditors::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEditors::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditors::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEditors::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    qDebug() << "mergeFormatOnWordOrSelection " << format.fontFamilies();

    QTextCursor cursor = currentEditor->textCursor();
    qDebug() << "cursor " << cursor.charFormat().fontFamilies();

    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);

    currentEditor->mergeCurrentCharFormat(format);
}

void TextEditors::cursorPositionChanged()
{
    qDebug() << "cursorPositionChanged";
    QTextCursor cursor = currentEditor->textCursor();
    if (cursor.charFormat().fontFamilies().toStringList().isEmpty()) {
        qDebug() << "new cursor format";
        QTextCharFormat newFormat;
        newFormat.setFont(QFont("Times New Roman", 14));
        cursor.setCharFormat(newFormat);
    }
    currentCharFormatChanged(cursor.charFormat());

    alignmentChanged(currentEditor->alignment());

    QTextList *list = currentEditor->textCursor().currentList();
    if (list) {
        switch (list->format().style()) {
        case QTextListFormat::ListDisc:
            comboStyle->setCurrentIndex(1);
            break;
        case QTextListFormat::ListCircle:
            comboStyle->setCurrentIndex(2);
            break;
        case QTextListFormat::ListSquare:
            comboStyle->setCurrentIndex(3);
            break;
        case QTextListFormat::ListDecimal:
            comboStyle->setCurrentIndex(6);
            break;
        case QTextListFormat::ListLowerAlpha:
            comboStyle->setCurrentIndex(7);
            break;
        case QTextListFormat::ListUpperAlpha:
            comboStyle->setCurrentIndex(8);
            break;
        case QTextListFormat::ListLowerRoman:
            comboStyle->setCurrentIndex(9);
            break;
        case QTextListFormat::ListUpperRoman:
            comboStyle->setCurrentIndex(10);
            break;
        default:
            comboStyle->setCurrentIndex(-1);
            break;
        }
    }
}
