#include <QMenu>
#include "PlainTextEditor.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QTextBlock>

PlainTextLineNumberArea::PlainTextLineNumberArea(PlainTextEditor* editor)
    : QWidget(editor), m_editor(editor)
{
}

QSize PlainTextLineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void PlainTextLineNumberArea::paintEvent(QPaintEvent* event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

PlainTextEditor::PlainTextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new PlainTextLineNumberArea(this);
    m_lineNumberArea->setVisible(false);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &PlainTextEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &PlainTextEditor::updateLineNumberArea);

    // Set up font
    QFont font("Cascadia Code", 13);
    font.setStyleHint(QFont::Monospace);
    if (!font.exactMatch()) {
        font.setFamily("Consolas");
    }
    if (!font.exactMatch()) {
        font.setFamily("Courier New");
    }
    setFont(font);

    // Tab stop: 4 spaces
    setTabStopDistance(QFontMetricsF(font).horizontalAdvance(' ') * 4);

    // Line wrap
    setLineWrapMode(QPlainTextEdit::WidgetWidth);

    updateLineNumberAreaWidth(0);

    // Connect internal modification signal
    connect(document(), &QTextDocument::modificationChanged,
            this, &PlainTextEditor::onDocumentModified);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested,
            this, &PlainTextEditor::onCustomContextMenu);
}

int PlainTextEditor::lineNumberAreaWidth() const
{
    if (!m_showLineNumbers) return 0;
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void PlainTextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void PlainTextEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (!m_showLineNumbers) return;
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void PlainTextEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    if (m_showLineNumbers && m_lineNumberArea) {
        QRect cr = contentsRect();
        m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

void PlainTextEditor::setLineNumbersVisible(bool visible)
{
    m_showLineNumbers = visible;
    if (m_lineNumberArea) {
        m_lineNumberArea->setVisible(visible);
    }
    updateLineNumberAreaWidth(0);
    if (visible && m_lineNumberArea) {
        QRect cr = contentsRect();
        m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
        m_lineNumberArea->update();
    }
    emit lineNumbersToggled(visible);
}

void PlainTextEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    if (!m_showLineNumbers) return;
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#0B0D12"));
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#4A5568"));
            painter.drawText(0, top, m_lineNumberArea->width() - 5, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void PlainTextEditor::setEditorFontFamily(const QString& family)
{
    QFont f = font();
    f.setFamily(family);
    setFont(f);
    setTabStopDistance(QFontMetricsF(f).horizontalAdvance(' ') * 4);
    updateLineNumberAreaWidth(0);
    if (m_showLineNumbers && m_lineNumberArea) {
        m_lineNumberArea->update();
    }
}

void PlainTextEditor::setEditorFontSize(int pointSize)
{
    QFont f = font();
    f.setPointSize(pointSize);
    setFont(f);
    setTabStopDistance(QFontMetricsF(f).horizontalAdvance(' ') * 4);
    updateLineNumberAreaWidth(0);
    if (m_showLineNumbers && m_lineNumberArea) {
        m_lineNumberArea->update();
    }
}

bool PlainTextEditor::isModified() const
{
    return m_modified;
}

void PlainTextEditor::setModified(bool modified)
{
    m_modified = modified;
    document()->setModified(modified);
    if (!modified) {
        document()->setModified(false);
    }
    emit modificationChanged(modified);
}

void PlainTextEditor::setFilePath(const QString& path)
{
    m_filePath = path;
    emit filePathChanged(path);
}

bool PlainTextEditor::loadFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Error",
            QString("Cannot open file:\n%1\n\n%2").arg(path, file.errorString()));
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    setPlainText(stream.readAll());
    file.close();

    setFilePath(path);
    setModified(false);
    document()->setModified(false);
    return true;
}

bool PlainTextEditor::saveFile()
{
    if (m_filePath.isEmpty()) return false;
    return saveFileAs(m_filePath);
}

bool PlainTextEditor::saveFileAs(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Error",
            QString("Cannot save file:\n%1\n\n%2").arg(path, file.errorString()));
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << toPlainText();
    file.close();

    setFilePath(path);
    setModified(false);
    document()->setModified(false);
    return true;
}

QString PlainTextEditor::displayName() const
{
    if (m_filePath.isEmpty()) return "Untitled.txt";
    return QFileInfo(m_filePath).fileName();
}

bool PlainTextEditor::canUndo() const
{
    return document()->isUndoAvailable();
}

bool PlainTextEditor::canRedo() const
{
    return document()->isRedoAvailable();
}

void PlainTextEditor::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QString indent = getIndentForCurrentLine();
        QPlainTextEdit::keyPressEvent(event);
        if (!indent.isEmpty()) {
            insertPlainText(indent);
        }
        return;
    }

    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}

QString PlainTextEditor::getIndentForCurrentLine() const
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QString line = cursor.selectedText();

    QString indent;
    for (QChar c : line) {
        if (c == ' ' || c == '\t') {
            indent += c;
        } else {
            break;
        }
    }
    return indent;
}

void PlainTextEditor::onDocumentModified()
{
    bool mod = document()->isModified();
    if (m_modified != mod) {
        m_modified = mod;
        emit modificationChanged(mod);
    }
}

void PlainTextEditor::onCustomContextMenu(const QPoint& pos) {
    QMenu menu(this);
    bool hasSel = textCursor().hasSelection();
    
    if (hasSel) {
        QAction* cutAct = menu.addAction("Cut");
        connect(cutAct, &QAction::triggered, this, &QPlainTextEdit::cut);
        
        QAction* copyAct = menu.addAction("Copy");
        connect(copyAct, &QAction::triggered, this, &QPlainTextEdit::copy);
    }
    
    QAction* pasteAct = menu.addAction("Paste");
    connect(pasteAct, &QAction::triggered, this, &QPlainTextEdit::paste);
    
    if (hasSel) {
        QAction* deleteAct = menu.addAction("Delete");
        connect(deleteAct, &QAction::triggered, this, [this]() {
            textCursor().removeSelectedText();
        });
    }
    
    menu.exec(mapToGlobal(pos));
}
