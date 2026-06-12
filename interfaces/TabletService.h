// 文件说明：声明平板服务接口。
// 服务自己 start 并在 loop 中监听/处理输入，Controller 只轮询并取走待处理订单，不使用订单回调。
#pragma once

#include "model/RobotTypes.h"

#include <QObject>
#include <QQueue>

class TabletService : public QObject
{
    Q_OBJECT
public:
    // 创建平板服务对象；真正的网络/SDK 初始化放在 start()。
    explicit TabletService(QObject* parent = nullptr);

    // 启动平板服务，读取配置中的 endpoint。
    bool start(const InterfaceConfig& config);
    // 周期轮询平板服务；真实实现会在这里处理输入事件。
    void loop();
    // 是否有待 Controller 取走的订单。
    bool hasOrder() const;
    // 取出队头订单；调用前应先 hasOrder()。
    Order takeOrder();
    // 前端调试专用：把按钮生成的订单塞进平板队列。
    void enqueueDebugOrder(const Order& order);
    // 向平板端上报机器人状态。
    void reportRobotState(const QString& state);
    // 向平板端上报任务结果。
    void reportTaskResult(const TaskResult& result);

signals:
    // 平板服务日志，由 Controller 转发到前端。
    void logMessage(const QString& message);

private:
    QString m_endpoint;       // 平板服务端点/监听地址。
    bool m_running = false;   // start() 成功后为 true。
    QQueue<Order> m_orders;   // 平板识别或接收到的订单队列。
};
