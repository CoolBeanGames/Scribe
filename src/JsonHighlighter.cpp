#include "JsonHighlighter.h"

JsonHighlighter::JsonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    // ── Formats ───────────────────────────────────────────────────────────────
    // Key names in JSON: Blue/Cyan
    keyFormat.setForeground(QColor("#61AFEF"));
    keyFormat.setFontWeight(QFont::DemiBold);

    // String values: Green
    valueStringFormat.setForeground(QColor("#98C379"));

    // Numbers: Orange
    numberFormat.setForeground(QColor("#D19A66"));

    // Literals: true, false, null: Amber / Purple
    literalFormat.setForeground(QColor("#E5C07B"));
    literalFormat.setFontWeight(QFont::Bold);

    // Comments: Grey italic
    commentFormat.setForeground(QColor("#5C6370"));
    commentFormat.setFontItalic(true);

    HighlightingRule rule;

    // ── Numbers ───────────────────────────────────────────────────────────────
    rule.pattern = QRegularExpression(R"(-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?)");
    rule.format  = numberFormat;
    highlightingRules.append(rule);

    // ── Literals ──────────────────────────────────────────────────────────────
    rule.pattern = QRegularExpression(R"(\b(true|false|null)\b)");
    rule.format  = literalFormat;
    highlightingRules.append(rule);

    // ── Comments (single-line JSONC) ──────────────────────────────────────────
    rule.pattern = QRegularExpression(R"(//[^\n]*)");
    rule.format  = commentFormat;
    highlightingRules.append(rule);

    // Multi-line comment delimiters
    commentStartExpression = QRegularExpression(R"(/\*)");
    commentEndExpression   = QRegularExpression(R"(\*/)");
}

void JsonHighlighter::highlightBlock(const QString &text) {
    // 1. First highlight strings and distinguish keys vs values
    // A key is a string followed optionally by whitespace and a ':'
    static const QRegularExpression stringOrKeyRegex(R"("(?:[^"\\]|\\.)*"\s*(?=:)|"(?:[^"\\]|\\.)*")");
    QRegularExpressionMatchIterator itStr = stringOrKeyRegex.globalMatch(text);
    while (itStr.hasNext()) {
        QRegularExpressionMatch match = itStr.next();
        QString matched = match.captured(0);
        int start = match.capturedStart();
        int len = match.capturedLength();

        if (matched.trimmed().endsWith('"') && text.mid(start + len).trimmed().startsWith(':')) {
            // It's a key
            setFormat(start, len, keyFormat);
        } else {
            // It's a string value
            setFormat(start, len, valueStringFormat);
        }
    }

    // 2. Apply rules for numbers, literals, comments
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            // Only format if not already formatted as a string
            int start = match.capturedStart();
            int len = match.capturedLength();
            if (format(start).foreground() != valueStringFormat.foreground() &&
                format(start).foreground() != keyFormat.foreground()) {
                setFormat(start, len, rule.format);
            }
        }
    }

    // 3. Multiline comments /* ... */
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
