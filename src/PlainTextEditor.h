#pragma once
#include "EditorBase.h"
#include <QPlainTextEdit>
#include <QString>

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

signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString& path);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QString m_filePath;
    bool    m_modified = false;

    QString getIndentForCurrentLine() const;

private slots:
    void onDocumentModified();
};
