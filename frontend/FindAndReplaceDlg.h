#ifndef FINDANDREPLACEDLG_H
#define FINDANDREPLACEDLG_H
#include "ui_FindAndReplaceDlg.h"
#include <QDebug>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QWidget>

namespace Ui {
class FindAndReplaceDlg;
}
enum FINDINGFLAGS {
    FINDUPORLOW = 0x100,
    FINDWHOLEWORD = 0x010,
    FINDREG = 0x001,
};

class FindAndReplaceDlg : public QWidget {
    Q_OBJECT

public:
    FindAndReplaceDlg(QWidget *parent);
    ~FindAndReplaceDlg();

    void showDlg();
    void setFindingResult();
    void setWordsCounter(int counter);
    void setWordIndex(int index);

signals:
    void closeDlg();
    void findWords(QString findWord, int findFlags);
    void replaceOneWord(QString replaceWord);
    void replaceAllWords(QString replaceWord);
    void findPreWord(int findFlags);
    void findNextWord(int findFlags);

private:
    void connectEvents();

    /// 查找不区分大小写0x100 , 查找完整单词0x010, 查找通过正则表达式0x001
    int findingFlags = FINDUPORLOW;
    /// 替换是否全部大写
    bool replaceOnlyUp = false;

    /// 查找到的单词的总个数
    int wordsCounter = 0;
    /// 当前单词的位置
    int nowWordIndex = 0;

    const QString normalText = QString::fromLocal8Bit("第%1项,共%2项");
    const QString noneText = QString::fromLocal8Bit("无结果");

    // 图标切换
    void replaceOnlyUpImage(bool replaceOnlyUp);
    void inputLineOnlyUpImage(int findingFlags);
    void wholeWordImage(int findingFlags);
    void REGImage(int findingFlags);

private:
    Ui::FindAndReplaceDlg *ui;
};

#endif // FINDANDREPLACEDLG_H
