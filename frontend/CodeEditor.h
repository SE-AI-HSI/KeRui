#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "AutoComplete.h"
#include "FindAndReplaceDlg.h"
#include "LabelManager.h"
#include "LineNumberList.h"
#include "TextEdit.h"
#include <QtCore/QRegExp>
#include <QScrollArea>
#include <QStack>
#include <QWidget>

/**
 * @class CodeEditor
 * @brief 代码编辑器,把LineNumberList,LabelManager,TextEdit封装在一起
 */

class CodeEditor : public QWidget {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent);
    ~CodeEditor();
    TextEdit *getTextEdit() const;
    void setTextToEditor(const QString &);
    void updateLabels();
    LabelManager *getLabelManager();

    /// 查找框相关函数
    void finishFindAndReplace(); // 关闭查找状态
    void searchForText();        // 查找新的text
    QRegExp textToREG(QString newText, int Flags);
    QString getCode();
    bool findNextWord();
    bool findPreWord();
    bool replaceOneWord(QString replaceWord);
    bool replaceAllWords(QString replaceWord);

    /// 传递分析信息
    void passInfo(const QList<IWarningInfo> &warningInfo, const QList<IErrorInfo> &errorInfo,
                  const QVector<QPair<int, QList<QList<ClonePosition>>>> &cloneSets, const QString &filePath,
                  const QString &fileRelativelyPath);
    void clearLabels();

    /// 查找框相关变量
    // 当前查找的值
    static QString wordToBeFound;
    // 要查找的文本转换成的正则表达式
    static QRegExp REGToBeUsed;
    // 上次查到的位置
    static int lastIndex;
    // 堆栈用于存储已经找到的字符的位置
    static QStack<int> findWordsSTK;
    void resizeWidget();

    void optimizeCloneCodes(QObject *const _label);

private:
    // init
    void setupLayout();
    void setupStyle();
    // events
    void connectEvents();
    void connectUrlEvents(); /** 响应点击标签中的链接 **/
    void resizeEvent(QResizeEvent *event) override;
    // others
signals:

private:
    QScrollArea *scrollArea;
    LabelManager *labelManager;
    LineNumberList *lineNumberList;
    TextEdit *textEdit;
};

#endif // CODEEDITOR_H
