#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class MarkdownHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };

  QList<HighlightingRule> highlightingRules;

  QTextCharFormat headerFormat;
  QTextCharFormat boldFormat;
  QTextCharFormat italicFormat;
  QTextCharFormat linkFormat;
};

#endif // MARKDOWNHIGHLIGHTER_H
