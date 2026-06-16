// 文件说明：声明 Fake 平板服务。
// Fake 不启动真实 gRPC，只保留订单队列和日志，方便 Qt 调试界面模拟平板下单。
#pragma once

#include "TabletService.h"


namespace asd_retail
{
class FakeTabletService : public TabletService
{
public:
    explicit FakeTabletService(QObject* parent = nullptr);

    // Fake 启动时只记录 gRPC 监听地址，便于日志确认配置已加载。
    bool Start(const InterfaceConfig& config) override;
    // Fake 没有外部网络事件，Loop 保留为空实现。
    void Loop() override;
    // Fake 状态上报只打印日志。
    void ReportRobotState(const QString& state) override;
    // Fake 任务结果上报只打印日志。
    void ReportTaskResult(const TaskResult& result) override;

private:
    QString m_grpc_listen; // 配置中的平板 gRPC 监听地址。
};

} // namespace asd_retail
