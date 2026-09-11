#include "PythonHighlighter.h"

PythonHighlighter::PythonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Keywords
    keywordFormat.setForeground(QColor("#8B7CFF")); // Zen Accent
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        "\\bdef\\b", "\\bclass\\b", "\\bimport\\b", "\\bfrom\\b", "\\bif\\b", "\\belif\\b",
        "\\belse\\b", "\\bfor\\b", "\\bwhile\\b", "\\breturn\\b", "\\byield\\b", "\\bpass\\b",
        "\\bbreak\\b", "\\bcontinue\\b", "\\band\\b", "\\bor\\b", "\\bnot\\b", "\\bis\\b",
        "\\bin\\b", "\\bas\\b", "\\bglobal\\b", "\\bnonlocal\\b", "\\blambda\\b", "\\btry\\b",
        "\\bexcept\\b", "\\bfinally\\b", "\\braise\\b", "\\bwith\\b", "\\bassert\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Numbers
    numberFormat.setForeground(QColor("#E5C07B"));
    rule.pattern = QRegularExpression("\\b[0-9]+(?:\\.[0-9]+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Strings
    stringFormat.setForeground(QColor("#98C379"));
    rule.pattern = QRegularExpression("\".*?\"|'.*?'");
    rule.format = stringFormat;
    highlightingRules.append(rule);
    
    // Comments
    commentFormat.setForeground(QColor("#5C6370"));
    rule.pattern = QRegularExpression("#[^\\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);
}

void PythonHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
