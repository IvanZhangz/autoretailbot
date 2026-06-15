// 文件说明：实现 Fake 语音服务。
#include "FakeVoiceService.h"


namespace asd_retail
{
FakeVoiceService::FakeVoiceService(QObject* parent)
    : VoiceService(parent)
{
}

bool FakeVoiceService::Start(const InterfaceConfig& config)
{
    m_config = config.m_voice;
    m_running = true;
    emit LogMessage("Fake 语音服务启动，模拟 ROS2 订单 topic: " + m_config.m_order_topic);
    return true;
}

void FakeVoiceService::Loop()
{
    if (!m_running) return;
    // Fake 语音没有 ROS2 消息；真实 Ros2VoiceService 会在 Loop 中 spin_some。
}

void FakeVoiceService::ReportRobotState(const QString& state)
{
    emit LogMessage("Fake 语音状态上报: " + state);
}

void FakeVoiceService::ReportTaskResult(const TaskResult& result)
{
    emit LogMessage(QString("Fake 语音订单结果: order=%1 success=%2")
                        .arg(result.m_order_id, result.m_success ? "true" : "false"));
}

} // namespace asd_retail
