#include "TextEdit.h"
#include <QDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QFont>

TextEditEventProxy TextEdit::eventProxy;

int TextEdit::_fontWidth = 0;
int TextEdit::_lineHeight = 0;

TextEdit::TextEdit(QWidget* parent) : QTextEdit(parent)
{
    codeHighlighter = new CodeHighlighter(document());
    autoComplete = new AutoComplete(this);
    autoComplete->hide();

    setupStyle();
    connectEvents();

    _lineHeight = this->fontMetrics().lineSpacing();
    _fontWidth = this->fontMetrics().horizontalAdvance(' ');
    setTabStopWidth(4 * _fontWidth);
}

void TextEdit::selectLines(int lowerBound, int upperBound)
{
    int beg = document()->findBlockByNumber(lowerBound).position();
    QTextBlock lastBlock = document()->findBlockByNumber(upperBound);
    int end = lastBlock.position() + lastBlock.length() - 1;

    QTextCursor cursor = textCursor();
    cursor.setPosition(beg, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void TextEdit::setText(const QString &text)
{
    clear();
    setLineHeight();
    insertPlainText(text);
    if(!isTextInitialized){
        isTextInitialized = true;
        QTextCursor c = textCursor();
        c.setPosition(0, QTextCursor::MoveAnchor);
        setTextCursor(c);
    }
}

QString TextEdit::wordUnderCursor()
{
    QString res;
    int pos = textCursor().position();
    QChar ch;
    while (pos >= 1) {
        ch = document()->characterAt(pos - 1);
        if (!(ch.isLetterOrNumber() || ch == '_')) ///汉字也会被识别成字母
            break;
        --pos;
    }
    ch = document()->characterAt(pos);
    while (ch.isLetterOrNumber() || ch == '_') {
        res += ch;
        ++pos;
        ch = document()->characterAt(pos);
    }
    return res;
}

QString TextEdit::stringBeforeCursor()
{ //返回光标前的字符串,遇到空格或者换行结束
    QString res;
    int pos = textCursor().position(), originalPos = pos;
    int blockFront = textCursor().block().position();

    QChar ch;
    while (pos > blockFront) {
        ch = document()->characterAt(pos - 1);
        if (ch.isSpace())
            break;
        --pos;
    }
    ch = document()->characterAt(pos);
    while (pos < originalPos) {
        res += ch;
        ++pos;
        ch = document()->characterAt(pos);
    }
    return res;
}

QString TextEdit::typingWord()
{
    int pos = textCursor().position();
    QChar ch = document()->characterAt(pos);
    if (ch != ' ' && !textCursor().atBlockEnd())
        return "";
    QString word = stringBeforeCursor();
    if (word.isEmpty())
        return "";
    if (word[0] == '#' || word[0].isLetter())
        return word;
    return "";
}

int TextEdit::lineCount() const
{
    return document()->blockCount();
}

void TextEdit::wheelEvent(QWheelEvent *)
{
    //保持空,不接受滚轮事件,让其父窗口处理//
}

void TextEdit::mousePressEvent(QMouseEvent *event)
{
    QTextEdit::mousePressEvent(event);
    clickedLine = event->y() / lineHeight();
    if (clickedLine < lineCount()) {
        emit selectLinesSignal(clickedLine, clickedLine);
    }
    autoComplete->hide();                                    //鼠标点击的时候要把自动补全隐藏
}

void TextEdit::mouseMoveEvent(QMouseEvent *event)
{
    QTextEdit::mouseMoveEvent(event);
    if (clickedLine != -1) {
        int lb = clickedLine, ub = event->y() / lineHeight();//ub=鼠标当前所在的行//
        if (lb > ub) {
            std::swap(lb, ub);
        }
        ub = std::min(lineCount() - 1, ub);
        emit selectLinesSignal(lb, ub);
    }
}

void TextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    QTextEdit::mouseReleaseEvent(e);
    clickedLine = -1;
}

void TextEdit::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << event->key();
    ///backspace
    if (event->key() == Qt::Key_Backspace) {
        backspace_key_event(event);
        autoComplete->hide();
    }
    ///tab
    else if (event->key() == Qt::Key_Tab) {
        this->textCursor().insertText("    ");
        autoComplete->hide();
    }
    ///shift
    else if (event->modifiers() == Qt::ShiftModifier) {
        shift_key_event(event);
        autoComplete->hide();
    }
    ///shift + alt + f格式化代码
    else if (event->key() == 70 && event->modifiers() == (Qt::ShiftModifier | Qt::AltModifier)) {
        emit eventProxy.formatCode();
        event->accept();
    }
    ///ctrl
    else if (event->modifiers() == Qt::ControlModifier) {
        ctrl_key_event(event);
    }
    ///auto complete
    else if (autoComplete->isVisible()) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            autoComplete->insert();
        } else if (event->key() == 16777235) {
            autoComplete->choosePrevious();
        } else if (event->key() == 16777237) {
            autoComplete->chooseNext();
        } else {
            QTextEdit::keyPressEvent(event);
            autoComplete->show();
        }
    }
    ///enter
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        //return是中间的回车, enter是数字键盘右边的enter
        enter_key_event();
    }
    ///others
    else {
        QTextEdit::keyPressEvent(event);
        autoComplete->show();
    }
}

void TextEdit::ctrl_key_event(QKeyEvent *event)
{
    if (event->key() == 47) { //ctrl+/(注释)
        QTextCursor c = this->textCursor();
        if (this->textCursor().hasSelection()) {
            int sbeg = c.selectionStart();
            int send = c.selectionEnd();
            if (sbeg > send)
                std::swap(sbeg, send);
            c.clearSelection();
            this->setTextCursor(c);
            //this->textCursor().clearSelection(); <- 这样是不会清除选中的

            QTextBlock firstBlock = this->document()->findBlock(sbeg);
            QTextBlock curBlock = firstBlock;
            QTextBlock lastBlock = this->document()->findBlock(send);
            bool isCommented = true;
            while (1) {
                if (curBlock.text().trimmed().startsWith("//")) {
                    if (curBlock.blockNumber() == lastBlock.blockNumber())
                        break;
                    else
                        curBlock = curBlock.next();
                } else {
                    isCommented = false;
                    break;
                }
            }
            curBlock = firstBlock;
            int pos = firstBlock.position();
            if (isCommented) {
                while (1) {
                    c.setPosition(pos);
                    this->setTextCursor(c);
                    while (document()->characterAt(pos) != '/') {
                        this->moveCursor(QTextCursor::Right);
                        ++pos;
                    }
                    this->textCursor().deleteChar();
                    this->textCursor().deleteChar();
                    if (curBlock.blockNumber() == lastBlock.blockNumber())
                        break;
                    curBlock = curBlock.next();
                    pos = curBlock.position();
                }
            } else {
                while (1) {
                    c.setPosition(pos);
                    this->setTextCursor(c);
                    this->insertPlainText("//");
                    if (curBlock.blockNumber() == lastBlock.blockNumber())
                        break;
                    curBlock = curBlock.next();
                    pos = curBlock.position();
                }
            }
        } else {
            QString lineText = c.block().text();
            this->moveCursor(QTextCursor::StartOfBlock);
            int pos = this->textCursor().position();
            if (lineText.trimmed().startsWith("//")) {
                while (this->document()->characterAt(pos) != '/') {
                    this->moveCursor(QTextCursor::Right);
                    ++pos;
                }
                this->textCursor().deleteChar();
                this->textCursor().deleteChar();
            } else {
                for (int i = 0; i < lineText.length(); ++i) {
                    if (lineText[i].isSpace())
                        ++pos;
                    else
                        break;
                }
                c.setPosition(pos);
                this->setTextCursor(c);
                this->insertPlainText("//");
            }
        }
    } else if (event->key() == 83) //ctrl+s
        emit eventProxy.saveFile();
    else { if (event->key() == Qt::Key_H)//ctrl+h
        {
            emit eventProxy.findAndReplaceWords();
        }
    else
        QTextEdit::keyPressEvent(event);
       }
}

void TextEdit::shift_key_event(QKeyEvent *event)
{
    if (this->textCursor().hasSelection()) {
        //如果选中了文本, 套上一对符号
        if (event->key() == 40) {        //左括号
            wrap_text_with_symbol("(", ")");
        } else if (event->key() == 34) { //双引号
            wrap_text_with_symbol("\"", "\"");
        } else if (event->key() == 123) {//左花括号
            wrap_text_with_symbol("{", "}");
        } else {
            QTextEdit::keyPressEvent(event);
        }
        return;
    }
    //本行的文本
    const QString curLineText = this->document()->findBlockByNumber(this->textCursor().blockNumber()).text();
    //光标在本行的位置(列数)
    int columnNumber = this->textCursor().columnNumber();
    if (columnNumber == curLineText.length() || curLineText.at(columnNumber) == QChar::Space) {
        //光标后有空, 插入一对
        if (event->key() == 40) {                                 //左括号
            this->insertPlainText("()");
            this->moveCursor(QTextCursor::PreviousCharacter);
        } else if (event->key() == 34) {                          //双引号
            this->insertPlainText("\"\"");
            this->moveCursor(QTextCursor::PreviousCharacter);
        } else if (event->key() == 123) {                         //左花括号
            this->insertPlainText("{}");
            this->moveCursor(QTextCursor::PreviousCharacter);
        } else {
            QTextEdit::keyPressEvent(event);
        }
    } else if (columnNumber != 0) {
        //光标在一对符号中间, 使光标后移一个字符
        QString aroundCourse = curLineText.mid(columnNumber - 1, 2);
        if (event->key() == 41 && aroundCourse == "()") {         //右括号
            this->moveCursor(QTextCursor::NextCharacter);
        } else if (event->key() == 34 && aroundCourse == "\"\"") {//双引号
            this->moveCursor(QTextCursor::NextCharacter);
        } else if (event->key() == 125 && aroundCourse == "{}") { //右花括号
            this->moveCursor(QTextCursor::NextCharacter);
        } else if (event->key() == 62 && aroundCourse == "<>") {  //右尖括号
            this->moveCursor(QTextCursor::NextCharacter);
        } else {
            QTextEdit::keyPressEvent(event);
        }
    } else {
        //光标在行的最左边, 插入单个
        QTextEdit::keyPressEvent(event);
    }
}

void TextEdit::backspace_key_event(QKeyEvent *event)
{
    if (this->textCursor().hasSelection()) {
        //如果选中了文本就直接删
        QTextEdit::keyPressEvent(event);
        return;
    }
    //本行的文本
    //const QString curLineText = this->document()->findBlockByNumber(this->textCursor().blockNumber()).text();
    const QString curLineText = this->textCursor().block().text();
    //光标在本行的位置(列数)
    int columnNumber = this->textCursor().columnNumber();
    if (columnNumber == curLineText.length() || columnNumber == 0) {
        QTextEdit::keyPressEvent(event);
    } else {
        QString aroundCourse = curLineText.mid(columnNumber - 1, 2);
        if (aroundCourse == "()" || aroundCourse == "{}" || aroundCourse == "[]" ||
            aroundCourse == "\"\"" || aroundCourse == "<>" || aroundCourse == "''") {
            this->moveCursor(QTextCursor::NextCharacter);
            this->textCursor().deletePreviousChar();
            this->textCursor().deletePreviousChar();
        } else {
            //光标不在最后
            QTextEdit::keyPressEvent(event);
        }
    }
}

void TextEdit::enter_key_event()
{
    //本行的文本
    const QString curLineText = this->document()->findBlockByNumber(this->textCursor().blockNumber()).text();
    //光标在本行的位置(列数)
    int columnNumber = this->textCursor().columnNumber();
    int firstChar = 0, lastChar = 0; //第一个不为空和最后一个不为空的字符的位置
    for (int i = 0; i < curLineText.length(); ++i) {
        if (curLineText[i] != ' ') {
            firstChar = i;
            break;
        }
    }
    for (int i = curLineText.length() - 1; i >= 0 ; --i) {
        if (curLineText[i] != ' ') {
            lastChar = i;
            break;
        }
    }

    if (this->textCursor().hasSelection()) {
        //选中了文本
        insert_new_line(curLineText);
    } else if (columnNumber <= firstChar) {
        //光标在第一个不非空字符之前
        QTextCursor c = this->textCursor();
        while (columnNumber < curLineText.length() && curLineText[columnNumber].isSpace())
            columnNumber++, c.movePosition(QTextCursor::NextCharacter); //将光标移到第一个非空字符前
        this->setTextCursor(c);
        insert_new_line(curLineText);
    } else if (columnNumber > lastChar) {
        //光标在最后一个非空字符之后
        QTextCursor c = this->textCursor();
        while (columnNumber < curLineText.length())
            columnNumber++, c.movePosition(QTextCursor::NextCharacter); //将光标移到本行最后
        this->setTextCursor(c);
        insert_new_line(curLineText);
        static const QRegExp rx(
            "\\}? ?(if|else if|for|while|foreach|switch) ?\\([^/]*\\)([^;]*| ?\\{.*)"
            "|\\}? ?else(( [^;]*)?| ?\\{.*)"
            "|case .*:.*"
            "|do ?\\{"
            );
        QString simplified = curLineText.simplified();
        if (rx.exactMatch(simplified))
            this->insertPlainText("    ");
    } else {
        //光标在中间
        int flag = 0;
        for (int i = columnNumber - 1; i >= 0; --i) {
            if (curLineText[i].isSpace()) continue;
            if (curLineText[i] == '{') {
                flag++;
                break;
            } else break;
        }
        for (int i = columnNumber; i < curLineText.length(); ++i) {
            if (curLineText[i].isSpace()) continue;
            if (curLineText[i] == '}') {
                flag++;
                break;
            } else break;
        }
        if (flag == 2) {                                                    //flag==2表示光标刚好在一对花括号中间
            QTextCursor c = this->textCursor();
            while (curLineText[columnNumber] != '}')
                columnNumber++, c.movePosition(QTextCursor::NextCharacter); //将光标移到'}'前
            this->setTextCursor(c);
            insert_new_line(curLineText);

            int pos = this->textCursor().position();                        //'{'与'}'中间一行的位置
            insert_new_line(curLineText);
            c.setPosition(pos);
            this->setTextCursor(c);
            this->textCursor().insertText("    ");
        } else {
            insert_new_line(curLineText);
        }
    }
}

void TextEdit::wrap_text_with_symbol(QString front, QString back)
{ //给选中的文本两边包上符号, 调用前确保选中了文本
    int beg = this->textCursor().selectionStart(), end = this->textCursor().selectionEnd();

    QTextCursor c = this->textCursor();
    c.clearSelection();
    setTextCursor(c);

    c.setPosition(beg);
    setTextCursor(c);
    this->insertPlainText(front);
    c.setPosition(end + 1);
    setTextCursor(c);
    this->insertPlainText(back);
}

void TextEdit::insert_new_line(const QString &curLineText)
{
    this->insertPlainText("\n");
    for (const QChar &c : curLineText) {
        if (c.isSpace())
            this->insertPlainText(c);
        else
            break;
    }
}

void TextEdit::setLineHeight()
{
    QTextCursor c = textCursor();
    QTextBlockFormat blockFormat = c.blockFormat();
    blockFormat.setLineHeight(_lineHeight, QTextBlockFormat::FixedHeight);

    c.select(QTextCursor::Document);
    c.mergeBlockFormat(blockFormat);
    c.clearSelection();
    setTextCursor(c);
}

void TextEdit::connectEvents()
{
    /** 文本内容发生了改变 **/
    connect(this, &TextEdit::textChanged, [ = ]{
        if (isTextInitialized) {
            //第一次读入文件内容的时候会触发textChanged,就会导致CodeBox标记该文件为已修改//
            //然后CodeBox自动保存的时候就会保存该文件,即刚打开什么都没改就又写回磁盘//
            //textIsInitialized就是作为一个标记防止这种情况发生//
            emit eventProxy.textChanged();
        }
    });
}

void TextEdit::setupStyle()
{
    setLineWrapMode(NoWrap);                             //关闭自动文字折行//
    setFont(QFont("Source Code Pro, Microsoft Yahei", 15));

    setStyleSheet("background-color: transparent;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//隐藏滑动条//
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    document()->setDocumentMargin(0);
}

void TextEdit::insertFromMimeData( const QMimeData * source)
{
    if (source->hasText() ) {
            QString context = source->text();
            QStringList conList = context.split("\n");
            QTextCursor cursor = this->textCursor();

//            int origState = cursor.block().userState();

            for (int i = 0; i < conList.length(); ++i) {
                cursor.insertBlock(/* 格式设置在此 */);//使用光标处的格式
                cursor.insertText(conList[i]); //插入文本内容
//                cursor.block().setUserState(origState);
            }
        }
}

void TextEdit::highlightAWord(int pos ,int length ,QColor color)//高亮一个单词
{
    QTextEdit::ExtraSelection selection;

    QTextCursor cursor=this->textCursor();
    cursor.setPosition(pos , QTextCursor::MoveAnchor);//定位到下标index的位置
    cursor.setPosition(pos + length, QTextCursor::KeepAnchor);   //文本选择范围[index,index + length]

    selection.format.setBackground(color);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    selection.cursor = cursor;
    extraSelections.append(selection);

    cursor.setPosition(pos , QTextCursor::MoveAnchor);
    this->setTextCursor(cursor);

    setExtraSelections(extraSelections);
}

void TextEdit::highlightALine(int pos, QColor color)
{
    if(highlightLineSelectionIndex > -1)
        extraSelections.removeAt( highlightLineSelectionIndex );
    QTextEdit::ExtraSelection selection;

    QTextCursor cursor = this->textCursor();
    cursor.setPosition(pos , QTextCursor::MoveAnchor);//定位到下标index的位置

    selection.format.setBackground(color);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    selection.cursor = cursor;
    extraSelections.append(selection);
    highlightLineSelectionIndex = extraSelections.size() - 1;

    cursor.setPosition(pos , QTextCursor::MoveAnchor);
    this->setTextCursor(cursor);

    setExtraSelections(extraSelections);
}

void TextEdit::deleteHighlight()
{
    QList<QTextEdit::ExtraSelection> deleteSelections;
    extraSelections.swap(deleteSelections);
    setExtraSelections(extraSelections);
}

void TextEdit::replaceAWord(int pos, int length, QString word)
{
    QTextCursor tc = textCursor();
    tc.setPosition( pos, QTextCursor::MoveAnchor );
    tc.setPosition( pos + length, QTextCursor::KeepAnchor );
    setTextCursor(tc);
    tc.removeSelectedText();//删除再添加
    tc.insertText(word);
}
