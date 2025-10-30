#ifndef MYPLAINTEXTEDIT_H
#define MYPLAINTEXTEDIT_H

#include "AutoComplete.h"
#include "HighlighterCode.h"
#include <QMimeData>
#include <QMouseEvent>
#include <QTextEdit>

class TextEditEventProxy;

class TextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);
    void selectLines(int lowerBound, int upperBound); // 选中[lb,ub]区间内的行//
    void setText(const QString &text);
    QString wordUnderCursor();
    QString stringBeforeCursor();

    QString typingWord();

    QString codeText();

    QList<QTextEdit::ExtraSelection> extraSelections; // 存储高亮效果的链表
    int highlightLineSelectionIndex = -1;
    void highlightAWord(int pos, int length, QColor color); // 高亮一个单词
    void highlightALine(int pos, QColor color);
    void deleteHighlight();                               // 清除全部高亮效果
    void replaceAWord(int pos, int length, QString word); // 换掉一个单词

    // get methods
    int lineCount() const;
    static inline int fontWidth() {
        return _fontWidth;
    }
    static inline int lineHeight() {
        return _lineHeight;
    }

private:
    // init
    void setupStyle();

    // events
    void connectEvents();
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;

    /// 按键响应
    void ctrl_key_event(QKeyEvent *event);
    void shift_key_event(QKeyEvent *event);
    void backspace_key_event(QKeyEvent *event);
    void enter_key_event();
    void wrap_text_with_symbol(QString front, QString back);
    void insert_new_line(const QString &curLineText);

    // others
    void setLineHeight();

    /// 粘贴文本
    virtual void insertFromMimeData(const QMimeData *source) override;

signals:
    /**
     * @signal: selectLinesSignal
     * @emit when: 选中行
     * @receiver: CodeEditor -> LineNumberList
     */
    void selectLinesSignal(int lowerBound, int upperBound);

private:
    int clickedLine = -1; // 鼠标按下时所在的行号//
    CodeHighlighter *codeHighlighter;
    AutoComplete *autoComplete;

    static int _lineHeight;
    static int _fontWidth;

public:
    bool isTextInitialized = false; // 用途见connectEvents//
    static TextEditEventProxy eventProxy;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

/**
 * @class TextEditEventProxy
 * @brief 放在PlainTextEdit中的<静态>对象,所有PlainTextEdit实例通过它来发送textChanged()
 * 这个信号,因此CodeBox只需要connect一个信号,而不需要为每个PlainTextEdit实例都connect一个信号
 */

class TextEditEventProxy : public QObject {
    Q_OBJECT

signals:
    /**
     * @signal: textChanged
     * @emit when: 收到PlainTextEdit::textChanged信号
     * @receiver: CodeBox
     */
    void textChanged();

    /**
     * @signal: saveFile
     * @emit when: 按下ctrl+s
     * @receiver: CodeBox
     */
    void saveFile();

    /**
     * @signal: formatCode
     * @note:
     * @emit when: 按下shift + alt + f键
     * @receiver: CodeBox
     */
    void formatCode();

    /**
     * @signal: findAndReplaceWords
     * @emit when: 按下ctrl+h
     * @receiver: CodeBox
     */
    void findAndReplaceWords();
};

#endif // MYPLAINTEXTEDIT_H
