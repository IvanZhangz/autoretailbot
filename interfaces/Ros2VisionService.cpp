// 文件说明：实现 ROS2 视觉服务骨架。
#include "Ros2VisionService.h"


namespace asd_retail
{
Ros2VisionService::Ros2VisionService(QObject* parent)
    : VisionService(parent)
{
}

bool Ros2VisionService::Start(const InterfaceConfig& config)
{
    m_ros_node_name = config.m_ros_node_name;
    m_config = config.m_vision;
    m_running = true;
    // TODO: 引入 rclcpp 后，在这里创建 node、request publisher、result subscriber。
    emit LogMessage("ROS2 视觉服务骨架启动，请求 topic: " + m_config.m_request_topic + "，结果 topic: " + m_config.m_result_topic);
    return true;
}

void Ros2VisionService::Loop()
{
    if (!m_running) return;
    // TODO: rclcpp::spin_some(node)；订阅回调中把 ROS2 视觉结果转成 VisionResult 并调用 StoreResult(result)。
}

bool Ros2VisionService::RequestDetection(const VisionRequest& request)
{
    if (!m_running) return false;
    // TODO: 将 request_id/product_id/slot_id/row/col/observe_profile 打包成 ROS2 消息并发布到 m_config.m_request_topic。
    emit LogMessage("ROS2 视觉请求发布骨架: " + request.m_request_id + " product=" + request.m_product_id);
    return true;
}

} // namespace asd_retail
