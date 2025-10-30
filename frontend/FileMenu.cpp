#include "FileMenu.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QtCore/QRegExp>
#include <QRegExpValidator>

FileMenu::FileMenu(QWidget *parent) : QMenu(parent), inputBox(new MenuInputBox) {
    inputBox->hide();

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlag(Qt::NoDropShadowWindowHint);

    setStyleSheet("QMenu{"
                  "background-color: #fff5ea;"
                  "margin: 0;"
                  "padding: 0;"
                  "border: 1.5px solid #fdcaae;"
                  "border-radius: 7px;"
                  "}"
                  "QMenu::item:selected{"
                  "background-color: #ffccaf;"
                  "color: #000000;"
                  "}"
                  "QMenu::item{"
                  "background-color: transparent;"
                  "margin: 1px 3px;"
                  "padding: 1px 12px;"
                  "color: #6f6f6f;"
                  "}"
                  "QMenu:separator{"
                  "height: 1px;"
                  "background-color: #fdcaae;"
                  "margin: 0px 5px;"
                  "}");
}

FileMenu::~FileMenu() {
    delete inputBox;
}

void FileMenu::show(bool isDir, const QPoint &pos1, const QPoint &pos2, int width, bool isExpanded) {
    /// pos1是给菜单的位置, pos2是给输入框的位置
    clear();
    if (isDir) {
        if (isExpanded) {
            QAction *a = new QAction(ACTION_COLLAPSE, this);
            addAction(a);
        } else {
            QAction *a = new QAction(ACTION_EXPAND, this);
            addAction(a);
        }
        addSeparator();

        QAction *a6 = new QAction(ACTION_SHOW_IN_EXPLORER, this);
        QAction *a1 = new QAction(ACTION_CREATE_FILE, this);
        QAction *a2 = new QAction(ACTION_CREATE_DIR, this);
        QAction *a3 = new QAction(ACTION_CREATE_SUBDIR, this);
        QAction *a4 = new QAction(ACTION_RENAME, this);
        QAction *a7 = new QAction(ACTION_COPY, this);
        QAction *a5 = new QAction(ACTION_DELETE, this);
        addAction(a6);
        addSeparator();

        addAction(a1);
        addSeparator();

        addAction(a2);
        addSeparator();

        addAction(a3);
        addSeparator();

        addAction(a7);
        addSeparator();

        if (!copySrcPath.isEmpty()) {
            QAction *a8 = new QAction(ACTION_PASTE, this);
            addAction(a8);
            addSeparator();
        }
        addAction(a4);
        addSeparator();

        addAction(a5);
    } else {
        QAction *a1 = new QAction(ACTION_OPEN, this);
        QAction *a6 = new QAction(ACTION_SHOW_IN_EXPLORER, this);
        QAction *a2 = new QAction(ACTION_CREATE_FILE, this);
        QAction *a3 = new QAction(ACTION_CREATE_DIR, this);
        QAction *a7 = new QAction(ACTION_COPY, this);
        QAction *a4 = new QAction(ACTION_RENAME, this);
        QAction *a5 = new QAction(ACTION_DELETE, this);
        addAction(a1);
        addSeparator();

        addAction(a6);
        addSeparator();

        addAction(a2);
        addSeparator();

        addAction(a3);
        addSeparator();

        addAction(a7);
        addSeparator();

        if (!copySrcPath.isEmpty()) {
            QAction *a8 = new QAction(ACTION_PASTE, this);
            addAction(a8);
            addSeparator();
        }
        addAction(a4);
        addSeparator();

        addAction(a5);
    }
    inputBoxPos = pos2;
    inputBoxWidth = width - 20 - 21;
    move(pos1);
    QMenu::show();
}

void FileMenu::showInputBox(bool create_dir, bool sub_dir) {
    inputBox->show(inputBoxPos, inputBoxWidth, create_dir, sub_dir);
}

void FileMenu::hideInputBox() {
    inputBox->hide();
}

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

MenuInputBox::MenuInputBox(QWidget *parent) : QFrame(parent) {
    setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);
    setFixedSize(180, 24);
    lineEdit = new QLineEdit(this);
    lineEdit->setFixedHeight(24);
    lineEdit->setFixedWidth(180);

    QRegExp filter("[^\\\\/:*?\"<>|]*");
    QRegExpValidator *validator = new QRegExpValidator(filter, this);
    lineEdit->setValidator(validator);

    installEventFilter(this);
}

void MenuInputBox::show(const QPoint &pos, int width, bool create_dir, bool sub_dir) {
    setFixedWidth(width);
    lineEdit->setFixedWidth(width);

    createDir = create_dir, subDir = sub_dir;
    if (create_dir) {
        if (sub_dir)
            lineEdit->setPlaceholderText(QString::fromLocal8Bit("输入子目录名"));
        else
            lineEdit->setPlaceholderText(QString::fromLocal8Bit("输入目录名"));
    } else
        lineEdit->setPlaceholderText(QString::fromLocal8Bit("输入文件名"));
    move(pos.x() + 20, pos.y());
    QWidget::show();
}

void MenuInputBox::hide() {
    QWidget::hide();
    lineEdit->clear();
}

bool MenuInputBox::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::ActivationChange) {
        if (QApplication::activeWindow() != this) {
            this->hide();
        }
    }
    return QWidget::eventFilter(o, e);
}

void MenuInputBox::keyPressEvent(QKeyEvent *event) {
    if (event->key() == 16777220 || event->key() == 16777221) {
        emit create(lineEdit->text(), createDir, subDir);
    }
}
