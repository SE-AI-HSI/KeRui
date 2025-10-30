#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <QKeyEvent>
#include <QListWidget>
#include <QTextEdit>

/**
 * @class AutoComplete
 * @brief 自动补全窗口
 */

class AutoComplete : public QListWidget {
    Q_OBJECT
public:
    AutoComplete(QWidget *parent); // parent是TextEdit
    int ldistance(std::string source, std::string target);
    void setIgnore(bool value);
    void show();
    void hide();
    void insert();
    void chooseNext();
    void choosePrevious();

private:
    /// init
    void setup();
    /// events

private:
    static QStringList reservedWords;
    bool ignore = false;
};

#endif // AUTOCOMPLETE_H
