#include "InsertTableDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

InsertTableDialog::InsertTableDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Insert Table");
    setModal(true);
    setMinimumWidth(260);

    auto* layout = new QFormLayout(this);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setVerticalSpacing(10);

    m_rowsSpin = new QSpinBox(this);
    m_rowsSpin->setRange(1, 100);
    m_rowsSpin->setValue(3);
    layout->addRow("Rows:", m_rowsSpin);

    m_columnsSpin = new QSpinBox(this);
    m_columnsSpin->setRange(1, 26);
    m_columnsSpin->setValue(3);
    layout->addRow("Columns:", m_columnsSpin);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

int InsertTableDialog::rows() const    { return m_rowsSpin->value(); }
int InsertTableDialog::columns() const { return m_columnsSpin->value(); }
