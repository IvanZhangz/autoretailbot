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
    explicit TabletService(QObject* parent = nullptr);

    bool start(const InterfaceConfig& config);
    void loop();
    bool hasOrder() const;
    Order takeOrder();
    void enqueueDebugOrder(const Order& order);
    void reportRobotState(const QString& state);
    void reportTaskResult(const TaskResult& result);

signals:
    void logMessage(const QString& message);

private:
    QString m_endpoint;
    bool m_running = false;
    QQueue<Order> m_orders;
};
