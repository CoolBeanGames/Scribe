#include "CodeEditorWidget.h"
#include <QPainter>
#include <QTextBlock>

CodeEditorWidget::CodeEditorWidget(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditorWidget::blockCountChanged, this, &CodeEditorWidget::updateLineNumberAreaWidth);
    connect(this, &CodeEditorWidget::updateRequest, this, &CodeEditorWidget::updateLineNumberArea);
    connect(this, &CodeEditorWidget::cursorPositionChanged, this, &CodeEditorWidget::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    
    highlighter = new PythonHighlighter(this->document());
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
}

int CodeEditorWidget::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditorWidget::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditorWidget::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditorWidget::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditorWidget::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor("#1D222C");
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void CodeEditorWidget::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor("#0B0D12"));
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#4A5568"));
            painter.drawText(0, top, lineNumberArea->width() - 3, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditorWidget::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Backtab) {
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
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        QString currentLine = textCursor().block().text();
        QPlainTextEdit::keyPressEvent(e);
        int spaceCount = 0;
        for (QChar c : currentLine) {
            if (c == ' ') spaceCount++;
            else if (c == '\t') spaceCount += 4;
            else break;
        }
        if (currentLine.trimmed().endsWith(':')) spaceCount += 4; // Python auto-indent
        if (spaceCount > 0) {
            insertPlainText(QString(spaceCount, ' '));
        }
        return;
    }
    QPlainTextEdit::keyPressEvent(e);
}


