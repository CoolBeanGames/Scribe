#pragma once
#include "EditorBase.h"
#include <QWidget>
#include <QTableWidget>
#include <QString>

// SpreadsheetEditor: handles .csv files using QTableWidget
// Supports arrow key navigation, Tab/Shift+Tab, Enter/Shift+Enter
class SpreadsheetEditor : public QWidget, public EditorBase {
    Q_OBJECT
public:
    explicit SpreadsheetEditor(QWidget* parent = nullptr);
    ~SpreadsheetEditor() override = default;

    // EditorBase interface
    DocumentType documentType() const override { return DocumentType::Spreadsheet; }
    bool isModified() const override { return m_modified; }
    void setModified(bool modified) override;
    QString filePath() const override { return m_filePath; }
    void setFilePath(const QString& path) override;
    bool loadFile(const QString& path) override;
    bool saveFile() override;
    bool saveFileAs(const QString& path) override;
    QWidget* widget() override { return this; }
    QString displayName() const override;

    // Access table widget
    QTableWidget* tableWidget() const { return m_table; }

    // Spreadsheet operations
    void addRow();
    void addColumn();
    void deleteRow();
    void deleteColumn();

signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString& path);

private:
    QTableWidget* m_table    = nullptr;
    QString       m_filePath;
    bool          m_modified = false;

    // CSV parsing
    QStringList parseCsvLine(const QString& line) const;
    QString toCsvField(const QString& value) const;

    // Update column headers (A, B, C...)
    void updateColumnHeaders();
    void ensureMinSize(int rows, int cols);

private slots:
    void onCellChanged();
};
