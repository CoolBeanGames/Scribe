#include <QMenu>
#include <cmath>
#include "SpreadsheetEditor.h"
#include <QLineEdit>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMouseEvent>
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

    void setEditor(SpreadsheetEditor* ed) { m_editor = ed; }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QTableWidget::paintEvent(event);
        if (m_editor && m_editor->inFormulaMode()) {
            QPainter painter(viewport());
            m_editor->paintFormulaSelections(painter);
        }
    }
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

private:
    SpreadsheetEditor* m_editor = nullptr;
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

    auto* sheetTable = new SpreadsheetTable(50, 26, this);
    sheetTable->setEditor(this);
    m_table = sheetTable;
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



static const QList<QColor> FORMULA_COLORS = {
    QColor("#61AFEF"), // Blue
    QColor("#E06C75"), // Red
    QColor("#98C379"), // Green
    QColor("#E5C07B"), // Amber
    QColor("#C678DD"), // Purple
    QColor("#56B6C2"), // Cyan
    QColor("#FF79C6"), // Pink
    QColor("#8B7CFF")  // Violet
};

void SpreadsheetEditor::paintFormulaSelections(QPainter& p)
{
    if (!m_inFormulaMode) return;
    for (const auto& sel : m_formulaSelections) {
        if (sel.topRow < 0 || sel.leftCol < 0) continue;
        int t = qMax(0, sel.topRow);
        int b = qMin(m_table->rowCount() - 1, sel.bottomRow);
        int l = qMax(0, sel.leftCol);
        int r = qMin(m_table->columnCount() - 1, sel.rightCol);

        QRect r1 = m_table->visualRect(m_table->model()->index(t, l));
        QRect r2 = m_table->visualRect(m_table->model()->index(b, r));
        QRect bound = r1.united(r2);
        if (bound.isValid() && !bound.isEmpty()) {
            p.save();
            p.setRenderHint(QPainter::Antialiasing, false);
            p.setPen(QPen(sel.color, 2, Qt::SolidLine));
            QColor fillCol = sel.color;
            fillCol.setAlpha(35);
            p.setBrush(fillCol);
            p.drawRect(bound.adjusted(0, 0, -1, -1));
            p.restore();
        }
    }
}

bool SpreadsheetEditor::eventFilter(QObject* obj, QEvent* event) {
    if (obj == m_table->viewport() || obj == m_table) {
        if (m_inFormulaMode) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) {
                    QPoint pos = (obj == m_table) ? m_table->viewport()->mapFrom(m_table, me->pos()) : me->pos();
                    int row = m_table->rowAt(pos.y());
                    int col = m_table->columnAt(pos.x());
                    if (row >= 0 && col >= 0) {
                        m_isDraggingSelection = true;
                        m_dragStartRow = row;
                        m_dragStartCol = col;
                        FormulaSelection sel;
                        sel.topRow = row;
                        sel.bottomRow = row;
                        sel.leftCol = col;
                        sel.rightCol = col;
                        sel.color = FORMULA_COLORS[m_formulaSelections.size() % FORMULA_COLORS.size()];
                        m_formulaSelections.append(sel);
                        m_table->viewport()->update();
                        return true;
                    }
                }
            } else if (event->type() == QEvent::MouseMove && m_isDraggingSelection) {
                auto* me = static_cast<QMouseEvent*>(event);
                QPoint pos = (obj == m_table) ? m_table->viewport()->mapFrom(m_table, me->pos()) : me->pos();
                int row = m_table->rowAt(pos.y());
                int col = m_table->columnAt(pos.x());
                if (row >= 0 && col >= 0 && !m_formulaSelections.isEmpty()) {
                    FormulaSelection& sel = m_formulaSelections.last();
                    sel.topRow = qMin(m_dragStartRow, row);
                    sel.bottomRow = qMax(m_dragStartRow, row);
                    sel.leftCol = qMin(m_dragStartCol, col);
                    sel.rightCol = qMax(m_dragStartCol, col);
                    m_table->viewport()->update();
                    return true;
                }
            } else if (event->type() == QEvent::MouseButtonRelease && m_isDraggingSelection) {
                m_isDraggingSelection = false;
                if (!m_formulaSelections.isEmpty()) {
                    const FormulaSelection& sel = m_formulaSelections.last();
                    QString ref;
                    if (sel.topRow == sel.bottomRow && sel.leftCol == sel.rightCol) {
                        ref = getCellName(sel.topRow, sel.leftCol);
                    } else {
                        ref = getCellName(sel.topRow, sel.leftCol) + ":" + getCellName(sel.bottomRow, sel.rightCol);
                    }
                    int cursor = m_formulaBar->cursorPosition();
                    QString cur = m_formulaBar->text();
                    QString next = cur.left(cursor) + ref + cur.mid(cursor);
                    m_formulaBar->setText(next);
                    m_formulaBar->setCursorPosition(cursor + ref.length());
                    m_formulaBar->setFocus();
                    m_table->viewport()->update();
                    return true;
                }
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
    m_formulaSelections.clear();
    m_isDraggingSelection = false;
    m_formulaRow = m_table->currentRow();
    m_formulaCol = m_table->currentColumn();
    m_formulaBar->show();
    m_formulaBar->setText("=");
    m_formulaBar->setFocus();
    m_table->viewport()->update();
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
    m_formulaSelections.clear();
    m_isDraggingSelection = false;
    m_formulaBar->hide();
    m_table->viewport()->update();
    m_table->setFocus();
}

bool SpreadsheetEditor::parseCellCoord(const QString& ref, int& row, int& col) const
{
    QString trimmed = ref.trimmed().toUpper();
    int letterEnd = 0;
    while (letterEnd < trimmed.length() && trimmed[letterEnd].isLetter()) {
        letterEnd++;
    }
    if (letterEnd == 0 || letterEnd == trimmed.length()) return false;
    QString colStr = trimmed.left(letterEnd);
    QString rowStr = trimmed.mid(letterEnd);
    bool ok = false;
    int r = rowStr.toInt(&ok) - 1;
    if (!ok || r < 0) return false;

    int c = 0;
    for (int i = 0; i < colStr.length(); ++i) {
        c = c * 26 + (colStr[i].toLatin1() - 'A' + 1);
    }
    c -= 1;
    if (c < 0) return false;

    row = r;
    col = c;
    return true;
}

QList<double> SpreadsheetEditor::resolveValues(const QString& token) const
{
    QList<double> values;
    QString trimmed = token.trimmed();
    if (trimmed.isEmpty()) return values;

    // Range like A1:B5
    if (trimmed.contains(':')) {
        QStringList parts = trimmed.split(':');
        if (parts.size() == 2) {
            int r1, c1, r2, c2;
            if (parseCellCoord(parts[0], r1, c1) && parseCellCoord(parts[1], r2, c2)) {
                int topRow = qMin(r1, r2);
                int bottomRow = qMax(r1, r2);
                int leftCol = qMin(c1, c2);
                int rightCol = qMax(c1, c2);
                for (int r = topRow; r <= bottomRow && r < m_table->rowCount(); ++r) {
                    for (int c = leftCol; c <= rightCol && c < m_table->columnCount(); ++c) {
                        QTableWidgetItem* item = m_table->item(r, c);
                        if (item && !item->text().trimmed().isEmpty()) {
                            bool ok = false;
                            double v = item->text().trimmed().toDouble(&ok);
                            values.append(ok ? v : 0.0);
                        } else {
                            values.append(0.0);
                        }
                    }
                }
                return values;
            }
        }
    }

    // Single cell like A1
    int r, c;
    if (parseCellCoord(trimmed, r, c)) {
        if (r < m_table->rowCount() && c < m_table->columnCount()) {
            QTableWidgetItem* item = m_table->item(r, c);
            if (item && !item->text().trimmed().isEmpty()) {
                bool ok = false;
                double v = item->text().trimmed().toDouble(&ok);
                values.append(ok ? v : 0.0);
            } else {
                values.append(0.0);
            }
        }
        return values;
    }

    // Direct number
    bool ok = false;
    double directVal = trimmed.toDouble(&ok);
    if (ok) {
        values.append(directVal);
        return values;
    }

    double evalVal = evaluateExpression(trimmed.startsWith("=") ? trimmed : ("=" + trimmed));
    values.append(evalVal);
    return values;
}

static QStringList splitArgs(const QString& str)
{
    QStringList args;
    QString current;
    int depth = 0;
    for (int i = 0; i < str.length(); ++i) {
        QChar c = str[i];
        if (c == '(') depth++;
        else if (c == ')') depth--;
        else if (c == ',' && depth == 0) {
            args.append(current.trimmed());
            current.clear();
            continue;
        }
        current += c;
    }
    if (!current.trimmed().isEmpty()) {
        args.append(current.trimmed());
    }
    return args;
}

static int findOperatorOutsideParens(const QString& str, const QStringList& ops)
{
    int depth = 0;
    for (int i = str.length() - 1; i >= 0; --i) {
        QChar c = str[i];
        if (c == ')') depth++;
        else if (c == '(') depth--;
        else if (depth == 0) {
            for (const QString& op : ops) {
                if (str.mid(i, op.length()) == op) {
                    if (op == "-" && (i == 0 || str[i-1] == '+' || str[i-1] == '-' || str[i-1] == '*' || str[i-1] == '/' || str[i-1] == '(' || str[i-1] == ',')) {
                        continue;
                    }
                    return i;
                }
            }
        }
    }
    return -1;
}

double SpreadsheetEditor::evaluateExpression(const QString& expr) const {
    QString exp = expr.trimmed();
    if (exp.startsWith("=")) {
        exp = exp.mid(1).trimmed();
    }
    if (exp.isEmpty()) return 0;

    // Check operators outside parens (+ and - first for lower precedence)
    int addSubIdx = findOperatorOutsideParens(exp, {"+", "-"});
    if (addSubIdx > 0) {
        QChar op = exp[addSubIdx];
        QString left = exp.left(addSubIdx).trimmed();
        QString right = exp.mid(addSubIdx + 1).trimmed();
        if (op == '+') {
            return evaluateExpression("=" + left) + evaluateExpression("=" + right);
        } else {
            return evaluateExpression("=" + left) - evaluateExpression("=" + right);
        }
    }

    // Next * and /
    int mulDivIdx = findOperatorOutsideParens(exp, {"*", "/"});
    if (mulDivIdx > 0) {
        QChar op = exp[mulDivIdx];
        QString left = exp.left(mulDivIdx).trimmed();
        QString right = exp.mid(mulDivIdx + 1).trimmed();
        if (op == '*') {
            return evaluateExpression("=" + left) * evaluateExpression("=" + right);
        } else {
            double denom = evaluateExpression("=" + right);
            return (denom == 0) ? 0 : evaluateExpression("=" + left) / denom;
        }
    }

    // Outer parentheses
    if (exp.startsWith("(") && exp.endsWith(")")) {
        int depth = 0;
        bool allEnclosed = true;
        for (int i = 0; i < exp.length() - 1; ++i) {
            if (exp[i] == '(') depth++;
            else if (exp[i] == ')') depth--;
            if (depth == 0) { allEnclosed = false; break; }
        }
        if (allEnclosed) {
            return evaluateExpression("=" + exp.mid(1, exp.length() - 2));
        }
    }

    // Functions: SUM, AVG, EXP, SQRT
    QString upper = exp.toUpper();
    if (upper.startsWith("SUM(") && exp.endsWith(")")) {
        QString inner = exp.mid(4, exp.length() - 5);
        QStringList args = splitArgs(inner);
        double total = 0;
        for (const QString& a : args) {
            QList<double> vals = resolveValues(a);
            for (double v : vals) total += v;
        }
        return total;
    }
    if (upper.startsWith("AVG(") && exp.endsWith(")")) {
        QString inner = exp.mid(4, exp.length() - 5);
        QStringList args = splitArgs(inner);
        double total = 0;
        int count = 0;
        for (const QString& a : args) {
            QList<double> vals = resolveValues(a);
            for (double v : vals) {
                total += v;
                count++;
            }
        }
        return (count == 0) ? 0 : (total / count);
    }
    if (upper.startsWith("EXP(") && exp.endsWith(")")) {
        QString inner = exp.mid(4, exp.length() - 5);
        QStringList args = splitArgs(inner);
        if (args.size() >= 2) {
            double base = evaluateExpression("=" + args[0]);
            double exponent = evaluateExpression("=" + args[1]);
            return std::pow(base, exponent);
        } else if (args.size() == 1) {
            double val = evaluateExpression("=" + args[0]);
            return std::exp(val);
        }
        return 0;
    }
    if (upper.startsWith("SQRT(") && exp.endsWith(")")) {
        QString inner = exp.mid(5, exp.length() - 6);
        QStringList args = splitArgs(inner);
        if (!args.isEmpty()) {
            double val = evaluateExpression("=" + args[0]);
            return (val >= 0) ? std::sqrt(val) : 0;
        }
        return 0;
    }

    // Direct number
    bool ok = false;
    double val = exp.toDouble(&ok);
    if (ok) return val;

    // Single cell coordinate
    int r, c;
    if (parseCellCoord(exp, r, c)) {
        if (r < m_table->rowCount() && c < m_table->columnCount()) {
            QTableWidgetItem* item = m_table->item(r, c);
            if (item) {
                return item->text().trimmed().toDouble();
            }
        }
        return 0;
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
    Q_UNUSED(row);
    Q_UNUSED(column);
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

