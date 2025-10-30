#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <QIcon>
#include <QPushButton>
#include <QSpacerItem>
#include <QString>
#include <QVector>
#include <QWidget>

namespace Ui {
class WelcomePage;
}
class ProjectBtn;
class WelcomePage : public QWidget {
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);
    ~WelcomePage();

    void addProjectBtn(QStringList pathList); // 添加按钮

signals:
    void clickProjectBtn(const QString &path);
    void clickNewPageBtn();
    void clickSelectFolderBtn();

private slots:
    void on_pushButton_3_clicked();

private:
    Ui::WelcomePage *ui;
    QVector<ProjectBtn *> myProjectBtn;
    QSpacerItem *mySpacer;

    void addProjectBtn(QString &path);
};

//////////    /////////  //    ///  /////////  //////     /////////  ///////
//////   //     ///      //   ///     ///      ///   //   ///        ///    //
/////   //     ///       //  ///     ///      ///   //   /////////  ///////
////   //     ///         /////     ///      ///   //   ///        /////
///////   /////////       ///   /////////  ///////    /////////  ///  ////

class ProjectBtn : public QPushButton {
public:
    ProjectBtn(const QString &path, QWidget *parent = nullptr);
    const QString projectPath;
};
#endif // WELCOMEPAGE_H
