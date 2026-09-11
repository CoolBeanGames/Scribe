#include <QMenu>
#include "SpreadsheetEditor.h"
#include <QLineEdit>
#include <QHeaderView>
#include <QKeyEvent>
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
    m_formulaBar = new QLineEdit(this);
    m_formulaBar->setPlaceholderText("Formula (press = to start)");
    m_formulaBar->hide();

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

    layout->addWidget(m_formulaBar);
    layout->addWidget(m_table);

    m_table->installEventFilter(this);
    m_table->viewport()->installEventFilter(this);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->viewport()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_table, &QWidget::customContextMenuRequested, this, &SpreadsheetEditor::onCustomContextMenu);
    connect(m_table->viewport(), &QWidget::customContextMenuRequested, this, &SpreadsheetEditor::onCustomContextMenu);
    m_formulaBar->installEventFilter(this);
    connect(m_table, &QTableWidget::cellClicked, this, &SpreadsheetEditor::onCellClicked);

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
    insertRowAt(m_table->currentRow() >= 0 ? m_table->currentRow() + 1 : m_table->rowCount());
}

void SpreadsheetEditor::addColumn()
{
    insertColumnAt(m_table->currentColumn() >= 0 ? m_table->currentColumn() + 1 : m_table->columnCount());
}

void SpreadsheetEditor::deleteRow()
{
    deleteRowAt(m_table->currentRow());
}

void SpreadsheetEditor::deleteColumn()
{
    deleteColumnAt(m_table->currentColumn());
}

void SpreadsheetEditor::insertRowAt(int row)
{
    if (row < 0 || row > m_table->rowCount()) {
        row = m_table->rowCount();
    }
    m_table->insertRow(row);
    for (int r = 0; r < m_table->rowCount(); ++r) {
        m_table->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
    }
    setModified(true);
}

void SpreadsheetEditor::insertColumnAt(int col)
{
    if (col < 0 || col > m_table->columnCount()) {
        col = m_table->columnCount();
    }
    m_table->insertColumn(col);
    updateColumnHeaders();
    setModified(true);
}

void SpreadsheetEditor::deleteRowAt(int row)
{
    if (row >= 0 && row < m_table->rowCount()) {
        m_table->removeRow(row);
        for (int r = 0; r < m_table->rowCount(); ++r) {
            m_table->setVerticalHeaderItem(r, new QTableWidgetItem(QString::number(r + 1)));
        }
        setModified(true);
    }
}

void SpreadsheetEditor::deleteColumnAt(int col)
{
    if (col >= 0 && col < m_table->columnCount()) {
        m_table->removeColumn(col);
        updateColumnHeaders();
        setModified(true);
    }
}

void SpreadsheetEditor::setCellColor(const QColor& color)
{
    QList<QTableWidgetItem*> items = m_table->selectedItems();
    for (QTableWidgetItem* item : items) {
        if (!item) {
            // QTableWidget might return null items if they haven't been edited
            int r = item ? item->row() : 0; // wait, if item is null we can't get row
            // We should just iterate rows/cols of selection
        }
    }
    // Correct way for QTableWidget:
    QList<QTableWidgetSelectionRange> ranges = m_table->selectedRanges();
    bool changed = false;
    for (auto range : ranges) {
        for (int r = range.topRow(); r <= range.bottomRow(); ++r) {
            for (int c = range.leftColumn(); c <= range.rightColumn(); ++c) {
                QTableWidgetItem* it = m_table->item(r, c);
                if (!it) {
                    it = new QTableWidgetItem();
                    m_table->setItem(r, c, it);
                }
                it->setBackground(color);
                changed = true;
            }
        }
    }
    if (changed) setModified(true);
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



bool SpreadsheetEditor::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_table->viewport() || obj == m_table) {
        if (event->type() == QEvent::MouseButtonPress && m_inFormulaMode) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint pos = (obj == m_table) ? mouseEvent->position().toPoint() : mouseEvent->position().toPoint();
            int row = m_table->rowAt(pos.y());
            int col = m_table->columnAt(pos.x());
            if (row >= 0 && col >= 0) {
                QString cellName = getCellName(row, col);
                m_formulaBar->setText(m_formulaBar->text() + cellName);
                m_formulaBar->setFocus();
                return true; // Eat mouse press so table doesn't change selection or start editing
            }
        }
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (!m_inFormulaMode && keyEvent->key() == Qt::Key_Equal && (obj == m_table || obj == m_table->viewport())) {
            enterFormulaMode();
            return true;
        } else if (m_inFormulaMode && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            exitFormulaMode(true);
            return true;
        } else if (m_inFormulaMode && keyEvent->key() == Qt::Key_Escape) {
            exitFormulaMode(false);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void SpreadsheetEditor::enterFormulaMode() {
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_inFormulaMode = true;
    m_formulaRow = m_table->currentRow();
    m_formulaCol = m_table->currentColumn();
    m_formulaBar->show();
    m_formulaBar->setText("=");
    m_formulaBar->setFocus();
}

void SpreadsheetEditor::exitFormulaMode(bool apply) {
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    if (apply && m_formulaRow >= 0 && m_formulaCol >= 0) {
        QTableWidgetItem* item = m_table->item(m_formulaRow, m_formulaCol);
        if (!item) {
            item = new QTableWidgetItem();
            m_table->setItem(m_formulaRow, m_formulaCol, item);
        }
        double val = evaluateExpression(m_formulaBar->text());
        item->setText(QString::number(val));
        setModified(true);
    }
    m_inFormulaMode = false;
    m_formulaBar->hide();
    m_table->setFocus();
}

double SpreadsheetEditor::evaluateExpression(const QString& expr) const {
    if (!expr.startsWith("=")) return 0;
    QString exp = expr.mid(1);
    
    if (exp.contains("+")) {
        QStringList parts = exp.split("+");
        return evaluateExpression("=" + parts[0]) + evaluateExpression("=" + parts[1]);
    }
    if (exp.contains("-")) {
        QStringList parts = exp.split("-");
        return evaluateExpression("=" + parts[0]) - evaluateExpression("=" + parts[1]);
    }
    if (exp.contains("*")) {
        QStringList parts = exp.split("*");
        return evaluateExpression("=" + parts[0]) * evaluateExpression("=" + parts[1]);
    }
    if (exp.contains("/")) {
        QStringList parts = exp.split("/");
        double denom = evaluateExpression("=" + parts[1]);
        return denom == 0 ? 0 : evaluateExpression("=" + parts[0]) / denom;
    }
    
    bool ok;
    double val = exp.toDouble(&ok);
    if (ok) return val;

    for (int r = 0; r < m_table->rowCount(); ++r) {
        for (int c = 0; c < m_table->columnCount(); ++c) {
            if (getCellName(r, c) == exp) {
                QTableWidgetItem* item = m_table->item(r, c);
                if (item) return item->text().toDouble();
                return 0;
            }
        }
    }
    return 0;
}

QString SpreadsheetEditor::getCellName(int r, int c) const {
    QString header;
    int n = c;
    do {
        header.prepend(QChar('A' + (n % 26)));
        n = n / 26 - 1;
    } while (n >= 0);
    return header + QString::number(r + 1);
}

void SpreadsheetEditor::onCellClicked(int row, int column) {
    if (m_inFormulaMode) {
        QString cellName = getCellName(row, column);
        m_formulaBar->setText(m_formulaBar->text() + cellName);
    }
}

void SpreadsheetEditor::onCustomContextMenu(const QPoint& pos) {
    int clickedRow = m_table->rowAt(pos.y());
    int clickedCol = m_table->columnAt(pos.x());

    if (clickedRow < 0) clickedRow = m_table->currentRow();
    if (clickedCol < 0) clickedCol = m_table->currentColumn();

    QMenu menu(this);

    if (clickedRow >= 0) {
        QAction* actInsertRowAbove = menu.addAction(QString("Insert Row Above (Row %1)").arg(clickedRow + 1));
        connect(actInsertRowAbove, &QAction::triggered, this, [this, clickedRow]() {
            insertRowAt(clickedRow);
        });

        QAction* actInsertRowBelow = menu.addAction(QString("Insert Row Below (Row %1)").arg(clickedRow + 2));
        connect(actInsertRowBelow, &QAction::triggered, this, [this, clickedRow]() {
            insertRowAt(clickedRow + 1);
        });

        QAction* actDeleteRow = menu.addAction(QString("Delete Row %1").arg(clickedRow + 1));
        connect(actDeleteRow, &QAction::triggered, this, [this, clickedRow]() {
            deleteRowAt(clickedRow);
        });
    }

    if (clickedCol >= 0) {
        if (clickedRow >= 0) menu.addSeparator();

        QString colHeader = getCellName(0, clickedCol);
        colHeader.chop(1); // remove row number '1' to get column letter(s)

        QAction* actInsertColLeft = menu.addAction(QString("Insert Column Left (%1)").arg(colHeader));
        connect(actInsertColLeft, &QAction::triggered, this, [this, clickedCol]() {
            insertColumnAt(clickedCol);
        });

        QAction* actInsertColRight = menu.addAction(QString("Insert Column Right"));
        connect(actInsertColRight, &QAction::triggered, this, [this, clickedCol]() {
            insertColumnAt(clickedCol + 1);
        });

        QAction* actDeleteCol = menu.addAction(QString("Delete Column %1").arg(colHeader));
        connect(actDeleteCol, &QAction::triggered, this, [this, clickedCol]() {
            deleteColumnAt(clickedCol);
        });
    }

    menu.addSeparator();
    QAction* actClear = menu.addAction("Clear Contents");
    connect(actClear, &QAction::triggered, this, [this]() {
        for (auto* item : m_table->selectedItems()) {
            item->setText("");
        }
        setModified(true);
    });

    menu.exec(m_table->viewport()->mapToGlobal(pos));
}

void SpreadsheetEditor::buildContextMenu(QMenu* menu)
{
    menu->addSeparator();
    QAction* actClear = menu->addAction("Clear Contents");
    connect(actClear, &QAction::triggered, this, [this]() {
        for (auto* item : m_table->selectedItems()) {
            item->setText("");
        }
        setModified(true);
    });
}

