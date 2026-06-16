// 文件说明：声明平板服务抽象接口。
// 真实平板通过 gRPC 与机器人控制系统交互；Fake 实现继续服务 Qt 调试按钮。
#pragma once

#include "RobotTypes.h"

#include <QObject>
#include <QQueue>


namespace asd_retail
{
class TabletService : public QObject
{
    Q_OBJECT
public:
    // 创建平板服务基类；parent 由 Controller 传入或保持 nullptr。
    explicit TabletService(QObject* parent = nullptr);
    ~TabletService() override = default;

    // 启动平板服务：Fake 只记录配置，gRPC 实现将在这里启动服务端。
    virtual bool Start(const InterfaceConfig& config) = 0;
    // 周期轮询平板服务：gRPC 实现可处理完成队列或异步事件。
    virtual void Loop() = 0;
    // 是否有待 Controller 取走的订单。
    bool HasOrder() const;
    // 取出队头订单；调用前应先 HasOrder()。
    Order TakeOrder();
    // 前端调试专用：把按钮生成的订单塞进平板队列，不代表真实 gRPC 协议。
    virtual void EnqueueDebugOrder(const Order& order);
    // 向平板端上报机器人状态；gRPC 可缓存状态供平板查询或推流。
    virtual void ReportRobotState(const QString& state) = 0;
    // 向平板端上报任务结果；gRPC 可写入事件队列或服务端流。
    virtual void ReportTaskResult(const TaskResult& result) = 0;

signals:
    // 平板服务日志，由 Controller 转发到前端。
    void LogMessage(const QString& message);

protected:
    // 子类收到订单后调用该函数，把统一 Order 放入 Controller 可轮询队列。
    void EnqueueOrder(const Order& order);

    bool m_running = false; // Start() 成功后为 true。
    QQueue<Order> m_orders; // 平板 gRPC 或调试入口生成的订单队列。
};

} // namespace asd_retail
