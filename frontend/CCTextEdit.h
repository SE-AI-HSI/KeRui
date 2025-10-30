#ifndef CCTEXTEDIT_H
#define CCTEXTEDIT_H
#include <QColor>
#include <QString>
#include <QTextEdit>

/**
 * @class CodeComparisonTextEdit
 * @brief 代码对比窗口中的文本框
 */

class CCTextEdit : public QTextEdit {
public:
    CCTextEdit(QWidget *parent);
    int lineCount();
    int minHeight();
    void setText(const QString &text);
    void insertNewLines(int line_below, int n = 1);
    QString getText(int line_beg, int line_end);
    void replaceText(const QString &t, int line_beg, int line_end);
    void deleteLine(int line);

    inline static int lineHeight() {
        return _lineHeight;
    }

private:
    /// inits
    void setupStyle();
    /// events
    void wheelEvent(QWheelEvent *e);
    /// others
    void setLineHeight();
    void gotoLine(int line);

    static int _lineHeight;
};

#endif // CCTEXTEDIT_H
