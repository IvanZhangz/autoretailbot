// 文件说明：声明视觉服务抽象接口。
// 感知视觉是独立软件，真实接入通过 ROS2 请求/结果消息与机器人控制系统交互。
#pragma once

#include "model/RobotTypes.h"

#include <QObject>
#include <QMap>

// 视觉识别请求：由 VisionDetectByProduct action 生成，发送给独立视觉软件。

namespace asd_retail
{
struct VisionRequest
{
    QString m_request_id;      // 单次视觉请求 ID，用于匹配异步结果。
    QString m_product_id;      // 要识别的商品 ID。
    QString m_slot_id;         // 当前货位 ID。
    int m_row = 0;            // 货位行号。
    int m_col = 0;            // 货位列号。
    QString m_observe_profile; // 观察配置名，视觉/头部可据此选择参数。
};

// 视觉识别结果：ROS2 视觉软件返回后转换成该结构。
struct VisionResult
{
    QString m_request_id;       // 对应的请求 ID。
    bool m_success = false;    // 是否识别成功。
    Pose m_detected_pose;       // 商品抓取位姿。
    double m_confidence = 0.0;  // 识别置信度。
    QString error_message;    // 失败原因。
};

class VisionService : public QObject
{
    Q_OBJECT
public:
    explicit VisionService(QObject* parent = nullptr);
    ~VisionService() override = default;

    // 启动视觉服务：ROS2 实现会创建节点、publisher、subscriber。
    virtual bool Start(const InterfaceConfig& config) = 0;
    // 周期轮询视觉服务：ROS2 实现会 spin_some 并处理结果消息。
    virtual void Loop() = 0;
    // 发送视觉识别请求；request_id 必须唯一。
    virtual bool RequestDetection(const VisionRequest& request) = 0;
    // 查询指定 request_id 是否已有结果。
    bool HasResult(const QString& request_id) const;
    // 取出指定 request_id 的结果；调用前应先 HasResult()。
    VisionResult TakeResult(const QString& request_id);

signals:
    // 视觉服务日志，由 Controller 转发到前端。
    void LogMessage(const QString& message);

protected:
    // 子类收到视觉结果后调用该函数缓存，动作状态机随后轮询取走。
    void StoreResult(const VisionResult& result);

    bool m_running = false;                  // Start() 成功后为 true。
    QMap<QString, VisionResult> m_results;   // request_id 到异步视觉结果的缓存。
};

} // namespace asd_retail
