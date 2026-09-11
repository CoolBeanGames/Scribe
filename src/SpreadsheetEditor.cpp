#include "SpreadsheetEditor.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QApplication>

// -------------------------------------------------------------------
// Custom QTableWidget with overridden key handling
// -------------------------------------------------------------------
class SpreadsheetTable : public QTableWidget {
public:
    explicit SpreadsheetTable(int rows, int cols, QWidget* parent = nullptr)
        : QTableWidget(rows, cols, parent) {}

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        int row = currentRow();
        int col = currentColumn();
        bool shift = (event->modifiers() & Qt::ShiftModifier);

        switch (event->key()) {
        case Qt::Key_Tab:
            if (shift) {
                // Shift+Tab: go left
                if (col > 0) setCurrentCell(row, col - 1);
                else if (row > 0) setCurrentCell(row - 1, columnCount() - 1);
            } else {
                // Tab: go right
                if (col < columnCount() - 1) setCurrentCell(row, col + 1);
                else if (row < rowCount() - 1) setCurrentCell(row + 1, 0);
            }
            return;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (shift) {
                // Shift+Enter: go up
                if (row > 0) setCurrentCell(row - 1, col);
            } else {
                // Enter: go down
                if (row < rowCount() - 1) setCurrentCell(row + 1, col);
            }
            return;

        default:
            break;
        }

        QTableWidget::keyPressEvent(event);
    }
};

// -------------------------------------------------------------------
// SpreadsheetEditor
// -------------------------------------------------------------------
SpreadsheetEditor::SpreadsheetEditor(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_table = new SpreadsheetTable(50, 26, this);
    m_table->setAlternatingRowColors(true);

    // Row headers: 1, 2, 3...
    for (int r = 0; r < m_table->rowCount(); ++r) {
        m_table->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
    }

    // Column headers: A, B, C...
    updateColumnHeaders();

    // Default row/column sizes
    m_table->horizontalHeader()->setDefaultSectionSize(100);
    m_table->verticalHeader()->setDefaultSectionSize(24);
    m_table->verticalHeader()->setMinimumWidth(40);

    // Allow in-place editing
    m_table->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::AnyKeyPressed);

    layout->addWidget(m_table);

    connect(m_table, &QTableWidget::cellChanged, this, &SpreadsheetEditor::onCellChanged);
}

void SpreadsheetEditor::setModified(bool modified)
{
    m_modified = modified;
    emit modificationChanged(modified);
}

void SpreadsheetEditor::setFilePath(const QString& path)
{
    m_filePath = path;
    emit filePathChanged(path);
}

bool SpreadsheetEditor::loadFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Error",
            QString("Cannot open file:\n%1\n\n%2").arg(path, file.errorString()));
        return false;
    }

    // Disconnect to avoid false modification signals while loading
    disconnect(m_table, &QTableWidget::cellChanged, this, &SpreadsheetEditor::onCellChanged);

    m_table->clearContents();

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    int row = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList fields = parseCsvLine(line);

        if (row >= m_table->rowCount()) {
            m_table->setRowCount(row + 1);
            m_table->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
        }
        if (fields.count() > m_table->columnCount()) {
            m_table->setColumnCount(fields.count());
            updateColumnHeaders();
        }

        for (int col = 0; col < fields.count(); ++col) {
            auto* item = new QTableWidgetItem(fields[col]);
            m_table->setItem(row, col, item);
        }
        ++row;
    }
    file.close();

    connect(m_table, &QTableWidget::cellChanged, this, &SpreadsheetEditor::onCellChanged);

    setFilePath(path);
    setModified(false);
    return true;
}

bool SpreadsheetEditor::saveFile()
{
    if (m_filePath.isEmpty()) return false;
    return saveFileAs(m_filePath);
}

bool SpreadsheetEditor::saveFileAs(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Error",
            QString("Cannot save file:\n%1\n\n%2").arg(path, file.errorString()));
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    for (int r = 0; r < m_table->rowCount(); ++r) {
        QStringList row;
        bool anyData = false;
        for (int c = 0; c < m_table->columnCount(); ++c) {
            QTableWidgetItem* item = m_table->item(r, c);
            QString val = item ? item->text() : QString();
            row << toCsvField(val);
            if (!val.isEmpty()) anyData = true;
        }
        if (anyData) {
            // Trim trailing empty fields
            while (!row.isEmpty() && row.last() == "") row.removeLast();
            stream << row.join(",") << "\n";
        } else {
            stream << "\n";
        }
    }

    file.close();

    setFilePath(path);
    setModified(false);
    return true;
}

QString SpreadsheetEditor::displayName() const
{
    if (m_filePath.isEmpty()) return "Untitled";
    return QFileInfo(m_filePath).fileName();
}

void SpreadsheetEditor::addRow()
{
    m_table->setRowCount(m_table->rowCount() + 1);
    int r = m_table->rowCount() - 1;
    m_table->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
    setModified(true);
}

void SpreadsheetEditor::addColumn()
{
    m_table->setColumnCount(m_table->columnCount() + 1);
    updateColumnHeaders();
    setModified(true);
}

void SpreadsheetEditor::deleteRow()
{
    int row = m_table->currentRow();
    if (row >= 0) {
        m_table->removeRow(row);
        // Refresh row headers
        for (int r = 0; r < m_table->rowCount(); ++r) {
            m_table->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
        }
        setModified(true);
    }
}

void SpreadsheetEditor::deleteColumn()
{
    int col = m_table->currentColumn();
    if (col >= 0) {
        m_table->removeColumn(col);
        updateColumnHeaders();
        setModified(true);
    }
}

QStringList SpreadsheetEditor::parseCsvLine(const QString& line) const
{
    QStringList result;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            result << field;
            field.clear();
        } else {
            field += c;
        }
    }
    result << field;
    return result;
}

QString SpreadsheetEditor::toCsvField(const QString& value) const
{
    if (value.contains(',') || value.contains('"') || value.contains('\n')) {
        return '"' + QString(value).replace("\"", "\"\"") + '"';
    }
    return value;
}

void SpreadsheetEditor::updateColumnHeaders()
{
    for (int c = 0; c < m_table->columnCount(); ++c) {
        // Excel-style: A, B, ... Z, AA, AB, ...
        QString header;
        int n = c;
        do {
            header.prepend(QChar('A' + (n % 26)));
            n = n / 26 - 1;
        } while (n >= 0);

        m_table->setHorizontalHeaderItem(c, new QTableWidgetItem(header));
    }
}

void SpreadsheetEditor::ensureMinSize(int rows, int cols)
{
    if (rows > m_table->rowCount()) {
        int old = m_table->rowCount();
        m_table->setRowCount(rows);
        for (int r = old; r < rows; ++r) {
            m_table->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
        }
    }
    if (cols > m_table->columnCount()) {
        m_table->setColumnCount(cols);
        updateColumnHeaders();
    }
}

void SpreadsheetEditor::onCellChanged()
{
    if (!m_modified) {
        setModified(true);
    }
}
