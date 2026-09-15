#include "HtmlHighlighter.h"

HtmlHighlighter::HtmlHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    // ── Formats ───────────────────────────────────────────────────────────────
    doctypeFormat.setForeground(QColor("#61AFEF")); // blue
    doctypeFormat.setFontWeight(QFont::Bold);

    tagFormat.setForeground(QColor("#56B6C2")); // cyan for <, >, </, />

    tagNameFormat.setForeground(QColor("#E06C75")); // red / coral
    tagNameFormat.setFontWeight(QFont::Bold);

    attributeNameFormat.setForeground(QColor("#D19A66")); // orange / amber

    valueFormat.setForeground(QColor("#98C379")); // green for string values

    entityFormat.setForeground(QColor("#C678DD")); // purple
    entityFormat.setFontWeight(QFont::DemiBold);

    commentFormat.setForeground(QColor("#5C6370")); // grey
    commentFormat.setFontItalic(true);

    commentStartExpression = QRegularExpression(R"(<!--)");
    commentEndExpression   = QRegularExpression(R"(-->)");
}

void HtmlHighlighter::highlightBlock(const QString &text) {
    // 1. Highlight HTML tags and their contents
    static const QRegularExpression tagRegex(R"(<(/?[A-Za-z0-9_\-]+)([^>]*)>)");
    QRegularExpressionMatchIterator itTag = tagRegex.globalMatch(text);
    while (itTag.hasNext()) {
        QRegularExpressionMatch m = itTag.next();
        int fullStart = m.capturedStart(0);
        int fullLen = m.capturedLength(0);

        // Brackets <, >
        setFormat(fullStart, 1, tagFormat);
        setFormat(fullStart + fullLen - 1, 1, tagFormat);

        // Tag name
        int nameStart = m.capturedStart(1);
        int nameLen = m.capturedLength(1);
        setFormat(nameStart, nameLen, tagNameFormat);

        // Within attributes content:
        QString attrContent = m.captured(2);
        int attrOffset = m.capturedStart(2);

        // Attributes (name=)
        static const QRegularExpression attrRegex(R"(([A-Za-z0-9_\-]+)\s*=)");
        QRegularExpressionMatchIterator itAttr = attrRegex.globalMatch(attrContent);
        while (itAttr.hasNext()) {
            QRegularExpressionMatch ma = itAttr.next();
            setFormat(attrOffset + ma.capturedStart(1), ma.capturedLength(1), attributeNameFormat);
        }

        // Attribute string values
        static const QRegularExpression valRegex(R"("[^"]*"|'[^']*')");
        QRegularExpressionMatchIterator itVal = valRegex.globalMatch(attrContent);
        while (itVal.hasNext()) {
            QRegularExpressionMatch mv = itVal.next();
            setFormat(attrOffset + mv.capturedStart(0), mv.capturedLength(0), valueFormat);
        }
    }

    // 2. DOCTYPE
    static const QRegularExpression doctypeRegex(R"(<!DOCTYPE[^>]*>)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator itDoc = doctypeRegex.globalMatch(text);
    while (itDoc.hasNext()) {
        QRegularExpressionMatch md = itDoc.next();
        setFormat(md.capturedStart(), md.capturedLength(), doctypeFormat);
    }

    // 3. Entities (&nbsp;, &#123;, etc.)
    static const QRegularExpression entityRegex(R"(&[A-Za-z0-9#]+;)");
    QRegularExpressionMatchIterator itEnt = entityRegex.globalMatch(text);
    while (itEnt.hasNext()) {
        QRegularExpressionMatch me = itEnt.next();
        setFormat(me.capturedStart(), me.capturedLength(), entityFormat);
    }

    // 4. Multiline comments <!-- ... -->
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
