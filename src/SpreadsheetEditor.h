#pragma once
#include "EditorBase.h"
#include <QWidget>
#include <QTableWidget>
#include <QString>
#include <QLineEdit>

class SpreadsheetEditor : public QWidget, public EditorBase {
    Q_OBJECT
public:
    explicit SpreadsheetEditor(QWidget* parent = nullptr);
    ~SpreadsheetEditor() override = default;

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

    QTableWidget* tableWidget() const { return m_table; }

    void addRow();
    void addColumn();
    void deleteRow();
    void deleteColumn();
    void setCellColor(const QColor& color);

signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString& path);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QTableWidget* m_table = nullptr;
    QLineEdit* m_formulaBar = nullptr;
    QString m_filePath;
    bool m_modified = false;

    bool m_inFormulaMode = false;
    int m_formulaRow = -1;
    int m_formulaCol = -1;
    void enterFormulaMode();
    void exitFormulaMode(bool apply);
    void buildContextMenu(QMenu* menu) override;

    QStringList parseCsvLine(const QString& line) const;
    QString toCsvField(const QString& value) const;
    void updateColumnHeaders();
    void ensureMinSize(int rows, int cols);
    QString getCellName(int r, int c) const;
    double evaluateExpression(const QString& expr) const;

private slots:
    void onCustomContextMenu(const QPoint& pos);
    void onCellChanged();
    void onCellClicked(int row, int column);
};




