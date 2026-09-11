#pragma once
#include <QPlainTextEdit>
#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>
#include "PythonHighlighter.h"

class QCompleter;

class CodeEditorWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditorWidget(QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;

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

    QWidget *lineNumberArea;
    PythonHighlighter *highlighter;
    QCompleter *m_completer = nullptr;
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
