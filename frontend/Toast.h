#ifndef TOAST_H
#define TOAST_H
#include "GraphicsEffectUtil.h"
#include <QFrame>
#include <QLabel>
#include <QTimer>

class Toast : public QObject {
    Q_OBJECT
public:
    enum Type { Success, Warning, Error };
    Toast(QWidget *parent);
    void showSuccess(QString success);
    void showWarning(QString warning);
    void showError(QString error);
    void showToast(Toast::Type t, const QString &text);

private:
    /// init
    void setupLayout(QWidget *parent);
    void setupStyle();
    /// events
    void connectEvents();
    /// others
    void showToast();

protected:
    QTimer *timer;

    QFrame *frame;
    QLabel *icon;
    QLabel *text;

    int alpha;
    QGraphicsOpacityEffect *opacityEffect;
};

#endif // TOAST_H
