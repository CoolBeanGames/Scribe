#pragma once
#include "EditorBase.h"
#include "CodeEditorWidget.h"
#include <QVBoxLayout>

class CodeEditor : public QWidget, public EditorBase {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent = nullptr);

    // EditorBase interface
    DocumentType documentType() const override { return DocumentType::Code; }
    QString displayName() const override;
    QWidget* widget() override { return this; }
    bool isModified() const override { return m_modified; }
    void setModified(bool modified) override { m_modified = modified; m_editor->document()->setModified(modified); }
    QString filePath() const override { return m_filePath; }
    void setFilePath(const QString& path) override { m_filePath = path; }
    bool loadFile(const QString& path) override;
    bool saveFile() override;
    bool saveFileAs(const QString& path) override;

    void undo() override { m_editor->undo(); }
    void redo() override { m_editor->redo(); }
    bool canUndo() const override { return m_editor->document()->isUndoAvailable(); }
    bool canRedo() const override { return m_editor->document()->isRedoAvailable(); }

private slots:
    void onCustomContextMenu(const QPoint& pos);
signals:
    void modificationChanged(bool modified);
    void runRequested();

private:
    CodeEditorWidget* m_editor;
    QString m_filePath;
    bool m_modified = false;
};

