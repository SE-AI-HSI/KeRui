#include "LabelManager.h"
#include "AnalyzeThread.h"
#include "FileManager.h"
#include "GraphicsEffectUtil.h"
#include "TextEdit.h"
#include "UrlInterpreter.h"
#include <QCoreApplication>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <algorithm>
#include <cmath>
#include <map>

int LabelManager::viewportWidth = 0;
int LabelManager::viewportHeight = 0;
int LabelManager::flags = 0b1111;
bool LabelManager::errorFlag = 1;
QJsonObject LabelManager::rootJson;

#define Qf(s) QString::fromLocal8Bit(s)
#define Qn(i) QString::number(i)
#define Endl "<br>" // html格式换行符,用\n会导致超链接失效//

inline bool compare_function(const AnalysisLabel *a, const AnalysisLabel *b) {
    if (a->data->row == b->data->row) {
        return a->data->ltype < b->data->ltype;
    }
    return a->data->row < b->data->row;
}

LabelManager::LabelManager(QWidget *parent, QWidget *_labelParent) : QWidget{parent}, labelParent(_labelParent) {
    timer = new QTimer(this);
    viewportWidth = parent->width();
    viewportHeight = parent->height();

    setupStyle();
    connectEvents();
}

LabelManager::~LabelManager() {
    this->disconnect(); // 断开与其他对象的所有信号连接//

    for (AnalysisLabel *&label : labelList) {
        delete label;
    }
    labelList.clear();
}

void LabelManager::initLabels() {
    if (!labelList.empty()) {
        std::map<int, int> warning_cnt;
        std::map<int, int> warning_total;
        std::map<int, int> error_cnt;
        std::map<int, int> error_total;
        std::map<int, int> clone_cnt;
        std::map<int, int> clone_total;

        for (AnalysisLabel *&label : labelList) {
            int block = label->data->row / labelBlockSize;
            if (label->data->ltype == LabelType::Warning) {
                if (warning_total.find(block) == warning_total.end()) {
                    warning_total[block] = 1;
                    warning_cnt[block] = 1;
                } else {
                    warning_total[block] += 1;
                }
            } else if (label->data->ltype == LabelType::Error) {
                if (error_total.find(block) == error_total.end()) {
                    error_total[block] = 1;
                    error_cnt[block] = 1;
                } else {
                    error_total[block] += 1;
                }
            } else {
                if (clone_total.find(block) == clone_total.end()) {
                    clone_total[block] = 1;
                    clone_cnt[block] = 1;
                } else {
                    clone_total[block] += 1;
                }
            }
        }
        for (AnalysisLabel *&label : labelList) {
            label->show();
            label->raise();
            label->setGraphicsEffect(GraphicsEffectUtil::newShadow(this, 96));
            while (label->verticalScrollBar()->isVisible()) {
                label->setFixedHeight(label->height() + 5);
            }
            label->setFixedHeight(label->height() + 30);
            bool flag = false;
            int block = label->data->row / labelBlockSize;

            if (label->data->ltype == LabelType::Warning) {
                label->setupBottomBar(warning_cnt[block], warning_total[block]);
                if (warning_cnt[block] == 1)
                    flag = true;
                warning_cnt[block] += 1;
            } else if (label->data->ltype == LabelType::Error) {
                label->setupBottomBar(error_cnt[block], error_total[block]);
                if (error_cnt[block] == 1)
                    flag = true;
                error_cnt[block] += 1;
            } else {
                label->setupBottomBar(clone_cnt[block], clone_total[block]);
                if (clone_cnt[block] == 1)
                    flag = true;
                clone_cnt[block] += 1;
            }

            int maxY = height() - label->height() - 20;
            int y = (block * labelBlockSize + labelBlockSize / 4 + int(label->data->ltype) * (labelBlockSize / 4)) *
                    TextEdit::lineHeight();
            label->move(viewportWidth - label->width() - 70 + int(label->data->ltype) * 20, y > maxY ? maxY : y);
            if (!flag) {
                label->hide();
                continue;
            }
            if ((label->data->ltype == Error) && (LabelManager::errorFlag == 0)) {
                label->hide();
                continue;
            }
            if (!(label->data->atype & LabelManager::flags)) {
                label->hide();
                continue;
            }
        }
        update();
    }
}

void LabelManager::updateLabels() {
    std::map<int, int> warning_cnt;
    std::map<int, int> warning_total;
    std::map<int, int> error_cnt;
    std::map<int, int> error_total;
    std::map<int, int> clone_cnt;
    std::map<int, int> clone_total;

    for (AnalysisLabel *&label : labelList) {
        int block = label->data->row / labelBlockSize;
        if (label->data->ltype == LabelType::Warning) {
            if (warning_total.find(block) == warning_total.end()) {
                warning_total[block] = 1;
                warning_cnt[block] = 1;
            } else {
                warning_total[block] += 1;
            }
        } else if (label->data->ltype == LabelType::Error) {
            if (error_total.find(block) == error_total.end()) {
                error_total[block] = 1;
                error_cnt[block] = 1;
            } else {
                error_total[block] += 1;
            }
        } else {
            if (clone_total.find(block) == clone_total.end()) {
                clone_total[block] = 1;
                clone_cnt[block] = 1;
            } else {
                clone_total[block] += 1;
            }
        }
    }

    for (AnalysisLabel *&label : labelList) {
        label->show();
        label->raise();

        bool flag = false;
        int block = label->data->row / labelBlockSize;

        if (label->data->ltype == LabelType::Warning) {
            if (warning_cnt[block] == 1)
                flag = true;
            warning_cnt[block] += 1;
        } else if (label->data->ltype == LabelType::Error) {
            if (error_cnt[block] == 1)
                flag = true;
            error_cnt[block] += 1;
        } else {
            if (clone_cnt[block] == 1)
                flag = true;
            clone_cnt[block] += 1;
        }

        int maxY = height() - label->height() - 20;
        int y = (block * labelBlockSize + labelBlockSize / 4 + int(label->data->ltype) * (labelBlockSize / 4)) *
                TextEdit::lineHeight();
        label->move(viewportWidth - label->width() - 70 + int(label->data->ltype) * 20, y > maxY ? maxY : y);

        if (!flag) {
            label->hide();
            continue;
        }
        if ((label->data->ltype == Error) && (LabelManager::errorFlag == 0)) {
            label->hide();
            continue;
        }
        if (!(label->data->atype & LabelManager::flags)) {
            label->hide();
            continue;
        }
    }
    update();
}

void LabelManager::highlight(int row, int lineCounter = 1) {
    highlightAlpha = 250;
    highlightLine = row;
    highlightLineCounter = lineCounter;
    timer->start(20);
}

AnalysisLabel *const LabelManager::jumpLabel(QObject *const source, bool next, int specific) const {
    for (int i = 0; i < labelList.size(); ++i) {
        if (labelList[i] == source) {
            int block = labelList[i]->data->row / labelBlockSize;
            if (specific > 0) {
                int cnt = 0;
                for (AnalysisLabel *label : labelList) {
                    if (label->data->ltype == labelList[i]->data->ltype && label->data->row / labelBlockSize == block)
                        ++cnt;
                    if (cnt == specific) {
                        label->spinBox->setValue(specific);
                        label->move(labelList[i]->pos());
                        labelList[i]->hide();
                        label->show();
                        label->raise();
                        return label;
                    }
                }
            } else if (next) {
                for (int j = i + 1; j < labelList.size(); ++j) {
                    if (labelList[i]->data->ltype == labelList[j]->data->ltype &&
                        labelList[j]->data->row / labelBlockSize == block) {
                        labelList[j]->move(labelList[i]->pos());
                        labelList[i]->hide();
                        labelList[j]->show();
                        labelList[j]->raise();
                        labelList[j]->spinBox->setValue(labelList[i]->spinBox->value() + (next ? 1 : -1));
                        return labelList[j];
                    }
                }
            } else {
                for (int j = i - 1; j >= 0; --j) {
                    if (labelList[i]->data->ltype == labelList[j]->data->ltype &&
                        labelList[j]->data->row / labelBlockSize == block) {
                        labelList[j]->move(labelList[i]->pos());
                        labelList[i]->hide();
                        labelList[j]->show();
                        labelList[j]->raise();
                        labelList[j]->spinBox->setValue(labelList[i]->spinBox->value() + (next ? 1 : -1));
                        return labelList[j];
                    }
                }
            }
            return labelList[i];
        }
    }
    return nullptr;
}

void LabelManager::warningInfo(const QList<IWarningInfo> &warningInfo) {
    // 传递进来全部的警告信息
    for (const IWarningInfo &winfo : warningInfo) {
        if (winfo.m_line > -1 && winfo.m_colum > -1) {
            QString text = Qf("【警告】等级: ") + Qn(winfo.m_warningLevel) + " " + Endl;
            text += Qf("警告代码: ");
            text += QString ::number(winfo.m_id);
            text += UrlInterpreter::urlOfMaskWarning(winfo.m_id);

            text += winfo.m_message;
            // if(rootJson.contains(QString::number(winfo.m_weCode.m_weCodeNum))){
            // text += Qf("警告代码: ");
            // text += QString ::number(winfo.m_weCode.m_weCodeNum);
            // text += rootJson.find(QString::number(winfo.m_weCode.m_weCodeNum)).value().toString() + Endl;
            // }
            // for(int i = 0; i < winfo.m_weCode.m_paramAmount; ++i){
            // switch(winfo.m_weCode.m_params[i].m_paramType){
            // case 0://string
            // text = text.arg(Qf(((std::string*)(winfo.m_weCode.m_params[i].m_paramValue))->c_str()));
            // qDebug() << Qf(((std::string*)(winfo.m_weCode.m_params[i].m_paramValue))->c_str());
            // break;
            // case 1://int
            // text = text.arg(QString::number((int)(winfo.m_weCode.m_params[i].m_paramValue)));

            // break;
            // case 2://symbol
            // text = text.arg(Qf(((Symbol*)(winfo.m_weCode.m_params[i].m_paramValue))->m_token->m_lexeme.c_str()));
            // qDebug() << Qf(((Symbol*)(winfo.m_weCode.m_params[i].m_paramValue))->m_token->m_lexeme.c_str());
            // break;
            // case 3://token
            // text = text.arg(Qf(((Token*)(winfo.m_weCode.m_params[i].m_paramValue))->m_lexeme.c_str()));
            // qDebug() << Qf(((Token*)(winfo.m_weCode.m_params[i].m_paramValue))->m_lexeme.c_str());
            // break;
            // }
            // }
            text += UrlInterpreter::urlOfJumpToRowColumn(winfo.m_line, winfo.m_colum);

            if (winfo.m_warningtype & EFFICIENCY) {
                text += Qf("[执行效率]");
            }
            if (winfo.m_warningtype & NORMATIVE) {
                text += Qf("[规范性]");
            }
            if (winfo.m_warningtype & MAINTAINABILITY) {
                text += Qf("[可维护性]");
            }
            if (winfo.m_warningtype & SECURITY) {
                text += Qf("[安全性]");
            }

            // if (winfo.m_commentmessage.length() > 1) {
            // text += Endl + Qf("【建议】") + Qf(winfo.m_commentmessage.c_str());
            // }

            if (winfo.m_calScore) {
                text += Qf("此类警告计入得分，是否屏蔽此类警告？"); // 后面加一个按钮，屏蔽警告
            }

            LabelData *data =
                new LabelData(LabelType::Warning, winfo.m_warningtype, winfo.m_line, winfo.m_colum, text, nullptr
                              // winfo.m_weCode.m_weCodeNum
                );

            labelList.emplace_back(new AnalysisLabel(labelParent, this, data));
        }
    }
}

void LabelManager::errorInfo(const QList<IErrorInfo> &errorInfo) {
    for (const IErrorInfo &einfo : errorInfo) {
        if (einfo.m_line > -1 && einfo.m_colum > -1) {
            QString text = Qf("【错误】");
            text += UrlInterpreter::urlOfJumpToRowColumn(einfo.m_line, einfo.m_colum) + Endl;
            text += Qf("错误代码: ");
            text += QString ::number(einfo.m_id) + Endl;
            text += einfo.m_message;
            // if(rootJson.contains(QString::number(einfo.m_weCode.m_weCodeNum))){
            // text += Qf("错误代码: ");
            // text += QString ::number(einfo.m_weCode.m_weCodeNum) + Endl;
            // text += rootJson.find(QString::number(einfo.m_weCode.m_weCodeNum)).value().toString();
            // }
            // for(int i = 0; i < einfo.m_weCode.m_paramAmount; ++i){
            // switch(einfo.m_weCode.m_params[i].m_paramType){
            // case 0://string
            // text = text.arg(Qf(((std::string*)(einfo.m_weCode.m_params[i].m_paramValue))->c_str()));
            // qDebug() << Qf(((std::string*)(einfo.m_weCode.m_params[i].m_paramValue))->c_str());
            // break;
            // case 1://int
            // text = text.arg(QString::number((int)(einfo.m_weCode.m_params[i].m_paramValue)));
            // break;
            // case 2://symbol
            // text = text.arg(Qf(((Symbol*)(einfo.m_weCode.m_params[i].m_paramValue))->m_token->m_lexeme.c_str()));
            // qDebug() << Qf(((Symbol*)(einfo.m_weCode.m_params[i].m_paramValue))->m_token->m_lexeme.c_str());
            // break;
            // case 3://token
            // text = text.arg(Qf(((Token*)(einfo.m_weCode.m_params[i].m_paramValue))->m_lexeme.c_str()));
            // qDebug() << Qf(((Token*)(einfo.m_weCode.m_params[i].m_paramValue))->m_lexeme.c_str());
            // break;
            // }
            // }
            LabelData *data = new LabelData(LabelType::Error,
                                            0b1111, // error都需要显示//
                                            einfo.m_line, einfo.m_colum, text);

            labelList.emplace_back(new AnalysisLabel(labelParent, this, data));
        }
    }
}

void LabelManager::cloneInfo(const QVector<QPair<int, QList<QList<ClonePosition>>>> &cloneSets, QString filePath,
                             QString fileRelativelyPath) { // 克隆信号传参
    // qDebug() << "cloneInfo-filePath:" <<filePath<<"cloneInfo-fileRelativelyPath:"<<fileRelativelyPath;
    // filePath.replace("/","\\");
    // fileRelativelyPath.replace("/","\\");
    QString cutPath = filePath.left(filePath.size() - fileRelativelyPath.size());
    // qDebug() << "remove cloneInfo-filePath:" <<findPath;
    for (const QPair<int, QList<QList<ClonePosition>>> &cloneSet : cloneSets) { // 循环每一个克隆集合
        for (int i = 0; i < cloneSet.second.size(); ++i) { // 循环每一个克隆集合中 不同文件的克隆集合
            QString cloneFile = FileManager::id2filePath[cloneSet.second[i][0].m_file];
            if (cloneFile == filePath) { // 有本文件的重复块
                // 获取这个重复块集合的全部文件名
                QStringList files;
                for (int f = 0; f < cloneSet.second.size(); ++f) {
                    QString fileName = FileManager::id2filePath[cloneSet.second[f][0].m_file];
                    qDebug() << __FILE__ << ":" << __LINE__ << fileName;
                    fileName.remove(cutPath);
                    fileName = fileName.mid(1, fileName.size() - 1);
                    files.append(fileName);
                }

                // 本文件的每一个重复位置都要构建标签
                for (int j = 0; j < cloneSet.second[i].size(); ++j) {
                    QString text = Qf("【重复】重复的代码块 ") + Endl;

                    // 这个块的位置//
                    text += Qf("开始位置 ") + UrlInterpreter::urlOfJumpToBlockBegin(cloneSet.second[i][j].m_beginLine,
                                                                                    cloneSet.second[i][j].m_beginColumn,
                                                                                    cloneSet.second[i][j].m_endLine,
                                                                                    cloneSet.second[i][j].m_endColumn);
                    text += Qf("结束位置 ") + UrlInterpreter::urlOfJumpToBlockEnd(cloneSet.second[i][j].m_beginLine,
                                                                                  cloneSet.second[i][j].m_beginColumn,
                                                                                  cloneSet.second[i][j].m_endLine,
                                                                                  cloneSet.second[i][j].m_endColumn);

                    text += Qf("[执行效率]") + Qf("[可维护性]") + Endl;
                    text += Qf(" 所在文件名:") + Endl;

                    for (int i = 0; i < cloneSet.second.size(); ++i) {
                        QString f_path = FileManager::id2filePath[cloneSet.second[i][0].m_file];
                        if (f_path == filePath)
                            continue;
                        text += UrlInterpreter::urlOfJumpFile(f_path);
                    }

                    if (cloneSet.first == 1) {
                        text += Qf("【重复等级: 普通】") + Endl;
                    } else {
                        text += Qf("【重复等级: 高】") + Endl;
                    }

                    // 上一个重复块位置//
                    if (j - 1 >= 0) {
                        text += Qf("上一个重复的代码块位于");
                        text += UrlInterpreter::urlOfJumpToBlockBegin(
                            cloneSet.second[i][j - 1].m_beginLine, cloneSet.second[i][j - 1].m_beginColumn,
                            cloneSet.second[i][j - 1].m_endLine, cloneSet.second[i][j - 1].m_endColumn);
                    } else {
                        if (i - 1 >= 0) { // 链接到上一个文件的最后一个位置
                            text += Qf("上一个重复的代码块位于文件 ") + files[i - 1] + Qf(" 中");
                            // text += linkOfAnotherBlock(cloneSet.second[i - 1].back().m_beginLine, cloneSet.second[i -
                            // 1].back().m_beginColumn, cloneSet.second[i - 1].back().m_endLine, cloneSet.second[i -
                            // 1].back().m_endColumn);
                            text += Endl;
                        }
                    }
                    // 下一个重复块位置//
                    if (j + 1 < cloneSet.second[i].size()) {
                        text += Qf("下一个重复的代码块位于");
                        text += UrlInterpreter::urlOfJumpToBlockBegin(
                            cloneSet.second[i][j + 1].m_beginLine, cloneSet.second[i][j + 1].m_beginColumn,
                            cloneSet.second[i][j + 1].m_endLine, cloneSet.second[i][j + 1].m_endColumn);
                    } else {
                        if (i + 1 < cloneSet.second.size()) {
                            text += Qf("下一个重复的代码块位于文件 ") + files[i + 1] + Qf(" 中");
                            // text += linkOfAnotherBlock(cloneSet.second[i + 1].back().m_beginLine, cloneSet.second[i +
                            // 1].back().m_beginColumn, cloneSet.second[i + 1].back().m_endLine, cloneSet.second[i +
                            // 1].back().m_endColumn);
                        }
                    }

                    // 下面是点击优化重复块的
                    text += UrlInterpreter::urlOfOptimizeClone(cloneSet.second[i][j].m_beginLine,
                                                               cloneSet.second[i][j].m_beginColumn);

                    LabelData *data = new LabelData(LabelType::Clone, 0b0110, cloneSet.second[i][j].m_beginLine,
                                                    cloneSet.second[i][j].m_beginColumn, text);

                    AnalysisLabel *label = new AnalysisLabel(labelParent, this, data);
                    label->hide();
                    labelList.emplace_back(label);
                }
                break;
            }
        }
    }
}

void LabelManager::sortAndShowLabels() {
    std::sort(labelList.begin(), labelList.end(), &compare_function); // 0s
    initLabels();                                                     // 3.599s
}

void LabelManager::setupStyle() {
    pen.setWidth(2);
    pen.setColor(QColor(255, 172, 112));

    brush.setStyle(Qt::SolidPattern);
}

void LabelManager::clearLabels() {
    for (AnalysisLabel *&label : labelList) {
        delete label;
    }
    labelList.clear();

    // labelInitialized = false;
}

void LabelManager::connectEvents() {
    /** timer实现某行的闪烁高亮 **/
    connect(timer, &QTimer::timeout, [=] {
        highlightAlpha -= 10;
        update();
        if (highlightAlpha == 0) {
            timer->stop();
        }
    });
}

void LabelManager::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setPen(pen);
    painter.setBrush(QColor(255, 172, 112));
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (AnalysisLabel *&label : labelList) {
        if (label->isVisible()) {
            int sx = label->data->column * TextEdit::fontWidth() + 5,
                sy = label->data->row * TextEdit::lineHeight() - TextEdit::lineHeight() / 2 + 3, ex = label->x(),
                ey = label->y() + label->height() / 2;
            painter.drawLine(sx, sy, ex - 5, ey);
            painter.drawEllipse(QPointF(sx, sy), 5, 5);
            painter.drawEllipse(QPointF(ex - 5, ey), 5, 5);
        }
    }
    if (highlightAlpha) {
        painter.setPen(Qt::NoPen);
        brush.setColor(QColor(255, 172, 112, highlightAlpha));
        painter.setBrush(brush);
        painter.drawRect(0, (highlightLine - 1) * TextEdit::lineHeight(), this->width(),
                         TextEdit::lineHeight() * highlightLineCounter);
    }
}

void LabelManager::resizeEvent(QResizeEvent *event) {
    int delta = width() - event->oldSize().width();
    for (AnalysisLabel *&label : labelList) {
        label->move(label->x() + delta, label->y());
    }
    viewportWidth = static_cast<QWidget *>(parent())->width();
    viewportHeight = static_cast<QWidget *>(parent())->height();
}

void LabelManager::initInfo() {
    // 把外部定义的文本读进来
    // qDebug()<<QCoreApplication::applicationDirPath() + "/analyseMessage.json";
    QPair<QString, QString> text = FileManager::read(QCoreApplication::applicationDirPath() + "/analyseMessage.json");
    // QJsonParseError类用于在JSON解析期间报告错误。
    QJsonParseError jsonError;
    // 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
    // 如果解析成功，返回QJsonDocument对象，否则返回null
    QJsonDocument doc = QJsonDocument::fromJson(text.second.toUtf8(), &jsonError);
    // 判断是否解析失败
    if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        qDebug() << "Json格式错误！" << jsonError.error;
        return;
    }
    LabelManager::rootJson = doc.object();
}
