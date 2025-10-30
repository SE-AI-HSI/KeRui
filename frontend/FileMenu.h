#ifndef FILEMENU_H
#define FILEMENU_H

#include <QFrame>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>

#define ACTION_CREATE_FILE QString::fromLocal8Bit("新建文件")
#define ACTION_SHOW_IN_EXPLORER QString::fromLocal8Bit("在Explorer中显示")
#define ACTION_CREATE_DIR QString::fromLocal8Bit("新建目录")
#define ACTION_CREATE_SUBDIR QString::fromLocal8Bit("新建子目录")
#define ACTION_DELETE QString::fromLocal8Bit("移到回收站")
#define ACTION_RENAME QString::fromLocal8Bit("重命名")
#define ACTION_OPEN QString::fromLocal8Bit("打开")
#define ACTION_COPY QString::fromLocal8Bit("复制")
#define ACTION_PASTE QString::fromLocal8Bit("粘贴")
#define ACTION_EXPAND QString::fromLocal8Bit("展开")
#define ACTION_COLLAPSE QString::fromLocal8Bit("折叠")

class MenuInputBox;

class FileMenu : public QMenu {
    Q_OBJECT
public:
    FileMenu(QWidget *parent);
    ~FileMenu();
    void show(bool isDir, const QPoint &pos1, const QPoint &pos2, int width, bool isExpanded = false);
    void showInputBox(bool create_dir, bool sub_dir = false);
    void hideInputBox();

private:
    QPoint inputBoxPos;
    int inputBoxWidth;

public:
    QString copySrcPath; // 复制的源路径
    bool copySrcIsDir;   // 复制的是目录吗
    MenuInputBox *const inputBox;
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class MenuInputBox : public QFrame {
    Q_OBJECT
public:
    MenuInputBox(QWidget *parent = nullptr);
    void show(const QPoint &pos, int width, bool create_dir, bool sub_dir);
    void hide();

private:
    bool eventFilter(QObject *o, QEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void create(QString file_name, bool create_dir, bool sub_dir);

private:
    bool createDir; // 要创建目录?
    bool subDir;    // 创建子目录?

    QLineEdit *lineEdit;
};

#endif // FILEMENU_H
