#pragma once
// Theme constants for Scribe's Zen design language

namespace Theme {
    inline const char* APP_STYLESHEET = R"(
QMainWindow, QDialog {
    background-color: #0B0D12;
    color: #F4F6FA;
}

QWidget {
    background-color: #0B0D12;
    color: #F4F6FA;
    font-family: "Segoe UI", system-ui, sans-serif;
    font-size: 13px;
}

/* Toolbar */
QToolBar {
    background-color: #0B0D12;
    border: none;
    spacing: 8px;
    padding: 8px 12px;
}
QToolBar::separator {
    background-color: #262C38;
    width: 1px;
    margin: 4px 12px;
}
QToolButton {
    background-color: transparent;
    color: #8A95A8;
    border: 1px solid transparent;
    border-radius: 6px;
    padding: 6px 10px;
    font-weight: 500;
}
QToolButton:hover {
    background-color: #161A22;
    color: #F4F6FA;
}
QToolButton:pressed, QToolButton:checked {
    background-color: #1D222C;
    color: #8B7CFF;
}
QToolButton:disabled {
    color: #4A5568;
}

/* Menu */
QMenuBar {
    background-color: #0B0D12;
    color: #8A95A8;
    border: none;
    padding: 4px 8px;
}
QMenuBar::item {
    background-color: transparent;
    padding: 6px 12px;
    border-radius: 6px;
    font-weight: 500;
}
QMenuBar::item:selected, QMenuBar::item:pressed {
    background-color: #161A22;
    color: #F4F6FA;
}
QMenu {
    background-color: #12151C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 8px;
    padding: 6px;
}
QMenu::item {
    padding: 6px 30px 6px 20px;
    border-radius: 6px;
}
QMenu::item:selected {
    background-color: #1D222C;
    color: #8B7CFF;
}
QMenu::item:disabled {
    color: #4A5568;
}
QMenu::separator {
    height: 1px;
    background: #262C38;
    margin: 6px 8px;
}

/* Tab Widget */
QTabWidget::pane {
    background-color: #0B0D12;
    border: none;
}
QTabBar::tab {
    background-color: #12151C;
    color: #8A95A8;
    padding: 8px 16px;
    border: 1px solid #262C38;
    border-radius: 8px;
    margin: 4px 4px 8px 4px;
    min-width: 100px;
    max-width: 250px;
}
QTabBar::tab:selected {
    background-color: #1D222C;
    color: #F4F6FA;
    border: 1px solid #8B7CFF;
}
QTabBar::tab:hover:!selected {
    background-color: #161A22;
    color: #F4F6FA;
}
QTabBar::close-button {
    image: none; /* Can be stylized if needed */
}

/* Editors */
QPlainTextEdit, QTextEdit, QTableWidget {
    background-color: #12151C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 8px;
    margin: 0px 12px 12px 12px;
    padding: 8px;
    selection-background-color: #2D3A52;
    selection-color: #F4F6FA;
}
QPlainTextEdit, QTextEdit {
    font-family: "Cascadia Code", "Consolas", monospace;
    font-size: 14px;
    line-height: 1.6;
}

/* Table / Spreadsheet */
QTableWidget {
    gridline-color: #262C38;
    alternate-background-color: #161A22;
}
QTableWidget::item {
    padding: 4px 8px;
}
QTableWidget::item:selected {
    background-color: #2D3A52;
    color: #F4F6FA;
}
QHeaderView {
    background-color: #12151C;
    color: #8A95A8;
    font-size: 12px;
    font-weight: 500;
}
QHeaderView::section {
    background-color: #161A22;
    color: #8A95A8;
    border: none;
    border-right: 1px solid #262C38;
    border-bottom: 1px solid #262C38;
    padding: 6px 8px;
}
QHeaderView::section:first {
    border-left: none;
}

/* Scrollbars */
QScrollBar:vertical {
    background: #0B0D12;
    width: 12px;
    margin: 0;
    border-radius: 6px;
}
QScrollBar::handle:vertical {
    background: #262C38;
    border-radius: 6px;
    min-height: 20px;
    margin: 2px;
}
QScrollBar::handle:vertical:hover {
    background: #3A4455;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal {
    background: #0B0D12;
    height: 12px;
    margin: 0;
    border-radius: 6px;
}
QScrollBar::handle:horizontal {
    background: #262C38;
    border-radius: 6px;
    min-width: 20px;
    margin: 2px;
}
QScrollBar::handle:horizontal:hover {
    background: #3A4455;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

/* Status bar */
QStatusBar {
    background-color: #0B0D12;
    color: #8A95A8;
    border-top: 1px solid #262C38;
    font-size: 12px;
    padding: 4px 12px;
}

/* ComboBox */
QComboBox {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 6px;
    padding: 4px 12px;
    min-height: 26px;
}
QComboBox:hover {
    border-color: #3A4455;
}
QComboBox::drop-down {
    border: none;
    width: 24px;
}
QComboBox::down-arrow {
    width: 12px;
    height: 12px;
}
QComboBox QAbstractItemView {
    background-color: #12151C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 8px;
    selection-background-color: #1D222C;
    selection-color: #8B7CFF;
}

/* SpinBox */
QSpinBox {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 6px;
    padding: 4px 8px;
    min-height: 26px;
}
QSpinBox:hover { border-color: #3A4455; }
QSpinBox::up-button, QSpinBox::down-button {
    background: transparent;
    border: none;
    width: 16px;
}

/* Push Button */
QPushButton {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 6px;
    padding: 6px 16px;
    min-height: 28px;
    font-weight: 500;
}
QPushButton:hover {
    background-color: #1D222C;
    border-color: #3A4455;
}
QPushButton:pressed {
    background-color: #2D3A52;
    border-color: #8B7CFF;
}
QPushButton:default {
    background-color: #8B7CFF;
    color: #FFFFFF;
    border: none;
}
QPushButton:default:hover {
    background-color: #9E91FF;
}

/* Dialogs */
QDialog {
    background-color: #0B0D12;
    border: 1px solid #262C38;
    border-radius: 8px;
}
QLabel {
    color: #F4F6FA;
    background: transparent;
}
)";
} // namespace Theme
