// 文件说明：声明 Qt 调试主窗口。
// MainWindow 是前端，只负责显示日志、展示商品列表和发送调试命令，不直接创建硬件或执行任务。
#pragma once

#include <QMainWindow>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSendDebugOrderClicked();
    void appendLog(const QString& message);
    void onControllerStateChanged(const QString& state);

private:
    QString resolveConfigDir() const;

    Ui::MainWindow* ui = nullptr;
};
