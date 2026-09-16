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

/* Toolbar Container */
#ToolbarContainer {
    background-color: #0E1117;
    border-bottom: 1px solid #1E232E;
    padding: 2px 8px;
}

/* Toolbar */
QToolBar {
    background-color: transparent;
    border: none;
    spacing: 4px;
    padding: 4px 6px;
}
QToolBar::separator {
    background-color: #262C38;
    width: 1px;
    margin: 4px 8px;
}
QToolButton {
    background-color: transparent;
    color: #8A95A8;
    border: 1px solid transparent;
    border-radius: 6px;
    padding: 5px 8px;
    font-size: 12px;
    font-weight: 500;
}
QToolButton:hover {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
}
QToolButton:pressed, QToolButton:checked {
    background-color: #1D222C;
    color: #8B7CFF;
    border: 1px solid #3A355A;
}
QToolButton:disabled {
    color: #333842;
}
QToolButton:focus {
    border: 1px solid #8B7CFF;
    background-color: #161A22;
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
    width: 14px;
    height: 14px;
    margin-left: 6px;
}
QTabBar::close-button:hover {
    background-color: #343A48;
    border-radius: 4px;
}
QTabBar:focus QTabBar::tab:selected {
    border: 2px solid #9E91FF;
}

/* Empty workspace */
#WelcomeWorkspace {
    background-color: #0B0D12;
}
#WelcomeCard {
    background-color: #12151C;
    border: 1px solid #262C38;
    border-radius: 15px;
}
#WelcomeEyebrow, #WelcomeSectionTitle {
    color: #8B7CFF;
    font-size: 11px;
    font-weight: 700;
}
#WelcomeTitle {
    color: #F4F6FA;
    font-size: 24px;
    font-weight: 650;
}
#WelcomeSubtitle, #RecentEmptyLabel {
    color: #8A95A8;
    font-size: 13px;
}
#RecentFilesList {
    background-color: #0E1117;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 8px;
    padding: 4px;
    outline: none;
}
#RecentFilesList::item {
    color: #F4F6FA;
    border-radius: 6px;
    padding: 6px 10px;
}
#RecentFilesList::item:hover, #RecentFilesList::item:selected {
    background-color: #1D222C;
    color: #8B7CFF;
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
QPlainTextEdit:focus, QTextEdit:focus, QTableWidget:focus, QListWidget:focus {
    border: 1px solid #8B7CFF;
}
QPlainTextEdit, QTextEdit {
    font-family: "Cascadia Code", "Consolas", monospace;
    font-size: 14px;
    line-height: 1.6;
}

/* Rich Text Page Layout */
#RichTextWorkspace {
    background-color: #0B0D12;
}
#RichTextPageEditor {
    background-color: #12151C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 6px;
    padding: 24px 32px;
    margin: 0;
    font-family: "Segoe UI", sans-serif;
    font-size: 13px;
}
#RichTextFooter {
    background-color: #0E1117;
    border-top: 1px solid #1E232E;
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
QComboBox:focus {
    border-color: #8B7CFF;
}
QComboBox:disabled {
    background-color: #12151C;
    color: #4A5568;
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
QSpinBox:focus { border-color: #8B7CFF; }
QSpinBox:disabled { color: #4A5568; background-color: #12151C; }
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
QPushButton:focus {
    border: 2px solid #9E91FF;
    padding: 5px 15px;
}
QPushButton:disabled {
    background-color: #12151C;
    color: #4A5568;
    border-color: #1E232E;
}
QPushButton:default {
    background-color: #8B7CFF;
    color: #FFFFFF;
    border: none;
}
QPushButton:default:hover {
    background-color: #9E91FF;
}

#PrimaryButton, #WelcomePrimaryButton {
    background-color: #8B7CFF;
    color: #FFFFFF;
    border-color: #8B7CFF;
    font-weight: 600;
}
#PrimaryButton:hover, #WelcomePrimaryButton:hover {
    background-color: #9E91FF;
}
#PrimaryButton:pressed, #WelcomePrimaryButton:pressed {
    background-color: #796AE8;
}
#SecondaryButton {
    background-color: #1D222C;
    color: #ABB2BF;
}
#CompactButton {
    min-height: 24px;
    padding: 4px 12px;
}

QLineEdit {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 6px;
    padding: 6px 9px;
    selection-background-color: #2D3A52;
}
QLineEdit:hover { border-color: #3A4455; }
QLineEdit:focus { border-color: #8B7CFF; }
QLineEdit:disabled { color: #4A5568; background-color: #12151C; }

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
#DialogTitle {
    color: #F4F6FA;
    font-size: 18px;
    font-weight: 650;
}
#DialogDescription {
    color: #8A95A8;
    font-size: 12px;
}
#DialogStatus {
    color: #8B7CFF;
    font-size: 12px;
}
#DefaultAppsTable {
    margin: 0;
    border-radius: 8px;
}

/* Completer Popup */
QAbstractItemView {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    selection-background-color: #2D3A52;
    selection-color: #F4F6FA;
    outline: none;
}
)";
} // namespace Theme
