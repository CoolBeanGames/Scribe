#pragma once
#include <QPlainTextEdit>
#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QSyntaxHighlighter>

class QCompleter;
class QStringListModel;
class QTimer;

enum class CodeLanguage {
    Python,
    CSharp,
    Json,
    Html,
    Css,
    Xml,
    Generic
};

class CodeEditorWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditorWidget(QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;

    void setLanguage(CodeLanguage lang);
    CodeLanguage language() const { return m_language; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent* e) override;
    void focusInEvent(QFocusEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void insertCompletion(const QString &completion);
    void updateCompleterWords();

private:
    QString textUnderCursor() const;
    bool handleAutoPair(QKeyEvent* event);
    bool handleMarkupClosingTag(QKeyEvent* event);
    bool isInsideJsonString() const;

    QWidget *lineNumberArea = nullptr;
    QSyntaxHighlighter *m_highlighter = nullptr;
    QCompleter *m_completer = nullptr;
    QStringListModel *m_completionModel = nullptr;
    QTimer *m_completionTimer = nullptr;
    CodeLanguage m_language = CodeLanguage::Python;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditorWidget *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override { codeEditor->lineNumberAreaPaintEvent(event); }
private:
    CodeEditorWidget *codeEditor;
};
