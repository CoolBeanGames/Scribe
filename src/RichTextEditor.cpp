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
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QFont>
#include <QColor>
#include <QImageReader>

RichTextEditor::RichTextEditor(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_editor = new QTextEdit(this);
    m_editor->setReadOnly(false);
    m_editor->setAcceptRichText(true);

    // Set default font
    QFont defaultFont("Segoe UI", 12);
    m_editor->setFont(defaultFont);
    m_editor->document()->setDefaultFont(defaultFont);

    layout->addWidget(m_editor);
    m_editor->installEventFilter(this);
    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editor, &QWidget::customContextMenuRequested, this, &RichTextEditor::onCustomContextMenu);

    // Track modifications
    connect(m_editor->document(), &QTextDocument::modificationChanged,
            this, &RichTextEditor::onDocumentModified);
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, &RichTextEditor::cursorPositionChanged);
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
        if (keyEvent->key() == Qt::Key_Space) {
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
            }
        } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QTextCursor cursor = m_editor->textCursor();
            if (cursor.currentList()) {
                QTextCursor blockCursor = cursor;
                blockCursor.select(QTextCursor::BlockUnderCursor);
                QString text = blockCursor.selectedText().remove(QChar::ParagraphSeparator).trimmed();
                if (text.isEmpty()) {
                    QTextBlockFormat blockFormat = cursor.blockFormat();
                    blockFormat.setObjectIndex(-1);
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


void RichTextEditor::toggleList()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList* list = cursor.currentList();
    if (list) {
        QTextBlockFormat bfmt = cursor.blockFormat();
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
    } else {
        QTextListFormat listFmt;
        listFmt.setStyle(QTextListFormat::ListDisc);
        cursor.createList(listFmt);
    }
}

