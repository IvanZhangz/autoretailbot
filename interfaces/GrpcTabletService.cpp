// 文件说明：实现 gRPC 平板服务骨架。
#include "GrpcTabletService.h"


namespace asd_retail
{
GrpcTabletService::GrpcTabletService(QObject* parent)
    : TabletService(parent)
{
}

bool GrpcTabletService::Start(const InterfaceConfig& config)
{
    m_grpc_listen = config.m_tablet.m_grpc_listen;
    m_running = true;
    // TODO: 引入 grpcpp 后，在这里创建 ServerBuilder、绑定 m_grpc_listen 并启动 RetailRobotControl 服务。
    emit LogMessage("gRPC 平板服务骨架启动，监听地址: " + m_grpc_listen);
    return true;
}

void GrpcTabletService::Loop()
{
    if (!m_running) return;
    // TODO: 处理 gRPC CompletionQueue；收到 SubmitOrder 后转换成 Order 并调用 EnqueueOrder(order)。
}

void GrpcTabletService::ReportRobotState(const QString& state)
{
    m_last_state = state;
    // TODO: 如果实现服务端流，在这里把状态变化写入平板事件流。
    emit LogMessage("gRPC 平板状态缓存: " + state);
}

void GrpcTabletService::ReportTaskResult(const TaskResult& result)
{
    // TODO: 将 TaskResult 转成 proto 消息并推送/缓存给平板客户端。
    emit LogMessage(QString("gRPC 平板任务结果骨架: order=%1 success=%2 message=%3")
                        .arg(result.m_order_id, result.m_success ? "true" : "false", result.m_message));
}

} // namespace asd_retail
