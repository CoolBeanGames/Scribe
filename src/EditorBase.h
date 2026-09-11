#pragma once
#include <QWidget>
#include <QString>

// Document type enum
enum class DocumentType {
    PlainText,   // .txt, .md
    RichText,    // .rtf
    Spreadsheet, Code  // .csv
};

// Base interface for all editor widgets
class EditorBase {
public:
    virtual ~EditorBase() = default;

    // Returns the document type
    virtual DocumentType documentType() const = 0;

    // Returns true if the document has unsaved changes
    virtual bool isModified() const = 0;

    // Sets the modified state
    virtual void setModified(bool modified) = 0;

    // Returns the file path (empty if untitled)
    virtual QString filePath() const = 0;

    // Sets the file path
    virtual void setFilePath(const QString& path) = 0;

    // Load content from file
    virtual bool loadFile(const QString& path) = 0;

    // Save content to the current file path
    virtual bool saveFile() = 0;

    // Save content to a specific path
    virtual bool saveFileAs(const QString& path) = 0;

    // Returns the widget (for adding to tab widget)
    virtual QWidget* widget() = 0;

    // Returns the display name (filename or "Untitled")
    virtual QString displayName() const = 0;

    // Undo / Redo (may be no-op for types that don't support it)
    virtual void undo() {}
    virtual void redo() {}
    virtual bool canUndo() const { return false; }
    virtual bool canRedo() const { return false; }
};
