#include "CSharpHighlighter.h"

CSharpHighlighter::CSharpHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // ── Keywords ──────────────────────────────────────────────────────────────
    keywordFormat.setForeground(QColor("#C678DD")); // purple
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        "\\babstract\\b", "\\bas\\b", "\\bbase\\b", "\\bbreak\\b",
        "\\bcase\\b", "\\bcatch\\b", "\\bchecked\\b", "\\bclass\\b",
        "\\bconst\\b", "\\bcontinue\\b", "\\bdefault\\b", "\\bdelegate\\b",
        "\\bdo\\b", "\\belse\\b", "\\benum\\b", "\\bevent\\b",
        "\\bexplicit\\b", "\\bextern\\b", "\\bfinally\\b", "\\bfixed\\b",
        "\\bfor\\b", "\\bforeach\\b", "\\bgoto\\b", "\\bif\\b",
        "\\bimplicit\\b", "\\bin\\b", "\\binterface\\b", "\\binternal\\b",
        "\\bis\\b", "\\block\\b", "\\bnamespace\\b", "\\bnew\\b",
        "\\boperator\\b", "\\bout\\b", "\\boverride\\b", "\\bparams\\b",
        "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b", "\\breadonly\\b",
        "\\brecord\\b", "\\bref\\b", "\\breturn\\b", "\\bsealed\\b",
        "\\bsizeof\\b", "\\bstackalloc\\b", "\\bstatic\\b", "\\bstruct\\b",
        "\\bswitch\\b", "\\bthis\\b", "\\bthrow\\b", "\\btry\\b",
        "\\btypeof\\b", "\\bunchecked\\b", "\\bunsafe\\b", "\\busing\\b",
        "\\bvirtual\\b", "\\bvolatile\\b", "\\bwhile\\b", "\\byield\\b",
        "\\basync\\b", "\\bawait\\b", "\\bvar\\b", "\\bdynamic\\b",
        "\\bget\\b", "\\bset\\b", "\\binit\\b", "\\bvalue\\b",
        "\\bwhen\\b", "\\bwhere\\b", "\\bwith\\b", "\\bglobal\\b",
        "\\bpartial\\b", "\\btrue\\b", "\\bfalse\\b", "\\bnull\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format  = keywordFormat;
        highlightingRules.append(rule);
    }

    // ── Types & Primitive Keywords ────────────────────────────────────────────
    typeFormat.setForeground(QColor("#E5C07B")); // amber
    const QString typePatterns[] = {
        "\\bbool\\b", "\\bbyte\\b", "\\bsbyte\\b", "\\bchar\\b",
        "\\bdecimal\\b", "\\bdouble\\b", "\\bfloat\\b", "\\bint\\b",
        "\\buint\\b", "\\bnint\\b", "\\bnuint\\b", "\\blong\\b",
        "\\bulong\\b", "\\bshort\\b", "\\bushort\\b", "\\bobject\\b",
        "\\bstring\\b", "\\bvoid\\b",
        // Common BCL Types
        "\\bConsole\\b", "\\bMath\\b", "\\bString\\b", "\\bInt32\\b",
        "\\bInt64\\b", "\\bBoolean\\b", "\\bList\\b", "\\bDictionary\\b",
        "\\bIEnumerable\\b", "\\bIList\\b", "\\bTask\\b", "\\bAction\\b",
        "\\bFunc\\b", "\\bNullable\\b", "\\bDateTime\\b", "\\bTimeSpan\\b",
        "\\bGuid\\b", "\\bException\\b", "\\bConvert\\b", "\\bFile\\b",
        "\\bDirectory\\b", "\\bPath\\b", "\\bStream\\b", "\\bStringBuilder\\b",
        "\\bThread\\b", "\\bArray\\b", "\\bObject\\b"
    };
    for (const QString &pattern : typePatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format  = typeFormat;
        highlightingRules.append(rule);
    }

    // ── Preprocessor Directives ───────────────────────────────────────────────
    preprocessorFormat.setForeground(QColor("#61AFEF")); // blue
    rule.pattern = QRegularExpression("^[ \t]*#[A-Za-z_]+[^\n]*");
    rule.format  = preprocessorFormat;
    highlightingRules.append(rule);

    // ── Functions & Methods ───────────────────────────────────────────────────
    functionFormat.setForeground(QColor("#61AFEF")); // blue
    rule.pattern = QRegularExpression("\\b([A-Za-z_][A-Za-z0-9_]*)\\s*(?=\\()");
    rule.format  = functionFormat;
    highlightingRules.append(rule);

    // ── Numbers ───────────────────────────────────────────────────────────────
    numberFormat.setForeground(QColor("#D19A66")); // orange
    rule.pattern = QRegularExpression(
        "\\b(0[xX][0-9A-Fa-f_]+|0[bB][01_]+|[0-9][0-9_]*(?:\\.[0-9][0-9_]*)?(?:[eE][+-]?[0-9_]+)?[fFdDmMuUlL]?)\\b");
    rule.format  = numberFormat;
    highlightingRules.append(rule);

    // ── Strings (single-line, verbatim, interpolated) ─────────────────────────
    stringFormat.setForeground(QColor("#98C379")); // green
    rule.pattern = QRegularExpression(R"(\$@"([^"]|"")*"|@"[^"]*"|\$"([^"\\]|\\.)*"|"([^"\\]|\\.)*"|'([^'\\]|\\.)*')");
    rule.format  = stringFormat;
    highlightingRules.append(rule);

    // ── Single-line Comments ──────────────────────────────────────────────────
    commentFormat.setForeground(QColor("#5C6370")); // grey
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format  = commentFormat;
    highlightingRules.append(rule);

    // ── Multi-line Comments ───────────────────────────────────────────────────
    commentStartExpression = QRegularExpression(R"(/\*)");
    commentEndExpression   = QRegularExpression(R"(\*/)");
}

void CSharpHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multiline block comments /* ... */
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
