#include "MainWidget.h"
#include "AnalyzerClient.h"
#include "CloneExtractPreview.h"
#include "CodeComparison.h"
#include "CreateProjectWidget.h"
#include "SettingWidget.h"
#include "ui_mainwidget.h"
#include <GraphicsEffectUtil.h>
#include <QDebug>
#include <QFileDialog>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MainWidget) {
    ui->setupUi(this);
    analyzeThread = new AnalyzeThread(this);

    setupLayout();
    setupStyle();
    connectEvents();
    LabelManager::initInfo();
}

MainWidget::~MainWidget() {
    delete ui;
}

void MainWidget::connectSender() {
    codeBox->connectSender();
}

void MainWidget::resizeEvent(QResizeEvent *event) {
    // qDebug()<<"horizontalSplitter size:"<<horizontalSplitter->size();
    horizontalSplitter->resize(event->size());
    // qDebug()<<"horizontalSplitter size:"<<horizontalSplitter->size();
    if (analyzingPopPage->isVisible())
        analyzingPopPage->setGeometry(0, 0, this->width(), this->height());
    if (fileLoadingPopPage->isVisible())
        fileLoadingPopPage->setGeometry(0, 0, this->width(), this->height());
}

void MainWidget::receiveScoresAndComment(int scores[], QString comment) {
}

void MainWidget::swapWelcomePage(bool showWelcomePage) {
    if (showWelcomePage && !welcomePage->isVisible()) {
        horizontalSplitter->replaceWidget(1, welcomePage);
        verticalSplitter->hide();
        verticalSplitter->setParent(this);
    } else if (!showWelcomePage && !verticalSplitter->isVisible()) {
        horizontalSplitter->replaceWidget(1, verticalSplitter);
        welcomePage->hide();
        welcomePage->setParent(this);
    }
}

void MainWidget::setupLayout() {
    ui->EmptyPageBtn->hide();
    ui->CloseProjectBtn->hide();
    // 设置一个layout让其内容大小自动调整,但是horizontalSplitter仍然需要通过resizeEvent调整大小//
    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->setMargin(0);
    vboxLayout->addWidget(ui->BackgroundWidget);

    // FileManager//
    fileManager = new FileManager(this);

    // ScoreWidget//
    scoreWidget = new ScoreWidget(this);

    // AnalyzingPopPage//
    analyzingPopPage = new AnalyzingPopPage(this);
    analyzingPopPage->hide();

    // FileLoadingPopPage//
    fileLoadingPopPage = new FileLoadingPopPage(this);
    fileLoadingPopPage->hide();

    // CodeBox//
    codeBox = new CodeBox(this);

    // verticalSplitter//
    verticalSplitter = new QSplitter(this);
    verticalSplitter->setOrientation(Qt::Vertical);
    verticalSplitter->addWidget(scoreWidget);
    verticalSplitter->addWidget(codeBox);
    verticalSplitter->addWidget(ui->BottomTabBar);
    verticalSplitter->hide(); // 没打开文件暂时不显示//

    // welcomePage//
    welcomePage = new WelcomePage(this);
    // 初始化最近打开的文件
    welcomePage->addProjectBtn(FileManager::getRecentlyFolderPath());

    // horizontalSplitter//
    horizontalSplitter = new QSplitter(ui->BackgroundWidget);
    horizontalSplitter->addWidget(fileManager); // 文件管理器//
    horizontalSplitter->addWidget(welcomePage);

    // 让FileManager的初始宽度为360//
    QList<int> list;
    list << 360 << width() - 360;
    horizontalSplitter->setSizes(list);

    // 将最左边的按钮栏提升到顶层,否则会被遮挡//
    ui->LeftButtonBar->raise();
}

void MainWidget::setupStyle() {
    horizontalSplitter->setChildrenCollapsible(false); // 不可被折叠
    horizontalSplitter->setHandleWidth(0);
    horizontalSplitter->setStyleSheet("background-color: transparent;");

    verticalSplitter->setChildrenCollapsible(false); // 不可被折叠
    verticalSplitter->setHandleWidth(0);

    // 添加阴影//
    ui->LeftButtonBar->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 96)); // 最左边的按钮栏
    ui->AnalyzeBtn->setGraphicsEffect(GraphicsEffectUtil::newShadow(this)); // 左边按钮栏下边的分析按钮
}

void MainWidget::connectEvents() {
    // welcomePage 的信号
    connect(welcomePage, &WelcomePage::clickNewPageBtn, this, [=]() {
        emit ui->NewProjectBtn->pressed();
    });
    connect(welcomePage, &WelcomePage::clickSelectFolderBtn, this, [=]() {
        emit ui->OpenProjectBtn->clicked();
    });
    // 最近打开的文件点击响应//
    connect(welcomePage, &WelcomePage::clickProjectBtn, this, [=](const QString &dir) {
        if (!dir.isEmpty()) {
            fileManager->index(dir);
            FileManager::write(QCoreApplication::applicationDirPath() + "/lastPath", "UTF-8", dir);
            FileManager::addRecentlyFolderPath(dir);
            analyzeThread->folderPath = dir;
            welcomePage->addProjectBtn(FileManager::getRecentlyFolderPath());

            ui->EmptyPageBtn->show();
            ui->CloseProjectBtn->show();
            ui->NewProjectBtn->hide();
        }
    });

    // 从FileManager中打开了文件,让CodeBox响应//
    connect(fileManager, &FileManager::openFile, codeBox, &CodeBox::openFile);

    /// 点击左侧"空白页"
    connect(ui->EmptyPageBtn, &QPushButton::pressed, [=] {
        codeBox->openFile("");
    });
    /// 点击左侧"新建项目"
    connect(ui->NewProjectBtn, &QPushButton::pressed, [=] {
        CreateProjectWidget *w = new CreateProjectWidget([this](QString path, QString file_name) {
            fileManager->index(path);
            if (!file_name.isEmpty()) {
                codeBox->openFile(path + "/" + file_name);
                fileManager->addRecentlyFolderPath(path);
                welcomePage->addProjectBtn(FileManager::getRecentlyFolderPath());
                analyzeThread->folderPath = path;
                ui->CloseProjectBtn->show();
            }
        });
    });
    /// 点击左侧"更多"
    connect(ui->MoreBtn, &QPushButton::pressed, [=] {
        auto setting = new SettingWidget();
        setting->show();
    });

    // 点击左侧的"打开项目"按钮//
    connect(ui->OpenProjectBtn, &QPushButton::clicked, this, [=] {
        QPair<QString, QString> lastPathFile = FileManager::read(QCoreApplication::applicationDirPath() + "lastPath");
        QString lastPath = "/home";
        if (lastPathFile.second != "")
            lastPath = lastPathFile.second;

        QString dir = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("选择项目"), lastPath,
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            codeBox->closeAllOpenedFiles();
            fileManager->index(dir);
            analyzeThread->folderPath = dir;
            // 更新按钮
            welcomePage->addProjectBtn(FileManager::getRecentlyFolderPath());
            ui->EmptyPageBtn->show();
            ui->CloseProjectBtn->show();
            ui->NewProjectBtn->hide();
        }
    });

    // CodeBox打开了新文件//
    connect(
        codeBox, &CodeBox::newFileOpened, this,
        [=] {
            swapWelcomePage(false);
        },
        Qt::DirectConnection);

    // CodeBox关闭了所有文件//
    connect(codeBox, &CodeBox::allFileClosed, this, [=] {
        swapWelcomePage(true);
    });

    // 点击左侧按钮栏的"开始分析"/"重新分析"按钮//
    connect(ui->AnalyzeBtn, &QPushButton::pressed, this, [=] {
        emit fileManager->startAnalysis();
    });

    // 点击FileManager中的大"开始分析"按钮//
    connect(fileManager, &FileManager::startAnalysis, this, [=] {
        if (!analyzeThread->isRunning()) {
            if (analyzeThread->folderPath != "") {
                analyzeThread->start();
            } else {
                // 提示弹窗
            }
        }
    });

    // 开始分析之前先初始化//
    connect(
        &AnalyzeThreadEventProxy::eventProxy, &AnalyzeThreadEventProxy::progressChanged, this,
        [=](AnalyzeProgress ap) {
            if (ap == AnalyzeProgress::Initializing) {
                // 开始分析之前将打开的文件中所有没有保存的文件保存//
                codeBox->saveAllFiles();
                // 开始分析之前先让ScoreWidget显示问号//
                scoreWidget->reset();
                // 开始分析之前需要把消息的集合清空//
                codeBox->initialListener();
                codeBox->closeSummaryWidget();
                // 直接在这个时候弹出分析中窗口//
                analyzingPopPage->setGeometry(0, 0, this->width(), this->height());
                analyzingPopPage->show();
                analyzingPopPage->initial();
                QCoreApplication::processEvents();
                // 上传文件到数据库//
                // 要先获取全部的代码文件的路径
                QStringList filePaths;
                FileManager::RecursiveAllFiles(FileManager::getCurrentPath(), filePaths);

                /// Begin以下是针对比赛做出的临时操作
                for (const QString &path : filePaths) {
                    QPair<QString, QString> p = FileManager::read(path); //<encoding, content>
                    p.second.replace("\t", "    ");
                    FileManager::write(path, p.first, p.second);
                }
                /// End

                DBManager::newAnalyze(filePaths);
                // 可以传log文件的文件指针
                //            DebugController::SetLogFile(DBManager::getLogFilePointer());
            } else if (ap == AnalyzeProgress::Finished) {
                // 结束一次分析要把当前打开的全部文件的标签更新//
                codeBox->upDateOpenFileLabels();
                // 分析结束后（把总览加上去）展示总览//
                codeBox->generateSummary();
                // 分析结束——上传分析log
                // 这个也要搞到log的文件名才行！
                DBManager::successfulAnalyze();
                analyzingPopPage->hide();
            }
        },
        Qt::BlockingQueuedConnection);

    // 接收分析得到的分数和评价//
    //Normalization Efficiency Maintanability Security
    //这边分数有问题！！
    connect(codeBox, &CodeBox::scoresAndComment, this, [=](int total, int s1, int s2, int s3, int s4, QString comment) {
        QVector<int> weights = SettingWidget::getScoreWeights();
        int allWeight = weights[0] + weights[1] + weights[2] + weights[3];
        total = (s1 * weights[0] + s2 * weights[1] + s3 * weights[2] + s4 * weights[3]) / allWeight;
        scoreWidget->setData(total, s1, s2, s3, s4, comment);
    });

    // 点击筛选的按钮改变显示的标签
    connect(scoreWidget, &ScoreWidget::LabelHide, this, [=](int flags, bool errorFlag) {
        LabelManager::flags = flags;
        LabelManager::errorFlag = errorFlag;
        codeBox->upDateLables();
    });

    // 中断分析//
    connect(analyzingPopPage, &AnalyzingPopPage::analyzeStop, this, [=]() {
        if (analyzeStopThreas == nullptr)
            analyzeStopThreas = new AnalyzeStopThread(this);
        analyzeStopThreas->start();
    });

    connect(codeBox, &CodeBox::loadingFile, this, [=]() {
        fileLoadingPopPage->setGeometry(0, 0, this->width(), this->height());
        fileLoadingPopPage->setVisible(true);
        QCoreApplication::processEvents();
        fileLoadingPopPage->popUp();
        // qDebug() << "show!";
    });

    connect(codeBox, &CodeBox::loadingFinish, this, [=]() {
        fileLoadingPopPage->setVisible(false);
        fileLoadingPopPage->pushDown();
    });

    // 分析阶段信息
    connect(codeBox, &CodeBox::analyzeStep, analyzingPopPage, &AnalyzingPopPage::CatchAnaStep);
}

void MainWidget::on_CloseProjectBtn_clicked() {
    fileManager->index("");
    analyzeThread->folderPath = "";

    ui->EmptyPageBtn->hide();
    ui->NewProjectBtn->show();
    ui->CloseProjectBtn->hide();

    codeBox->closeAllOpenedFiles();
}
