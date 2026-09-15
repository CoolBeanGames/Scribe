#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class XmlHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit XmlHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat tagFormat;
    QTextCharFormat tagNameFormat;
    QTextCharFormat attributeNameFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat entityFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat processingInstructionFormat;
    QTextCharFormat cdataFormat;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
    QRegularExpression cdataStartExpression;
    QRegularExpression cdataEndExpression;
};
