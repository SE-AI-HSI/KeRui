#include "AutoComplete.h"
#include "TextEdit.h"
#include <QApplication>
#include <QScrollBar>

using std::max;
using std::min;
using std::string;
using std::vector;

QStringList AutoComplete::reservedWords;

AutoComplete::AutoComplete(QWidget *parent) : QListWidget(parent) {
    TextEdit *textEdit = qobject_cast<TextEdit *>(parent);
    assert(textEdit); // 必须是TextEdit
    setup();
}

int AutoComplete::ldistance(std::string source, std::string target) {
    // step 1
    int n = source.length();
    int m = target.length();
    if (m == 0)
        return n;
    if (n == 0)
        return m;
    // Construct a matrix
    typedef vector<vector<int>> Tmatrix;
    Tmatrix matrix(n + 1);
    for (int i = 0; i <= n; i++)
        matrix[i].resize(m + 1);
    // step 2 Initialize
    for (int i = 1; i <= n; i++)
        matrix[i][0] = i;
    for (int i = 1; i <= m; i++)
        matrix[0][i] = i;
    // step 3
    for (int i = 1; i <= n; i++) {
        const char si = source[i - 1];
        // step 4
        for (int j = 1; j <= m; j++) {
            const char dj = target[j - 1];
            // step 5
            int cost;
            if (si == dj) {
                cost = 0;
            } else {
                cost = 1;
            }
            // step 6
            const int above = matrix[i - 1][j] + 1;
            const int left = matrix[i][j - 1] + 1;
            const int diag = matrix[i - 1][j - 1] + cost;
            matrix[i][j] = min(above, min(left, diag));
        }
    } // step7
    return matrix[n][m];
}

void AutoComplete::setIgnore(bool value) {
    ignore = value;
}

void AutoComplete::show() {
    QListWidget::clear(); /// 在这里清空了
    if (ignore) {
        hide();
        return;
    }
    TextEdit *textEdit = (TextEdit *)parent();
    QString word = textEdit->typingWord();
    if (word.isEmpty()) {
        hide();
        return;
    }

    int maxLength = 0;
    QMap<QString, int> distance;
    vector<QString> itemList;
    for (QString &rword : reservedWords) {
        if (rword.contains(word)) {
            itemList.emplace_back(rword);
            // 计算两个词之间的相似度距离
            distance[rword] = ldistance(rword.toStdString(), word.toStdString());
            maxLength = max(maxLength, rword.length());
        }
    }
    if (itemList.size() == 0) { // 没有匹配的词
        hide();
        return;
    }
    sort(itemList.begin(), itemList.end(), [&](const QString &s1, const QString &s2) -> bool {
        return distance[s1] < distance[s2];
    });
    for (QString &item : itemList) {
        QListWidget::addItem(new QListWidgetItem(item));
    }
    QListWidget::setCurrentRow(0, QItemSelectionModel::Select);

    QRect pos = textEdit->cursorRect();
    int x = pos.x();
    int y = pos.y() + textEdit->lineHeight();

    move(x, y);
    QListWidget::show();

    int lines = max(3ull, itemList.size());
    lines = min(10, lines);
    setFixedHeight(lines * textEdit->lineHeight() + 10);

    int width = max(12, maxLength) * textEdit->fontWidth() + 10;
    if (verticalScrollBar()->isVisible())
        width += verticalScrollBar()->width();
    setFixedWidth(width);

    if (x + width > textEdit->width())
        move(textEdit->width() - width, y);
    if (y + height() > textEdit->height())
        move(x, y - height() - textEdit->lineHeight());
}

void AutoComplete::hide() {
    QListWidget::clear();
    QListWidget::hide();
}

void AutoComplete::insert() {
    TextEdit *textEdit = (TextEdit *)parent();
    QString insertText = currentItem()->text();
    QString word = textEdit->typingWord();

    ignore = true;
    for (int i = 0; i < word.count(); ++i)
        textEdit->textCursor().deletePreviousChar();
    textEdit->insertPlainText(insertText);
    if (insertText.contains("#include"))
        textEdit->moveCursor(QTextCursor::PreviousCharacter);
    ignore = false;
    hide();
}

void AutoComplete::chooseNext() {
    if (currentRow() < count() - 1)
        setCurrentRow(currentRow() + 1);
    else
        setCurrentRow(0);
}

void AutoComplete::choosePrevious() {
    if (currentRow() > 0)
        setCurrentRow(currentRow() - 1);
    else
        setCurrentRow(count() - 1);
}

void AutoComplete::setup() {
    setFont(QFont("Source Code Pro", 15));
    setStyleSheet("border: 1px solid gray;"
                  "background-color: #fff8f2;");

    reservedWords << "char"
                  << "class"
                  << "const"
                  << "double"
                  << "enum"
                  << "explicit"
                  << "friend"
                  << "inline"
                  << "int"
                  << "long"
                  << "namespace"
                  << "operator"
                  << "private"
                  << "protected"
                  << "public"
                  << "short"
                  << "signals"
                  << "signed"
                  << "slots"
                  << "static"
                  << "struct"
                  << "template"
                  << "typedef"
                  << "typename"
                  << "union"
                  << "unsigned"
                  << "virtual"
                  << "void"
                  << "volatile"
                  << "bool"
                  << "using"
                  << "constexpr"
                  << "sizeof"
                  << "if"
                  << "for"
                  << "foreach"
                  << "while"
                  << "do"
                  << "case"
                  << "break"
                  << "continue"
                  << "template"
                  << "delete"
                  << "new"
                  << "default"
                  << "try"
                  << "return"
                  << "throw"
                  << "catch"
                  << "goto"
                  << "else"
                  << "extren"
                  << "this"
                  << "switch"
                  << "#include <>"
                  << "#include \"\""
                  << "#define"
                  << "iostream"
                  << "main";
}
