#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>

// Simple dialog to insert a hyperlink (URL + display text)
class InsertLinkDialog : public QDialog {
    Q_OBJECT
public:
    explicit InsertLinkDialog(const QString& selectedText = QString(),
                              QWidget* parent = nullptr);

    QString url() const;
    QString displayText() const;

private:
    QLineEdit* m_urlEdit  = nullptr;
    QLineEdit* m_textEdit = nullptr;
};
