#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>

// Simple dialog to ask for rows/columns when inserting a table
class InsertTableDialog : public QDialog {
    Q_OBJECT
public:
    explicit InsertTableDialog(QWidget* parent = nullptr);

    int rows() const;
    int columns() const;

private:
    QSpinBox* m_rowsSpin    = nullptr;
    QSpinBox* m_columnsSpin = nullptr;
};
