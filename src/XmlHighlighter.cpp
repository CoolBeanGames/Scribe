#include "XmlHighlighter.h"

XmlHighlighter::XmlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Tags and brackets
    tagFormat.setForeground(QColor("#ABB2BF"));
    
    // Tag names: <tag or </tag
    tagNameFormat.setForeground(QColor("#E06C75")); // Zen red/coral
    tagNameFormat.setFontWeight(QFont::DemiBold);

    // Attribute names
    attributeNameFormat.setForeground(QColor("#D19A66")); // Amber/orange

    // Values in strings
    valueFormat.setForeground(QColor("#98C379")); // Green

    // Entities: &amp;, &lt;, etc.
    entityFormat.setForeground(QColor("#56B6C2")); // Cyan
    entityFormat.setFontWeight(QFont::Bold);

    // XML declaration / processing instructions <?xml ... ?>
    processingInstructionFormat.setForeground(QColor("#C678DD")); // Purple
    processingInstructionFormat.setFontItalic(true);

    // CDATA: <![CDATA[ ... ]]>
    cdataFormat.setForeground(QColor("#E5C07B")); // Gold/yellow

    // Comments: <!-- ... -->
    commentFormat.setForeground(QColor("#5C6370")); // Muted grey
    commentFormat.setFontItalic(true);

    HighlightingRule rule;

    // Processing instructions: <? ... ?>
    rule.pattern = QRegularExpression(QStringLiteral("<\\?[^>]*\\?>"));
    rule.format = processingInstructionFormat;
    highlightingRules.append(rule);

    // Tag names (opening, closing, self-closing)
    rule.pattern = QRegularExpression(QStringLiteral("</?[a-zA-Z0-9_\\-\\.:]+"));
    rule.format = tagNameFormat;
    highlightingRules.append(rule);

    // Attribute names
    rule.pattern = QRegularExpression(QStringLiteral("\\b[a-zA-Z0-9_\\-\\.:]+(?=\\s*=)"));
    rule.format = attributeNameFormat;
    highlightingRules.append(rule);

    // Attribute values (double quotes)
    rule.pattern = QRegularExpression(QStringLiteral("\"[^\"]*\""));
    rule.format = valueFormat;
    highlightingRules.append(rule);

    // Attribute values (single quotes)
    rule.pattern = QRegularExpression(QStringLiteral("'[^']*'"));
    rule.format = valueFormat;
    highlightingRules.append(rule);

    // Entity references (&name; or &#1234;)
    rule.pattern = QRegularExpression(QStringLiteral("&[a-zA-Z0-9_#]+;"));
    rule.format = entityFormat;
    highlightingRules.append(rule);

    // Tag brackets: <, >, </, />
    rule.pattern = QRegularExpression(QStringLiteral("[<>/]"));
    rule.format = tagFormat;
    highlightingRules.append(rule);

    // Multiline comment expressions
    commentStartExpression = QRegularExpression(QStringLiteral("<!--"));
    commentEndExpression = QRegularExpression(QStringLiteral("-->"));

    // Multiline CDATA expressions
    cdataStartExpression = QRegularExpression(QStringLiteral("<!\\[CDATA\\["));
    cdataEndExpression = QRegularExpression(QStringLiteral("\\]\\]>"));
}

void XmlHighlighter::highlightBlock(const QString &text)
{
    // Apply standard regex rules
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multiline CDATA handling (state 2)
    setCurrentBlockState(0);

    int startIdx = 0;
    if (previousBlockState() == 2) {
        startIdx = 0;
    } else {
        auto m = cdataStartExpression.match(text);
        startIdx = m.hasMatch() ? m.capturedStart() : -1;
    }

    while (startIdx >= 0) {
        auto endMatch = cdataEndExpression.match(text, startIdx);
        int endIdx = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int cdataLength = 0;
        if (endIdx == -1) {
            setCurrentBlockState(2);
            cdataLength = text.length() - startIdx;
        } else {
            cdataLength = endIdx - startIdx + endMatch.capturedLength();
        }
        setFormat(startIdx, cdataLength, cdataFormat);
        auto nextMatch = cdataStartExpression.match(text, startIdx + cdataLength);
        startIdx = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
    }

    // Multiline Comment handling (state 1)
    if (currentBlockState() != 2) {
        int commentStart = 0;
        if (previousBlockState() == 1) {
            commentStart = 0;
        } else {
            auto m = commentStartExpression.match(text);
            commentStart = m.hasMatch() ? m.capturedStart() : -1;
        }

        while (commentStart >= 0) {
            auto endMatch = commentEndExpression.match(text, commentStart);
            int commentEnd = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
            int commentLength = 0;
            if (commentEnd == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - commentStart;
            } else {
                commentLength = commentEnd - commentStart + endMatch.capturedLength();
            }
            setFormat(commentStart, commentLength, commentFormat);
            auto nextMatch = commentStartExpression.match(text, commentStart + commentLength);
            commentStart = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
        }
    }
}
