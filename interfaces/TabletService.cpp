// 文件说明：实现平板服务占位逻辑。
// 真实接入时 start 中启动网络服务，loop 中轮询 socket/ROS/IPC 并把新订单写入队列。
#include "TabletService.h"

TabletService::TabletService(QObject* parent)
    : QObject(parent)
{
}

bool TabletService::start(const InterfaceConfig& config)
{
    m_endpoint = config.tabletEndpoint;
    m_running = true;
    emit logMessage("平板服务启动: " + m_endpoint);
    return true;
}

void TabletService::loop()
{
    if (!m_running) return;
    // Fake 服务暂无外部监听，保留 loop 入口给真实协议实现。
}

bool TabletService::hasOrder() const
{
    return !m_orders.isEmpty();
}

Order TabletService::takeOrder()
{
    return m_orders.dequeue();
}

void TabletService::enqueueDebugOrder(const Order& order)
{
    m_orders.enqueue(order);
    emit logMessage("平板服务收到调试订单: " + order.orderId);
}

void TabletService::reportRobotState(const QString& state)
{
    emit logMessage("平板状态上报: " + state);
}

void TabletService::reportTaskResult(const TaskResult& result)
{
    emit logMessage(QString("平板订单结果: order=%1 success=%2 message=%3")
                        .arg(result.orderId, result.success ? "true" : "false", result.message));
}
