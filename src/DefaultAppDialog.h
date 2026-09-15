#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include "FileAssociations.h"

class DefaultAppDialog : public QDialog {
    Q_OBJECT
public:
    explicit DefaultAppDialog(QWidget* parent = nullptr);

private slots:
    void onSelectAll();
    void onSelectNone();
    void onSetAsDefault();
    void onRegister();
    void onOpenWindowsSettings();
    void refreshStatuses();

private:
    QTableWidget* m_table = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_btnSetDefault = nullptr;
    QPushButton* m_btnRegister = nullptr;
    QPushButton* m_btnSettings = nullptr;
    QList<FileTypeAssociation> m_types;
};
