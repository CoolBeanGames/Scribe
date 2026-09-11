#include "MainWindow.h"
#include "FlowLayout.h"
#include <QVBoxLayout>
#include "Theme.h"
#include "InsertTableDialog.h"
#include "InsertLinkDialog.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileInfo>
#include <QFontComboBox>
#include <QColorDialog>
#include <QLabel>
#include <QKeySequence>
#include <QSizePolicy>
#include <QColor>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Scribe");
    resize(1200, 800);
    setMinimumSize(800, 500);

    // Main layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Toolbar container with FlowLayout
    QWidget* toolbarContainer = new QWidget(centralWidget);
    FlowLayout* flowLayout = new FlowLayout(toolbarContainer, 0, 0, 0);

    m_mainToolbar = new QToolBar(toolbarContainer);
    m_formatToolbar = new QToolBar(toolbarContainer);
    m_sheetToolbar = new QToolBar(toolbarContainer);

    flowLayout->addWidget(m_mainToolbar);
    flowLayout->addWidget(m_formatToolbar);
    flowLayout->addWidget(m_sheetToolbar);

    // Central widget: the tab widget
    m_tabWidget = new TabWidget(centralWidget);
    
    mainLayout->addWidget(toolbarContainer);
    mainLayout->addWidget(m_tabWidget);
    setCentralWidget(centralWidget);

    setupMenuBar();
    setupMainToolbar();
    setupFormatToolbar();
    setupSheetToolbar();
    setupStatusBar();
    setupShortcuts();

    connect(m_tabWidget, &TabWidget::editorChanged,
            this, &MainWindow::onEditorChanged);
    connect(m_tabWidget, &TabWidget::editorCloseRequested,
            this, &MainWindow::onEditorCloseRequested);

    // Start with no tabs; show empty state
    updateToolbarsForEditor(nullptr);
    updateWindowTitle(nullptr);
}

// ---------------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------------
void MainWindow::setupMenuBar()
{
    QMenuBar* mb = menuBar();

    // ── File ──────────────────────────────────────────────────────────────
    QMenu* fileMenu = mb->addMenu("&File");

    QMenu* newMenu = fileMenu->addMenu("&New");
    
    QMenu* textMenu = newMenu->addMenu("Text");
    m_actNewTxt = textMenu->addAction(".txt");
    m_actNewMd = textMenu->addAction(".md");

    QMenu* codeMenu = newMenu->addMenu("Code");
    m_actNewPy = codeMenu->addAction(".py");
    
    QMenu* richTextMenu = newMenu->addMenu("Rich Text");
    m_actNewDocx = richTextMenu->addAction(".docx");
    m_actNewWord = richTextMenu->addAction(".word");
    
    QMenu* sheetMenu = newMenu->addMenu("Spreadsheet");
    m_actNewExcel = sheetMenu->addAction("excel");
    m_actNewCsv = sheetMenu->addAction("csv");

    fileMenu->addSeparator();
    m_actOpen   = fileMenu->addAction("&Open...");
    m_actOpen->setShortcut(QKeySequence::Open);

    fileMenu->addSeparator();
    m_actSave   = fileMenu->addAction("&Save");
    m_actSave->setShortcut(QKeySequence::Save);
    m_actSaveAs = fileMenu->addAction("Save &As...");
    m_actSaveAs->setShortcut(QKeySequence("Ctrl+Shift+S"));

    fileMenu->addSeparator();
    m_actExit = fileMenu->addAction("E&xit");
    m_actExit->setShortcut(QKeySequence::Quit);

    // ── Edit ──────────────────────────────────────────────────────────────
    QMenu* editMenu = mb->addMenu("&Edit");
    m_actUndo = editMenu->addAction("&Undo");
    m_actUndo->setShortcut(QKeySequence::Undo);
    m_actRedo = editMenu->addAction("&Redo");
    m_actRedo->setShortcut(QKeySequence::Redo);

    // ── Connect actions ───────────────────────────────────────────────────
    connect(m_actNewTxt, &QAction::triggered, this, &MainWindow::newPlainText);
    connect(m_actNewMd, &QAction::triggered, this, &MainWindow::newPlainText);
    connect(m_actNewPy, &QAction::triggered, this, &MainWindow::newCodeEditor);
    connect(m_actNewDocx, &QAction::triggered, this, &MainWindow::newRichText);
    connect(m_actNewWord, &QAction::triggered, this, &MainWindow::newRichText);
    connect(m_actNewExcel, &QAction::triggered, this, &MainWindow::newSpreadsheet);
    connect(m_actNewCsv, &QAction::triggered, this, &MainWindow::newSpreadsheet);
    connect(m_actOpen,   &QAction::triggered, this, &MainWindow::openFile);
    connect(m_actSave,   &QAction::triggered, this, &MainWindow::saveCurrentFile);
    connect(m_actSaveAs, &QAction::triggered, this, &MainWindow::saveCurrentFileAs);
    connect(m_actExit,   &QAction::triggered, this, &QMainWindow::close);
    connect(m_actUndo,   &QAction::triggered, this, &MainWindow::onUndoAction);
    connect(m_actRedo,   &QAction::triggered, this, &MainWindow::onRedoAction);
}

// ---------------------------------------------------------------------------
// Main toolbar (always visible)
// ---------------------------------------------------------------------------
void MainWindow::setupMainToolbar()
{
    m_actRunCode = new QAction("? Run", this);
    m_actRunCode->setToolTip("Run Code");
    connect(m_actRunCode, &QAction::triggered, this, &MainWindow::runCurrentCode);
    m_mainToolbar->addAction(m_actRunCode);
    // m_mainToolbar initialized in constructor
    m_mainToolbar->setMovable(false);
    m_mainToolbar->setObjectName("MainToolBar");


    QAction* open   = m_mainToolbar->addAction("📂 Open");
    QAction* save   = m_mainToolbar->addAction("💾 Save");
    m_mainToolbar->addSeparator();
    QAction* undo   = m_mainToolbar->addAction("↩ Undo");
    QAction* redo   = m_mainToolbar->addAction("↪ Redo");

open->setToolTip("Open file (Ctrl+O)");
    save->setToolTip("Save (Ctrl+S)");
    undo->setToolTip("Undo (Ctrl+Z)");
    redo->setToolTip("Redo (Ctrl+Y)");

connect(open,   &QAction::triggered, this, &MainWindow::openFile);
    connect(save,   &QAction::triggered, this, &MainWindow::saveCurrentFile);
    connect(undo,   &QAction::triggered, this, &MainWindow::onUndoAction);
    connect(redo,   &QAction::triggered, this, &MainWindow::onRedoAction);
}

// ---------------------------------------------------------------------------
// Rich text formatting toolbar
// ---------------------------------------------------------------------------
void MainWindow::setupFormatToolbar()
{
    // m_formatToolbar initialized in constructor
    m_formatToolbar->setMovable(false);
    m_formatToolbar->setObjectName("FormatToolBar");

    // Font family
    m_fontCombo = new QFontComboBox(m_formatToolbar);
    m_fontCombo->setFixedWidth(180);
    m_fontCombo->setToolTip("Font family");
    m_formatToolbar->addWidget(m_fontCombo);

    m_formatToolbar->addSeparator();

    // Font size
    m_fontSizeSpin = new QSpinBox(m_formatToolbar);
    m_fontSizeSpin->setRange(6, 144);
    m_fontSizeSpin->setValue(12);
    m_fontSizeSpin->setFixedWidth(60);
    m_fontSizeSpin->setToolTip("Font size");
    m_formatToolbar->addWidget(m_fontSizeSpin);

    m_formatToolbar->addSeparator();

    // Bold / Italic / Underline
    m_actBold      = m_formatToolbar->addAction("B");
    m_actItalic    = m_formatToolbar->addAction("I");
    m_actUnderline = m_formatToolbar->addAction("U");
    m_actBold->setCheckable(true);
    m_actItalic->setCheckable(true);
    m_actUnderline->setCheckable(true);
    m_actBold->setToolTip("Bold (Ctrl+B)");
    m_actItalic->setToolTip("Italic (Ctrl+I)");
    m_actUnderline->setToolTip("Underline (Ctrl+U)");

    // Style the bold/italic/underline text
    QFont boldFont = m_actBold->font();
    boldFont.setBold(true);
    boldFont.setPointSize(13);
    m_actBold->setFont(boldFont);

    QFont italFont = m_actItalic->font();
    italFont.setItalic(true);
    italFont.setPointSize(13);
    m_actItalic->setFont(italFont);

    QFont ulFont = m_actUnderline->font();
    ulFont.setUnderline(true);
    ulFont.setPointSize(13);
    m_actUnderline->setFont(ulFont);

    m_formatToolbar->addSeparator();

    // Font color
    m_actFontColor = m_formatToolbar->addAction("A");
    m_actFontColor->setToolTip("Font color");

    m_formatToolbar->addSeparator();

    // Alignment
    m_actAlignLeft   = m_formatToolbar->addAction("≡L");
    m_actAlignCenter = m_formatToolbar->addAction("≡C");
    m_actAlignRight  = m_formatToolbar->addAction("≡R");
    m_actAlignJust   = m_formatToolbar->addAction("≡J");
    m_actAlignLeft->setCheckable(true);
    m_actAlignCenter->setCheckable(true);
    m_actAlignRight->setCheckable(true);
    m_actAlignJust->setCheckable(true);
    m_actAlignLeft->setChecked(true);
    m_actAlignLeft->setToolTip("Align left");
    m_actAlignCenter->setToolTip("Align center");
    m_actAlignRight->setToolTip("Align right");
    m_actAlignJust->setToolTip("Justify");

    m_formatToolbar->addSeparator();

    // Heading style
    m_headingCombo = new QComboBox(m_formatToolbar);
    m_headingCombo->addItem("Normal");
    for (int i = 1; i <= 6; ++i)
        m_headingCombo->addItem(QString("H%1").arg(i));
    m_headingCombo->setFixedWidth(80);
    m_headingCombo->setToolTip("Heading style");
    m_formatToolbar->addWidget(m_headingCombo);

    m_formatToolbar->addSeparator();

    // Insert actions
    m_actInsertTable = m_formatToolbar->addAction("⊞ Table");
    m_actInsertImage = m_formatToolbar->addAction("🖼 Image");
    m_actInsertLink  = m_formatToolbar->addAction("🔗 Link");
    m_actInsertTable->setToolTip("Insert table");
    m_actInsertImage->setToolTip("Insert image");
    m_actInsertLink->setToolTip("Insert hyperlink");

    // Connect
    connect(m_fontCombo,    &QFontComboBox::currentFontChanged, this, [this](const QFont& f) {
        onFontFamilyChanged(f.family());
    });
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onFontSizeChanged);
    connect(m_actBold,      &QAction::toggled, this, &MainWindow::onBoldToggled);
    connect(m_actItalic,    &QAction::toggled, this, &MainWindow::onItalicToggled);
    connect(m_actUnderline, &QAction::toggled, this, &MainWindow::onUnderlineToggled);
    connect(m_actFontColor, &QAction::triggered, this, &MainWindow::onFontColor);
    connect(m_actAlignLeft,   &QAction::triggered, this, &MainWindow::onAlignLeft);
    connect(m_actAlignCenter, &QAction::triggered, this, &MainWindow::onAlignCenter);
    connect(m_actAlignRight,  &QAction::triggered, this, &MainWindow::onAlignRight);
    connect(m_actAlignJust,   &QAction::triggered, this, &MainWindow::onAlignJustify);
    connect(m_headingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onHeadingChanged);
    connect(m_actInsertTable, &QAction::triggered, this, &MainWindow::onInsertTable);
    connect(m_actInsertImage, &QAction::triggered, this, &MainWindow::onInsertImage);
    connect(m_actInsertLink,  &QAction::triggered, this, &MainWindow::onInsertLink);

    m_formatToolbar->setVisible(false);
}

// ---------------------------------------------------------------------------
// Spreadsheet toolbar
// ---------------------------------------------------------------------------
void MainWindow::setupSheetToolbar()
{
    // m_sheetToolbar initialized in constructor
    m_sheetToolbar->setMovable(false);
    m_sheetToolbar->setObjectName("SheetToolBar");

    m_actAddRow = m_sheetToolbar->addAction("+ Row");
    m_actAddCol = m_sheetToolbar->addAction("+ Column");
    m_sheetToolbar->addSeparator();
    m_actDelRow = m_sheetToolbar->addAction("- Row");
    m_actDelCol = m_sheetToolbar->addAction("- Column");
    m_sheetToolbar->addSeparator();
    m_actCellColor = m_sheetToolbar->addAction("Fill Color");

    m_actAddRow->setToolTip("Add row");
    m_actAddCol->setToolTip("Add column");
    m_actDelRow->setToolTip("Delete current row");
    m_actDelCol->setToolTip("Delete current column");

    connect(m_actAddRow, &QAction::triggered, this, &MainWindow::onAddRow);
    connect(m_actAddCol, &QAction::triggered, this, &MainWindow::onAddColumn);
    connect(m_actDelRow, &QAction::triggered, this, &MainWindow::onDeleteRow);
    connect(m_actDelCol, &QAction::triggered, this, &MainWindow::onDeleteColumn);
    connect(m_actCellColor, &QAction::triggered, this, &MainWindow::onCellColor);

    m_sheetToolbar->setVisible(false);
}

// ---------------------------------------------------------------------------
// Status bar
// ---------------------------------------------------------------------------
void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage("Ready");
}

// ---------------------------------------------------------------------------
// Global keyboard shortcuts
// ---------------------------------------------------------------------------
void MainWindow::setupShortcuts()
{
    // Ctrl+Z / Ctrl+Y are set on the actions themselves
    // Ctrl+B, Ctrl+I, Ctrl+U for rich text
    auto* shortcutB = new QAction(this);
    shortcutB->setShortcut(QKeySequence("Ctrl+B"));
    connect(shortcutB, &QAction::triggered, this, [this]() {
        if (m_actBold) m_actBold->toggle();
    });
    addAction(shortcutB);

    auto* shortcutI = new QAction(this);
    shortcutI->setShortcut(QKeySequence("Ctrl+I"));
    connect(shortcutI, &QAction::triggered, this, [this]() {
        if (m_actItalic) m_actItalic->toggle();
    });
    addAction(shortcutI);

    auto* shortcutU = new QAction(this);
    shortcutU->setShortcut(QKeySequence("Ctrl+U"));
    connect(shortcutU, &QAction::triggered, this, [this]() {
        if (m_actUnderline) m_actUnderline->toggle();
    });
    addAction(shortcutU);
}

// ---------------------------------------------------------------------------
// Editor creation
// ---------------------------------------------------------------------------
void MainWindow::newPlainText()
{
    auto* editor = new PlainTextEditor(this);
    m_tabWidget->addEditor(editor);
    updateWindowTitle(editor);
}

void MainWindow::newRichText()
{
    auto* editor = new RichTextEditor(this);
    // Connect cursor position changed to sync toolbar
    connect(editor, &RichTextEditor::cursorPositionChanged,
            this, &MainWindow::syncFormatToolbar);
    m_tabWidget->addEditor(editor);
    updateWindowTitle(editor);
}

void MainWindow::newSpreadsheet()
{
    auto* editor = new SpreadsheetEditor(this);
    m_tabWidget->addEditor(editor);
    updateWindowTitle(editor);
}

void MainWindow::newCodeEditor()
{
    auto* editor = new CodeEditor(this);
    m_tabWidget->addEditor(editor);
    updateWindowTitle(editor);
}

void MainWindow::runCurrentCode()
{
    QMessageBox::information(this, "Run Code", "Python execution is mocked for now.");
}

// ---------------------------------------------------------------------------
// File operations
// ---------------------------------------------------------------------------
void MainWindow::openFile()
{
    QString filter =
        "All Supported Files (*.txt *.md *.rtf *.csv);;"
        "Plain Text (*.txt *.md);;"
        "Rich Text (*.rtf);;"
        "CSV Spreadsheet (*.csv);;"
        "All Files (*)";

    QString path = QFileDialog::getOpenFileName(this, "Open File", QString(), filter);
    if (path.isEmpty()) return;

    QString ext = QFileInfo(path).suffix().toLower();
    EditorBase* editor = nullptr;

    if (ext == "rtf") {
        auto* rich = new RichTextEditor(this);
        connect(rich, &RichTextEditor::cursorPositionChanged,
                this, &MainWindow::syncFormatToolbar);
        if (!rich->loadFile(path)) { delete rich; return; }
        editor = rich;
    } else if (ext == "csv") {
        auto* sheet = new SpreadsheetEditor(this);
        if (!sheet->loadFile(path)) { delete sheet; return; }
        editor = sheet;
    } else {
        // .txt, .md, or unknown → plain text
        auto* plain = new PlainTextEditor(this);
        if (!plain->loadFile(path)) { delete plain; return; }
        editor = plain;
    }

    m_tabWidget->addEditor(editor);
    updateWindowTitle(editor);
}

void MainWindow::saveCurrentFile()
{
    EditorBase* editor = currentEditor();
    if (!editor) return;

    if (editor->filePath().isEmpty()) {
        saveCurrentFileAs();
    } else {
        editor->saveFile();
        m_tabWidget->updateTabLabel(editor);
        updateWindowTitle(editor);
    }
}

void MainWindow::saveCurrentFileAs()
{
    EditorBase* editor = currentEditor();
    if (!editor) return;

    QString filter;
    QString defaultExt;
    switch (editor->documentType()) {
    case DocumentType::PlainText:
        filter = "Plain Text (*.txt);;Markdown (*.md);;All Files (*)";
        defaultExt = ".txt";
        break;
    case DocumentType::RichText:
        filter = "Rich Text (*.rtf);;HTML (*.html);;All Files (*)";
        defaultExt = ".rtf";
        break;
    case DocumentType::Spreadsheet:
        filter = "CSV Spreadsheet (*.csv);;All Files (*)";
        defaultExt = ".csv";
        break;
    }

    QString suggestedPath = editor->filePath();
    if (suggestedPath.isEmpty()) {
        suggestedPath = "Untitled" + defaultExt;
    }

    QString path = QFileDialog::getSaveFileName(this, "Save As", suggestedPath, filter);
    if (path.isEmpty()) return;

    if (editor->saveFileAs(path)) {
        m_tabWidget->updateTabLabel(editor);
        updateWindowTitle(editor);
    }
}

void MainWindow::closeEditor(EditorBase* editor)
{
    if (!editor) return;

    if (editor->isModified()) {
        QString name = editor->displayName();
        QMessageBox::StandardButton btn = QMessageBox::question(
            this, "Unsaved Changes",
            QString("'%1' has unsaved changes.\nDo you want to save before closing?").arg(name),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (btn == QMessageBox::Save) {
            if (editor->filePath().isEmpty()) {
                saveCurrentFileAs();
            } else {
                editor->saveFile();
            }
        } else if (btn == QMessageBox::Cancel) {
            return;
        }
    }

    m_tabWidget->removeEditor(editor);
    delete editor->widget();

    EditorBase* next = currentEditor();
    updateWindowTitle(next);
    updateToolbarsForEditor(next);
}

// ---------------------------------------------------------------------------
// Toolbar / title update
// ---------------------------------------------------------------------------
void MainWindow::updateToolbarsForEditor(EditorBase* editor)
{
    if (!editor) {
        m_actRunCode->setVisible(false);
        m_formatToolbar->setVisible(false);
        m_sheetToolbar->setVisible(false);
        m_actSave->setEnabled(false);
        m_actSaveAs->setEnabled(false);
        m_actUndo->setEnabled(false);
        m_actRedo->setEnabled(false);
        return;
    }

    m_actSave->setEnabled(true);
    m_actSaveAs->setEnabled(true);

    bool isRich  = (editor->documentType() == DocumentType::RichText);
    bool isSheet = (editor->documentType() == DocumentType::Spreadsheet);
    bool isCode = (editor->documentType() == DocumentType::Code);

    m_formatToolbar->setVisible(isRich);
    m_sheetToolbar->setVisible(isSheet);
    m_actRunCode->setVisible(isCode);
    m_actRunCode->setVisible(isCode);

    // Undo/redo only for text editors
    m_actUndo->setEnabled(!isSheet);
    m_actRedo->setEnabled(!isSheet);

    if (isRich) {
        syncFormatToolbar();
    }
}

void MainWindow::updateWindowTitle(EditorBase* editor)
{
    if (!editor) {
        m_actRunCode->setVisible(false);
        setWindowTitle("Scribe");
        return;
    }
    QString name = editor->displayName();
    if (editor->isModified()) name = "• " + name;
    setWindowTitle(name + " — Scribe");
}

void MainWindow::updateStatusBar(EditorBase* editor)
{
    if (!editor) {
        m_actRunCode->setVisible(false);
        m_statusBar->clearMessage();
        return;
    }
    QString docType;
    switch (editor->documentType()) {
    case DocumentType::PlainText:  docType = "Plain Text"; break;
    case DocumentType::RichText:   docType = "Rich Text";  break;
    case DocumentType::Spreadsheet:docType = "Spreadsheet";break;
    }
    m_statusBar->showMessage(editor->filePath().isEmpty()
        ? docType + " — Untitled"
        : docType + " — " + editor->filePath());
}

void MainWindow::updateUndoRedoActions()
{
    EditorBase* editor = currentEditor();
    if (editor) {
        m_actUndo->setEnabled(editor->canUndo());
        m_actRedo->setEnabled(editor->canRedo());
    }
}

// ---------------------------------------------------------------------------
// Rich text formatting slots
// ---------------------------------------------------------------------------
void MainWindow::onFontFamilyChanged(const QString& family)
{
    if (auto* rich = currentRichEditor())
        rich->setFontFamily(family);
}

void MainWindow::onFontSizeChanged(int size)
{
    if (auto* rich = currentRichEditor())
        rich->setFontSize(size);
}

void MainWindow::onBoldToggled(bool bold)
{
    if (auto* rich = currentRichEditor())
        rich->setBold(bold);
}

void MainWindow::onItalicToggled(bool italic)
{
    if (auto* rich = currentRichEditor())
        rich->setItalic(italic);
}

void MainWindow::onUnderlineToggled(bool underline)
{
    if (auto* rich = currentRichEditor())
        rich->setUnderline(underline);
}

void MainWindow::onAlignLeft()
{
    if (auto* rich = currentRichEditor()) {
        rich->setAlignment(Qt::AlignLeft);
        m_actAlignLeft->setChecked(true);
        m_actAlignCenter->setChecked(false);
        m_actAlignRight->setChecked(false);
        m_actAlignJust->setChecked(false);
    }
}

void MainWindow::onAlignCenter()
{
    if (auto* rich = currentRichEditor()) {
        rich->setAlignment(Qt::AlignHCenter);
        m_actAlignLeft->setChecked(false);
        m_actAlignCenter->setChecked(true);
        m_actAlignRight->setChecked(false);
        m_actAlignJust->setChecked(false);
    }
}

void MainWindow::onAlignRight()
{
    if (auto* rich = currentRichEditor()) {
        rich->setAlignment(Qt::AlignRight);
        m_actAlignLeft->setChecked(false);
        m_actAlignCenter->setChecked(false);
        m_actAlignRight->setChecked(true);
        m_actAlignJust->setChecked(false);
    }
}

void MainWindow::onAlignJustify()
{
    if (auto* rich = currentRichEditor()) {
        rich->setAlignment(Qt::AlignJustify);
        m_actAlignLeft->setChecked(false);
        m_actAlignCenter->setChecked(false);
        m_actAlignRight->setChecked(false);
        m_actAlignJust->setChecked(true);
    }
}

void MainWindow::onFontColor()
{
    if (auto* rich = currentRichEditor()) {
        QColor current = rich->currentCharFormat().foreground().color();
        QColor color = QColorDialog::getColor(current.isValid() ? current : Qt::white, this, "Font Color");
        if (color.isValid()) {
            rich->setFontColor(color);
        }
    }
}

void MainWindow::onHeadingChanged(int index)
{
    if (auto* rich = currentRichEditor()) {
        if (index == 0) {
            // Normal — reset to default size
            rich->setFontSize(12);
            rich->setBold(false);
        } else {
            rich->setHeadingStyle(index); // 1-6
        }
    }
}

void MainWindow::onInsertTable()
{
    if (auto* rich = currentRichEditor()) {
        InsertTableDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            rich->insertTable(dlg.rows(), dlg.columns());
        }
    }
}

void MainWindow::onInsertImage()
{
    if (auto* rich = currentRichEditor()) {
        QString path = QFileDialog::getOpenFileName(this, "Insert Image", QString(),
            "Images (*.png *.jpg *.jpeg *.bmp *.gif *.svg);;All Files (*)");
        if (!path.isEmpty()) {
            rich->insertImage(path);
        }
    }
}

void MainWindow::onInsertLink()
{
    if (auto* rich = currentRichEditor()) {
        QString selected = rich->textEdit()->textCursor().selectedText();
        InsertLinkDialog dlg(selected, this);
        if (dlg.exec() == QDialog::Accepted) {
            rich->insertHyperlink(dlg.url(), dlg.displayText());
        }
    }
}

// ---------------------------------------------------------------------------
// Spreadsheet toolbar slots
// ---------------------------------------------------------------------------
void MainWindow::onAddRow()
{
    if (auto* sheet = currentSheetEditor()) sheet->addRow();
}

void MainWindow::onAddColumn()
{
    if (auto* sheet = currentSheetEditor()) sheet->addColumn();
}

void MainWindow::onDeleteRow()
{
    if (auto* sheet = currentSheetEditor()) sheet->deleteRow();
}

void MainWindow::onDeleteColumn()
{
    if (auto* sheet = currentSheetEditor()) sheet->deleteColumn();
}

void MainWindow::onCellColor()
{
    if (auto* sheet = currentSheetEditor()) {
        QColor color = QColorDialog::getColor(Qt::white, this, "Select Cell Color");
        if (color.isValid()) {
            sheet->setCellColor(color);
        }
    }
}

// ---------------------------------------------------------------------------
// Sync format toolbar from current cursor position
// ---------------------------------------------------------------------------
void MainWindow::syncFormatToolbar()
{
    auto* rich = currentRichEditor();
    if (!rich) return;

    QTextCharFormat fmt = rich->currentCharFormat();

    // Block signals to avoid recursive updates
    m_fontCombo->blockSignals(true);
    m_fontSizeSpin->blockSignals(true);
    m_actBold->blockSignals(true);
    m_actItalic->blockSignals(true);
    m_actUnderline->blockSignals(true);

    m_fontCombo->setCurrentFont(QFont(fmt.font().family()));
    int pts = static_cast<int>(fmt.fontPointSize());
    if (pts > 0) m_fontSizeSpin->setValue(pts);
    m_actBold->setChecked(fmt.fontWeight() >= QFont::Bold);
    m_actItalic->setChecked(fmt.fontItalic());
    m_actUnderline->setChecked(fmt.fontUnderline());

    m_fontCombo->blockSignals(false);
    m_fontSizeSpin->blockSignals(false);
    m_actBold->blockSignals(false);
    m_actItalic->blockSignals(false);
    m_actUnderline->blockSignals(false);

    // Alignment
    Qt::Alignment align = rich->currentAlignment();
    m_actAlignLeft->setChecked(align == Qt::AlignLeft || align == Qt::AlignLeading);
    m_actAlignCenter->setChecked(align == Qt::AlignHCenter);
    m_actAlignRight->setChecked(align == Qt::AlignRight || align == Qt::AlignTrailing);
    m_actAlignJust->setChecked(align == Qt::AlignJustify);
}

// ---------------------------------------------------------------------------
// Editor access helpers
// ---------------------------------------------------------------------------
EditorBase* MainWindow::currentEditor() const
{
    return m_tabWidget->currentEditor();
}

PlainTextEditor* MainWindow::currentPlainEditor() const
{
    EditorBase* e = currentEditor();
    if (e && e->documentType() == DocumentType::PlainText) {
        return static_cast<PlainTextEditor*>(e);
    }
    return nullptr;
}

RichTextEditor* MainWindow::currentRichEditor() const
{
    EditorBase* e = currentEditor();
    if (e && e->documentType() == DocumentType::RichText) {
        return static_cast<RichTextEditor*>(e);
    }
    return nullptr;
}

SpreadsheetEditor* MainWindow::currentSheetEditor() const
{
    EditorBase* e = currentEditor();
    if (e && e->documentType() == DocumentType::Spreadsheet) {
        return static_cast<SpreadsheetEditor*>(e);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------
void MainWindow::onEditorChanged(EditorBase* editor)
{
    updateToolbarsForEditor(editor);
    updateWindowTitle(editor);
    updateStatusBar(editor);
}

void MainWindow::onEditorCloseRequested(EditorBase* editor)
{
    closeEditor(editor);
}

void MainWindow::onUndoAction()
{
    EditorBase* editor = currentEditor();
    if (editor) editor->undo();
}

void MainWindow::onRedoAction()
{
    EditorBase* editor = currentEditor();
    if (editor) editor->redo();
}

// ---------------------------------------------------------------------------
// Close event
// ---------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
    // Check all open tabs for unsaved changes
    int count = m_tabWidget->count();
    for (int i = 0; i < count; ++i) {
        EditorBase* editor = m_tabWidget->editorAt(i);
        if (editor && editor->isModified()) {
            // Switch to this tab
            m_tabWidget->setCurrentIndex(i);
            QString name = editor->displayName();
            QMessageBox::StandardButton btn = QMessageBox::question(
                this, "Unsaved Changes",
                QString("'%1' has unsaved changes.\nDo you want to save before exiting?").arg(name),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save);

            if (btn == QMessageBox::Cancel) {
                event->ignore();
                return;
            } else if (btn == QMessageBox::Save) {
                if (editor->filePath().isEmpty()) {
                    saveCurrentFileAs();
                } else {
                    editor->saveFile();
                }
            }
        }
    }
    event->accept();
}







