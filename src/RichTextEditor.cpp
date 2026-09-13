#include "InsertTableDialog.h"
#include <QFileDialog>
#include <QMenu>
#include "RichTextEditor.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextTable>
#include <QTextImageFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextList>
#include <QFont>
#include <QColor>
#include <QImageReader>
#include <QLabel>
#include <QScrollBar>
#include <QPainter>
#include <cmath>

class PageTextEdit : public QTextEdit {
public:
    explicit PageTextEdit(QWidget* parent = nullptr)
        : QTextEdit(parent) {}

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QTextEdit::paintEvent(event);

        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, false);

        const int pageH = 1056;
        int scrollY = verticalScrollBar()->value();
        int viewH = viewport()->height();
        int viewW = viewport()->width();

        int docH = (int)document()->size().height();
        int totalPages = qMax(1, qMax(document()->pageCount(), (int)std::ceil(docH / (double)pageH)));

        for (int page = 1; page < totalPages; ++page) {
            int pageY = page * pageH - scrollY;
            if (pageY >= 0 && pageY <= viewH) {
                p.setPen(QPen(QColor("#3A4252"), 1, Qt::DashLine));
                p.drawLine(20, pageY, viewW - 20, pageY);

                p.setFont(QFont("Segoe UI", 8, QFont::DemiBold));
                p.setPen(QColor("#8A95A8"));
                QString tag = QString("Page %1").arg(page + 1);
                p.drawText(QRect(viewW - 90, pageY - 16, 70, 14), Qt::AlignRight | Qt::AlignVCenter, tag);
            }
        }
    }
};

RichTextEditor::RichTextEditor(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Page workspace container (near-black background)
    QWidget* workspace = new QWidget(this);
    workspace->setObjectName("RichTextWorkspace");
    auto* wsLayout = new QHBoxLayout(workspace);
    wsLayout->setContentsMargins(16, 16, 16, 8);
    wsLayout->setAlignment(Qt::AlignHCenter);

    m_editor = new PageTextEdit(workspace);
    m_editor->setObjectName("RichTextPageEditor");
    m_editor->setReadOnly(false);
    m_editor->setAcceptRichText(true);
    m_editor->setFixedWidth(816); // Standard Letter width at 96 DPI

    // Standard print page size: Letter (816 x 1056 px) at 96 DPI
    QSizeF pageSize(816, 1056);
    m_editor->document()->setPageSize(pageSize);
    m_editor->document()->setDocumentMargin(72); // 0.75 in print margin

    // Set default font
    QFont defaultFont("Segoe UI", 12);
    m_editor->setFont(defaultFont);
    m_editor->document()->setDefaultFont(defaultFont);

    wsLayout->addWidget(m_editor);
    mainLayout->addWidget(workspace, 1);

    // Page footer status bar
    QWidget* footerBar = new QWidget(this);
    footerBar->setObjectName("RichTextFooter");
    auto* footerLayout = new QHBoxLayout(footerBar);
    footerLayout->setContentsMargins(16, 4, 16, 4);
    m_pageLabel = new QLabel("Page 1 of 1  •  Standard Letter (8.5\" × 11\")", footerBar);
    m_pageLabel->setStyleSheet("color: #8A95A8; font-size: 11px; font-weight: 500;");
    footerLayout->addWidget(m_pageLabel);
    footerLayout->addStretch();
    mainLayout->addWidget(footerBar);

    m_editor->installEventFilter(this);
    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editor, &QWidget::customContextMenuRequested, this, &RichTextEditor::onCustomContextMenu);

    // Track modifications
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, &RichTextEditor::onDocumentModified);
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, [this]() {
                updatePageInfo();
                emit cursorPositionChanged();
            });
    connect(m_editor->document(), &QTextDocument::contentsChanged,
            this, &RichTextEditor::updatePageInfo);
}

bool RichTextEditor::isModified() const
{
    return m_modified;
}

void RichTextEditor::setModified(bool modified)
{
    m_modified = modified;
    m_editor->document()->setModified(modified);
    emit modificationChanged(modified);
}

void RichTextEditor::setFilePath(const QString& path)
{
    m_filePath = path;
    emit filePathChanged(path);
}

bool RichTextEditor::loadFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Open Error",
            QString("Cannot open file:\n%1\n\n%2").arg(path, file.errorString()));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "rtf") {
        // Qt's QTextEdit can read RTF via setContent / directly
        // We'll use the HTML approach via QTextDocument
        // Qt doesn't natively decode RTF very well;
        // we'll store as rich text HTML internally and use RTF export
        // For loading, try to detect HTML vs RTF
        if (data.startsWith("{\\rtf")) {
            // Basic RTF - Qt supports limited RTF via paste mechanism
            // Use setHtml as fallback, or let Qt handle it
            m_editor->document()->clear();
            // Qt 6 doesn't have a built-in RTF reader, so we load it as
            // plain text with an indication, then show properly
            // Actually, QTextEdit::insertFromMimeData handles RTF if pasted
            // For file loading, use plain approach:
            QTextStream ts(&data);
            // Try setting as HTML (won't look right for RTF, but it's a start)
            // For proper RTF support: set the raw content
            m_editor->setPlainText(QString::fromUtf8(data));
        } else if (data.startsWith("<") || data.contains("<!DOCTYPE") || data.contains("<html")) {
            m_editor->setHtml(QString::fromUtf8(data));
        } else {
            m_editor->setPlainText(QString::fromUtf8(data));
        }
    } else if (ext == "html" || ext == "htm") {
        m_editor->setHtml(QString::fromUtf8(data));
    } else {
        m_editor->setPlainText(QString::fromUtf8(data));
    }

    setFilePath(path);
    setModified(false);
    m_editor->document()->setModified(false);
    return true;
}

bool RichTextEditor::saveFile()
{
    if (m_filePath.isEmpty()) return false;
    return saveFileAs(m_filePath);
}

bool RichTextEditor::saveFileAs(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Save Error",
            QString("Cannot save file:\n%1\n\n%2").arg(path, file.errorString()));
        return false;
    }

    QString ext = QFileInfo(path).suffix().toLower();
    QByteArray data;

    if (ext == "rtf") {
        // Export as HTML since Qt doesn't have a native RTF writer.
        // The file is saved as HTML with .rtf extension, which many apps can read.
        // For a true RTF: we embed basic RTF header.
        QString html = m_editor->toHtml();
        // Wrap in a simple RTF that references the HTML content as plain text
        // True minimal approach: save HTML with .rtf, noting it's HTML
        data = html.toUtf8();
    } else if (ext == "html" || ext == "htm") {
        data = m_editor->toHtml().toUtf8();
    } else {
        data = m_editor->toPlainText().toUtf8();
    }

    file.write(data);
    file.close();

    setFilePath(path);
    setModified(false);
    m_editor->document()->setModified(false);
    return true;
}

QString RichTextEditor::displayName() const
{
    if (m_filePath.isEmpty()) return "Untitled";
    return QFileInfo(m_filePath).fileName();
}

bool RichTextEditor::canUndo() const
{
    return m_editor->document()->isUndoAvailable();
}

bool RichTextEditor::canRedo() const
{
    return m_editor->document()->isRedoAvailable();
}

void RichTextEditor::undo()
{
    m_editor->undo();
}

void RichTextEditor::redo()
{
    m_editor->redo();
}

bool RichTextEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_editor && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            QTextCursor cursor = m_editor->textCursor();
            if (cursor.currentList()) {
                indentList();
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            QTextCursor cursor = m_editor->textCursor();
            if (cursor.currentList()) {
                unindentList();
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Space) {
            QTextCursor cursor = m_editor->textCursor();
            QTextCursor lineCursor = cursor;
            lineCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
            QString text = lineCursor.selectedText();
            if (text == "-" || text == "*") {
                lineCursor.removeSelectedText();
                QTextListFormat listFormat;
                listFormat.setStyle(QTextListFormat::ListDisc);
                listFormat.setIndent(1);
                cursor.createList(listFormat);
                return true;
            } else if (text == "1." || text == "1)") {
                lineCursor.removeSelectedText();
                QTextListFormat listFormat;
                listFormat.setStyle(QTextListFormat::ListDecimal);
                listFormat.setIndent(1);
                cursor.createList(listFormat);
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ControlModifier) {
                insertPageBreak();
                return true;
            }
            QTextCursor cursor = m_editor->textCursor();
            if (cursor.currentList()) {
                QTextCursor blockCursor = cursor;
                blockCursor.select(QTextCursor::BlockUnderCursor);
                QString text = blockCursor.selectedText().remove(QChar::ParagraphSeparator).trimmed();
                if (text.isEmpty()) {
                    QTextBlockFormat blockFormat = cursor.blockFormat();
                    blockFormat.setObjectIndex(-1);
                    blockFormat.setIndent(0);
                    cursor.setBlockFormat(blockFormat);
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    cursor.removeSelectedText();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void RichTextEditor::setBold(bool bold)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::setItalic(bool italic)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(italic);
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::setUnderline(bool underline)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(underline);
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(fmt);
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::setFontFamily(const QString& family)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(family);
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::setFontSize(int size)
{
    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::setFontColor(const QColor& color)
{
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(color));
    m_editor->mergeCurrentCharFormat(fmt);
}

void RichTextEditor::setAlignment(Qt::Alignment alignment)
{
    m_editor->setAlignment(alignment);
}

void RichTextEditor::setHeadingStyle(int level)
{
    if (level < 1 || level > 6) return;

    // Map heading level to font size and weight
    static const int sizes[]   = { 28, 22, 18, 16, 14, 12 };
    static const int weights[] = {
        QFont::Bold, QFont::Bold, QFont::Bold,
        QFont::Bold, QFont::Bold, QFont::Bold
    };

    QTextCharFormat charFmt;
    charFmt.setFontPointSize(sizes[level - 1]);
    charFmt.setFontWeight(weights[level - 1]);

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.mergeCharFormat(charFmt);
    cursor.endEditBlock();
    m_editor->mergeCurrentCharFormat(charFmt);
}

void RichTextEditor::insertTable(int rows, int cols)
{
    if (rows < 1 || cols < 1) return;

    QTextCursor cursor = m_editor->textCursor();

    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(6);
    tableFormat.setCellSpacing(0);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setBorder(1);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));

    cursor.insertTable(rows, cols, tableFormat);
}

void RichTextEditor::insertImage(const QString& imagePath)
{
    if (imagePath.isEmpty()) return;

    // Load image and add as resource
    QImageReader reader(imagePath);
    QImage image = reader.read();
    if (image.isNull()) {
        QMessageBox::warning(this, "Image Error",
            QString("Cannot load image:\n%1").arg(imagePath));
        return;
    }

    // Scale down if too large
    if (image.width() > 800 || image.height() > 600) {
        image = image.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QString name = QFileInfo(imagePath).fileName();
    m_editor->document()->addResource(QTextDocument::ImageResource,
                                       QUrl(name), image);
    
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertHtml(QString("<img src=\"%1\" style=\"float: left; margin: 10px;\" width=\"%2\" height=\"%3\" />")
        .arg(name).arg(image.width()).arg(image.height()));
}

void RichTextEditor::insertHyperlink(const QString& url, const QString& text)
{
    if (url.isEmpty()) return;

    QString displayText = text.isEmpty() ? url : text;

    QTextCharFormat fmt;
    fmt.setAnchor(true);
    fmt.setAnchorHref(url);
    fmt.setForeground(QColor("#8B7CFF"));
    fmt.setFontUnderline(true);

    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText(displayText, fmt);
}

void RichTextEditor::insertSectionBreak()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable* table = cursor.currentTable();
    if (table) {
        int pos = table->lastPosition() + 1;
        if (pos < m_editor->document()->characterCount()) {
            cursor.setPosition(pos);
        } else {
            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
        }
    }
    cursor.insertBlock();
    cursor.insertHtml("<div style=\"margin: 20px 0;\"><hr style=\"border: none; border-top: 2px solid #8B7CFF; margin: 6px 0;\"/><div style=\"text-align: center; color: #8B7CFF; font-size: 8pt; font-weight: bold; letter-spacing: 1px;\">═══ SECTION BREAK ═══</div><hr style=\"border: none; border-top: 1px solid #262C38; margin: 6px 0;\"/></div>");
    cursor.insertBlock();
    QTextBlockFormat normalBf;
    cursor.setBlockFormat(normalBf);
    m_editor->setTextCursor(cursor);
    setModified(true);
}

void RichTextEditor::insertPageBreak()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable* table = cursor.currentTable();
    if (table) {
        int pos = table->lastPosition() + 1;
        if (pos < m_editor->document()->characterCount()) {
            cursor.setPosition(pos);
        } else {
            cursor.movePosition(QTextCursor::End);
            cursor.insertBlock();
        }
    }
    cursor.insertHtml("<div style=\"margin: 12px 0;\"><hr style=\"border: none; border-top: 1px dashed #6A758A; margin: 4px 0;\"/><div style=\"text-align: center; color: #8A95A8; font-size: 8pt; letter-spacing: 1px;\">─── PAGE BREAK ───</div><hr style=\"border: none; border-top: 1px dashed #6A758A; margin: 4px 0;\"/></div>");
    cursor.insertBlock();
    QTextBlockFormat bf = cursor.blockFormat();
    bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
    cursor.setBlockFormat(bf);
    m_editor->setTextCursor(cursor);
    setModified(true);
    updatePageInfo();
}

void RichTextEditor::setColumns(int numColumns)
{
    QString selectedHtml;
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        selectedHtml = cursor.selection().toHtml();
        cursor.removeSelectedText();
        m_editor->setTextCursor(cursor);
    }

    insertSectionBreak();

    if (numColumns <= 1) {
        if (!selectedHtml.isEmpty()) {
            m_editor->textCursor().insertHtml(selectedHtml);
        }
        return;
    }

    cursor = m_editor->textCursor();
    QTextTableFormat tf;
    tf.setBorder(0);
    tf.setCellPadding(12);
    tf.setCellSpacing(0);
    tf.setWidth(QTextLength(QTextLength::PercentageLength, 100));

    QVector<QTextLength> constraints;
    int percent = 100 / numColumns;
    for (int i = 0; i < numColumns; ++i) {
        constraints.append(QTextLength(QTextLength::PercentageLength, percent));
    }
    tf.setColumnWidthConstraints(constraints);

    QTextTable* table = cursor.insertTable(1, numColumns, tf);
    if (table && table->cellAt(0, 0).isValid()) {
        QTextCursor cellCursor = table->cellAt(0, 0).firstCursorPosition();
        if (!selectedHtml.isEmpty()) {
            cellCursor.insertHtml(selectedHtml);
        }
        m_editor->setTextCursor(cellCursor);
    }
    setModified(true);
}

int RichTextEditor::currentColumnCount() const
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable* table = cursor.currentTable();
    if (table && table->columns() > 1) {
        return table->columns();
    }
    return 1;
}

QTextCharFormat RichTextEditor::currentCharFormat() const
{
    return m_editor->currentCharFormat();
}

Qt::Alignment RichTextEditor::currentAlignment() const
{
    return m_editor->alignment();
}

void RichTextEditor::onDocumentModified()
{
    bool mod = m_editor->document()->isModified();
    if (m_modified != mod) {
        m_modified = mod;
        emit modificationChanged(mod);
    }
}



void RichTextEditor::onCustomContextMenu(const QPoint& pos) {
    QMenu* menu = m_editor->createStandardContextMenu();
    buildContextMenu(menu);
    menu->exec(m_editor->mapToGlobal(pos));
    delete menu;
}


void RichTextEditor::toggleBulletList()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList* list = cursor.currentList();
    if (list) {
        if (list->format().style() == QTextListFormat::ListDisc) {
            // Remove from list
            QTextBlockFormat bfmt = cursor.blockFormat();
            bfmt.setObjectIndex(-1);
            bfmt.setIndent(0);
            cursor.setBlockFormat(bfmt);
        } else {
            // Switch style to bullet
            QTextListFormat listFmt = list->format();
            listFmt.setStyle(QTextListFormat::ListDisc);
            list->setFormat(listFmt);
        }
    } else {
        QTextListFormat listFmt;
        listFmt.setStyle(QTextListFormat::ListDisc);
        listFmt.setIndent(1);
        cursor.createList(listFmt);
    }
}

void RichTextEditor::toggleNumberedList()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList* list = cursor.currentList();
    if (list) {
        if (list->format().style() == QTextListFormat::ListDecimal) {
            // Remove from list
            QTextBlockFormat bfmt = cursor.blockFormat();
            bfmt.setObjectIndex(-1);
            bfmt.setIndent(0);
            cursor.setBlockFormat(bfmt);
        } else {
            // Switch style to numbered
            QTextListFormat listFmt = list->format();
            listFmt.setStyle(QTextListFormat::ListDecimal);
            list->setFormat(listFmt);
        }
    } else {
        QTextListFormat listFmt;
        listFmt.setStyle(QTextListFormat::ListDecimal);
        listFmt.setIndent(1);
        cursor.createList(listFmt);
    }
}

void RichTextEditor::indentList()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList* list = cursor.currentList();
    if (list) {
        QTextListFormat listFmt = list->format();
        listFmt.setIndent(listFmt.indent() + 1);
        cursor.createList(listFmt);
    } else {
        QTextBlockFormat bfmt = cursor.blockFormat();
        bfmt.setIndent(bfmt.indent() + 1);
        cursor.setBlockFormat(bfmt);
    }
}

void RichTextEditor::unindentList()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList* list = cursor.currentList();
    if (list) {
        QTextListFormat listFmt = list->format();
        if (listFmt.indent() > 1) {
            listFmt.setIndent(listFmt.indent() - 1);
            cursor.createList(listFmt);
        } else {
            // Remove from list completely
            QTextBlockFormat bfmt = cursor.blockFormat();
            bfmt.setObjectIndex(-1);
            bfmt.setIndent(0);
            cursor.setBlockFormat(bfmt);
        }
    } else {
        QTextBlockFormat bfmt = cursor.blockFormat();
        if (bfmt.indent() > 0) {
            bfmt.setIndent(bfmt.indent() - 1);
            cursor.setBlockFormat(bfmt);
        }
    }
}

void RichTextEditor::toggleList()
{
    toggleBulletList();
}


void RichTextEditor::buildContextMenu(QMenu* menu)
{
    menu->addSeparator();
    QAction* actPaste = menu->addAction("Paste");
    connect(actPaste, &QAction::triggered, m_editor, &QTextEdit::paste);
    
    QAction* actTable = menu->addAction("Insert Table");
    connect(actTable, &QAction::triggered, this, [this]() {
        InsertTableDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            this->insertTable(dlg.rows(), dlg.columns());
        }
    });

    QAction* actImage = menu->addAction("Insert Image");
    connect(actImage, &QAction::triggered, this, [this]() {
        QString filter = "Images (*.png *.jpg *.jpeg *.bmp *.gif)";
        QString path = QFileDialog::getOpenFileName(this, "Insert Image", QString(), filter);
        if (!path.isEmpty()) {
            this->insertImage(path);
        }
    });

    menu->addSeparator();
    QAction* actSecBreak = menu->addAction("Insert Section Break");
    connect(actSecBreak, &QAction::triggered, this, &RichTextEditor::insertSectionBreak);

    QAction* actPageBreak = menu->addAction("Insert Page Break");
    connect(actPageBreak, &QAction::triggered, this, &RichTextEditor::insertPageBreak);

    QMenu* colMenu = menu->addMenu("Columns");
    QAction* col1 = colMenu->addAction("1 Column");
    QAction* col2 = colMenu->addAction("2 Columns");
    QAction* col3 = colMenu->addAction("3 Columns");
    connect(col1, &QAction::triggered, this, [this]() { setColumns(1); });
    connect(col2, &QAction::triggered, this, [this]() { setColumns(2); });
    connect(col3, &QAction::triggered, this, [this]() { setColumns(3); });
}

void RichTextEditor::updatePageInfo()
{
    if (!m_pageLabel || !m_editor) return;
    int scrollY = m_editor->verticalScrollBar() ? m_editor->verticalScrollBar()->value() : 0;
    int cursorY = m_editor->cursorRect().top() + scrollY;
    const int pageH = 1056;
    int curPage = qMax(1, (cursorY / pageH) + 1);
    int docH = (int)m_editor->document()->size().height();
    int totalPages = qMax(1, qMax(m_editor->document()->pageCount(), (int)std::ceil(docH / (double)pageH)));
    m_pageLabel->setText(QString("Page %1 of %2  •  Standard Letter (8.5\" × 11\")").arg(curPage).arg(totalPages));
}

