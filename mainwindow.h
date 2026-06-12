// 文件说明：声明 Qt 调试主窗口。
// MainWindow 是前端，只负责显示日志、展示商品列表和发送调试命令，不直接创建硬件或执行任务。
#pragma once

#include <QMainWindow>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    // 构造调试窗口，并在构造过程中初始化后端 Controller。
    explicit MainWindow(QWidget* parent = nullptr);
    // 释放 Qt Designer 生成的 ui 对象。
    ~MainWindow() override;

private slots:
    // “发送调试订单”按钮回调：读取当前商品并提交给 Controller。
    void onSendDebugOrderClicked();
    // 把后端日志追加到 QTextEdit 日志框。
    void appendLog(const QString& message);
    // Controller 状态变化后更新窗口状态栏。
    void onControllerStateChanged(const QString& state);

private:
    // 解析配置目录：优先程序目录，其次当前目录，再其次源码目录，最后资源文件。
    QString resolveConfigDir() const;

    Ui::MainWindow* ui = nullptr; // Qt Designer 生成的界面对象。
};
