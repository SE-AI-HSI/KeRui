#ifndef HIGHLIGHTERCODE_H
#define HIGHLIGHTERCODE_H

#include "HighlighterLabel.h"
#include <QtCore/QRegExp>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <vector>

/**
 * @class CodeHighlighter
 * @brief 代码语法高亮器
 */

class CodeHighlighter : public QSyntaxHighlighter {
public:
    explicit CodeHighlighter(QTextDocument *document);
    ~CodeHighlighter();

private:
    void highlightBlock(const QString &text) override;

private:
    static std::vector<HighlightingRule> rules; // 可以共享//
    static int counter;                         // 计数,为0时清空rules//

    /** 多行注释相关 **/
    static QTextCharFormat multiLineCommentFormat;
    static QRegExp commentStartExp;
    static QRegExp commentEndExp;
};

#endif // HIGHLIGHTERCODE_H
