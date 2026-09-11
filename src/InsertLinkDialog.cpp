#include "InsertLinkDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>

InsertLinkDialog::InsertLinkDialog(const QString& selectedText, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Insert Hyperlink");
    setModal(true);
    setMinimumWidth(380);

    auto* layout = new QFormLayout(this);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setVerticalSpacing(10);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://example.com");
    layout->addRow("URL:", m_urlEdit);

    m_textEdit = new QLineEdit(this);
    m_textEdit->setPlaceholderText("Display text");
    if (!selectedText.isEmpty()) {
        m_textEdit->setText(selectedText);
    }
    layout->addRow("Text:", m_textEdit);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString InsertLinkDialog::url() const         { return m_urlEdit->text(); }
QString InsertLinkDialog::displayText() const { return m_textEdit->text(); }
