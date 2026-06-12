// 文件说明：实现语音服务占位逻辑。
// 真实接入时 loop 中轮询语音 SDK 或 IPC，解析订单意图并写入 m_orders。
#include "VoiceService.h"

VoiceService::VoiceService(QObject* parent)
    : QObject(parent)
{
}

bool VoiceService::start(const InterfaceConfig& config)
{
    m_endpoint = config.voiceEndpoint;
    m_running = true;
    emit logMessage("语音服务启动: " + m_endpoint);
    return true;
}

void VoiceService::loop()
{
    if (!m_running) return;
}

bool VoiceService::hasOrder() const
{
    return !m_orders.isEmpty();
}

Order VoiceService::takeOrder()
{
    return m_orders.dequeue();
}

void VoiceService::reportRobotState(const QString& state)
{
    emit logMessage("语音状态上报: " + state);
}

void VoiceService::reportTaskResult(const TaskResult& result)
{
    emit logMessage(QString("语音订单结果: order=%1 success=%2").arg(result.orderId, result.success ? "true" : "false"));
}
