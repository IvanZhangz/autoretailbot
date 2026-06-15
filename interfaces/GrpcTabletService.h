// 文件说明：声明 gRPC 平板服务骨架。
// 当前不引入 grpcpp 依赖，只保留可编译 TODO；真实接入时在此实现 gRPC Server。
#pragma once

#include "interfaces/TabletService.h"


namespace asd_retail
{
class GrpcTabletService : public TabletService
{
public:
    explicit GrpcTabletService(QObject* parent = nullptr);

    // TODO: 启动 gRPC Server，监听 config.m_tablet.m_grpc_listen，注册 SubmitOrder/GetState/TaskEvent 服务。
    bool Start(const InterfaceConfig& config) override;
    // TODO: 轮询或处理 gRPC 异步完成队列，把 SubmitOrder 请求转换成 Order 后 EnqueueOrder()。
    void Loop() override;
    // TODO: 缓存机器人状态，供平板 GetState 或服务端流读取。
    void ReportRobotState(const QString& state) override;
    // TODO: 将任务结果写入 gRPC 事件队列或推送给订阅中的平板客户端。
    void ReportTaskResult(const TaskResult& result) override;

private:
    QString m_grpc_listen; // gRPC 监听地址，例如 0.0.0.0:18080。
    QString m_last_state;  // 最近一次机器人状态，真实 gRPC 查询时返回。
};

} // namespace asd_retail
