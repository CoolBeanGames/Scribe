#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class CssHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CssHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat selectorFormat;
    QTextCharFormat propertyFormat;
    QTextCharFormat valueKeywordFormat;
    QTextCharFormat numberUnitFormat;
    QTextCharFormat colorHexFormat;
    QTextCharFormat atRuleFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
};
