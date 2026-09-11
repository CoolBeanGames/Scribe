#include "CodeEditor.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

CodeEditor::CodeEditor(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_editor = new CodeEditorWidget(this);
    layout->addWidget(m_editor);

    connect(m_editor->document(), &QTextDocument::modificationChanged, this, [this](bool m) {
        if (m_modified != m) {
            m_modified = m;
            emit modificationChanged(m);
        }
    });
}

QString CodeEditor::displayName() const {
    if (m_filePath.isEmpty()) return "Untitled.py";
    return QFileInfo(m_filePath).fileName();
}

bool CodeEditor::saveFile() {
    return saveFileAs(m_filePath);
}

bool CodeEditor::saveFileAs(const QString& path) {
    if (path.isEmpty()) return false;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << m_editor->toPlainText();
    m_filePath = path;
    setModified(false);
    return true;
}

bool CodeEditor::loadFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream in(&file);
    m_editor->setPlainText(in.readAll());
    m_filePath = path;
    setModified(false);
    return true;
}
