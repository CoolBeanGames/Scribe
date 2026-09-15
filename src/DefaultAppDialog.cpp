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
    resize(640, 480);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    auto* titleLabel = new QLabel("Windows File Associations", this);
    titleLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #F4F6FA;");
    mainLayout->addWidget(titleLabel);

    auto* descLabel = new QLabel(
        "Register Scribe with Windows so it appears in the 'Open with' menu and can be selected "
        "as the default application for supported file formats.", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("color: #8A95A8; font-size: 12px; margin-bottom: 4px;");
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
    m_table->setStyleSheet(
        "QTableWidget { background-color: #12151C; border: 1px solid #262C38; border-radius: 6px; } "
        "QHeaderView::section { background-color: #161A22; color: #ABB2BF; padding: 6px; border: none; border-bottom: 1px solid #262C38; } "
        "QTableWidget::item { padding: 4px 8px; color: #F4F6FA; } "
        "QTableWidget::item:selected { background-color: #2D3A52; }");

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
    btnSelectAll->setStyleSheet("background-color: #1D222C; border: 1px solid #262C38; padding: 5px 12px; border-radius: 4px; color: #F4F6FA;");
    btnSelectNone->setStyleSheet("background-color: #1D222C; border: 1px solid #262C38; padding: 5px 12px; border-radius: 4px; color: #F4F6FA;");
    connect(btnSelectAll, &QPushButton::clicked, this, &DefaultAppDialog::onSelectAll);
    connect(btnSelectNone, &QPushButton::clicked, this, &DefaultAppDialog::onSelectNone);
    selLayout->addWidget(btnSelectAll);
    selLayout->addWidget(btnSelectNone);
    selLayout->addStretch();
    mainLayout->addLayout(selLayout);

    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #8B7CFF; font-size: 12px;");
    mainLayout->addWidget(m_statusLabel);

    // Action button row
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_btnRegister = new QPushButton("Register in Windows", this);
    m_btnRegister->setStyleSheet(
        "QPushButton { background-color: #8B7CFF; color: #0B0D12; font-weight: 600; padding: 8px 16px; border-radius: 6px; border: none; } "
        "QPushButton:hover { background-color: #9D91FF; } "
        "QPushButton:pressed { background-color: #796AE8; }");
    connect(m_btnRegister, &QPushButton::clicked, this, &DefaultAppDialog::onRegister);
    btnLayout->addWidget(m_btnRegister);

    m_btnSettings = new QPushButton("Open Windows Default Apps Settings", this);
    m_btnSettings->setStyleSheet(
        "QPushButton { background-color: #1D222C; color: #F4F6FA; border: 1px solid #262C38; padding: 8px 16px; border-radius: 6px; } "
        "QPushButton:hover { background-color: #262C38; }");
    connect(m_btnSettings, &QPushButton::clicked, this, &DefaultAppDialog::onOpenWindowsSettings);
    btnLayout->addWidget(m_btnSettings);

    btnLayout->addStretch();

    auto* btnClose = new QPushButton("Close", this);
    btnClose->setStyleSheet(
        "QPushButton { background-color: #1D222C; color: #ABB2BF; border: 1px solid #262C38; padding: 8px 16px; border-radius: 6px; } "
        "QPushButton:hover { background-color: #262C38; color: #F4F6FA; }");
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
            statusText = "Registered";
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

void DefaultAppDialog::onRegister() {
    QList<QString> selectedExts;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        if (auto* item = m_table->item(i, 0)) {
            if (item->checkState() == Qt::Checked) {
                selectedExts.append(item->text());
            }
        }
    }

    if (selectedExts.isEmpty()) {
        m_statusLabel->setText("No file types selected.");
        m_statusLabel->setStyleSheet("color: #F48771;");
        return;
    }

    bool ok = FileAssociations::registerTypes(selectedExts);
    if (ok) {
        m_statusLabel->setText(QString("Successfully registered %1 file type(s) with Windows!").arg(selectedExts.size()));
        m_statusLabel->setStyleSheet("color: #8B7CFF;");
        refreshStatuses();
    } else {
        m_statusLabel->setText("Registration failed.");
        m_statusLabel->setStyleSheet("color: #F48771;");
    }
}

void DefaultAppDialog::onOpenWindowsSettings() {
    bool ok = FileAssociations::launchAdvancedAssociationUI();
    if (!ok) {
        m_statusLabel->setText("Could not open Windows Settings directly.");
        m_statusLabel->setStyleSheet("color: #F48771;");
    }
}
