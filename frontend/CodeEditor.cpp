#include "CodeEditor.h"

#include <QColor>
#include <QDebug>
#include <QScrollBar>
#include <QVBoxLayout>

#include "AnalysisLabel.h"
#include "UrlInterpreter.h"

#define PADDING 4  // PlainTextEdit里面的padding
#define HIGHLIGHTWORDCOLOR QColor(Qt::yellow).lighter(170)
#define HIGHLIGHTLINECOLOR QColor(Qt::yellow).lighter(185)

QString CodeEditor::wordToBeFound = "";
QRegExp CodeEditor::REGToBeUsed = QRegExp();
int CodeEditor::lastIndex = 0;
QStack<int> CodeEditor::findWordsSTK = QStack<int>();

CodeEditor::CodeEditor(QWidget *parent) : QWidget{parent} {
    setupLayout();
    setupStyle();

    connectEvents();
    connectUrlEvents();
}

CodeEditor::~CodeEditor() {
    this->disconnect();
}

TextEdit *CodeEditor::getTextEdit() const {
    return textEdit;
}

void CodeEditor::setTextToEditor(const QString &str) {
    textEdit->setText(str);
}

void CodeEditor::updateLabels() {
    labelManager->updateLabels();
}

LabelManager *CodeEditor::getLabelManager() {
    return labelManager;
}

QString CodeEditor::getCode() {
    return textEdit->toPlainText();
}

void CodeEditor::finishFindAndReplace() {
    textEdit->deleteHighlight();
    textEdit->highlightLineSelectionIndex = -1;
    CodeEditor::findWordsSTK.clear();
    CodeEditor::REGToBeUsed = QRegExp();
    CodeEditor::lastIndex = 0;
    CodeEditor::wordToBeFound = "";
}

void CodeEditor::searchForText() {
    CodeEditor::findWordsSTK.clear();
    textEdit->deleteHighlight();
    QString codeText = textEdit->toPlainText();
    int pos = 0, counter = -1;
    pos = CodeEditor::REGToBeUsed.indexIn(codeText);
    while (pos > -1) {
        codeText = codeText.remove(0, pos + 1);
        counter += pos + 1;
        CodeEditor::findWordsSTK.append(counter);
        pos = CodeEditor::REGToBeUsed.indexIn(codeText);
    }

    if (CodeEditor::findWordsSTK.size() != 0) {
        // 给全部找到的单词浅高亮
        for (int temInt : CodeEditor::findWordsSTK)
            textEdit->highlightAWord(temInt, CodeEditor::wordToBeFound.size(), HIGHLIGHTWORDCOLOR);
        // 给其中一个的一行高量
        if (CodeEditor::lastIndex == 0 || CodeEditor::lastIndex > CodeEditor::findWordsSTK.size())
            CodeEditor::lastIndex = 1;
        textEdit->highlightALine(CodeEditor::findWordsSTK.at(CodeEditor::lastIndex - 1), HIGHLIGHTLINECOLOR);
        textEdit->highlightAWord(CodeEditor::findWordsSTK.at(CodeEditor::lastIndex - 1),
                                 CodeEditor::wordToBeFound.size(), HIGHLIGHTWORDCOLOR);
    }
}

QRegExp CodeEditor::textToREG(QString newText, int Flags) {
    if (Flags & FINDREG) {
        if (Flags & FINDWHOLEWORD) {
            if (!newText.startsWith("\\b"))
                newText = "\\b" + newText;
            if (!newText.endsWith("\\b"))
                newText = newText + "\\b";
        }
    } else {
        //* . ? + ^ $ | \ / [ ] ( ) { } 需要加转义符
        newText = newText.replace("*", "\\*");
        newText = newText.replace(".", "\\.");
        newText = newText.replace("?", "\\?");
        newText = newText.replace("+", "\\+");
        newText = newText.replace("^", "\\^");
        newText = newText.replace("$", "\\$");
        newText = newText.replace("|", "\\|");
        newText = newText.replace("\\", "\\\\");
        newText = newText.replace("/", "\\/");
        newText = newText.replace("[", "\\[");
        newText = newText.replace("]", "\\]");
        newText = newText.replace("{", "\\{");
        newText = newText.replace("}", "\\}");
        newText = newText.replace("(", "\\(");
        newText = newText.replace(")", "\\)");
        if (Flags & FINDWHOLEWORD)
            newText = "\\b" + newText + "\\b";
    }
    return QRegExp(newText);
}

bool CodeEditor::findNextWord() {
    if (CodeEditor::lastIndex == CodeEditor::findWordsSTK.size())  // 最后一个
        CodeEditor::lastIndex = 0;
    textEdit->highlightALine(CodeEditor::findWordsSTK.at(++CodeEditor::lastIndex - 1), HIGHLIGHTLINECOLOR);
    textEdit->highlightAWord(CodeEditor::findWordsSTK.at(CodeEditor::lastIndex - 1), CodeEditor::wordToBeFound.size(),
                             HIGHLIGHTWORDCOLOR);
    return true;
}

bool CodeEditor::findPreWord() {
    if (CodeEditor::lastIndex == 1)  // 第一个
        CodeEditor::lastIndex = CodeEditor::findWordsSTK.size() + 1;
    textEdit->highlightALine(CodeEditor::findWordsSTK.at(--CodeEditor::lastIndex - 1), HIGHLIGHTLINECOLOR);
    textEdit->highlightAWord(CodeEditor::findWordsSTK.at(CodeEditor::lastIndex - 1), CodeEditor::wordToBeFound.size(),
                             HIGHLIGHTWORDCOLOR);
    return true;
}

bool CodeEditor::replaceOneWord(QString replaceWord) {
    if (CodeEditor::findWordsSTK.size() == 0 || CodeEditor::lastIndex < 1 ||
        CodeEditor::findWordsSTK.size() < CodeEditor::lastIndex)
        return false;

    textEdit->replaceAWord(CodeEditor::findWordsSTK.at(CodeEditor::lastIndex - 1), CodeEditor::wordToBeFound.size(),
                           replaceWord);
    if (CodeEditor::lastIndex > 1)
        CodeEditor::findWordsSTK.remove(--CodeEditor::lastIndex);
    else  // lastIndex == 1 第一个
        CodeEditor::findWordsSTK.pop_front();

    return true;
}

bool CodeEditor::replaceAllWords(QString replaceWord) {
    if (CodeEditor::findWordsSTK.size() == 0 || CodeEditor::lastIndex < 1 ||
        CodeEditor::findWordsSTK.size() < CodeEditor::lastIndex)
        return false;

    int diff = replaceWord.size() - CodeEditor::wordToBeFound.size();

    for (QStack<int>::Iterator it = CodeEditor::findWordsSTK.begin(); it != CodeEditor::findWordsSTK.end(); it++) {
        textEdit->replaceAWord(*it, CodeEditor::wordToBeFound.size(), replaceWord);
        for (QStack<int>::Iterator innerIt = it + 1; innerIt != CodeEditor::findWordsSTK.end(); innerIt++)
            *innerIt += diff;  // 替换之后的字符数量会变
    }

    CodeEditor::findWordsSTK.clear();
    CodeEditor::lastIndex = 0;
    return true;
}

void CodeEditor::passInfo(const QList<IWarningInfo> &warningInfo, const QList<IErrorInfo> &errorInfo,
                          const QVector<QPair<int, QList<QList<ClonePosition>>>> &cloneSets, const QString &filePath,
                          const QString &fileRelativelyPath) {
    labelManager->warningInfo(warningInfo);
    labelManager->errorInfo(errorInfo);
    labelManager->cloneInfo(cloneSets, filePath, fileRelativelyPath);
    labelManager->sortAndShowLabels();
}

void CodeEditor::clearLabels() {
    labelManager->clearLabels();
}

void CodeEditor::setupLayout() {
    scrollArea = new QScrollArea(this);
    textEdit = new TextEdit();
    labelManager = new LabelManager(scrollArea, textEdit);
    textEdit->setParent(labelManager);
    lineNumberList = new LineNumberList(this, TextEdit::lineHeight());

    QVBoxLayout *labelManagerLayout = new QVBoxLayout(labelManager);
    labelManagerLayout->setContentsMargins(4, 0, 4, 0);
    labelManagerLayout->addWidget(textEdit);

    scrollArea->setWidget(labelManager);

    QWidget *background = new QWidget(this);  // LineNumberList的占位背景//
    background->setStyleSheet("background-color: rgb(255, 155, 82);");
    background->setFixedWidth(lineNumberList->width());

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 7, 0);  // 使滑动条距离右边有一定的距离
    layout->setSpacing(0);
    layout->addWidget(background);
    layout->addWidget(scrollArea, 1);

    lineNumberList->raise();
}

void CodeEditor::optimizeCloneCodes(QObject *const _label) {
    //    AnalysisLabel* label = static_cast<AnalysisLabel*>(_label);
    //    std::vector<void*> cloneSet = label->data->cloneNodeSet->getSimNodes();
    //    CodeOptimizer::g_optimizer.OptimizeCodes(&cloneSet);
}

void CodeEditor::setupStyle() {
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QWidget{"
        "border: none;"
        "background-color: transparent;"
        "}"
        "QScrollBar{"
        "background-color: rgb(255, 245, 239);"
        "}"
        "QScrollBar:vertical {"
        "width: 18px;"
        "}"
        "QScrollBar:horizontal {"
        "background: transparent;"
        "height: 18px;"
        "}"
        "QScrollBar::handle {"
        "background-color: rgb(255, 172, 112);"
        "border-radius: 9px;"
        "}"
        "QScrollBar::handle:vertical {"
        "min-height: 60px;"
        "}"
        "QScrollBar::add-line {"
        "width: 0px;"
        "height: 0px;"
        "}"
        "QScrollBar::sub-line{"
        "width: 0px;"
        "height: 0px;"
        "}"
        "QScrollBar::add-page {"
        "background: none;"
        "}"
        "QScrollBar::sub-page{"
        "background: none;"
        "}");
}

void CodeEditor::connectEvents() {
    // 文本区光标移动//
    connect(textEdit, &QTextEdit::cursorPositionChanged, [=]() {
        if (!textEdit->isTextInitialized)
            return;
        /**
         * @todo 按delete的时候cursor的位置是不动的,即不会触发这个槽函数,到时候需要在键盘响应事件中监听一下
         */
        resizeWidget();
        // 滚动scrollArea确保光标在可见窗口内//
        scrollArea->ensureVisible(textEdit->cursorRect().x(), textEdit->cursorRect().y(),
                                  textEdit->fontWidth() + PADDING, textEdit->lineHeight() + PADDING);
        lineNumberList->move(0, labelManager->y());
    });

    // 滚动时LineNumberList跟随滚动//
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            [=] { lineNumberList->move(0, labelManager->y()); });

    // 鼠标在LineNumberList上滚动//
    connect(lineNumberList, &LineNumberList::wheel, [=](QWheelEvent *e) {
        if (e->orientation() == Qt::Vertical) {
            if (e->angleDelta().y() > 0) {  // 向上滚动//
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() -
                                                          scrollArea->verticalScrollBar()->maximum() * 0.2);
            } else {
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() +
                                                          scrollArea->verticalScrollBar()->maximum() * 0.2);
            }
        }
    });

    // 从LineNumberList中选中行,更新PlainTextEdit//
    connect(lineNumberList, &LineNumberList::selectLinesSignal, textEdit, &TextEdit::selectLines);
}

void CodeEditor::connectUrlEvents() {
    /**
     * 必须显示指定接收者为this,否则当某个CodeEditor关闭之后,再收到信号会崩
     * 有可能是因为没有指定接收者,CodeEditor被析构之后没有断开信号与槽连接
     */

    /**
     * 所有CodeEditor实例都会收到这些信号
     * 所以参数中的sourceParent用于确定哪个PlainTextEdit应该做出反应
     */

    /** 点击标签中[行, 列]或[第x行]链接,跳转到对应位置 **/
    connect(&UrlInterpreter::eventProxy, &UrlInterpreterEventproxy::jumpToLines, this,
            [=](QObject *const sourceParent, int row, int col, int lineCount) {
                if (sourceParent == textEdit) {
                    scrollArea->ensureVisible(col, row * textEdit->lineHeight(), textEdit->fontWidth() + PADDING,
                                              textEdit->lineHeight());
                    // 高亮对应行//
                    labelManager->highlight(row, lineCount);
                }
            });

    /** 点击标签中跳转按钮, 跳转到对应标签 **/
    connect(&UrlInterpreter::eventProxy, &UrlInterpreterEventproxy::jumpToLabel, this,
            [=](QObject *const source, QObject *const sourceParent, bool next, int specific) {
                if (sourceParent == textEdit) {
                    // 不出意外的情况下labelManager->jumpLabel<一定>不会返回nullptr//
                    // 如果返回nullptr那就是出意外了//
                    scrollArea->ensureWidgetVisible(labelManager->jumpLabel(source, next, specific));
                    labelManager->update();
                }
            });
}

void CodeEditor::resizeEvent(QResizeEvent *) {
    resizeWidget();
}

void CodeEditor::resizeWidget() {
    // size是文本占的实际大小//
    QSize size = textEdit->document()->size().toSize();
    labelManager->resize(
        (std::max)(scrollArea->width() -
                       (scrollArea->verticalScrollBar()->isVisible() ? scrollArea->verticalScrollBar()->width() : 0),
                   size.width() + PADDING),
        (std::max)(scrollArea->height(), size.height() + PADDING));

    lineNumberList->resize(lineNumberList->width(), labelManager->height());
    lineNumberList->setLineCount(textEdit->lineCount());
}
