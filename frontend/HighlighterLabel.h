#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QtCore/QRegExp>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <vector>

struct HighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
};

/**
 * @class LabelHighlighter
 * @brief 用于AnalysisLabel的高亮
 */

class LabelHighlighter : public QSyntaxHighlighter {
public:
    explicit LabelHighlighter(QTextDocument *document);
    ~LabelHighlighter();

private:
    void highlightBlock(const QString &text) override;

private:
    static std::vector<HighlightingRule> rules; // 可以共享//
    static int counter;                         // 计数,为0时清空rules//
};

#endif // HIGHLIGHTER_H
