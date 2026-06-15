// 文件说明：声明 Fake 视觉服务。
// Fake 不连接 ROS2，请求到达后立即/短延迟生成一个模拟 detected_pose，便于本地验证任务流程。
#pragma once

#include "interfaces/VisionService.h"


namespace asd_retail
{
class FakeVisionService : public VisionService
{
public:
    explicit FakeVisionService(QObject* parent = nullptr);

    // Fake 启动时缓存视觉配置。
    bool Start(const InterfaceConfig& config) override;
    // Fake 当前无需异步轮询，保留入口以对齐 ROS2 实现。
    void Loop() override;
    // Fake 根据 row/col 生成模拟识别结果并缓存。
    bool RequestDetection(const VisionRequest& request) override;

private:
    VisionInterfaceConfig m_config; // 视觉 ROS2 topic/超时配置缓存。
};

} // namespace asd_retail
