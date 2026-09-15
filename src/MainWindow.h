#pragma once
#include <QMainWindow>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QFontComboBox>
#include "TabWidget.h"
#include "EditorBase.h"
#include "PlainTextEditor.h"
#include "RichTextEditor.h"
#include "SpreadsheetEditor.h"
#include "CodeEditor.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    void openFile(const QString& path);
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    // UI components
    TabWidget*    m_tabWidget     = nullptr;
    QToolBar*     m_mainToolbar   = nullptr;
    QToolBar*     m_textToolbar   = nullptr;   // plain text toolbar (line numbers, font)
    QToolBar*     m_formatToolbar = nullptr;   // rich text formatting toolbar
    QToolBar*     m_sheetToolbar  = nullptr;   // spreadsheet toolbar
    QStatusBar*   m_statusBar     = nullptr;

    // Menu actions
    QAction* m_actNewTxt = nullptr;
    QAction* m_actNewMd = nullptr;
    QAction* m_actNewPy = nullptr;
    QAction* m_actRunCode = nullptr;
    QAction* m_actNewDocx = nullptr;
    QAction* m_actNewWord = nullptr;
    QAction* m_actNewExcel = nullptr;
    QAction* m_actNewCsv = nullptr;
    QAction* m_actOpen     = nullptr;
    QAction* m_actSave     = nullptr;
    QAction* m_actSaveAs   = nullptr;
    QAction* m_actPrint    = nullptr;
    QAction* m_actPrintToolbar = nullptr;
    QAction* m_actUndo     = nullptr;
    QAction* m_actRedo     = nullptr;
    QAction* m_actDefaultApp = nullptr;
    QAction* m_actExit     = nullptr;

    // Rich text toolbar actions / widgets
    QFontComboBox* m_fontCombo     = nullptr;
    QSpinBox*      m_fontSizeSpin  = nullptr;
    QAction*       m_actBold       = nullptr;
    QAction*       m_actItalic     = nullptr;
    QAction* m_actUnderline = nullptr;
    QAction* m_actBulletList = nullptr;
    QAction* m_actNumberedList = nullptr;
    QAction*       m_actAlignLeft  = nullptr;
    QAction*       m_actAlignCenter= nullptr;
    QAction*       m_actAlignRight = nullptr;
    QAction*       m_actAlignJust  = nullptr;
    QAction*       m_actFontColor  = nullptr;
    QComboBox*     m_headingCombo  = nullptr;
    QAction*       m_actInsertTable= nullptr;
    QAction*       m_actInsertImage= nullptr;
    QAction*       m_actInsertLink = nullptr;
    QAction*       m_actSectionBreak = nullptr;
    QAction*       m_actPageBreak = nullptr;
    QComboBox*     m_columnsCombo = nullptr;

    // Spreadsheet toolbar actions
    QAction* m_actAddRow    = nullptr;
    QAction* m_actAddCol    = nullptr;
    QAction* m_actDelRow    = nullptr;
    QAction* m_actDelCol = nullptr;
    QAction* m_actCellColor = nullptr;

    // Setup helpers
    void setupMenuBar();
    void setupMainToolbar();
    void setupTextToolbar();
    void setupFormatToolbar();
    void setupSheetToolbar();
    void setupStatusBar();
    void setupShortcuts();

    // Editor management
    void newPlainText();
    void newRichText();
    void newSpreadsheet();
    void newCodeEditor();
    void runCurrentCode();
    void openFile();
    
    void saveCurrentFile();
    void saveCurrentFileAs();
    void onPrint();
    void onDefaultAppSettings();
    void closeEditor(EditorBase* editor);

    // Query current editor
    EditorBase*      currentEditor()      const;
    PlainTextEditor* currentPlainEditor() const;
    RichTextEditor*  currentRichEditor()  const;
    SpreadsheetEditor* currentSheetEditor() const;

    // Update toolbar visibility and state based on active editor type
    void updateToolbarsForEditor(EditorBase* editor);
    void updateWindowTitle(EditorBase* editor);
    void updateStatusBar(EditorBase* editor);
    void updateUndoRedoActions();

    // Rich text formatting (forwards to current rich editor)
    void onFontFamilyChanged(const QString& family);
    void onFontSizeChanged(int size);
    void onBoldToggled(bool bold);
    void onItalicToggled(bool italic);
    void onUnderlineToggled(bool underline);
    void onAlignLeft();
    void onAlignCenter();
    void onAlignRight();
    void onAlignJustify();
    void onFontColor();
    void onHeadingChanged(int index);
    void onInsertTable();
    void onInsertImage();
    void onInsertLink();
    void onInsertSectionBreak();
    void onInsertPageBreak();
    void onColumnsChanged(int index);

    // Spreadsheet actions
    void onAddRow();
    void onAddColumn();
    void onDeleteRow();
    void onDeleteColumn();
    void onCellColor();

    // Plain text toolbar actions / widgets
    QAction*       m_actToggleLineNumbers = nullptr;
    QFontComboBox* m_plainFontCombo       = nullptr;
    QComboBox*     m_plainFontSizeCombo   = nullptr;

    // Sync toolbars
    void syncTextToolbar();
    void syncFormatToolbar();

    void onToggleLineNumbers();
    void onPlainFontChanged(const QFont& font);
    void onPlainFontSizeTextChanged(const QString& text);

private slots:
    void onEditorChanged(EditorBase* editor);
    void onEditorCloseRequested(EditorBase* editor);
    void onUndoAction();
    void onRedoAction();
};









