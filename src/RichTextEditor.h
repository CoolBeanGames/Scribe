#pragma once
#include "EditorBase.h"
#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QTextCharFormat>

// RichTextEditor: handles .rtf files using QTextEdit
// Provides all rich text formatting operations
class RichTextEditor : public QWidget, public EditorBase {
    Q_OBJECT
public:
    explicit RichTextEditor(QWidget* parent = nullptr);
    ~RichTextEditor() override = default;

    // EditorBase interface
    DocumentType documentType() const override { return DocumentType::RichText; }
    bool isModified() const override;
    void setModified(bool modified) override;
    QString filePath() const override { return m_filePath; }
    void setFilePath(const QString& path) override;
    bool loadFile(const QString& path) override;
    bool saveFile() override;
    bool saveFileAs(const QString& path) override;
    QWidget* widget() override { return this; }
    QString displayName() const override;
    void undo() override;
    void redo() override;
    bool canUndo() const override;
    bool canRedo() const override;

    // Rich text formatting
    void setBold(bool bold);
    void setItalic(bool italic);
    void setUnderline(bool underline);
    void setFontFamily(const QString& family);
    void setFontSize(int size);
    void setFontColor(const QColor& color);
    void setAlignment(Qt::Alignment alignment);
    void setHeadingStyle(int level);  // 1-6 for H1-H6
    void insertTable(int rows, int cols);
    void insertImage(const QString& imagePath);
    void insertHyperlink(const QString& url, const QString& text);

    // Query current char format
    QTextCharFormat currentCharFormat() const;
    Qt::Alignment currentAlignment() const;

    // Access the underlying QTextEdit for toolbar connections
    QTextEdit* textEdit() const { return m_editor; }

signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString& path);
    void cursorPositionChanged();

private:
    QTextEdit* m_editor    = nullptr;
    QString    m_filePath;
    bool       m_modified  = false;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onDocumentModified();
};

