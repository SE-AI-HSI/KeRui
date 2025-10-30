#include "FileManager.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QStandardItemModel>
#include <QTextCodec>

#include "AnalyzerClient.h"
#include "GraphicsEffectUtil.h"
#include "SummaryWidget.h"
#include "ui_FileManager.h"

#define COLLAPSED_WIDTH 96  // 折叠之后的总宽度
#define MAX_WIDTH 540
#define MIN_WIDTH 340
#define MAX_RECENTLY_PATH 6

QString FileManager::currentPath = "";
QStringList FileManager::folderPath = QStringList();
QMap<int, QString> FileManager::id2filePath;

FileManager::FileManager(QWidget *parent) : QWidget(parent), ui(new Ui::FileManager) {
    ui->setupUi(this);

    model = new FileModel(this);
    emptyModel = new FileModel(this);

    filterProxyModel = new SortFilterProxyModel(this);
    filterProxyModel->setSourceModel(model);  // 使用MySortFilterProxyModel对model进行代理

    ui->TreeView->setModel(filterProxyModel);

    filterProxyModel->setFilterKeyColumn(0);  // 设置过滤的列为文件名
    QRegExp regExp("^.*\\.(h|cpp|c|kr)$", Qt::CaseInsensitive);
    filterProxyModel->setFilterRegExp(regExp);

    ui->TreeView->setContextMenuPolicy(Qt::CustomContextMenu);  // 设置右键弹出菜单
    menu = new FileMenu(this);

    setupStyle();
    connectEvents();
}

FileManager::~FileManager() {
    delete ui;
}

void FileManager::index(const QString &path) {
    currentPath = path;
    if (path.isEmpty()) {
        ui->TreeView->setModel(emptyModel);
    } else {
        model->setRootPath(path);
        ui->TreeView->setModel(filterProxyModel);
        ui->TreeView->setRootIndex(filterProxyModel->mapFromSource(model->index(path)));
        QString projectName = path.mid(path.lastIndexOf("/") + 1);
        model->setProjectName(projectName);
        // 设置currentPath,要保存到文件//
        write(QCoreApplication::applicationDirPath() + "/lastPath", "UTF-8", path);
        addRecentlyFolderPath(path);
    }
}

QString FileManager::getCurrentPath() {
    return currentPath;
}

QPair<QString, QString> FileManager::read(const QString &path) {
    QFile file(path);
    //    qDebug()<<path;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray content = file.readAll();
        file.close();

        QTextCodec::ConverterState state;
        QTextCodec *codec = QTextCodec::codecForName(UTF8);
        QString text = codec->toUnicode(content.constData(), content.size(), &state);

        if (state.invalidChars > 0) {
            // 如果不是UTF-8就转GBK,如果也不是GBK那目前暂时没办法判断//
            text = QTextCodec::codecForName(GBK)->toUnicode(content);
            // qDebug() << "FileManager: read" << path << ",encoding" << GBK;
            return {QString(GBK), text};
        } else {
            // qDebug() << "FileManager: read" << path << ", encoding" << UTF8;
            return {QString(UTF8), content};
        }
    } else {
        return {QString(UTF8), ""};
    }
}

QString FileManager::read(const QString &path, bool no_encoding) {
    Q_UNUSED(no_encoding);  // 该参数只是用于区别函数重载

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray content = file.readAll();
        file.close();

        QTextCodec::ConverterState state;
        QTextCodec *codec = QTextCodec::codecForName(UTF8);
        QString text = codec->toUnicode(content.constData(), content.size(), &state);

        if (state.invalidChars > 0) {
            // 如果不是UTF-8就转GBK,如果也不是GBK那目前暂时没办法判断//
            text = QTextCodec::codecForName(GBK)->toUnicode(content);
            // qDebug() << "FileManager: read" << path << ",encoding" << GBK;
            return text;
        } else {
            // qDebug() << "FileManager: read" << path << ", encoding" << UTF8;
            return content;
        }
    } else {
        return "";
    }
}

bool FileManager::write(const QString &path, const QString &encoding, const QString &text) {
    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    if (!file.isOpen()) {
        qDebug() << "FileManager::write " + path + " failed.";
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec(encoding == UTF8 ? UTF8 : GBK);  // 编码,UTF-8 or GBK//
    //    qDebug() << "FileManager: WRITE" << path << ", encoding" << (encoding == UTF8 ? UTF8 : GBK);
    stream << text;
    file.close();
    return true;
}

std::ofstream *FileManager::write(const QString &path, const QString &text) {
    std::ofstream *log = new std::ofstream;
    log->open(path.toStdString(), std::ios::app);
    if (!log) {  // true则说明文件打开出错
        qDebug() << "Write error! Path: " + path;
    } else {
        if (text != "") {
            *log << text.toStdString();
        }
        return log;
    }
    return nullptr;
}

QString FileManager::projectPath() {
    return currentPath;
}

QStringList FileManager::getRecentlyFolderPath() {
    // 先把最近文件夹路径读出来
    QPair<QString, QString> pathFile = read(QCoreApplication::applicationDirPath() + "/recentlyPath");
    // 以换行为一个分隔
    folderPath = pathFile.second.split("\n");
    folderPath.pop_back();  // 最后一个为空！

    QStringList res;
    for (QString &path : folderPath) {
        QDir dir(path);
        if (dir.exists()) {
            res.append(path);
        }
    }
    return res;
}

void FileManager::addRecentlyFolderPath(const QString &path) {
    // 先把最近文件夹路径读出来
    QPair<QString, QString> pathFile = read(QCoreApplication::applicationDirPath() + "/recentlyPath");
    // 以换行为一个分隔
    folderPath = pathFile.second.split("\n");

    // 新的路径
    int pos = -1;
    for (int i = 0; i < folderPath.size(); i++) {
        if (folderPath.at(i) == path) {
            pos = i;
            break;
        }
    }
    if (pos == -1) {
        folderPath.insert(0, path);
    } else {
        folderPath.move(pos, 0);  // 把文件移动到前面
    }

    while (folderPath.size() > MAX_RECENTLY_PATH) {
        folderPath.removeLast();
    }
    QString pathText = folderPath.join("\n");
    write(QCoreApplication::applicationDirPath() + "/recentlyPath", UTF8, pathText);
}

void FileManager::RecursiveAllFiles(QString path, QStringList &strListFile) {
    QDir dir(path);
    dir.setSorting(QDir::Name);
    QFileInfoList listFileInfos = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (QFileInfo fileInfo : listFileInfos) {
        if (fileInfo.isDir()) {
            RecursiveAllFiles(fileInfo.absoluteFilePath(), strListFile);
        } else {
            QStringList list = fileInfo.fileName().split('.');
            //            qDebug()<< list.back();
            if (list.back() == "cpp" || list.back() == "h" || list.back() == "c") {
                strListFile.push_back(fileInfo.absoluteFilePath());
            }
        }
    }
    //    qDebug() << strListFile;
}

std::ofstream FileManager::newLog(QDateTime time) {
    /// 注意时间的格式没有冒号(:)因为文件名不支持
    QString path = QApplication::applicationDirPath() + "/log/" + time.toString("yyyy-MM-dd hh.mm.ss.zzz") + ".txt";
    std::ofstream stream(path.toStdString());
    return stream;
}

void FileManager::setupStyle() {
    for (int i = 1; i < model->columnCount(); ++i) {
        ui->TreeView->hideColumn(i);  // 隐藏没用的信息列
    }

    this->setMaximumWidth(MAX_WIDTH);
    this->setMinimumWidth(MIN_WIDTH);

    // 添加阴影
    ui->frame_2->setGraphicsEffect(GraphicsEffectUtil::newShadow(this));  // 文件管理器的白色区域
    ui->AnalyzeBtn->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 64, 20, 0, 1));  // 分析按钮
}

void FileManager::connectEvents() {
    // 点击折叠//
    connect(ui->CollapseBtn, &QPushButton::pressed, [=] {
        widthBeforeCollapsed = this->width();
        this->setFixedWidth(COLLAPSED_WIDTH);  // 使其不可通过拖动的方式展开
        ui->StackedWidget->setCurrentIndex(1);
    });

    // 点击展开//
    connect(ui->ExpandBtn, &QPushButton::pressed, [=] {
        this->setMaximumWidth(MAX_WIDTH);
        this->setMinimumWidth(MIN_WIDTH);
        this->resize(widthBeforeCollapsed, height());
        ui->StackedWidget->setCurrentIndex(0);
    });

    // 搜索文件//
    connect(ui->SearchInput, &QLineEdit::textEdited, [&, this](const QString &text) {
        if (text.isEmpty()) {
            QRegExp regExp("^.*\\.(h|cpp|c|kr)$", Qt::CaseInsensitive);
            filterProxyModel->setFilterRegExp(regExp);
        } else {
            QRegExp regExp("^.*" + text + ".*\\.(h|cpp|c|kr)$", Qt::CaseInsensitive);
            filterProxyModel->setFilterRegExp(regExp);
        }
    });

    // 双击打开文件//
    connect(ui->TreeView, &QTreeView::doubleClicked, [&, this](const QModelIndex &mindex) {
        if (!model->isDir(filterProxyModel->mapToSource(mindex))) {  // 如果点击的是目录则不用管//
            QString filePath = model->filePath(filterProxyModel->mapToSource(mindex));

            //            emit loadingFile();
            emit openFile(filePath);
            //            emit loadingFinish();
        }
    });

    // 点击大的"开始分析"按钮//
    connect(ui->AnalyzeBtn, &QPushButton::pressed, [=] {
        if (currentPath.isEmpty()) {
            emit AnalyzerClient::eventProxy.showToast(Toast::Warning, QString::fromLocal8Bit("请打开一个项目"));
        } else {
            emit startAnalysis();
        }
    });

    /// 记录点击的是哪个项,右键菜单需要用到
    connect(ui->TreeView, &QTreeView::pressed, this,
            [&, this](const QModelIndex &index) { clickedIndex = const_cast<QModelIndex *>(&index); });

    /// 右键弹出菜单
    connect(ui->TreeView, &QTreeView::customContextMenuRequested, this, [&, this](const QPoint &pos) {
        if (clickedIndex != nullptr) {
            // pos->相对于TreeView的点位置
            int y = pos.y();
            if (y + 200 > ui->TreeView->height()) {
                y -= (y + 200 - ui->TreeView->height());
            }
            menu->show(model->isDir(filterProxyModel->mapToSource(*clickedIndex)),
                       ui->TreeView->mapToGlobal(QPoint(pos.x(), y)), ui->TreeView->mapToGlobal(QPoint(0, y)),
                       ui->TreeView->width(), ui->TreeView->isExpanded(*clickedIndex));
        }
    });

    /// 点击菜单action
    connect(menu, &FileMenu::triggered, this, [=](QAction *a) { onClickMenuAction(a->text()); });

    /// 创建新文件输入完毕
    connect(menu->inputBox, &MenuInputBox::create, this,
            [=](QString file_name, bool create_dir, bool sub_dir) { createFile(file_name, create_dir, sub_dir); });
}

void FileManager::onClickMenuAction(QString action_name) {
    const QModelIndex &idx = filterProxyModel->mapToSource(*clickedIndex);
    if (action_name == ACTION_OPEN) {
        emit openFile(model->filePath(idx));
    } else if (action_name == ACTION_SHOW_IN_EXPLORER) {
        QStringList arguments;
        arguments << "/select," << QDir::toNativeSeparators(model->filePath(idx));
        QProcess::startDetached("explorer", arguments);
    } else if (action_name == ACTION_RENAME) {
        ui->TreeView->edit(*clickedIndex);
    } else if (action_name == ACTION_DELETE) {
        QFile f(model->filePath(idx));
        // bool s = model->remove(idx); //<-直接删除文件不移入回收站
        bool s = false;
        // f.moveToTrash();
        if (s) {
            emit AnalyzerClient::eventProxy.showToast(Toast::Success, QString::fromLocal8Bit("删除成功"));
        } else {
            emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("删除失败"));
        }
    } else if (action_name == ACTION_CREATE_FILE) {
        menu->showInputBox(false);
    } else if (action_name == ACTION_CREATE_DIR) {
        menu->showInputBox(true);
    } else if (action_name == ACTION_CREATE_SUBDIR) {
        menu->showInputBox(true, true);
    } else if (action_name == ACTION_EXPAND) {
        ui->TreeView->expand(*clickedIndex);
    } else if (action_name == ACTION_COLLAPSE) {
        ui->TreeView->collapse(*clickedIndex);
    } else if (action_name == ACTION_COPY) {
        menu->copySrcPath = model->filePath(idx);
        menu->copySrcIsDir = model->isDir(idx);
    } else if (action_name == ACTION_PASTE) {
        pasteFile();
    }
}

void FileManager::createFile(const QString &file_name, bool create_dir, bool sub_dir) {
    if (file_name.isEmpty()) {
        emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("文件名不能为空"));
    } else {
        bool flag = false;
        QModelIndex idx = filterProxyModel->mapToSource(*clickedIndex);
        if (sub_dir) {
            flag = model->existFileName(idx, file_name);
        } else {
            flag = model->existFileName(idx.parent(), file_name);
        }
        if (flag) {
            emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("当前目录存在相同文件名"));
        } else {
            menu->hideInputBox();
            if (sub_dir) {
                QModelIndex res = model->mkdir(idx, file_name);
                if (res.isValid()) {
                    emit AnalyzerClient::eventProxy.showToast(Toast::Success, QString::fromLocal8Bit("创建子目录成功"));
                    emit ui->TreeView->dataChanged(QModelIndex(), QModelIndex());
                } else {
                    emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("创建子目录失败"));
                }
            } else if (create_dir) {
                QModelIndex res = model->mkdir(idx.parent(), file_name);
                if (res.isValid()) {
                    emit AnalyzerClient::eventProxy.showToast(Toast::Success, QString::fromLocal8Bit("创建目录成功"));
                    emit ui->TreeView->dataChanged(QModelIndex(), QModelIndex());
                } else {
                    emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("创建目录失败"));
                }
            } else {
                QString filePath;
                if (model->isDir(idx)) {
                    if (!create_dir || sub_dir) {
                        filePath = model->filePath(idx) + "/" + file_name;
                    } else {
                        filePath = model->filePath(idx.parent()) + "/" + file_name;
                    }
                } else {
                    filePath = model->filePath(idx.parent()) + "/" + file_name;
                }
                QFile f(filePath);
                if (!f.open(QIODevice::NewOnly)) {
                    emit AnalyzerClient::eventProxy.showToast(Toast::Error,
                                                              QString::fromLocal8Bit("创建失败, 文件已存在"));
                } else {
                    emit AnalyzerClient::eventProxy.showToast(Toast::Success, QString::fromLocal8Bit("文件创建成功"));
                }
                f.close();
            }
        }
    }
}

void FileManager::pasteFile() {
    const QModelIndex &idx = filterProxyModel->mapToSource(*clickedIndex);
    QString dest_path;  // 最终的目标路径,同时指明了复制过去之后的文件名
    QString new_name = menu->copySrcPath.mid(menu->copySrcPath.lastIndexOf("/") + 1);  // 复制过去设置的最终的文件名
    if (model->isDir(idx)) {
        // 右击文件夹->粘贴
        dest_path = model->filePath(idx);
    } else {
        // 右击文件->粘贴
        dest_path = model->filePath(idx.parent());
    }
    if (menu->copySrcIsDir) {
        while (existDir(dest_path + "/" + new_name)) {
            new_name += "-copy";  // 存在同名的就加后缀
        }
        dest_path += QDir::separator();
        dest_path += new_name;

        QString command = "Xcopy " + QDir::toNativeSeparators(menu->copySrcPath + " " + dest_path) + " /s /e\n";

        QProcess p;
        p.start("cmd");
        p.waitForStarted();
        p.write(command.toStdString().data());
        p.write("D");
        p.write("\nexit\n");
        p.closeWriteChannel();
        p.waitForFinished();
    } else {
        QString suffix = "";
        int pos = new_name.lastIndexOf(".");
        if (pos > 0) {
            //.号开头的文件不认为是带有后缀名, 如.config
            // 因此重名的话应该是变为.config-copy而不是-copy.config
            suffix = new_name.mid(new_name.lastIndexOf("."));
            new_name = new_name.left(pos);
        }
        while (existFile(dest_path + "/" + new_name + suffix)) {
            new_name += "-copy";  // 存在同名的就加后缀
        }
        dest_path += QDir::separator();
        dest_path += new_name + suffix;

        QString command = "copy " + QDir::toNativeSeparators(menu->copySrcPath + " " + dest_path) + "\n";

        QProcess p;
        p.start("cmd");
        p.waitForStarted();
        p.write(command.toStdString().data());
        p.write("\nexit\n");
        p.closeWriteChannel();
        p.waitForFinished();
    }
}

bool FileManager::existDir(const QString &dir_path) {
    QDir dir(dir_path);
    return dir.exists();
}

bool FileManager::existFile(const QString &file_path) {
    QFile f(file_path);
    return f.exists();
}

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

SortFilterProxyModel::SortFilterProxyModel(QObject *parent) : QSortFilterProxyModel{parent} {
}

void SortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel) {
    QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel);
    Q_UNUSED(fsm);
    assert(fsm);  // sourceModel必须是QFileSystemModel

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool SortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QFileSystemModel *fsm = qobject_cast<QFileSystemModel *>(sourceModel());

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // 如果是文件夹则直接显示,如果不显示文件夹那么它里面的文件就无法显示
    if (fsm->isDir(index)) {
        return true;
    }

    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        // 判断该文件的上级目录是不是在本项目中
        if (parent == fsm->index(fsm->rootPath())) {
            // QSortFilterProxyModel::filterAcceptsRow,即由设定的过滤规则(比如正则)来决定要不要显示
            return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
        }
        // 再返回上一级目录
        parent = parent.parent();
    }
    return false;
}

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

FileModel::FileModel(QObject *parent) : QFileSystemModel(parent) {
    projectName = QString::fromLocal8Bit("未打开任何项目");
}

void FileModel::setProjectName(const QString &name) {
    projectName = name;
}

bool FileModel::invalidFileName(const QString &name) {
    QStringList filterChars;
    filterChars << "\\"
                << "/"
                << ":"
                << "*"
                << "?"
                << "\""
                << "<"
                << ">"
                << "|";
    for (QString &s : filterChars) {
        if (name.contains(s)) {
            return true;
        }
    }
    return false;
}

bool FileModel::existFileName(const QModelIndex &parent, const QString &name) {
    if (!parent.isValid()) {
        return false;
    }
    for (int i = 0;; ++i) {
        QModelIndex child = parent.child(i, 0);
        if (child.isValid()) {
            if (child.data().toString() == name) {
                return true;
            }
        } else {
            break;
        }
    }
    return false;
}

QVariant FileModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
        case Qt::DecorationRole:
            if (section == 0) {
                QImage pixmap(16, 1, QImage::Format_ARGB32_Premultiplied);
                pixmap.fill(Qt::transparent);
                return pixmap;
            }
            break;
        case Qt::TextAlignmentRole:
            return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    QString returnValue;
    switch (section) {
        case 0:
            returnValue = projectName;  //<-只改了这里,其他都是源码//
            break;
        case 1:
            returnValue = tr("Size");
            break;
        case 2:
            returnValue =
#ifdef Q_OS_MAC
                tr("Kind", "Match OS X Finder");
#else
                tr("Type", "All other platforms");
#endif
            break;
        case 3:
            returnValue = tr("Date Modified");
            break;
        default:
            return QVariant();
    }
    return returnValue;
}

Qt::ItemFlags FileModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return 0;
    }

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    // 设置节点为允许编辑
    flags |= Qt::ItemIsEditable;

    return flags;
}

bool FileModel::setData(const QModelIndex &idx, const QVariant &value, int role) {
    QString newName = value.toString();
    QString oldName = idx.data().toString();
    if (newName == oldName) {
        return true;
    }

    if (newName.isEmpty()) {
        emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("文件名不能为空"));
        return false;
    }

    if (invalidFileName(newName)) {
        emit AnalyzerClient::eventProxy.showToast(
            Toast::Error, QString::fromLocal8Bit("文件名不能包含下列任何字符: \\ / : * ? \" < > |"));
        return false;
    }

    if (existFileName(idx.parent(), newName)) {
        emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("重命名失败, 文件名重复"));
        return false;
    }

    bool successful = QFileSystemModel::setData(idx, value, role);
    if (successful) {
        emit AnalyzerClient::eventProxy.showToast(Toast::Success, QString::fromLocal8Bit("重命名成功"));
        return true;
    } else {
        emit AnalyzerClient::eventProxy.showToast(Toast::Error, QString::fromLocal8Bit("重命名失败"));
        return true;
    }
}
