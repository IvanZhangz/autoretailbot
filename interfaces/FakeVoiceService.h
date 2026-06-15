// 文件说明：声明 Fake 语音服务。
// Fake 不连接 ROS2，只打印状态和任务结果日志。
#pragma once

#include "interfaces/VoiceService.h"


namespace asd_retail
{
class FakeVoiceService : public VoiceService
{
public:
    explicit FakeVoiceService(QObject* parent = nullptr);

    // Fake 启动时只记录 ROS2 topic 配置，便于确认配置正确。
    bool Start(const InterfaceConfig& config) override;
    // Fake 没有 ROS2 消息需要轮询。
    void Loop() override;
    // Fake 状态上报只打印日志。
    void ReportRobotState(const QString& state) override;
    // Fake 任务结果上报只打印日志。
    void ReportTaskResult(const TaskResult& result) override;

private:
    VoiceInterfaceConfig m_config; // 语音 ROS2 topic 配置缓存。
};

} // namespace asd_retail
