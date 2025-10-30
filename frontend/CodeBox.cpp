#include "CodeBox.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <chrono>

#include "AnalyzeConfig.h"
#include "AnalyzeThread.h"
#include "AnalyzerClient.h"
#include "CloneExtractPreview.h"
#include "FileManager.h"
#include "GraphicsEffectUtil.h"
#include "SummaryWidget.h"
#include "UrlInterpreter.h"
#include "ui_CodeBox.h"

CodeBox::CodeBox(QWidget *parent) : QWidget(parent), ui(new Ui::CodeBox), ARHandler(new AnalyzeResultHandler()) {
    ui->setupUi(this);
    autoSaveTimer = new QTimer(this);

    setupLayout();
    setupStyle();
    connectEvents();
}

CodeBox::~CodeBox() {
    delete ui;
    autoSaveTimer->stop();
    saveAllFiles();  // 正常关闭就能保证所有文件都保存//
    delete ARHandler;
}

void CodeBox::openFile(const QString &filePath) {
    //    auto start1 = std::chrono::high_resolution_clock::now();
    if (!filePath.isEmpty()) {
        /// 扫描一遍看有没有打开过///
        for (CodeBoxFile &f : fileList) {
            if (f.path == filePath) {
                tabBar->switchTab(filePath, "");
                return;
            }
        }
    }
    //    auto start2 = std::chrono::high_resolution_clock::now();//5微秒
    /// 没打开过再新建///
    if (filePath.endsWith(SUMMARY_FILE_NAME)) {
        // 打开的是总览文件
        static std::vector<SummaryRowData> vec;
        vec.clear();
        SummaryWidget::readSummary(vec);

        SummaryWidget *sw = new SummaryWidget(vec, this);
        ui->StackedWidget->addWidget(sw);
        CodeBoxFile file(filePath, "UTF-8", nullptr, sw);
        fileList.prepend(file);
        tabBar->addTab(fileList.first());

        //        vec.clear();
        // CodeEditor会通过FileTabBar发送回来的信号更新//
        emit newFileOpened();
    } else {
        // 打开的是代码文件
        //        auto start3 = std::chrono::high_resolution_clock::now();

        CodeEditor *editor = new CodeEditor(this);
        ui->StackedWidget->addWidget(editor);  // 0.227s

        QPair<QString, QString> p(UTF8, "");
        if (!filePath.isEmpty())
            p = FileManager::read(filePath);
        CodeBoxFile file(filePath, p.first, editor);
        fileList.prepend(file);

        editor->setTextToEditor(p.second);  // 4.069s
        tabBar->addTab(fileList.first());  // 不能直接引用上面的file,因为函数结束后它就会被释放掉,是个无效地址//

        // CodeEditor会通过FileTabBar发送回来的信号更新//
        emit newFileOpened();  // 0s
        if (!filePath.isEmpty()) {
            // 查找分析信息，构建标签//
            emit loadingFile();
            loadingLabels(this->editingFile);
            emit loadingFinish();
        }

        if (!autoSaveTimer->isActive()) {
            autoSaveTimer->start();
        }
        //        auto start7 = std::chrono::high_resolution_clock::now();
        //        auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(start4 - start3);
        //        auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(start5 - start4);
        //        auto duration5 = std::chrono::duration_cast<std::chrono::microseconds>(start6 - start5);
        //        auto duration6 = std::chrono::duration_cast<std::chrono::microseconds>(start7 - start6);
        //        qDebug() << duration3.count() << " microseconds";
        //        qDebug() << duration4.count() << " microseconds";
        //        qDebug() << duration5.count() << " microseconds";
        //        qDebug() << duration6.count() << " microseconds";
    }
}

void CodeBox::createEmptyPage() {
}

void CodeBox::upDateLables() {
    if (editingFile != nullptr)
        editingFile->editor->updateLabels();
}

void CodeBox::setupLayout() {
    setContentsMargins(30, 10, 30, 0);

    tabBar = new FileTabBar(this);
    // 将占位窗口替换为FileTabBar//
    delete this->layout()->replaceWidget(ui->TabBarPlaceholder, tabBar);

    ui->Background->raise();  // 升起来阴影才不会被tab遮挡//
}

void CodeBox::setupStyle() {
    ui->Background->setGraphicsEffect(GraphicsEffectUtil::newShadow(this));
}

void CodeBox::connectEvents() {
    // 文件切换了,显示对应的CodeEditor//
    connect(tabBar, &FileTabBar::tabSwitched, this, [=](CodeBoxFile *const file) {
        if (file->editor != nullptr) {
            /// 这是个代码文件
            ui->frame_7->show();
            ui->StackedWidget->setCurrentWidget(file->editor);

            if (findAndReplaceDlg != nullptr)  // 关闭一下之前文件的查找替换
                if (editingFile != nullptr)
                    editingFile->editor->finishFindAndReplace();

            editingFile = file;

            // editingFile->editor->showLabels();似乎不需要这句?
            // 打开一下当前文件的查找替换
            if (findAndReplaceDlg != nullptr)  // 打开当前文件的查找替换
                findAndReplaceDlg->showDlg();
        } else {
            /// 这是个总览文件
            ui->frame_7->hide();
            ui->StackedWidget->setCurrentWidget(file->summaryWidget);
            editingFile = nullptr;
        }
    });

    // 某个文件被关闭了//
    connect(tabBar, &FileTabBar::tabClosed, [&, this](const CodeBoxFile &file) {
        for (CodeBoxFile::Iterator it = fileList.begin(); it != fileList.end(); ++it) {
            if (it.operator->() == &file) {  // 比较地址//
                if (file.editor != nullptr) {
                    if (!file.isSaved && !file.path.isEmpty()) {
                        FileManager::write(file.path, file.encoding, file.editor->getTextEdit()->toPlainText());
                    }
                    delete file.editor;
                } else {
                    delete file.summaryWidget;
                }
                if (fileList.size() != 1) {
                    editingFile = &(*--it);
                    ++it;
                } else {
                    editingFile = nullptr;
                }
                fileList.erase(it);
                break;
            }
        }
        if (fileList.isEmpty()) {
            autoSaveTimer->stop();  // 没有打开的文件了就关闭定时器//
            emit allFileClosed();
        }
    });

    // 正在编辑的文件的内容变动了//
    connect(&TextEdit::eventProxy, &TextEditEventProxy::textChanged, this, [=] {
        if (editingFile != nullptr && editingFile->isSaved) {
            editingFile->isSaved = false;
            tabBar->markUnsaved(editingFile->path);
        }
    });

    // 定时保存文件//
    connect(autoSaveTimer, &QTimer::timeout, this, &CodeBox::saveAllFiles);
    autoSaveTimer->start(5000);

    // 总评信息向上传递//
    //Normalization Efficiency Maintanability Security
    connect(ARHandler, &AnalyzeResultHandler::scoresAndComment, this,
            [=](int total, int s1, int s2, int s3, int s4, QString comment) {
                emit scoresAndComment(total, s1, s2, s3, s4, comment);
            });

    /// 主动按ctrl+s保存文件
    connect(&TextEdit::eventProxy, &TextEditEventProxy::saveFile, this, [=] { saveEditingFile(); });

    /// 主动按ctrl+h调出弹窗
    connect(&TextEdit::eventProxy, &TextEditEventProxy::findAndReplaceWords, this, [=] { findAndReplace(); });

    /// shift + alt + f格式化代码
    connect(&TextEdit::eventProxy, &TextEditEventProxy::formatCode, this, [=] { formatCode(); });

    /// 总览中点击打开某个文件
    connect(&SummaryWidget::eventProxy, &SummaryWidgetEventProxy::openFile, this,
            [=](const QString &file_path) { openFile(file_path); });

    connect(&UrlInterpreter::eventProxy, &UrlInterpreterEventproxy::optimizeClone, this,
            [=](QObject *const source, QObject *const sourceParent, int block_beg_row, int block_beg_col) {
                for (CodeBoxFile &f : fileList) {
                    if (f.editor != nullptr && f.editor->getTextEdit() == sourceParent) {
                        f.editor->optimizeCloneCodes(source);
                        showCodeGeneratePreview(f.path);
                        break;
                    }
                }
            });
    connect(&UrlInterpreter::eventProxy, &UrlInterpreterEventproxy::maskWarning, this,
            [=](QObject *const sourceParent, int w_code) {
                emit AnalyzerClient::eventProxy.showToast(
                    Toast::Success, QString::fromLocal8Bit("后续分析将不再显示该类警告，可在设置中恢复"));
                AnalyzeConfig::maskWarning(w_code);
            });
    // 分析步骤信息
    connect(ARHandler, &AnalyzeResultHandler::analyzeStep, this,
            [=](const AnalyzeStep &stepInfo) { emit analyzeStep(stepInfo); });

    connect(&UrlInterpreter::eventProxy, &UrlInterpreterEventproxy::jumpToFile, this,
            [=](const QString &file_path) { openFile(file_path); });
}

void CodeBox::formatCode() {
    /// shift + alt + f格式化代码
    if (editingFile != nullptr) {
        /// 先保存
        saveEditingFile();

        QProcess p(0);
        QString command = "cd " + QCoreApplication::applicationDirPath() + "/uncrustify\n";
        command += "uncrustify -c kr-indent.cfg --no-backup ";
        command += editingFile->path;
        command += "\n";  // 没有\n不会执行
        p.start("cmd");
        p.waitForStarted();
        p.write(command.toStdString().data());
        p.write("\nexit\n");  // 不清楚有没有必要
        p.closeWriteChannel();
        p.waitForFinished();
        // qDebug() << QString::fromLocal8Bit(p.readAllStandardOutput());
        QString res = QString::fromLocal8Bit(p.readAllStandardError());
        p.close();

        QRegExp r("do_source_file\\(1507\\): Parsing: .* as language CPP\\r\\n");
        if (r.exactMatch(res)) {
            /// 重新读取
            int cpos = editingFile->editor->getTextEdit()->textCursor().position();
            editingFile->editor->getTextEdit()->setText(FileManager::read(editingFile->path).second);
            // 还原光标位置
            QTextCursor c = editingFile->editor->getTextEdit()->textCursor();
            c.setPosition(cpos);
            editingFile->editor->getTextEdit()->setTextCursor(c);
        } else {
            qDebug() << QString::fromLocal8Bit("格式化失败, 代码存在问题");
        }
    }
}

void CodeBox::saveAllFiles() {
    // QTimer是运行在主线程上的,也就是说它没有自己的线程,所以操作都是同步的,不用考虑异步的问题//
    // 如果太卡了,那到时候再改独立线程//
    for (CodeBoxFile &f : fileList) {
        if (!f.isSaved && !f.path.isEmpty()) {
            // qDebug() << "CodeBox:" << f.name() << "is auto saved";
            FileManager::write(f.path, f.encoding, f.editor->getTextEdit()->toPlainText());
            f.isSaved = true;
            tabBar->markSaved(f.path);
        }
    }
}

void CodeBox::saveEditingFile() {
    if (editingFile != nullptr && !editingFile->isSaved) {
        if (editingFile->path.isEmpty()) {
            QString pos = FileManager::projectPath();
            QString path = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("选择保存位置"),
                                                        pos.isEmpty() ? "/" : pos, "Header(*.h);;Source(*.cpp)");
            if (!path.isEmpty()) {
                editingFile->changePath(path);
            } else
                return;
        }
        FileManager::write(editingFile->path, editingFile->encoding, editingFile->editor->getTextEdit()->toPlainText());
        editingFile->isSaved = true;
        tabBar->markSaved(editingFile->path);
        AnalyzerClient::eventProxy.showToast(Toast::Success, QString::fromLocal8Bit("保存成功"));
    }
}

void CodeBox::findAndReplace() {
    if (findAndReplaceDlg == nullptr) {
        findAndReplaceDlg = new FindAndReplaceDlg(this);
        findAndReplaceDlg->move(this->width() - findAndReplaceDlg->width() * 1.15, 68);
        findAndReplaceConnectEvents();
        findAndReplaceDlg->showDlg();
        findAndReplaceDlg->show();
        return;
    } else {
        if (findAndReplaceDlg->isHidden() == true) {
            findAndReplaceDlg->showDlg();
            findAndReplaceDlg->show();
        } else {
            findAndReplaceDlg->hide();
            editingFile->editor->finishFindAndReplace();
        }
    }
}

void CodeBox::loadingLabels(CodeBoxFile *cf) {
    cf->editor->clearLabels();
    cf->editor->passInfo(ARHandler->getFileWarningInfo(cf->path), ARHandler->getFileErrorInfo(cf->path),
                         ARHandler->getFileCloneSets(), cf->path, cf->relativelyPath);
}

void CodeBox::generateSummary() {
    QStringList fileNames;
    FileManager::RecursiveAllFiles(FileManager::projectPath(), fileNames);

    std::vector<SummaryRowData> vec;
    QString prefix = FileManager::projectPath() + '/';
    for (QString &name : fileNames) {
        QList<int> list = ARHandler->getFileSummary(name);
        // qDebug() << prefix + name ;
        vec.emplace_back(SummaryRowData(name, list[0], list[1], list[2], list[3], list[4]));
    }
    SummaryWidget::saveSummary(vec);
    openFile(prefix + SUMMARY_FILE_NAME);
}

void CodeBox::showCodeGeneratePreview(QString file_path) {
    std::vector<CodeGenerateInfo> &list = ARHandler->getCodeGenerateInfo();
    if (list.size() > 0) {
        CloneExtractPreview *c = new CloneExtractPreview(list, file_path, this);
    } else {
        qDebug() << "There's no CodeGenerateInfo";
    }
}

void CodeBox::closeSummaryWidget() {
    tabBar->closeSummaryWidget();
}

void CodeBox::closeAllFiles() {
    tabBar->closeAllTabs();
}

void CodeBox::findAndReplaceConnectEvents() {
    /// 查找替换弹窗有关的事件
    ///  关闭弹窗
    connect(findAndReplaceDlg, &FindAndReplaceDlg::closeDlg, this, [=]() {
        findAndReplaceDlg->hide();
        editingFile->editor->finishFindAndReplace();
    });
    /// 查找新单词
    connect(findAndReplaceDlg, &FindAndReplaceDlg::findWords, this, [=](QString findText, int Flags) {
        CodeEditor::wordToBeFound = findText;
        CodeEditor::REGToBeUsed = editingFile->editor->textToREG(findText, Flags);
        if (Flags & FINDUPORLOW)
            CodeEditor::REGToBeUsed.setCaseSensitivity(Qt::CaseInsensitive);
        else
            CodeEditor::REGToBeUsed.setCaseSensitivity(Qt::CaseSensitive);

        editingFile->editor->searchForText();
        findAndReplaceDlg->setWordsCounter(CodeEditor::findWordsSTK.size());
        findAndReplaceDlg->setWordIndex(CodeEditor::lastIndex);
    });

    /// 高亮下一个单词
    connect(findAndReplaceDlg, &FindAndReplaceDlg::findNextWord, this, [=]() {
        if (editingFile->editor->findNextWord())
            findAndReplaceDlg->setWordIndex(CodeEditor::lastIndex);
    });

    /// 高亮上一个单词
    connect(findAndReplaceDlg, &FindAndReplaceDlg::findPreWord, this, [=]() {
        if (editingFile->editor->findPreWord())
            findAndReplaceDlg->setWordIndex(CodeEditor::lastIndex);
    });

    /// 替换当前单词
    connect(findAndReplaceDlg, &FindAndReplaceDlg::replaceOneWord, this, [=](QString replaceWord) {
        if (editingFile->editor->replaceOneWord(replaceWord)) {
            editingFile->editor->searchForText();
            findAndReplaceDlg->setWordsCounter(CodeEditor::findWordsSTK.size());
            findAndReplaceDlg->setWordIndex(CodeEditor::lastIndex);
        }
    });

    /// 替换所有单词
    connect(findAndReplaceDlg, &FindAndReplaceDlg::replaceAllWords, this, [=](QString replaceWord) {
        if (editingFile->editor->replaceAllWords(replaceWord)) {
            editingFile->editor->searchForText();
            findAndReplaceDlg->setWordsCounter(CodeEditor::findWordsSTK.size());
            findAndReplaceDlg->setWordIndex(CodeEditor::lastIndex);
        }
    });
}

void CodeBox::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (findAndReplaceDlg != nullptr)
        findAndReplaceDlg->move(this->width() - findAndReplaceDlg->width() * 1.2, 77);
}

void CodeBox::initialListener() {
    ARHandler->InitialARHandler();
}

void CodeBox::upDateOpenFileLabels() {
    for (QLinkedList<CodeBoxFile>::iterator it = fileList.begin(); it != fileList.end(); ++it) {
        if (it->editor != nullptr)
            loadingLabels(&*it);
    }
}

void CodeBox::updateCode(const QString &file_path) {
    for (CodeBoxFile &f : fileList) {
        if (f.path == file_path) {
            f.editor->setTextToEditor(FileManager::read(file_path, true));
            f.editor->getLabelManager()->clearLabels();
            break;
        }
    }
}

void CodeBox::closeAllOpenedFiles() {
    tabBar->closeAllTabs();
}

void CodeBox::connectSender() {
    qRegisterMetaType<AnalyzeStep>("AnalyzeStep");
    qRegisterMetaType<IErrorInfo>("IErrorInfo");
    qRegisterMetaType<IWarningInfo>("IWarningInfo");
    qRegisterMetaType<CloneSetInfo>("CloneSetInfo");
    qRegisterMetaType<ClonePosition>("ClonePosition");
    qRegisterMetaType<GlobalInfo>("GlobalInfo");
    qRegisterMetaType<FileTable>("FileTable");
    qRegisterMetaType<CodeNamingInfo>("CodeNamingInfo");
    connect(JsonPaser::sender, &Sender::errorInfoSending, this->ARHandler, &AnalyzeResultHandler::CatchError);
    connect(JsonPaser::sender, &Sender::warningInfoSending, this->ARHandler, &AnalyzeResultHandler::CatchWarning);
    connect(JsonPaser::sender, &Sender::clonePositionSending, this->ARHandler,
            &AnalyzeResultHandler::CatchClonePosotion);
    connect(JsonPaser::sender, &Sender::cloneSetInfoSending, this->ARHandler, &AnalyzeResultHandler::CatchCloneSet);
    connect(JsonPaser::sender, &Sender::analyzeStepInfoSending, this->ARHandler, &AnalyzeResultHandler::CatchAnaStep);
    connect(JsonPaser::sender, &Sender::globalInfoSending, this->ARHandler, &AnalyzeResultHandler::CatchGlobalInfo);
    connect(JsonPaser::sender, &Sender::fileTableSending, this->ARHandler, &AnalyzeResultHandler::CatchFileTable);
    connect(JsonPaser::sender, &Sender::codeNamingInfoSending, this->ARHandler,
            &AnalyzeResultHandler::CatchCodeNamingInfo);
}
