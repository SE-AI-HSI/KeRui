#pragma comment(lib, "User32.lib")
#include <QApplication>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>

#include "AnalyzerClient.h"
#include "LocalDB.h"
#include "SettingWidget.h"
#include "Windows.h"

Sender *JsonPaser::sender = new Sender;

int main(int argc, char *argv[]) {

    AllocConsole();
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    HWND hWnd = GetDesktopWindow();
    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    // 获取监视器逻辑宽度与高度
    MONITORINFOEX miex;
    miex.cbSize = sizeof(miex);
    GetMonitorInfo(hMonitor, &miex);
    int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
    int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

    // qDebug() << cxLogical << cyLogical;

    // 获取监视器物理宽度与高度
    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
    int cxPhysical = dm.dmPelsWidth;
    int cyPhysical = dm.dmPelsHeight;

    // qDebug() << cxPhysical << cyPhysical;
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //    if (cxPhysical > 1920 && cyPhysical > 1080)
    //        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    // 不知道为什么这个函数我找不到标识符？只能注释掉了

    QFont defaultFont;
    defaultFont.setFamily("Microsoft YaHei");
    defaultFont.setPointSize(11);

    QApplication a(argc, argv);

    QFontDatabase::addApplicationFont("./Source Code Pro.ttf");

    a.setFont(defaultFont);

    AnalyzerClient w;
    // w.setStyleSheet(SettingWidget::getGlobalFontQSS());
    w.updateCoin(true);
    w.show();

    // 危险的做法, 绑定的对象(在这里是w)必须存活, w是窗口对象所以满足
    auto callback = std::bind(&AnalyzerClient::updateCoin, &w, std::placeholders::_1);

    if (DBManager::initDB(callback)) {  // 传给RemoteDB作为回调函数更新图标, 返回true表示RemoteDB可用
        std::thread pushT(&DBManager::pushLocalDataToRemote);
        pushT.detach();
    }

    std::thread t([&w]() {
        boost::asio::io_context io_context;
        AnalyzerClient::server = new Server(io_context, 12345);
        qRegisterMetaType<QStringList>("QStringList");
        w.connectMessageSender();
        io_context.run();
    });

    t.detach();
    return a.exec();
}
