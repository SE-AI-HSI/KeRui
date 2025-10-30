#ifndef FILETABBAR_H
#define FILETABBAR_H

#include "CodeBoxFile.h"
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QLinkedList>
#include <QWidget>

class FileTab;
typedef QLinkedList<FileTab>::iterator TabIterator;

/**
 * @class FileTabBar
 * @brief CodeBox顶部的Tab窗口
 */

class FileTabBar : public QWidget {
    Q_OBJECT

public:
    explicit FileTabBar(QWidget *parent);
    void switchTab(const QString &filePath, const QString &tab_name);
    void addTab(const CodeBoxFile &file);
    void markUnsaved(const QString &file_path);
    void markSaved(const QString &file_path);
    void closeSummaryWidget();
    void closeAllTabs();

private:
    // init
    void setupLayout();
    void setupStyle();
    // events
    void connectEvents();
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void onClickTab(QObject *object);

    // others
    void updateLayout(); // 更新tab的个数和可见性等//
    void setActiveTab(QFrame *const tab);
    void switchTabFromComboBox(const QString &filePath); // 从comboBox中切换出了一个tab//
    void closeTab(TabIterator &it);
    // qss
    QString tabStyleSheet(bool active);
    QString comboBoxStyleSheet();

signals:
    /**
     * @signal tabSwitched
     * @emit when switchTab() is called
     * @receiver CodeBox
     */
    void tabSwitched(CodeBoxFile *file);

    /**
     * @signal tabClosed
     * @emit when closeTab() is called
     * @receiver CodeBox
     */
    void tabClosed(const CodeBoxFile &file);

private:
    QWidget *const parent; // shadowMask需要知道CodeBox的contentMargin//
    QWidget *shadowMask;   // 用于遮挡选中的tab下面的阴影//
    bool enableShadowMask = true;

    QWidget *tabContainer;
    QComboBox *comboBox;
    QLinkedList<FileTab> tabList;
    FileTab *activeTab = NULL; // 当前选中的tab//
    int emptyPageCount = 0;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

/**
 * @class FileTab
 * @brief 存储tab的相关信息
 */

class FileTab {
public:
    FileTab(const CodeBoxFile &_file, QFrame *_tab, QLabel *_name, QLabel *button)
        : file(_file), tab(_tab), name(_name), closeButton(button) {
    }

    // data members
    const CodeBoxFile &file;
    QFrame *const tab;
    QLabel *const name;
    QLabel *const closeButton;
};

#endif // FILETABBAR_H
