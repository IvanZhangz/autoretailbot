// 文件说明：声明语音服务抽象接口。
// 真实语音软件通过 ROS2 topic 与机器人控制系统交互，Fake 实现只保留队列和日志。
#pragma once

#include "model/RobotTypes.h"

#include <QObject>
#include <QQueue>


namespace asd_retail
{
class VoiceService : public QObject
{
    Q_OBJECT
public:
    // 创建语音服务对象；真正的网络/SDK 初始化放在 start()。
    explicit VoiceService(QObject* parent = nullptr);
    ~VoiceService() override = default;

    // 启动语音服务：ROS2 实现会创建节点、订阅订单 topic、发布状态/结果 topic。
    virtual bool Start(const InterfaceConfig& config) = 0;
    // 周期轮询语音服务：ROS2 实现会在这里 spin_some 并处理消息。
    virtual void Loop() = 0;
    // 是否有待 Controller 取走的语音订单。
    bool HasOrder() const;
    // 取出队头语音订单；调用前应先 HasOrder()。
    Order TakeOrder();
    // 向语音端上报机器人状态。
    virtual void ReportRobotState(const QString& state) = 0;
    // 向语音端上报任务结果。
    virtual void ReportTaskResult(const TaskResult& result) = 0;

signals:
    // 语音服务日志，由 Controller 转发到前端。
    void LogMessage(const QString& message);

protected:
    // 子类收到 ROS2 语音订单/意图后调用该函数，把统一 Order 放入 Controller 可轮询队列。
    void EnqueueOrder(const Order& order);

    bool m_running = false; // Start() 成功后为 true。
    QQueue<Order> m_orders; // 语音识别或意图解析生成的订单队列。
};

} // namespace asd_retail
