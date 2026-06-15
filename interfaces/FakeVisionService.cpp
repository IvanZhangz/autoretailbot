// 文件说明：实现 Fake 视觉服务。
#include "FakeVisionService.h"


namespace asd_retail
{
FakeVisionService::FakeVisionService(QObject* parent)
    : VisionService(parent)
{
}

bool FakeVisionService::Start(const InterfaceConfig& config)
{
    m_config = config.m_vision;
    m_running = true;
    emit LogMessage("Fake 视觉服务启动，模拟 ROS2 请求 topic: " + m_config.m_request_topic);
    return true;
}

void FakeVisionService::Loop()
{
    if (!m_running) return;
    // Fake 视觉在 RequestDetection 中同步写入结果；真实 Ros2VisionService 会在 Loop 中接收异步结果。
}

bool FakeVisionService::RequestDetection(const VisionRequest& request)
{
    if (!m_running) return false;

    VisionResult result;
    result.m_request_id = request.m_request_id;
    result.m_success = true;
    result.m_detected_pose.m_x = 0.35;
    result.m_detected_pose.m_y = 0.02 * request.m_col;
    result.m_detected_pose.m_z = 0.25 + 0.30 * (request.m_row - 1);
    result.m_confidence = 0.98;
    StoreResult(result);
    emit LogMessage(QString("Fake 视觉生成结果: request=%1 product=%2 m_confidence=%3")
                        .arg(request.m_request_id, request.m_product_id)
                        .arg(result.m_confidence));
    return true;
}

} // namespace asd_retail
