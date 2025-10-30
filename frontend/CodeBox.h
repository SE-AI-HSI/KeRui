#ifndef CODEBOX_H
#define CODEBOX_H

#include "CodeBoxFile.h"
#include "CodeEditor.h"
#include "FileTabBar.h"
#include <QLinkedList>
#include <QStack>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QWidget>

class CodeBoxFile;

namespace Ui {
class CodeBox;
}

/**
 * @class CodeBox
 * @brief 将FileTabBar和多个CodeEditor(放在StackedWidget)整合在一起
 */

class CodeBox : public QWidget {
    Q_OBJECT

public:
    explicit CodeBox(QWidget *parent);
    ~CodeBox();
    void openFile(const QString &filePath);
    void createEmptyPage();
    void upDateLables();
    void initialListener();      // 初始化监听器
    void saveAllFiles();         // 保存所有文件
    void upDateOpenFileLabels(); // 每次分析结束之后更新打开文件的标签
    void generateSummary();
    void showCodeGeneratePreview(QString file_path); // 重复块代码优化,如果有的话
    void closeSummaryWidget();
    void closeAllFiles();
    void updateCode(const QString &file_path);
    void closeAllOpenedFiles();

    void connectSender();

private:
    // init
    void setupLayout();
    void setupStyle();
    // events
    void connectEvents();
    void findAndReplaceConnectEvents();
    void resizeEvent(QResizeEvent *event);
    // others
    void saveEditingFile();
    void formatCode();
    void findAndReplace();

    // findAndReplace
    void findWord(QString findWord, int Flags);
    void findPreWord();
    void findNextWord();
    void replaceOneWord(QString replaceWord);
    void replaceAllWords(QString replaceWord);

    // loadingFile
    void loadingLabels(CodeBoxFile *cf);

signals:
    void newFileOpened();                                                              // receiver: MainWidget
    void allFileClosed();                                                              // receiver: MainWidget
    void scoresAndComment(int total, int s1, int s2, int s3, int s4, QString comment); // receiver: MainWidget
    void analyzeStep(AnalyzeStep aStep);
    void loadingFile();   // receiver: MainWidget
    void loadingFinish(); // receiver: MainWidget

private:
    Ui::CodeBox *ui;
    FileTabBar *tabBar;

    QLinkedList<CodeBoxFile> fileList;
    CodeBoxFile *editingFile = nullptr;
    FindAndReplaceDlg *findAndReplaceDlg = nullptr;
    AnalyzeResultHandler *ARHandler = nullptr;

    QTimer *autoSaveTimer; /** 用于定时保存文件 **/
};

#endif // CODEBOX_H
