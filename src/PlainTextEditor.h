#pragma once
#include "EditorBase.h"
#include <QPlainTextEdit>
#include <QString>
#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>

class PlainTextEditor;

class PlainTextLineNumberArea : public QWidget {
public:
    PlainTextLineNumberArea(PlainTextEditor* editor);
    QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    PlainTextEditor* m_editor;
};

// PlainTextEditor: handles .txt and .md files
// Uses QPlainTextEdit with auto-indent on Enter
class PlainTextEditor : public QPlainTextEdit, public EditorBase {
    Q_OBJECT
public:
    explicit PlainTextEditor(QWidget* parent = nullptr);
    ~PlainTextEditor() override = default;

    // EditorBase interface
    DocumentType documentType() const override { return DocumentType::PlainText; }
    bool isModified() const override;
    void setModified(bool modified) override;
    QString filePath() const override { return m_filePath; }
    void setFilePath(const QString& path) override;
    bool loadFile(const QString& path) override;
    bool saveFile() override;
    bool saveFileAs(const QString& path) override;
    QWidget* widget() override { return this; }
    QString displayName() const override;
    void undo() override { QPlainTextEdit::undo(); }
    void redo() override { QPlainTextEdit::redo(); }
    bool canUndo() const override;
    bool canRedo() const override;

    // Line numbers
    bool lineNumbersVisible() const { return m_showLineNumbers; }
    void setLineNumbersVisible(bool visible);
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent* event);

    // Font family and size
    void setEditorFontFamily(const QString& family);
    void setEditorFontSize(int pointSize);

signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString& path);
    void lineNumbersToggled(bool visible);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QString m_filePath;
    bool    m_modified = false;
    bool    m_showLineNumbers = false;
    PlainTextLineNumberArea* m_lineNumberArea = nullptr;

    QString getIndentForCurrentLine() const;

private slots:
    void onCustomContextMenu(const QPoint& pos);
    void onDocumentModified();
    void updateLineNumberAreaWidth(int newBlockCount = 0);
    void updateLineNumberArea(const QRect& rect, int dy);
};
