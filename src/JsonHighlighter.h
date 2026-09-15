#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class JsonHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit JsonHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keyFormat;
    QTextCharFormat valueStringFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat literalFormat;
    QTextCharFormat commentFormat;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
};
