#include <QMenu>
#include "CodeEditor.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

CodeEditor::CodeEditor(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_editor = new CodeEditorWidget(this);
    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editor, &QWidget::customContextMenuRequested, this, &CodeEditor::onCustomContextMenu);
    layout->addWidget(m_editor);

    connect(m_editor->document(), &QTextDocument::modificationChanged, this, [this](bool m) {
        if (m_modified != m) {
            m_modified = m;
            emit modificationChanged(m);
        }
    });
}

QString CodeEditor::displayName() const {
    if (m_filePath.isEmpty()) {
        switch (m_editor->language()) {
        case CodeLanguage::CSharp: return "Untitled.cs";
        case CodeLanguage::Json:   return "Untitled.json";
        case CodeLanguage::Html:   return "Untitled.html";
        case CodeLanguage::Css:    return "Untitled.css";
        case CodeLanguage::Xml:    return "Untitled.xml";
        case CodeLanguage::Python:
        default:                   return "Untitled.py";
        }
    }
    return QFileInfo(m_filePath).fileName();
}

void CodeEditor::setLanguage(CodeLanguage lang) {
    m_editor->setLanguage(lang);
}

CodeLanguage CodeEditor::language() const {
    return m_editor->language();
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

    QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "cs") {
        setLanguage(CodeLanguage::CSharp);
    } else if (ext == "json") {
        setLanguage(CodeLanguage::Json);
    } else if (ext == "html" || ext == "htm") {
        setLanguage(CodeLanguage::Html);
    } else if (ext == "css") {
        setLanguage(CodeLanguage::Css);
    } else if (ext == "xml") {
        setLanguage(CodeLanguage::Xml);
    } else {
        setLanguage(CodeLanguage::Python);
    }

    // Force the highlighter to re-scan the whole document now that text is loaded
    m_editor->document()->setModified(false);
    return true;
}

void CodeEditor::onCustomContextMenu(const QPoint& pos) {
    QMenu* menu = m_editor->createStandardContextMenu();
    buildContextMenu(menu);

    if (m_editor->language() == CodeLanguage::Json) {
        menu->addSeparator();
        QAction* actFormat = menu->addAction("Format JSON");
        connect(actFormat, &QAction::triggered, this, [this]() {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(m_editor->toPlainText().toUtf8(), &err);
            if (err.error == QJsonParseError::NoError) {
                m_editor->setPlainText(QString::fromUtf8(doc.toJson(QJsonDocument::Indented)));
            }
        });
    } else if (m_editor->language() == CodeLanguage::Xml) {
        menu->addSeparator();
        QAction* actFormat = menu->addAction("Format XML");
        connect(actFormat, &QAction::triggered, this, [this]() {
            QString inXml = m_editor->toPlainText();
            QXmlStreamReader reader(inXml);
            QString outXml;
            QXmlStreamWriter writer(&outXml);
            writer.setAutoFormatting(true);
            writer.setAutoFormattingIndent(4);
            while (!reader.atEnd()) {
                reader.readNext();
                if (reader.error()) {
                    return;
                }
                if (reader.isWhitespace()) {
                    continue;
                }
                writer.writeCurrentToken(reader);
            }
            if (!reader.hasError()) {
                m_editor->setPlainText(outXml);
            }
        });
    }

    menu->exec(m_editor->mapToGlobal(pos));
    delete menu;
}

