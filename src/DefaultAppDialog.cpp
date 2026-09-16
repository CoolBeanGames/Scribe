#include "DefaultAppDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

DefaultAppDialog::DefaultAppDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Default Application & File Associations");
    setModal(true);
    resize(680, 500);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    auto* titleLabel = new QLabel("Windows File Associations", this);
    titleLabel->setObjectName("DialogTitle");
    mainLayout->addWidget(titleLabel);

    auto* descLabel = new QLabel(
        "Choose file types to open directly with Scribe as the default Windows application, "
        "or register Scribe in the Windows 'Open with' menu.", this);
    descLabel->setWordWrap(true);
    descLabel->setObjectName("DialogDescription");
    mainLayout->addWidget(descLabel);

    // Table of file types
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"File Type", "Description", "Category", "Status"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(false);
    m_table->setObjectName("DefaultAppsTable");

    m_types = FileAssociations::supportedTypes();
    m_table->setRowCount(m_types.size());

    for (int i = 0; i < m_types.size(); ++i) {
        const auto& item = m_types[i];

        auto* checkItem = new QTableWidgetItem(item.extension);
        checkItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        checkItem->setCheckState(Qt::Checked);
        m_table->setItem(i, 0, checkItem);

        auto* nameItem = new QTableWidgetItem(item.name);
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_table->setItem(i, 1, nameItem);

        auto* catItem = new QTableWidgetItem(item.category);
        catItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        catItem->setForeground(QBrush(QColor("#8A95A8")));
        m_table->setItem(i, 2, catItem);

        auto* statusItem = new QTableWidgetItem("Checking...");
        statusItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_table->setItem(i, 3, statusItem);
    }
    mainLayout->addWidget(m_table);

    // Selection buttons
    auto* selLayout = new QHBoxLayout();
    auto* btnSelectAll = new QPushButton("Select All", this);
    auto* btnSelectNone = new QPushButton("Select None", this);
    btnSelectAll->setObjectName("CompactButton");
    btnSelectNone->setObjectName("CompactButton");
    connect(btnSelectAll, &QPushButton::clicked, this, &DefaultAppDialog::onSelectAll);
    connect(btnSelectNone, &QPushButton::clicked, this, &DefaultAppDialog::onSelectNone);
    selLayout->addWidget(btnSelectAll);
    selLayout->addWidget(btnSelectNone);
    selLayout->addStretch();
    mainLayout->addLayout(selLayout);

    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("DialogStatus");
    mainLayout->addWidget(m_statusLabel);

    // Action button row
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_btnSetDefault = new QPushButton("Set as Default Application", this);
    m_btnSetDefault->setObjectName("PrimaryButton");
    connect(m_btnSetDefault, &QPushButton::clicked, this, &DefaultAppDialog::onSetAsDefault);
    btnLayout->addWidget(m_btnSetDefault);

    m_btnRegister = new QPushButton("Register in 'Open with' Menu", this);
    m_btnRegister->setObjectName("SecondaryButton");
    connect(m_btnRegister, &QPushButton::clicked, this, &DefaultAppDialog::onRegister);
    btnLayout->addWidget(m_btnRegister);

    m_btnSettings = new QPushButton("Windows Settings", this);
    m_btnSettings->setObjectName("SecondaryButton");
    connect(m_btnSettings, &QPushButton::clicked, this, &DefaultAppDialog::onOpenWindowsSettings);
    btnLayout->addWidget(m_btnSettings);

    btnLayout->addStretch();

    auto* btnClose = new QPushButton("Close", this);
    btnClose->setObjectName("SecondaryButton");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(btnClose);

    mainLayout->addLayout(btnLayout);

    refreshStatuses();
}

void DefaultAppDialog::onSelectAll() {
    for (int i = 0; i < m_table->rowCount(); ++i) {
        if (auto* item = m_table->item(i, 0)) {
            item->setCheckState(Qt::Checked);
        }
    }
}

void DefaultAppDialog::onSelectNone() {
    for (int i = 0; i < m_table->rowCount(); ++i) {
        if (auto* item = m_table->item(i, 0)) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void DefaultAppDialog::refreshStatuses() {
    for (int i = 0; i < m_types.size(); ++i) {
        const auto& item = m_types[i];
        bool isReg = FileAssociations::isTypeRegistered(item.extension);
        bool isDef = FileAssociations::isTypeDefault(item.extension);

        QString statusText;
        QColor statusColor;

        if (isDef) {
            statusText = "Current Default";
            statusColor = QColor("#57742F"); // Zen subtle green
        } else if (isReg) {
            statusText = "Registered (Open With)";
            statusColor = QColor("#8B7CFF"); // Zen violet
        } else {
            statusText = "Not Registered";
            statusColor = QColor("#8A95A8"); // Muted grey
        }

        if (auto* sItem = m_table->item(i, 3)) {
            sItem->setText(statusText);
            sItem->setForeground(QBrush(statusColor));
        }
    }
}

void DefaultAppDialog::onSetAsDefault() {
    QList<QString> selected;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        auto* item = m_table->item(i, 0);
        if (item && item->checkState() == Qt::Checked) {
            selected.append(item->text());
        }
    }

    if (selected.isEmpty()) {
        m_statusLabel->setText("No file types selected.");
        return;
    }

    bool success = FileAssociations::setAsDefault(selected);
    if (success) {
        m_statusLabel->setText(QString("Successfully set Scribe as the default application for %1 file type(s).").arg(selected.size()));
    } else {
        m_statusLabel->setText("Failed to set default application for some types.");
    }
    refreshStatuses();
}

void DefaultAppDialog::onRegister() {
    QList<QString> selected;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        auto* item = m_table->item(i, 0);
        if (item && item->checkState() == Qt::Checked) {
            selected.append(item->text());
        }
    }

    if (selected.isEmpty()) {
        m_statusLabel->setText("No file types selected.");
        return;
    }

    bool success = FileAssociations::registerTypes(selected);
    if (success) {
        m_statusLabel->setText(QString("Successfully registered %1 file type(s) in Windows.").arg(selected.size()));
    } else {
        m_statusLabel->setText("Failed to register some file types.");
    }
    refreshStatuses();
}

void DefaultAppDialog::onOpenWindowsSettings() {
    if (!FileAssociations::launchAdvancedAssociationUI()) {
        m_statusLabel->setText("Could not open Windows Default Apps settings automatically.");
    }
}
