#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "FileMenu.h"
#include <QFile>
#include <QFileSystemModel>
#include <QMouseEvent>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <fstream>
#include <QMap>

class SortFilterProxyModel;
class FileModel;
class FileLoadingPopPage;

namespace Ui {
class FileManager;
}

#define GBK "GBK"
#define UTF8 "UTF-8"

class FileManager : public QWidget {
    Q_OBJECT

public:
    explicit FileManager(QWidget *parent);
    ~FileManager();
    void index(const QString &path); // 设置文件索引//

    static QString getCurrentPath();

    // return QPair<encoding, content>
    static QPair<QString, QString> read(const QString &path);
    // return content only
    static QString read(const QString &path, bool no_encoding);

    static bool write(const QString &path, const QString &encoding, const QString &text);
    static std::ofstream *write(const QString &path, const QString &text);
    static QString projectPath();
    static QStringList getRecentlyFolderPath();
    static void addRecentlyFolderPath(const QString &path);                // 最近打开的文件//
    static void RecursiveAllFiles(QString path, QStringList &strListFile); // 递归查找文件夹里面的文件

    static std::ofstream newLog(QDateTime time);

//    static QVector<QString> id2filePath;
    static QMap<int, QString> id2filePath;

private:
    // init
    void setupStyle();
    // events
    void connectEvents();
    // 菜单相关
    void onClickMenuAction(QString action_name);
    void createFile(const QString &file_name, bool create_dir, bool sub_dir);
    void pasteFile();
    // others
    bool existDir(const QString &dir_path);
    bool existFile(const QString &file_path);
signals:
    /**
     * @emit when 双击打开文件
     * @receiver MainWidget -> CodeBox
     */
    void openFile(QString filePath);
    /**
     * @signal: startAnalysis
     * @emit when: 点击大的"开始分析"按钮
     * @receiver: MainWidget
     */
    void startAnalysis();
    void loadingFile();
    void loadingFinish();

private:
    Ui::FileManager *ui;
    int widthBeforeCollapsed; // 折叠之前的宽度

    static QString currentPath;
    static QStringList folderPath; // 保存最近打开的文件//

    FileModel *model;
    FileModel *emptyModel; // 为关闭项目之后使用
    SortFilterProxyModel *filterProxyModel;

    FileMenu *menu = nullptr;
    QModelIndex *clickedIndex = nullptr; // 记录点击了哪一项
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

/**
 * @class SortFilterProxyModel
 * @brief 需要通过这个ProxyModel来实现FileManager搜索时的文件过滤
 */

class SortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit SortFilterProxyModel(QObject *parent);
    void setSourceModel(QAbstractItemModel *sourceModel) override;

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

/**
 * @class FileModel
 * @brief 重写headerData函数,实现更改表头名称,重命名文件等
 */

class FileModel : public QFileSystemModel {
    Q_OBJECT
public:
    explicit FileModel(QObject *parent);
    void setProjectName(const QString &name); // 设置表头的项目名称//
    bool invalidFileName(const QString &name);
    bool existFileName(const QModelIndex &index, const QString &name);

private:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    QString projectName;
};

#endif // FILEMANAGER_H
