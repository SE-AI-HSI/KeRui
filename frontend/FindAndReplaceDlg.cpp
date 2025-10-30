#include "FindAndReplaceDlg.h"
#include "ui_FindAndReplaceDlg.h"
#include <QIcon>
FindAndReplaceDlg::FindAndReplaceDlg(QWidget *parent) : QWidget(parent), ui(new Ui::FindAndReplaceDlg) {
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);
    connectEvents();
    replaceOnlyUpImage(replaceOnlyUp);
    inputLineOnlyUpImage(findingFlags);
    wholeWordImage(findingFlags);
    REGImage(findingFlags);
}

FindAndReplaceDlg::~FindAndReplaceDlg() {
    delete ui;
}

void FindAndReplaceDlg::showDlg() {
    if (ui->LineInput_findWord->text().trimmed() != "") {
        QString findWord = ui->LineInput_findWord->text().trimmed();
        emit findWords(findWord, findingFlags);
    }
}

void FindAndReplaceDlg::setWordsCounter(int counter) {
    if (counter >= 0) {
        wordsCounter = counter;
        setFindingResult();
    } else
        qDebug() << "error!";
}

void FindAndReplaceDlg::setWordIndex(int index) {
    if (index <= wordsCounter && index > 0) {
        nowWordIndex = index;
        setFindingResult();
    } else
        qDebug() << "error!";
}

void FindAndReplaceDlg::setFindingResult() {
    if (wordsCounter == 0) {
        ui->findingResult->setText(noneText);
        ui->findingResult->setStyleSheet("color:red;");
    } else {
        if (wordsCounter > 0 && nowWordIndex <= wordsCounter && nowWordIndex > 0) {
            QString str = normalText.arg(nowWordIndex).arg(wordsCounter);
            ui->findingResult->setText(str);
            ui->findingResult->setStyleSheet("");
        }
    }
}

void FindAndReplaceDlg::connectEvents() {
    /// 关闭窗口
    connect(ui->Btn_close, &QPushButton::clicked, this, [=]() {
        emit closeDlg();
    });

    /// 上一个单词
    connect(ui->Btn_pre_word, &QPushButton::clicked, this, [=]() {
        if (wordsCounter > 1) {
            emit findPreWord(findingFlags);
        }
    });

    /// 下一个单词
    connect(ui->Btn_next_word, &QPushButton::clicked, this, [=]() {
        if (wordsCounter > 1) {
            emit findNextWord(findingFlags);
        }
    });

    /// 输入框不区分大小写
    connect(ui->Btn_findUpOrLow, &QPushButton::clicked, this, [=]() {
        findingFlags ^= FINDUPORLOW;
        inputLineOnlyUpImage(findingFlags);
        if (ui->LineInput_findWord->text().trimmed() != "") {
            QString findWord = ui->LineInput_findWord->text().trimmed();
            emit findWords(findWord, findingFlags);
        }
    });

    /// 整字匹配
    connect(ui->Btn_findWholeWord, &QPushButton::clicked, this, [=]() {
        findingFlags ^= FINDWHOLEWORD;
        wholeWordImage(findingFlags);
        if (ui->LineInput_findWord->text().trimmed() != "") {
            QString findWord = ui->LineInput_findWord->text().trimmed();
            emit findWords(findWord, findingFlags);
        }
    });

    /// 正则表达式匹配
    connect(ui->Btn_REG, &QPushButton::clicked, this, [=]() {
        findingFlags ^= FINDREG;
        REGImage(findingFlags);
        if (ui->LineInput_findWord->text().trimmed() != "") {
            QString findWord = ui->LineInput_findWord->text().trimmed();
            emit findWords(findWord, findingFlags);
        }
    });

    /// 单个替换
    connect(ui->Btn_replaceOne, &QPushButton::clicked, this, [=]() {
        QString replaceWord = ui->LineInput_replace->text();
        if (replaceOnlyUp)
            replaceWord = replaceWord.toUpper();
        emit replaceOneWord(replaceWord);
    });

    /// 全部替换
    connect(ui->Btn_replaceAll, &QPushButton::clicked, this, [=]() {
        QString replaceWord = ui->LineInput_replace->text();
        if (replaceOnlyUp)
            replaceWord = replaceWord.toUpper();
        emit replaceAllWords(replaceWord);
    });

    /// 替换全大写
    connect(ui->Btn_replaceUpOnly, &QPushButton::clicked, this, [=]() {
        replaceOnlyUp ^= 1;
        replaceOnlyUpImage(replaceOnlyUp);
    });

    connect(ui->LineInput_findWord, &QLineEdit::textChanged, this, [=]() {
        if (ui->LineInput_findWord->text().trimmed() != "") {
            QString findWord = ui->LineInput_findWord->text().trimmed();
            emit findWords(findWord, findingFlags);
        }
    });
}

void FindAndReplaceDlg::replaceOnlyUpImage(bool replaceOnlyUp) {
    if (replaceOnlyUp)
        ui->Btn_replaceUpOnly->setStyleSheet(
            "QPushButton{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/replaceOnlyUpper-selected.svg);}"
            "QPushButton::hover{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/replaceOnlyUpper-selected.svg);}");
    else
        ui->Btn_replaceUpOnly->setStyleSheet(
            "QPushButton{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/replaceOnlyUpper-unselected.svg);}"
            "QPushButton::hover{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/replaceOnlyUpper-selected.svg);}");
}

void FindAndReplaceDlg::inputLineOnlyUpImage(int findingFlags) {
    if (findingFlags & FINDUPORLOW)
        ui->Btn_findUpOrLow->setStyleSheet(
            "QPushButton{border: none;image: url(:/findAndReplace/images/findAndReplace/inputOnlyUpper-selected.svg);}"
            "QPushButton::hover{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/inputOnlyUpper-selected.svg);}");
    else
        ui->Btn_findUpOrLow->setStyleSheet("QPushButton{border: none;image: "
                                           "url(:/findAndReplace/images/findAndReplace/inputOnlyUpper-unselected.svg);}"
                                           "QPushButton::hover{border: none;image: "
                                           "url(:/findAndReplace/images/findAndReplace/inputOnlyUpper-selected.svg);}");
}

void FindAndReplaceDlg::wholeWordImage(int findingFlags) {
    if (findingFlags & FINDWHOLEWORD)
        ui->Btn_findWholeWord->setStyleSheet(
            "QPushButton{border: none;image: url(:/findAndReplace/images/findAndReplace/wholeWord-selected.svg);}"
            "QPushButton::hover{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/wholeWord-selected.svg);}");
    else
        ui->Btn_findWholeWord->setStyleSheet(
            "QPushButton{border: none;image: url(:/findAndReplace/images/findAndReplace/wholeWord-unselected.svg);}"
            "QPushButton::hover{border: none;image: "
            "url(:/findAndReplace/images/findAndReplace/wholeWord-selected.svg);}");
}

void FindAndReplaceDlg::REGImage(int findingFlags) {
    if (findingFlags & FINDREG)
        ui->Btn_REG->setStyleSheet(
            "QPushButton{border: none;image: url(:/findAndReplace/images/findAndReplace/REG-selected.svg);}"
            "QPushButton::hover{border: none;image: url(:/findAndReplace/images/findAndReplace/REG-selected.svg);}");
    else
        ui->Btn_REG->setStyleSheet(
            "QPushButton{border: none;image: url(:/findAndReplace/images/findAndReplace/REG-unselected.svg);}"
            "QPushButton::hover{border: none;image: url(:/findAndReplace/images/findAndReplace/REG-selected.svg);}");
}
