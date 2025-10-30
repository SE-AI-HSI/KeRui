#ifndef CREATEPROJECTWIDGET_H
#define CREATEPROJECTWIDGET_H

#include <QWidget>
#include <functional>

namespace Ui {
class CreateProjectWidget;
}

class CreateProjectWidget : public QWidget {
    Q_OBJECT

public:
    explicit CreateProjectWidget(std::function<void(QString, QString)> callback);
    ~CreateProjectWidget();
private slots:
    void on_Close_clicked();

    void on_Cancel_clicked();

    void on_Finish_clicked();

    void on_pushButton_clicked();

private:
    void setupLayout();
    /// events
    void connectEvents();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    /// others
    bool existProjectName();

private:
    Ui::CreateProjectWidget *ui;
    std::function<void(QString, QString)> callback;

    bool isDragging = false;
    QPoint mousePressPos;
};

#endif  // CREATEPROJECTWIDGET_H
