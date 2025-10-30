#ifndef SERVEREX_H
#define SERVEREX_H

#include <QDebug>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <memory>
#include <string>

#include "InfoStruct.h"
using boost::asio::ip::tcp;

class Sender : public QObject {
    Q_OBJECT
public:
signals:
    void errorInfoSending(IErrorInfo);
    void warningInfoSending(IWarningInfo);
    void cloneSetInfoSending(CloneSetInfo);
    void globalInfoSending(GlobalInfo);
    void analyzeStepInfoSending(AnalyzeStep);
    void clonePositionSending(ClonePosition);
    void codeGenerateInfoSending(CodeGenerateInfo);
    void codeNamingInfoSending(CodeNamingInfo);
    void fileTableSending(FileTable);
};

class JsonPaser : public QObject {
    Q_OBJECT
public:
    JsonPaser() {
        opt.allow_invalid_utf8 = true;
    }

    void jsonPase(QString &message) {
        try {
            val = boost::json::parse(message.toStdString(), {}, opt);
            obj = val.as_object();
            //            qDebug() << __FILE__ << ":" << __LINE__ << QString::fromUtf8(obj["msg"]);
            QString type = QString::fromLocal8Bit(obj["TYPE"].as_string().c_str());
            if (type == "IErrorInfo") {
                boost::json::value_to_tag<IErrorInfo> tag;
                emit sender->errorInfoSending(tag_invoke(tag, val));
            } else {
                if (type == "IWarningInfo") {
                    boost::json::value_to_tag<IWarningInfo> tag;
                    emit sender->warningInfoSending(tag_invoke(tag, val));
                } else {
                    if (type == "CloneSetInfo") {
                        qDebug() << "ClonePosition + 1 !!!";
                        boost::json::value_to_tag<CloneSetInfo> tag;
                        emit sender->cloneSetInfoSending(tag_invoke(tag, val));
                    } else {
                        if (type == "GlobalInfo") {
                            boost::json::value_to_tag<GlobalInfo> tag;
                            emit sender->globalInfoSending(tag_invoke(tag, val));
                        } else {
                            if (type == "AnalyzeStep") {
                                boost::json::value_to_tag<AnalyzeStep> tag;
                                emit sender->analyzeStepInfoSending(tag_invoke(tag, val));
                                                                                                                                } else {
                                if (type == "ClonePosition") {
                                    boost::json::value_to_tag<ClonePosition> tag;
                                    emit sender->clonePositionSending(tag_invoke(tag, val));
                                } else {
                                    if (type == "CodeGenerateInfo") {
                                        //                boost::json::value_to_tag<CodeGenerateInfo> tag;
                                        //                emit sender->codeGenerateInfoSending(tag_invoke(tag, val));
                                    } else {
                                        if (type == "FileTable") {
                                            boost::json::value_to_tag<FileTable> tag;
                                            emit sender->fileTableSending(tag_invoke(tag, val));
                                        } else {
                                            if (type == "CodeNamingInfo") {
                                                boost::json::value_to_tag<CodeNamingInfo> tag;
                                                emit sender->codeNamingInfoSending(tag_invoke(tag, val));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } catch (std::exception e) {
            qDebug() << "Parsing" << message << e.what();
        }
    }

    static Sender *sender;
    boost::json::parse_options opt;
    boost::json::object obj;
    boost::json::value val;
};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {
    }

    void Start() {
        //        qDebug() << "Session Started";
        DoRead();
    }

private:
    void DoRead() {
        auto self(shared_from_this());
        socket_.async_read_some(
            boost::asio::buffer(buffer_), [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string s1(buffer_.data(), length);
                    messageBuffer += s1;
                    // 这里调用处理函数
                    messagesDecode();

                    DoRead();
                } else {
                    qDebug() << __FILE__ << ":" << __LINE__ << QString::fromLocal8Bit(ec.message().c_str());
                }
            });
    }

    //    void DoWrite(std::size_t length) {
    //        auto self(shared_from_this());
    //        boost::asio::async_write(socket_, boost::asio::buffer(buffer_, length), [this,
    //        self](boost::system::error_code ec, std::size_t length) {
    //            if (!ec) {
    //                DoWrite(length);
    //                qDebug() << "Write" << length << "bytes to client: "
    //                    << QString::fromStdString(std::string(buffer_.data(), length));
    //            }
    //            });
    //    }

#define PACKAGE_LENGTH_FIELD_LENGTH 10

    void messagesDecode() {
    START:
        if (nextPackageLen == -1) {
            if (messageBuffer.length() >= PACKAGE_LENGTH_FIELD_LENGTH) {
                nextPackageLen = QString::fromStdString(messageBuffer.substr(0, PACKAGE_LENGTH_FIELD_LENGTH)).toInt();
                messageBuffer.replace(0, PACKAGE_LENGTH_FIELD_LENGTH, "");
            }
        }
        if (nextPackageLen != -1 && messageBuffer.length() >= nextPackageLen) {
            QString message = QString::fromLocal8Bit(messageBuffer.substr(0, nextPackageLen).c_str());
            messageBuffer.replace(0, nextPackageLen, "");

            nextPackageLen = -1;
            // 这边json解析器解析，然后sender送走
            jsonPaser.jsonPase(message);
            goto START;
        }
    }

    tcp::socket socket_;
    std::array<char, 1024> buffer_;

    std::string messageBuffer;
    int bufferedLength = 0;
    int nextPackageLen = -1;  //-1表示还没读取到下一个json包的长度

    JsonPaser jsonPaser;
};

class Server {
public:
    Server(boost::asio::io_context &io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        DoAccept();
    }

    void DoAccept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto session = std::make_shared<Session>(std::move(socket));
                session->Start();
            }
            //            qDebug() << "Accept Success";
            DoAccept();
        });
    }

    tcp::acceptor acceptor_;
};

#endif
