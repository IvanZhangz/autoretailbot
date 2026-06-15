// 文件说明：实现 Fake 平板服务。
#include "FakeTabletService.h"


namespace asd_retail
{
FakeTabletService::FakeTabletService(QObject* parent)
    : TabletService(parent)
{
}

bool FakeTabletService::Start(const InterfaceConfig& config)
{
    m_grpc_listen = config.m_tablet.m_grpc_listen;
    m_running = true;
    emit LogMessage("Fake 平板服务启动，模拟 gRPC 监听: " + m_grpc_listen);
    return true;
}

void FakeTabletService::Loop()
{
    if (!m_running) return;
    // Fake 平板没有网络事件；真实 GrpcTabletService 会在 Loop 中处理异步请求/事件队列。
}

void FakeTabletService::ReportRobotState(const QString& state)
{
    emit LogMessage("Fake 平板状态上报: " + state);
}

void FakeTabletService::ReportTaskResult(const TaskResult& result)
{
    emit LogMessage(QString("Fake 平板订单结果: order=%1 success=%2 message=%3")
                        .arg(result.m_order_id, result.m_success ? "true" : "false", result.m_message));
}

} // namespace asd_retail
