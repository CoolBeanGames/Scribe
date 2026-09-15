#include "CodeEditorWidget.h"
#include "PythonHighlighter.h"
#include "CSharpHighlighter.h"
#include "JsonHighlighter.h"
#include <QPainter>
#include <QTextBlock>
#include <QPalette>
#include <QFont>

#include <QCompleter>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QSet>
#include <QRegularExpression>

CodeEditorWidget::CodeEditorWidget(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditorWidget::blockCountChanged, this, &CodeEditorWidget::updateLineNumberAreaWidth);
    connect(this, &CodeEditorWidget::updateRequest, this, &CodeEditorWidget::updateLineNumberArea);
    connect(this, &CodeEditorWidget::cursorPositionChanged, this, &CodeEditorWidget::highlightCurrentLine);

    // Set palette explicitly so the QSyntaxHighlighter colours are not
    // overridden by the global stylesheet's "color:" rule.
    QPalette pal = palette();
    pal.setColor(QPalette::Base,            QColor("#12151C"));
    pal.setColor(QPalette::Text,            QColor("#ABB2BF"));
    pal.setColor(QPalette::Highlight,       QColor("#2D3A52"));
    pal.setColor(QPalette::HighlightedText, QColor("#F4F6FA"));
    setPalette(pal);
    setStyleSheet(""); // clear any inherited stylesheet to avoid conflicts

    // Monospace font
    QFont codeFont("Cascadia Code", 13);
    codeFont.setStyleHint(QFont::Monospace);
    if (!codeFont.exactMatch()) codeFont.setFamily("Consolas");
    if (!codeFont.exactMatch()) codeFont.setFamily("Courier New");
    setFont(codeFont);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setTabStopDistance(QFontMetricsF(codeFont).horizontalAdvance(' ') * 4);

    // Setup autocomplete
    auto* comp = new QCompleter(this);
    comp->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    comp->setWrapAround(false);
    setCompleter(comp);

    setLanguage(CodeLanguage::Python);

    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditorWidget::updateCompleterWords);
    updateCompleterWords();
}

void CodeEditorWidget::setLanguage(CodeLanguage lang) {
    if (m_highlighter && m_language == lang) return;
    m_language = lang;

    if (m_highlighter) {
        delete m_highlighter;
        m_highlighter = nullptr;
    }

    switch (m_language) {
    case CodeLanguage::CSharp:
        m_highlighter = new CSharpHighlighter(this->document());
        break;
    case CodeLanguage::Json:
        m_highlighter = new JsonHighlighter(this->document());
        break;
    case CodeLanguage::Python:
    default:
        m_highlighter = new PythonHighlighter(this->document());
        break;
    }

    updateCompleterWords();
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
        if (m_completer && m_completer->popup()->isVisible()) {
            // Let completer handle Enter
            e->ignore();
            return;
        }
        QTextCursor cursor = textCursor();
        bool betweenBraces = false;
        if (!cursor.atBlockEnd() && !cursor.atBlockStart()) {
            QChar before = cursor.document()->characterAt(cursor.position() - 1);
            QChar after = cursor.document()->characterAt(cursor.position());
            if ((before == '{' && after == '}') || (before == '[' && after == ']')) {
                betweenBraces = true;
            }
        }

        QString currentLine = textCursor().block().text();
        int spaceCount = 0;
        for (QChar c : currentLine) {
            if (c == ' ') spaceCount++;
            else if (c == '\t') spaceCount += 4;
            else break;
        }

        if (betweenBraces) {
            cursor.beginEditBlock();
            cursor.insertText("\n" + QString(spaceCount + 4, ' ') + "\n" + QString(spaceCount, ' '));
            cursor.movePosition(QTextCursor::PreviousBlock);
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.endEditBlock();
            setTextCursor(cursor);
            return;
        }

        QString trimmed = currentLine.trimmed();
        QPlainTextEdit::keyPressEvent(e);
        if (m_language == CodeLanguage::Python && trimmed.endsWith(':')) {
            spaceCount += 4;
        } else if ((m_language == CodeLanguage::CSharp || m_language == CodeLanguage::Generic) && trimmed.endsWith('{')) {
            spaceCount += 4;
        } else if (m_language == CodeLanguage::Json && (trimmed.endsWith('{') || trimmed.endsWith('['))) {
            spaceCount += 4;
        }
        if (spaceCount > 0) {
            insertPlainText(QString(spaceCount, ' '));
        }
        return;
    }

    if (m_completer && m_completer->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return;
        default:
            break;
        }
    }

    const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Space);
    if (!m_completer || !isShortcut) {
        QPlainTextEdit::keyPressEvent(e);
    }

    const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
                             e->modifiers().testFlag(Qt::ShiftModifier);
    if (!m_completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static const QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // End of word
    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 2
                      || eow.contains(e->text().right(1)))) {
        m_completer->popup()->hide();
        return;
    }

    if (completionPrefix != m_completer->completionPrefix()) {
        m_completer->setCompletionPrefix(completionPrefix);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                + m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(cr);
}

void CodeEditorWidget::setCompleter(QCompleter *completer)
{
    if (m_completer)
        m_completer->disconnect(this);

    m_completer = completer;
    if (!m_completer) return;

    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditorWidget::insertCompletion);
}

QCompleter *CodeEditorWidget::completer() const
{
    return m_completer;
}

void CodeEditorWidget::insertCompletion(const QString& completion)
{
    if (m_completer->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CodeEditorWidget::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CodeEditorWidget::focusInEvent(QFocusEvent *e)
{
    if (m_completer)
        m_completer->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditorWidget::updateCompleterWords()
{
    if (!m_completer) return;

    static const QStringList pythonKeywords = {
        // Standard Python keywords
        "False", "None", "True", "and", "as", "assert", "async", "await",
        "break", "class", "continue", "def", "del", "elif", "else", "except",
        "finally", "for", "from", "global", "if", "import", "in", "is",
        "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
        "while", "with", "yield",
        // Extended scripting keywords requested by user
        "var", "func", "let", "const", "function",
        // Builtin functions and types
        "int", "float", "complex", "str", "string", "char", "bool", "bytes", "bytearray",
        "list", "tuple", "set", "frozenset", "dict", "range", "type", "object",
        "print", "len", "enumerate", "zip", "map", "filter", "sorted", "reversed",
        "sum", "min", "max", "abs", "round", "open", "super", "self", "cls",
        "hasattr", "getattr", "setattr", "isinstance", "issubclass",
        "Exception", "ValueError", "TypeError", "RuntimeError", "KeyError", "IndexError"
    };

    static const QStringList csharpKeywords = {
        // Keywords
        "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char",
        "checked", "class", "const", "continue", "decimal", "default", "delegate",
        "do", "double", "else", "enum", "event", "explicit", "extern", "false",
        "finally", "fixed", "float", "for", "foreach", "goto", "if", "implicit",
        "in", "int", "interface", "internal", "is", "lock", "long", "namespace",
        "new", "null", "object", "operator", "out", "override", "params",
        "private", "protected", "public", "readonly", "record", "ref", "return",
        "sbyte", "sealed", "short", "sizeof", "stackalloc", "static", "string",
        "struct", "switch", "this", "throw", "true", "try", "typeof", "uint",
        "ulong", "unchecked", "unsafe", "ushort", "using", "virtual", "void",
        "volatile", "while", "yield", "async", "await", "var", "dynamic",
        "get", "set", "init", "value", "when", "where", "with", "global", "partial",
        // Common BCL Types and Global Commands
        "Console", "WriteLine", "Write", "ReadLine", "ReadKey", "Clear",
        "Math", "Abs", "Min", "Max", "Round", "Sqrt", "Pow", "Sin", "Cos",
        "String", "IsNullOrEmpty", "IsNullOrWhiteSpace", "Join", "Format", "Concat",
        "List", "Dictionary", "HashSet", "Queue", "Stack", "IEnumerable", "IList", "IDictionary",
        "Task", "Run", "WhenAll", "WhenAny", "Delay", "FromResult",
        "Action", "Func", "Predicate", "EventHandler",
        "DateTime", "Now", "UtcNow", "Today", "TimeSpan", "FromSeconds", "FromMinutes", "FromHours", "FromDays",
        "Guid", "NewGuid", "Convert", "ToInt32", "ToString", "ToDouble", "ToBoolean",
        "File", "ReadAllText", "WriteAllText", "ReadAllLines", "WriteAllLines", "Exists", "Delete",
        "Directory", "CreateDirectory", "GetFiles", "GetDirectories",
        "Path", "Combine", "GetFileName", "GetExtension", "GetDirectoryName",
        "Stream", "StreamReader", "StreamWriter", "MemoryStream", "FileStream",
        "StringBuilder", "Thread", "Sleep", "Array", "Exception",
        "ArgumentNullException", "ArgumentException", "InvalidOperationException", "NotImplementedException"
    };

    static const QStringList jsonKeywords = {
        "true", "false", "null"
    };

    QSet<QString> wordSet;
    if (m_language == CodeLanguage::CSharp) {
        wordSet = QSet<QString>(csharpKeywords.begin(), csharpKeywords.end());
    } else if (m_language == CodeLanguage::Json) {
        wordSet = QSet<QString>(jsonKeywords.begin(), jsonKeywords.end());
    } else {
        wordSet = QSet<QString>(pythonKeywords.begin(), pythonKeywords.end());
    }

    // Extract all identifier words and quoted keys from current document
    QString docText = toPlainText();
    QRegularExpression wordRegex(R"(\b[A-Za-z_][A-Za-z0-9_]*\b)");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(docText);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        wordSet.insert(m.captured(0));
    }

    if (m_language == CodeLanguage::Json) {
        QRegularExpression keyRegex("\"([A-Za-z0-9_.-]+)\"\\s*:");
        QRegularExpressionMatchIterator itKey = keyRegex.globalMatch(docText);
        while (itKey.hasNext()) {
            wordSet.insert(itKey.next().captured(1));
        }
    }

    QStringList wordList = wordSet.values();
    wordList.sort(Qt::CaseInsensitive);

    auto* model = qobject_cast<QStringListModel*>(m_completer->model());
    if (!model) {
        model = new QStringListModel(wordList, m_completer);
        m_completer->setModel(model);
    } else {
        model->setStringList(wordList);
    }
}


