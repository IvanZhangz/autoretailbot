// 文件说明：声明 ROS2 语音服务骨架。
// 当前不引入 rclcpp 依赖，只保留可编译 TODO；真实接入时在此订阅语音订单并发布状态/结果。
#pragma once

#include "VoiceService.h"


namespace asd_retail
{
class Ros2VoiceService : public VoiceService
{
public:
    explicit Ros2VoiceService(QObject* parent = nullptr);

    // TODO: 创建 ROS2 node，订阅 m_voice.m_order_topic，发布 m_voice.m_state_topic 和 m_voice.m_task_result_topic。
    bool Start(const InterfaceConfig& config) override;
    // TODO: 调用 rclcpp::spin_some，收到语音订单后转换成 Order 并 EnqueueOrder()。
    void Loop() override;
    // TODO: 发布机器人状态 ROS2 消息。
    void ReportRobotState(const QString& state) override;
    // TODO: 发布任务结果 ROS2 消息。
    void ReportTaskResult(const TaskResult& result) override;

private:
    QString m_ros_node_name;          // ROS2 节点名。
    VoiceInterfaceConfig m_config;  // 语音 topic 配置。
};

} // namespace asd_retail
