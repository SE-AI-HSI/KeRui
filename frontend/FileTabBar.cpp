#include "FileTabBar.h"
#include <QAbstractItemView>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QMessageBox>
#include <QMouseEvent>

#define TAB_MIN_WIDTH 100
#define TAB_HEIGHT 40
#define TAB_SPACING 10  // tab之间的间隔//
#define MARGIN_FRONT 80 // 等于LineNumberList的宽度//
#define MARGIN_END 25   // 等于comboBox需要的宽度//

FileTabBar::FileTabBar(QWidget *_parent) : QWidget{_parent}, parent(_parent) {
    shadowMask = new QWidget(parent); // shadowMask的parent必须是CodeBox才会有用//
    shadowMask->hide();

    setupLayout();
    setupStyle();
    connectEvents();
}

void FileTabBar::switchTab(const QString &filePath, const QString &tab_name) {
    for (FileTab &T : tabList) {
        if ((!filePath.isEmpty() && T.file.path == filePath) || (filePath.isEmpty() && T.name->text() == tab_name)) {
            setActiveTab(T.tab);                                         // 这两句的顺序不能换//
            activeTab = &T;                                              // 这两句的顺序不能换//
            emit tabSwitched(const_cast<CodeBoxFile *const>(&(T.file))); // 无语死了不知道为什么只提供const *//
            return;
        }
    }
}

void FileTabBar::addTab(const CodeBoxFile &file) {
    // QFrame外壳//
    QFrame *tab = new QFrame(tabContainer);
    // tab->setFixedSize(TAB_WIDTH, TAB_HEIGHT);
    tab->setObjectName("tab"); // 用于绑定样式,样式会在调用setActiveTab的时候设置//
    // QLabel显示文件名//
    QLabel *name = new QLabel(file.name(), tab);
    if (file.path.isEmpty()) {
        emptyPageCount += 1;
        name->setText(QString::fromLocal8Bit("空白页") + QString::number(emptyPageCount));
    }
    name->adjustSize();
    // QLabel充当关闭按钮//
    QLabel *closeButton = new QLabel(tab);
    closeButton->setFixedSize(31, 31);
    closeButton->setAttribute(Qt::WA_NoMousePropagation); // 不能把点击事件传递给tab//
    closeButton->setContentsMargins(0, 0, 5, 0);
    closeButton->setStyleSheet("QLabel{image: url(:/images/close.svg);}"
                               "QLabel:hover{image: url(:/images/close-hover.svg);}");
    closeButton->setCursor(Qt::PointingHandCursor);

    tab->setFixedSize((std::max)(50 + name->width(), TAB_MIN_WIDTH), TAB_HEIGHT);
    // 将以上组件添加到layout中//
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(0);
    layout->setContentsMargins(10, 0, 0, 0);
    layout->addWidget(name, 1);
    layout->addWidget(closeButton);

    // 安装事件过滤器,用于响应点击事件//
    tab->installEventFilter(this);
    closeButton->installEventFilter(this);

    // 将整个tab添加到tabList中//
    tabList.prepend(FileTab(file, tab, name, closeButton));
    updateLayout();

    // 切换到该tab//
    switchTab(file.path, name->text());
}

void FileTabBar::markUnsaved(const QString &file_path) {
    for (FileTab &T : tabList) {
        if (T.file.path == file_path) {
            QString s = T.name->text();
            s += "*";
            T.name->setText(s);
            return;
        }
    }
}

void FileTabBar::markSaved(const QString &file_path) {
    for (FileTab &T : tabList) {
        if (T.file.path == file_path) {
            T.name->setText(T.file.name());
            return;
        }
    }
}

void FileTabBar::closeSummaryWidget() {
    for (TabIterator it = tabList.begin(); it != tabList.end(); ++it) {
        if (it->name->text() == SUMMARY_FILE_NAME) {
            closeTab(it);
            return;
        }
    }
}

void FileTabBar::closeAllTabs() {
    while (!tabList.empty()) {
        TabIterator it = tabList.begin();
        closeTab(it);
    }
}

void FileTabBar::setupLayout() {
    this->setFixedHeight(TAB_HEIGHT); // 不设置则不会显示//

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(MARGIN_FRONT, 0, 0, 0);
    layout->setSpacing(5);

    tabContainer = new QWidget(this);
    tabContainer->show();
    comboBox = new QComboBox(this);
    comboBox->setFixedWidth(20);
    comboBox->show();

    layout->addWidget(tabContainer);
    layout->addWidget(comboBox);
    layout->addStretch();
}

void FileTabBar::setupStyle() {
    /*comboBox*/
    comboBox->setView(new QListView(comboBox));
    comboBox->view()->setFixedWidth(150);
    comboBox->setStyleSheet(comboBoxStyleSheet());
    /*shadowMask*/
    shadowMask->setFixedSize(0, 15);
    shadowMask->setStyleSheet("background-color: rgb(255, 245, 239);");
}

void FileTabBar::connectEvents() {
    connect(comboBox, QOverload<int>::of(&QComboBox::activated), [=] {
        switchTabFromComboBox(comboBox->currentText());
    });
}

void FileTabBar::resizeEvent(QResizeEvent *) {
    updateLayout();
    if (activeTab != NULL && !activeTab->tab->isVisible()) {
        // 如果导致当前tab看不见了,就把tab移到第一位//
        switchTabFromComboBox(activeTab->file.path);
    }
}

bool FileTabBar::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event); // 下行转换是不安全的,但是qobject_cast没有这个重载//
        if (mouseEvent != NULL) {
            if (mouseEvent->button() == Qt::LeftButton) {
                // 点击的可能是tab,或者是关闭按钮//
                onClickTab(watched);
            }
        }
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void FileTabBar::onClickTab(QObject *object) {
    if (activeTab != NULL && object == activeTab->tab) { // 如果点击的是当前tab那就不用管//
        return;
    }
    for (TabIterator it = tabList.begin(); it != tabList.end(); ++it) {
        if (object == it->tab) { // 点击tab,切换//
            switchTab(it->file.path, it->name->text());
            return;
        } else if (object == it->closeButton) { // 点击×,关闭//
            closeTab(it);
            return;
        }
    }
}

void FileTabBar::updateLayout() {
    comboBox->clear(); // 先清空comboBox的内容//
    int cumulativeWidth = 0;
    bool flag = false; // 是否有tab不可见//
    foreach (const FileTab &T, tabList) {
        if (flag || cumulativeWidth + T.tab->width() > this->width() - MARGIN_FRONT - MARGIN_END) {
            // 如果这个tab由于tabContainer宽度不够而不可见,把它隐藏并加入到comboBox//
            flag = true;
            T.tab->hide();
            comboBox->addItem(T.file.name());
            continue;
        }
        T.tab->move(cumulativeWidth, 0);
        T.tab->show();
        cumulativeWidth += T.tab->width();
        tabContainer->setFixedWidth(cumulativeWidth);
        cumulativeWidth += TAB_SPACING;
    }
    comboBox->setVisible(flag);
}

void FileTabBar::setActiveTab(QFrame *const tab) {
    if (activeTab != NULL) {
        // 如果之前有选中的tab,改变其样式//
        activeTab->tab->setStyleSheet(tabStyleSheet(false));
    }
    tab->setStyleSheet(tabStyleSheet(true));
    if (enableShadowMask) {
        shadowMask->setFixedWidth(tab->width()); // 减2是因为tab边框的宽度//
        // shadowMask移动到相应的位置//
        shadowMask->move(parent->contentsMargins().left() + MARGIN_FRONT + tab->x(),
                         parent->contentsMargins().top() + TAB_HEIGHT - 10);
        shadowMask->raise();
        shadowMask->show();
    }
}

/**
 * @brief 实际是将某个tab移到第一位
 */
void FileTabBar::switchTabFromComboBox(const QString &fileName) {
    for (TabIterator it = tabList.begin(); it != tabList.end(); ++it) {
        // 因为是从前往后扫描,所以需要判断tab可见性,防止同名(但是在不同目录的)文件冲突//
        // 如果comboBox中有多个同名文件,那我只能说用户自作自受//
        if (!it->tab->isVisible() && it->file.name() == fileName) {
            tabList.prepend(*it);
            if (activeTab == it.operator->()) {
                // 不赋值NULL以后访问会crash//
                activeTab = NULL;
            }
            tabList.erase(it);
            break;
        }
    }
    updateLayout();
    switchTab(tabList.first().file.path, tabList.first().name->text()); // 不能再使用形参fileName了,否则会crash//
    // 因为fileName是引用类型,上面fileList.erase(it);这行有可能已经把它所在的内存释放掉了//
}

void FileTabBar::closeTab(TabIterator &it) {
    const bool flag = it.operator->() == activeTab; // 关闭的是当前tab吗//

    if (it->file.path.isEmpty()) {
        qDebug() << __FILE__ << ":" << __LINE__ << "empty";
        // 空白页
        emptyPageCount -= 1;
        QString text = QString::fromLocal8Bit("要保存该文件（");
        text += it->name->text();
        text += QString::fromLocal8Bit("）吗?");
        int choice = QMessageBox::question(nullptr /*不能写this,否则黑一片*/, QString::fromLocal8Bit("提示"), text,
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Close);
        if (choice == QMessageBox::Yes) {
            /// 保存文件
            QString pos = FileManager::projectPath();
            QString path = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("选择保存位置"),
                                                        pos.isEmpty() ? "/" : pos, "Header(*.h);;Source(*.cpp)");
            if (!path.isEmpty()) {
                QString name = path.mid(path.lastIndexOf("/") + 1);
                it->name->setText(name);
                const_cast<CodeBoxFile &>(it->file).changePath(path);
            } else
                return;
        } else if (choice == QMessageBox::Close) {
            return;
        }
        /// 选不保存没有路径,然后CodeBox不会保存
    }
    // 先等CodeBox关闭和保存文件//
    emit tabClosed(it->file);

    // 移除事件过滤器//
    it->tab->removeEventFilter(this);
    it->closeButton->removeEventFilter(this);

    it->tab->deleteLater();
    it = tabList.erase(it);

    updateLayout(); // 更新tab布局//

    if (flag) {
        activeTab = NULL;
        if (it != tabList.end()) { // 后面还有tab,则切换后一个tab//
            switchTab(it->file.path, it->name->text());
            return;
        } else if (!tabList.isEmpty()) { // 切换前一个tab//
            switchTab(tabList.last().file.path, tabList.last().name->text());
            return;
        } else { // 没有打开的文件了//
            shadowMask->hide();
        }
    } else if (enableShadowMask && activeTab != NULL) {
        shadowMask->move(parent->contentsMargins().left() + MARGIN_FRONT + activeTab->tab->x(),
                         parent->contentsMargins().top() + TAB_HEIGHT - 5);
    }
}

QString FileTabBar::tabStyleSheet(bool active) {
    if (active) {
        return "QFrame#tab{"
               "background-color: rgb(255, 248, 242);"
               "border-bottom: none;"
               "border-top-left-radius: 5px;"
               "border-top-right-radius: 5px;"
               "}";
    }
    return "QFrame#tab{"
           "background-color: rgba(255, 248, 242, 80%);"
           "border-bottom: none;"
           "border-top-left-radius: 5px;"
           "border-top-right-radius: 5px;"
           "}"
           "QFrame:hover#tab{"
           "background-color: #ffccaf;"
           "border-bottom: none;"
           "border-top-left-radius: 5px;"
           "border-top-right-radius: 5px;"
           "}";
}

QString FileTabBar::comboBoxStyleSheet() {
    return "QComboBox {"
           "background-color: rgb(255, 245, 239);"
           "border: 1px solid gray;"
           "}"
           "QComboBox::drop-down {"
           "border: none;"
           "width: 18px;"
           "}"
           "QComboBox::down-arrow {"
           "image: url(:/images/combobox-arrow-down.svg);"
           "width: 18px;"
           "height: 18px;"
           "}"
           "QComboBox QAbstractItemView {"
           "outline: none;"
           "border: 1px solid gray;"
           "background-color::rgb(255, 245, 239);"
           "}"
           "QComboBox QAbstractItemView::item {"
           "height: 20px;   /* 选项高度(通过代码setView(new QListView());后才起作用) */"
           "color: black;"
           "background-color: transparent;"
           "}"
           "QComboBox QAbstractItemView::item:hover {"
           "border-left: 5px solid rgb(255, 155, 82);"
           "}";
}
