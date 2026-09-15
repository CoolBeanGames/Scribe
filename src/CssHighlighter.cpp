#include "CssHighlighter.h"

CssHighlighter::CssHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // ── At-rules (@media, @keyframes, @import, @font-face) ────────────────────
    atRuleFormat.setForeground(QColor("#C678DD")); // purple
    atRuleFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(@[A-Za-z_-]+)");
    rule.format  = atRuleFormat;
    highlightingRules.append(rule);

    // ── Properties (name before :) ────────────────────────────────────────────
    propertyFormat.setForeground(QColor("#56B6C2")); // cyan
    rule.pattern = QRegularExpression(R"(\b[A-Za-z_-]+(?=\s*:))");
    rule.format  = propertyFormat;
    highlightingRules.append(rule);

    // ── Selectors (classes .name, IDs #name, pseudo :hover) ───────────────────
    selectorFormat.setForeground(QColor("#E06C75")); // red / coral
    rule.pattern = QRegularExpression(R"((?:\.|\#|::|:)[A-Za-z_-][A-Za-z0-9_-]*)");
    rule.format  = selectorFormat;
    highlightingRules.append(rule);

    // ── CSS Value Keywords ────────────────────────────────────────────────────
    valueKeywordFormat.setForeground(QColor("#E5C07B")); // amber
    const QString valueKeywords[] = {
        "\\bnone\\b", "\\bblock\\b", "\\binline\\b", "\\binline-block\\b",
        "\\bflex\\b", "\\binline-flex\\b", "\\bgrid\\b", "\\binline-grid\\b",
        "\\babsolute\\b", "\\brelative\\b", "\\bfixed\\b", "\\bsticky\\b", "\\bstatic\\b",
        "\\bauto\\b", "\\bcenter\\b", "\\bleft\\b", "\\bright\\b", "\\btop\\b", "\\bbottom\\b",
        "\\bhidden\\b", "\\bvisible\\b", "\\bcollapse\\b", "\\bscroll\\b",
        "\\bpointer\\b", "\\bdefault\\b", "\\bcursor\\b",
        "\\bsolid\\b", "\\bdashed\\b", "\\bdotted\\b", "\\bdouble\\b",
        "\\bbold\\b", "\\bnormal\\b", "\\bitalic\\b",
        "\\binherit\\b", "\\binitial\\b", "\\bunset\\b", "\\brevert\\b",
        "\\btransparent\\b", "\\bcurrentColor\\b",
        "\\brow\\b", "\\bcolumn\\b", "\\bwrap\\b", "\\bnowrap\\b",
        "\\bspace-between\\b", "\\bspace-around\\b", "\\bspace-evenly\\b"
    };
    for (const QString &pattern : valueKeywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format  = valueKeywordFormat;
        highlightingRules.append(rule);
    }

    // ── Colors (#fff, #12151c, etc.) ──────────────────────────────────────────
    colorHexFormat.setForeground(QColor("#98C379")); // green
    rule.pattern = QRegularExpression(R"(#[0-9A-Fa-f]{3,8}\b)");
    rule.format  = colorHexFormat;
    highlightingRules.append(rule);

    // ── Numbers and Units (12px, 1.5rem, 100%, 0.3s, etc.) ────────────────────
    numberUnitFormat.setForeground(QColor("#D19A66")); // orange
    rule.pattern = QRegularExpression(R"(\b[0-9]+(?:\.[0-9]+)?(?:px|rem|em|%|vh|vw|vmin|vmax|deg|rad|turn|s|ms|pt|cm|mm|in)?\b)");
    rule.format  = numberUnitFormat;
    highlightingRules.append(rule);

    // ── Strings ───────────────────────────────────────────────────────────────
    stringFormat.setForeground(QColor("#98C379")); // green
    rule.pattern = QRegularExpression(R"("[^"\\]*(?:\\.[^"\\]*)*"|'[^'\\]*(?:\\.[^'\\]*)*')");
    rule.format  = stringFormat;
    highlightingRules.append(rule);

    // ── Comments ──────────────────────────────────────────────────────────────
    commentFormat.setForeground(QColor("#5C6370")); // grey
    commentFormat.setFontItalic(true);

    commentStartExpression = QRegularExpression(R"(/\*)");
    commentEndExpression   = QRegularExpression(R"(\*/)");
}

void CssHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multiline comments /* ... */
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        QRegularExpressionMatch match = commentStartExpression.match(text);
        startIndex = match.hasMatch() ? match.capturedStart() : -1;
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int commentLength;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, commentFormat);

        QRegularExpressionMatch nextMatch = commentStartExpression.match(text, startIndex + commentLength);
        startIndex = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
    }
}
