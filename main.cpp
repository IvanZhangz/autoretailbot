// 文件说明：Qt 程序入口。
// main 只创建 QApplication、后端 RobotController 单例和前端 MainWindow，然后进入 Qt 事件循环。
#include "controller/RobotController.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 后端控制器单例在 main 中显式创建，避免把机器人启动逻辑放进 MainWindow。
    RobotController& controller = RobotController::instance();
    Q_UNUSED(controller);

    MainWindow w;
    w.show();

    return app.exec();
}
