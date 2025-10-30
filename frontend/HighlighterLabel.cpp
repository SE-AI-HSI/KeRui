#include "HighlighterLabel.h"

std::vector<HighlightingRule> LabelHighlighter::rules;
int LabelHighlighter::counter = 0;

LabelHighlighter::LabelHighlighter(QTextDocument *document) : QSyntaxHighlighter{document} {
    ++counter;
    if (counter == 1) {
        HighlightingRule rule;
        QTextCharFormat format;

        format.setForeground(QColor(255, 112, 101));
        rule.pattern = QRegExp(QString::fromLocal8Bit("\\[规范性\\]"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(255, 142, 60));
        rule.pattern = QRegExp(QString::fromLocal8Bit("\\[执行效率\\]"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(91, 203, 238));
        rule.pattern = QRegExp(QString::fromLocal8Bit("\\[安全性\\]"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(94, 228, 107));
        rule.pattern = QRegExp(QString::fromLocal8Bit("\\[可维护性\\]"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(255, 112, 101));
        rule.pattern = QRegExp(QString::fromLocal8Bit("【错误】"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(255, 142, 60));
        rule.pattern = QRegExp(QString::fromLocal8Bit("【警告】"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(182, 148, 255));
        rule.pattern = QRegExp(QString::fromLocal8Bit("【重复】"));
        rule.format = format;
        rules.push_back(rule);

        format.setForeground(QColor(255, 0, 230));
        rule.pattern = QRegExp(QString::fromLocal8Bit("【建议】"));
        rule.format = format;
        rules.push_back(rule);
    }
}

LabelHighlighter::~LabelHighlighter() {
    --counter;
    if (counter == 0) {
        rules.clear();
        rules.shrink_to_fit();
    }
}

void LabelHighlighter::highlightBlock(const QString &text) {
    for (HighlightingRule &rule : rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text), length;
        while (index >= 0) {
            length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}
