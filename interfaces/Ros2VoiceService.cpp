// 文件说明：实现 ROS2 语音服务骨架。
#include "Ros2VoiceService.h"


namespace asd_retail
{
Ros2VoiceService::Ros2VoiceService(QObject* parent)
    : VoiceService(parent)
{
}

bool Ros2VoiceService::Start(const InterfaceConfig& config)
{
    m_ros_node_name = config.m_ros_node_name;
    m_config = config.m_voice;
    m_running = true;
    // TODO: 引入 rclcpp 后，在这里创建节点、订阅 m_config.m_order_topic、创建状态和结果 publisher。
    emit LogMessage("ROS2 语音服务骨架启动，订单 topic: " + m_config.m_order_topic);
    return true;
}

void Ros2VoiceService::Loop()
{
    if (!m_running) return;
    // TODO: rclcpp::spin_some(node)；回调中解析语音订单消息并 EnqueueOrder(order)。
}

void Ros2VoiceService::ReportRobotState(const QString& state)
{
    // TODO: 将 state 转换成 ROS2 状态消息并发布到 m_config.m_state_topic。
    emit LogMessage("ROS2 语音状态发布骨架: " + state);
}

void Ros2VoiceService::ReportTaskResult(const TaskResult& result)
{
    // TODO: 将 TaskResult 转换成 ROS2 任务结果消息并发布到 m_config.m_task_result_topic。
    emit LogMessage(QString("ROS2 语音任务结果骨架: order=%1 success=%2")
                        .arg(result.m_order_id, result.m_success ? "true" : "false"));
}

} // namespace asd_retail
