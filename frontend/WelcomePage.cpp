#include "WelcomePage.h"
#include "ui_WelcomePage.h"
#include <QDir>
#include <QProcess>
#include <QPushButton>
#include <QtDebug>

#define MIN_ICON_SIZE 40
WelcomePage::WelcomePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::WelcomePage),
      mySpacer(new QSpacerItem(20, 0, QSizePolicy::Fixed, QSizePolicy::Expanding)) {
    ui->setupUi(this);
    connect(ui->newPage, &QPushButton::clicked, this, [=]() {
        emit clickNewPageBtn();
    });
    connect(ui->selectFolder, &QPushButton::clicked, this, [=]() {
        emit clickSelectFolderBtn();
    });
}

WelcomePage::~WelcomePage() {
    delete ui;
    for (ProjectBtn *btn : myProjectBtn)
        delete btn;
}

void WelcomePage::addProjectBtn(QStringList pathList) {
    while (ui->recentlyProject_layout->count()) {
        ui->recentlyProject_layout->removeItem(ui->recentlyProject_layout->itemAt(0));
    }

    for (ProjectBtn *btn : myProjectBtn)
        delete btn;
    myProjectBtn.clear();

    for (QString path : pathList)
        addProjectBtn(path);

    ui->recentlyProject_layout->addItem(mySpacer);
}

void WelcomePage::addProjectBtn(QString &path) {
    ProjectBtn *btn = new ProjectBtn(path, this);
    myProjectBtn.append(btn);

    ui->recentlyProject_layout->addWidget(btn);

    connect(btn, &QPushButton::clicked, this, [=]() {
        emit clickProjectBtn(btn->projectPath);
    });
}

ProjectBtn::ProjectBtn(const QString &path, QWidget *parent) : projectPath(path), QPushButton(parent) {
    QString text = projectPath.right(projectPath.size() - projectPath.lastIndexOf("/") - 1);
    setText("  " + text);
    setFlat(true);
    setStyleSheet("color:rgb(78, 78, 78);"
                  "border-image: url(:/welcomePage/images/welcomePage/transparentBackground.svg);"
                  "text-align:left;"
                  "font-size:20px;"
                  "border:10px;");
    QString classify = path.right(3);
    if (classify == "cpp")
        setIcon(QIcon(":/welcomePage/images/welcomePage/page.svg"));
    else
        setIcon(QIcon(":/welcomePage/images/welcomePage/floder.svg"));

    setCursor(Qt::PointingHandCursor);

    setIconSize(QSize(MIN_ICON_SIZE, MIN_ICON_SIZE));
}

void WelcomePage::on_pushButton_3_clicked() {
    QString command = "start /r" + QApplication::applicationDirPath() + "/Help.pdf";
    QProcess p;
    p.start("cmd", QStringList() << command);
    p.waitForFinished();
}
