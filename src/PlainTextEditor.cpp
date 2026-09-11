#include "PlainTextEditor.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>

PlainTextEditor::PlainTextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
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

    // Connect internal modification signal
    connect(document(), &QTextDocument::modificationChanged,
            this, &PlainTextEditor::onDocumentModified);
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
    if (m_filePath.isEmpty()) return "Untitled";
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
    if (event->key() == Qt::Key_Backtab) {
        QTextCursor cursor = textCursor();
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.beginEditBlock();
        while (cursor.position() <= end || (!cursor.hasSelection() && cursor.position() == end)) {
            cursor.movePosition(QTextCursor::StartOfLine);
            QTextCursor delCursor = cursor;
            delCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
            QString text = delCursor.selectedText();
            int charsToRemove = 0;
            for (int i=0; i<text.length(); ++i) {
                if (text[i] == ' ') charsToRemove++;
                else if (text[i] == '\t' && i == 0) { charsToRemove = 1; break; }
                else break;
            }
            if (charsToRemove > 0) {
                delCursor.setPosition(cursor.position());
                delCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, charsToRemove);
                delCursor.removeSelectedText();
                end -= charsToRemove;
            }
            if (!cursor.movePosition(QTextCursor::NextBlock)) break;
        }
        cursor.endEditBlock();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // Auto-indent: preserve the indentation of the current line
        QString indent = getIndentForCurrentLine();
        QPlainTextEdit::keyPressEvent(event); // insert the newline
        QTextCursor cursor = textCursor();
        cursor.insertText(indent);             // re-insert leading whitespace
        setTextCursor(cursor);
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}

QString PlainTextEditor::getIndentForCurrentLine() const
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    const QString line = cursor.selectedText();

    QString indent;
    for (const QChar& ch : line) {
        if (ch == ' ' || ch == '\t') {
            indent += ch;
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

