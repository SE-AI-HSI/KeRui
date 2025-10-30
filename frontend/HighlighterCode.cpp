#include "HighlighterCode.h"

std::vector<HighlightingRule> CodeHighlighter::rules;
int CodeHighlighter::counter = 0;
QRegExp CodeHighlighter::commentStartExp = QRegExp("/\\*");
QRegExp CodeHighlighter::commentEndExp = QRegExp("\\*/");
QTextCharFormat CodeHighlighter::multiLineCommentFormat;

CodeHighlighter::CodeHighlighter(QTextDocument *document) : QSyntaxHighlighter(document) {
    ++counter;
    if (counter == 1) {
        HighlightingRule rule;
        QTextCharFormat format;

        /** 关键字1 **/
        format.setForeground(QColor(192, 0, 192));
        // format.setFontWeight(QFont::Normal);
        QStringList keywordPatterns;
        keywordPatterns << "\\bchar\\b"
                        << "\\bclass\\b"
                        << "\\bconst\\b"
                        << "\\bdouble\\b"
                        << "\\benum\\b"
                        << "\\bexplicit\\b"
                        << "\\bfriend\\b"
                        << "\\binline\\b"
                        << "\\bint\\b"
                        << "\\blong\\b"
                        << "\\bnamespace\\b"
                        << "\\boperator\\b"
                        << "\\bprivate\\b"
                        << "\\bprotected\\b"
                        << "\\bpublic\\b"
                        << "\\bshort\\b"
                        << "\\bsignals\\b"
                        << "\\bsigned\\b"
                        << "\\bslots\\b"
                        << "\\bstatic\\b"
                        << "\\bstruct\\b"
                        << "\\btemplate\\b"
                        << "\\btypedef\\b"
                        << "\\btypename\\b"
                        << "\\bunion\\b"
                        << "\\bunsigned\\b"
                        << "\\bvirtual\\b"
                        << "\\bvoid\\b"
                        << "\\bvolatile\\b"
                        << "\\b<<\\b"
                        << "\\binclude\\b";
        for (QString &pattern : keywordPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = format;
            rules.push_back(rule);
        }

        /** 关键字2 **/
        format.setForeground(QColor(0, 164, 198));
        // format.setFontWeight(QFont::Normal);
        keywordPatterns.clear();
        keywordPatterns << "\\bwhile\\b"
                        << "\\bif\\b"
                        << "\\bswitch\\b"
                        << "\\bcout\\b"
                        << "\\bcase\\b"
                        << "\\bbreak\\b"
                        << "\\bfor\\b"
                        << "\\bcontinue\\b"
                        << "\\bdelete\\b"
                        << "\\bdo\\b"
                        << "\\bextern\\b"
                        << "\\bfalse\\b"
                        << "\\btrue\\b"
                        << "\\bnew\\b"
                        << "\\bdelete\\b"
                        << "\\bsizeof\\b"
                        << "\\bthis\\b"
                        << "\\busing\\b"
                        << "\\breturn\\b"
                        << "\\belse\\b"
                        << "\\bcin\\b"
                        << "\\bmain\\b";
        for (QString &pattern : keywordPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = format;
            rules.push_back(rule);
        }

        /** 函数名 **/
        format.setForeground(QColor(128, 0, 0));
        format.setFontItalic(true);
        rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
        rule.format = format;
        rules.push_back(rule);

        /** 类名 **/
        format.setForeground(Qt::darkMagenta);
        rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
        rule.format = format;
        rules.push_back(rule);

        /** 引号包围的字符串 **/
        format.setForeground(QColor(0, 203, 0));
        rule.pattern = QRegExp("\".*\"");
        rule.format = format;
        rules.push_back(rule);

        /** 单行注释 **/
        format.setForeground(QColor(0, 203, 0));
        rule.pattern = QRegExp("//[^\n]*");
        rule.format = format;
        rules.push_back(rule);

        /** 多行注释 **/
        multiLineCommentFormat.setForeground(QColor(0, 203, 0));
    }
}

CodeHighlighter::~CodeHighlighter() {
    --counter;
    if (counter == 0) {
        rules.clear();
        rules.shrink_to_fit();
    }
}

void CodeHighlighter::highlightBlock(const QString &text) {
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

    /** 下面是对多行注释进行高亮 **/
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExp.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExp.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + commentEndExp.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExp.indexIn(text, startIndex + commentLength);
    }
}
