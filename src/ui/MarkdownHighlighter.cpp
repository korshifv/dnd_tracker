#include "MarkdownHighlighter.h"
#include <QApplication>
#include <QPalette>
#include <utility>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Headers: # Header
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setForeground(Qt::darkBlue);
    // You can't easily scale fonts dynamically per block in QSyntaxHighlighter without
    // causing re-layout issues, so we'll just bold and color it, maybe set slightly larger point size
    // but typically text char format font size is fixed in highlighting.
    headerFormat.setFontPointSize(14); 

    rule.pattern = QRegularExpression(QStringLiteral("^(#+)[ \\t]+(.+)"));
    rule.format = headerFormat;
    highlightingRules.append(rule);

    // Bold: **text** or __text__
    boldFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(QStringLiteral("(\\*\\*|__)(.+?)\\1"));
    rule.format = boldFormat;
    highlightingRules.append(rule);

    // Italic: *text* or _text_
    italicFormat.setFontItalic(true);
    // Negative lookbehind/lookahead to prevent matching ** as italic
    rule.pattern = QRegularExpression(QStringLiteral("(?<!\\*)(?<!_)(\\*|_)(?!\\*|_)(.+?)\\1(?!\\*|_)"));
    rule.format = italicFormat;
    highlightingRules.append(rule);

    // Wiki-links: [[Link]]
    // We use the application palette's Link color
    linkFormat.setForeground(QApplication::palette().color(QPalette::Link));
    linkFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression(QStringLiteral("\\[\\[([^\\]\\n]+?)\\]\\]"));
    rule.format = linkFormat;
    highlightingRules.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
