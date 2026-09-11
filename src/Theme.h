#pragma once
// Theme constants for Scribe's Zen design language

namespace Theme {
    // Background colors
    constexpr auto BG_WORKSPACE  = "#0B0D12";   // near-black workspace
    constexpr auto BG_PANEL      = "#12151C";   // dark panel
    constexpr auto BG_PANEL2     = "#161A22";   // secondary panel
    constexpr auto BG_PANEL3     = "#1D222C";   // tertiary panel
    constexpr auto BORDER        = "#262C38";   // quiet border
    constexpr auto BORDER_FOCUS  = "#3A4455";   // focused border

    // Text colors
    constexpr auto TEXT_PRIMARY  = "#F4F6FA";   // warm white primary
    constexpr auto TEXT_SECONDARY= "#8A95A8";   // muted secondary text
    constexpr auto TEXT_DISABLED = "#4A5568";   // disabled text

    // Accent
    constexpr auto ACCENT        = "#8B7CFF";   // violet accent
    constexpr auto ACCENT_HOVER  = "#9E91FF";   // hover state
    constexpr auto ACCENT_PRESS  = "#7A6BEE";   // pressed state

    // Selection / highlight
    constexpr auto SELECTION_BG  = "#2D3A52";
    constexpr auto SELECTION_TEXT= "#F4F6FA";

    // Editor background (slightly different from workspace)
    constexpr auto EDITOR_BG     = "#0E1017";

    // Tab colors
    constexpr auto TAB_ACTIVE_BG    = "#1D222C";
    constexpr auto TAB_INACTIVE_BG  = "#12151C";
    constexpr auto TAB_MODIFIED_DOT = "#8B7CFF";

    // Toolbar
    constexpr auto TOOLBAR_BG    = "#161A22";
    constexpr auto TOOLBAR_BORDER= "#262C38";

    // Scrollbar
    constexpr auto SCROLLBAR_BG  = "#12151C";
    constexpr auto SCROLLBAR_HANDLE = "#2D3A52";
    constexpr auto SCROLLBAR_HANDLE_HOVER = "#3A4A6A";

    // Status bar
    constexpr auto STATUSBAR_BG  = "#0B0D12";

    // Full application stylesheet
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
    background-color: #161A22;
    border: none;
    border-bottom: 1px solid #262C38;
    spacing: 4px;
    padding: 3px 6px;
}
QToolBar::separator {
    background-color: #262C38;
    width: 1px;
    margin: 4px 4px;
}
QToolButton {
    background-color: transparent;
    color: #F4F6FA;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 3px 7px;
    min-width: 24px;
    min-height: 24px;
}
QToolButton:hover {
    background-color: #1D222C;
    border: 1px solid #262C38;
}
QToolButton:pressed, QToolButton:checked {
    background-color: #2D3A52;
    border: 1px solid #8B7CFF;
}
QToolButton:disabled {
    color: #4A5568;
}

/* Menu */
QMenuBar {
    background-color: #12151C;
    color: #F4F6FA;
    border-bottom: 1px solid #262C38;
    padding: 2px;
}
QMenuBar::item {
    background-color: transparent;
    padding: 4px 10px;
    border-radius: 3px;
}
QMenuBar::item:selected, QMenuBar::item:pressed {
    background-color: #1D222C;
}
QMenu {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 6px;
    padding: 4px;
}
QMenu::item {
    padding: 5px 30px 5px 20px;
    border-radius: 3px;
}
QMenu::item:selected {
    background-color: #2D3A52;
    color: #F4F6FA;
}
QMenu::item:disabled {
    color: #4A5568;
}
QMenu::separator {
    height: 1px;
    background: #262C38;
    margin: 3px 8px;
}

/* Tab Widget */
QTabWidget::pane {
    background-color: #0E1017;
    border: none;
    border-top: 1px solid #262C38;
}
QTabBar::tab {
    background-color: #12151C;
    color: #8A95A8;
    padding: 6px 14px;
    border: none;
    border-right: 1px solid #262C38;
    min-width: 100px;
    max-width: 200px;
}
QTabBar::tab:selected {
    background-color: #1D222C;
    color: #F4F6FA;
    border-bottom: 2px solid #8B7CFF;
}
QTabBar::tab:hover:!selected {
    background-color: #161A22;
    color: #F4F6FA;
}
QTabBar::close-button {
    image: none;
    subcontrol-position: right;
    padding: 2px;
}

/* Editors */
QPlainTextEdit, QTextEdit {
    background-color: #0E1017;
    color: #F4F6FA;
    border: none;
    selection-background-color: #2D3A52;
    selection-color: #F4F6FA;
    font-family: "Cascadia Code", "Consolas", "Courier New", monospace;
    font-size: 14px;
    line-height: 1.5;
}

/* Table / Spreadsheet */
QTableWidget {
    background-color: #0E1017;
    color: #F4F6FA;
    border: none;
    gridline-color: #262C38;
    selection-background-color: #2D3A52;
    selection-color: #F4F6FA;
    alternate-background-color: #12151C;
}
QTableWidget::item {
    padding: 2px 6px;
}
QTableWidget::item:selected {
    background-color: #2D3A52;
    color: #F4F6FA;
}
QHeaderView {
    background-color: #12151C;
    color: #8A95A8;
    font-size: 12px;
}
QHeaderView::section {
    background-color: #161A22;
    color: #8A95A8;
    border: none;
    border-right: 1px solid #262C38;
    border-bottom: 1px solid #262C38;
    padding: 4px 6px;
}
QHeaderView::section:first {
    border-left: none;
}

/* Scrollbars */
QScrollBar:vertical {
    background: #12151C;
    width: 10px;
    margin: 0;
    border-radius: 5px;
}
QScrollBar::handle:vertical {
    background: #2D3A52;
    border-radius: 5px;
    min-height: 20px;
}
QScrollBar::handle:vertical:hover {
    background: #3A4A6A;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal {
    background: #12151C;
    height: 10px;
    margin: 0;
    border-radius: 5px;
}
QScrollBar::handle:horizontal {
    background: #2D3A52;
    border-radius: 5px;
    min-width: 20px;
}
QScrollBar::handle:horizontal:hover {
    background: #3A4A6A;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

/* Status bar */
QStatusBar {
    background-color: #0B0D12;
    color: #8A95A8;
    border-top: 1px solid #262C38;
    font-size: 11px;
    padding: 2px 8px;
}

/* ComboBox */
QComboBox {
    background-color: #1D222C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 4px;
    padding: 2px 8px;
    min-height: 24px;
}
QComboBox:hover {
    border-color: #3A4455;
}
QComboBox::drop-down {
    border: none;
    width: 20px;
}
QComboBox::down-arrow {
    width: 10px;
    height: 10px;
}
QComboBox QAbstractItemView {
    background-color: #161A22;
    color: #F4F6FA;
    border: 1px solid #262C38;
    selection-background-color: #2D3A52;
}

/* SpinBox */
QSpinBox {
    background-color: #1D222C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 4px;
    padding: 2px 4px;
    min-height: 24px;
}
QSpinBox:hover { border-color: #3A4455; }
QSpinBox::up-button, QSpinBox::down-button {
    background: transparent;
    border: none;
    width: 16px;
}

/* Line Edit */
QLineEdit {
    background-color: #1D222C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 4px;
    padding: 3px 8px;
    min-height: 24px;
}
QLineEdit:focus {
    border-color: #8B7CFF;
}

/* Push Button */
QPushButton {
    background-color: #1D222C;
    color: #F4F6FA;
    border: 1px solid #262C38;
    border-radius: 5px;
    padding: 5px 16px;
    min-height: 28px;
}
QPushButton:hover {
    background-color: #2D3A52;
    border-color: #3A4455;
}
QPushButton:pressed {
    background-color: #8B7CFF;
    border-color: #8B7CFF;
}
QPushButton:default {
    border-color: #8B7CFF;
}

/* Dialogs */
QDialog {
    background-color: #12151C;
    border: 1px solid #262C38;
}
QLabel {
    color: #F4F6FA;
    background: transparent;
}

/* Splitter */
QSplitter::handle {
    background: #262C38;
}
)";
} // namespace Theme
