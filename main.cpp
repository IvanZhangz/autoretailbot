// 文件说明：Qt 程序入口。
// main 只创建 QApplication 和前端 MainWindow，然后进入 Qt 事件循环。
// RobotController 单例由 MainWindow 初始化，避免入口处和窗口处重复取单例造成误解。
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}
