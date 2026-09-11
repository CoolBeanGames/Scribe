#include "PythonHighlighter.h"

PythonHighlighter::PythonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // ── Keywords ──────────────────────────────────────────────────────────────
    keywordFormat.setForeground(QColor("#C678DD")); // purple
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        "\\bFalse\\b",  "\\bNone\\b",    "\\bTrue\\b",
        "\\band\\b",    "\\bas\\b",      "\\bassert\\b",
        "\\basync\\b",  "\\bawait\\b",   "\\bbreak\\b",
        "\\bclass\\b",  "\\bcontinue\\b","\\bdel\\b",
        "\\belif\\b",   "\\belse\\b",    "\\bexcept\\b",
        "\\bfinally\\b","\\bfor\\b",     "\\bfrom\\b",
        "\\bglobal\\b", "\\bif\\b",      "\\bimport\\b",
        "\\bin\\b",     "\\bis\\b",      "\\blambda\\b",
        "\\bnonlocal\\b","\\bnot\\b",    "\\bor\\b",
        "\\bpass\\b",   "\\braise\\b",   "\\breturn\\b",
        "\\btry\\b",    "\\bwhile\\b",   "\\bwith\\b",
        "\\byield\\b",  "\\bdef\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format  = keywordFormat;
        highlightingRules.append(rule);
    }

    // ── Built-in types ────────────────────────────────────────────────────────
    builtinFormat.setForeground(QColor("#E5C07B")); // amber
    const QString builtinPatterns[] = {
        "\\bint\\b",   "\\bfloat\\b",  "\\bcomplex\\b","\\bstr\\b",
        "\\bbool\\b",  "\\bbytes\\b",  "\\bbytearray\\b",
        "\\blist\\b",  "\\btuple\\b",  "\\bset\\b",    "\\bfrozenset\\b",
        "\\bdict\\b",  "\\brange\\b",  "\\btype\\b",   "\\bobject\\b",
        "\\bprint\\b", "\\blen\\b",    "\\brange\\b",  "\\benumerate\\b",
        "\\bzip\\b",   "\\bmap\\b",    "\\bfilter\\b", "\\bsorted\\b",
        "\\breversed\\b","\\bsum\\b",  "\\bmin\\b",    "\\bmax\\b",
        "\\babs\\b",   "\\bround\\b",  "\\bopen\\b",   "\\bsuper\\b",
        "\\bhasattr\\b","\\bgetattr\\b","\\bsetattr\\b","\\bisinstance\\b",
        "\\bException\\b","\\bValueError\\b","\\bTypeError\\b",
        "\\bRuntimeError\\b","\\bKeyError\\b","\\bIndexError\\b"
    };
    for (const QString &pattern : builtinPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format  = builtinFormat;
        highlightingRules.append(rule);
    }

    // ── self / cls ────────────────────────────────────────────────────────────
    selfFormat.setForeground(QColor("#E06C75")); // red
    selfFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("\\b(self|cls)\\b");
    rule.format  = selfFormat;
    highlightingRules.append(rule);

    // ── Decorators ────────────────────────────────────────────────────────────
    decoratorFormat.setForeground(QColor("#61AFEF")); // blue
    rule.pattern = QRegularExpression("@[A-Za-z_][A-Za-z0-9_.]*");
    rule.format  = decoratorFormat;
    highlightingRules.append(rule);

    // ── Function definitions ──────────────────────────────────────────────────
    functionFormat.setForeground(QColor("#61AFEF")); // blue
    rule.pattern = QRegularExpression("\\bdef\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\(");
    rule.format  = functionFormat;
    highlightingRules.append(rule);

    // ── Numbers ───────────────────────────────────────────────────────────────
    numberFormat.setForeground(QColor("#D19A66")); // orange
    rule.pattern = QRegularExpression(
        "\\b(0[xX][0-9A-Fa-f]+|0[oO][0-7]+|0[bB][01]+|[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)\\b");
    rule.format  = numberFormat;
    highlightingRules.append(rule);

    // ── Strings (single-line) ─────────────────────────────────────────────────
    stringFormat.setForeground(QColor("#98C379")); // green
    // f-strings, b-strings, r-strings with both quote styles
    rule.pattern = QRegularExpression(R"([fFbBrRuU]?("[^"\\]*(\\.[^"\\]*)*"|'[^'\\]*(\\.[^'\\]*)*'))");
    rule.format  = stringFormat;
    highlightingRules.append(rule);

    // ── Comments ──────────────────────────────────────────────────────────────
    commentFormat.setForeground(QColor("#5C6370")); // grey
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format  = commentFormat;
    highlightingRules.append(rule);

    // ── Multi-line string formats ─────────────────────────────────────────────
    multilineStringFormat.setForeground(QColor("#98C379"));
    tripleDoubleQuote = QRegularExpression(R"(""")");
    tripleSingleQuote = QRegularExpression(R"(''')");
}

void PythonHighlighter::highlightBlock(const QString &text) {
    // Single-line rules
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }

    // Multi-line triple-quoted strings
    // State: 0 = normal, 1 = inside """, 2 = inside '''
    setCurrentBlockState(0);

    auto processTriple = [&](const QRegularExpression &startExpr, int stateId) {
        int startIndex = 0;
        if (previousBlockState() != stateId)
            startIndex = startExpr.match(text).capturedStart();

        while (startIndex >= 0) {
            QRegularExpressionMatch endMatch = startExpr.match(text, startIndex + 3);
            int endIndex   = endMatch.capturedStart();
            int blockLen;
            if (endIndex == -1) {
                setCurrentBlockState(stateId);
                blockLen = text.length() - startIndex;
            } else {
                blockLen = endIndex - startIndex + endMatch.capturedLength();
            }
            setFormat(startIndex, blockLen, multilineStringFormat);
            startIndex = startExpr.match(text, startIndex + blockLen).capturedStart();
        }
    };

    if (previousBlockState() == 1 || text.contains("\"\"\""))
        processTriple(tripleDoubleQuote, 1);
    else if (previousBlockState() == 2 || text.contains("'''"))
        processTriple(tripleSingleQuote, 2);
}
