#include "TabWidget.h"
#include "PlainTextEditor.h"
#include "RichTextEditor.h"
#include "SpreadsheetEditor.h"
#include "CodeEditor.h"
#include <QTabBar>
#include <QMessageBox>
#include <QPushButton>
#include <QPainter>
#include <QIcon>
#include <QColor>

TabWidget::TabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);

    connect(this, &QTabWidget::tabCloseRequested,
            this, &TabWidget::onTabCloseRequested);
    connect(this, &QTabWidget::currentChanged,
            this, &TabWidget::onCurrentChanged);
}

int TabWidget::addEditor(EditorBase* editor)
{
    if (!editor) return -1;

    m_editors.append(editor);
    QString label = editor->displayName();
    int idx = addTab(editor->widget(), label);

    // Connect modification signal to update tab label
    if (auto* plain = qobject_cast<PlainTextEditor*>(editor->widget())) {
        connect(plain, &PlainTextEditor::modificationChanged, this, [this, editor](bool) {
            updateTabLabel(editor);
        });
        connect(plain, &PlainTextEditor::filePathChanged, this, [this, editor](const QString&) {
            updateTabLabel(editor);
        });
    } else if (auto* rich = qobject_cast<RichTextEditor*>(editor->widget())) {
        connect(rich, &RichTextEditor::modificationChanged, this, [this, editor](bool) {
            updateTabLabel(editor);
        });
        connect(rich, &RichTextEditor::filePathChanged, this, [this, editor](const QString&) {
            updateTabLabel(editor);
        });
    } else if (auto* sheet = qobject_cast<SpreadsheetEditor*>(editor->widget())) {
        connect(sheet, &SpreadsheetEditor::modificationChanged, this, [this, editor](bool) {
            updateTabLabel(editor);
        });
        connect(sheet, &SpreadsheetEditor::filePathChanged, this, [this, editor](const QString&) {
            updateTabLabel(editor);
        });
    } else if (auto* code = qobject_cast<CodeEditor*>(editor->widget())) {
        connect(code, &CodeEditor::modificationChanged, this, [this, editor](bool) {
            updateTabLabel(editor);
        });
    }

    setCurrentIndex(idx);
    return idx;
}

EditorBase* TabWidget::editorAt(int index) const
{
    if (index < 0 || index >= m_editors.count()) return nullptr;
    return m_editors.at(index);
}

EditorBase* TabWidget::currentEditor() const
{
    return editorAt(currentIndex());
}

void TabWidget::updateTabLabel(EditorBase* editor)
{
    int idx = m_editors.indexOf(editor);
    if (idx < 0) return;

    QString name = editor->displayName();
    if (editor->isModified()) {
        name = "• " + name;
    }
    setTabText(idx, name);

    QPixmap pm(14, 14);
    pm.fill(Qt::transparent);
    {
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        if (editor->documentType() == DocumentType::PlainText) {
            p.setBrush(QColor("#61AFEF")); // Blue
        } else if (editor->documentType() == DocumentType::RichText) {
            p.setBrush(QColor("#8B7CFF")); // Violet
        } else if (editor->documentType() == DocumentType::Code) {
            p.setBrush(QColor("#98C379")); // Green
        } else {
            p.setBrush(QColor("#E5C07B")); // Amber / Spreadsheet
        }
        p.drawRoundedRect(1, 2, 10, 10, 3, 3);
    }
    setTabIcon(idx, QIcon(pm));
}

void TabWidget::removeEditor(EditorBase* editor)
{
    int idx = m_editors.indexOf(editor);
    if (idx < 0) return;
    removeTab(idx);
    m_editors.removeAt(idx);
}

void TabWidget::onTabCloseRequested(int index)
{
    EditorBase* editor = editorAt(index);
    if (editor) {
        emit editorCloseRequested(editor);
    }
}

void TabWidget::onCurrentChanged(int index)
{
    emit editorChanged(editorAt(index));
}


