// 文件说明：声明 ROS2 视觉服务骨架。
// 当前不引入 rclcpp 依赖，只保留可编译 TODO；真实接入时与独立视觉软件通过 ROS2 topic 交互。
#pragma once

#include "VisionService.h"


namespace asd_retail
{
class Ros2VisionService : public VisionService
{
public:
    explicit Ros2VisionService(QObject* parent = nullptr);

    // TODO: 创建 ROS2 node，发布 VisionRequest，订阅 VisionResult。
    bool Start(const InterfaceConfig& config) override;
    // TODO: 调用 rclcpp::spin_some，接收视觉结果消息并 StoreResult()。
    void Loop() override;
    // TODO: 将 VisionRequest 转为 ROS2 消息并发布到 m_request_topic。
    bool RequestDetection(const VisionRequest& request) override;

private:
    QString m_ros_node_name;            // ROS2 节点名。
    VisionInterfaceConfig m_config;   // 视觉请求/结果 topic 和超时时间。
};

} // namespace asd_retail
