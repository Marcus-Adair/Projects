#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
        HighlightingRule rule;

        keywordFormat.setForeground(Qt::darkBlue);
        keywordFormat.setFontWeight(QFont::Bold);
        const QString keywordPatterns[] = {
            QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcase\\b"), QStringLiteral("\\bcatch\\b"),
            QStringLiteral("\\bcontinue\\b"), QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bdelete\\b"),
            QStringLiteral("\\bdo\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("\\bfinally\\b"),
            QStringLiteral("\\bfor\\b"), QStringLiteral("\\bfunction\\b"), QStringLiteral("\\bif\\b"),
            QStringLiteral("\\bin\\b"), QStringLiteral("\\binstanceof\\b"), QStringLiteral("\\bnew\\b"),
            QStringLiteral("\\breturn\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bthis\\b"),
            QStringLiteral("\\bthrow\\b"), QStringLiteral("\\btry\\b"), QStringLiteral("\\btypeof\\b"),
            QStringLiteral("\\bvar\\b"), QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bwhile\\b"),
            QStringLiteral("\\bwith\\b"), QStringLiteral("\\bnull\\b"), QStringLiteral("\\btrue\\b"),
            QStringLiteral("\\bfalse\\b")
        };

        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegularExpression(pattern);
            rule.format = keywordFormat;
            highlightingRules.append(rule);
        }

        classFormat.setFontWeight(QFont::Bold);
        classFormat.setForeground(Qt::darkMagenta);
        rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
        rule.format = classFormat;
        highlightingRules.append(rule);

        quotationFormat.setForeground(Qt::darkGreen);
        rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
        rule.format = quotationFormat;
        highlightingRules.append(rule);

        functionFormat.setFontItalic(true);
        functionFormat.setForeground(Qt::blue);
        rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
        rule.format = functionFormat;
        highlightingRules.append(rule);

        singleLineCommentFormat.setForeground(Qt::red);
        rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);

        multiLineCommentFormat.setForeground(Qt::red);

        commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
        commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
