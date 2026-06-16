// 文件说明：声明硬件抽象接口和动作执行器接口。
// Controller 不直接依赖具体厂家 SDK，只通过 ActionActuator 轮询设备和动作。
#pragma once

#include "RobotMission.h"
#include "RobotTypes.h"

#include <QObject>
#include <QString>


namespace asd_retail
{
class VisionService;

// 动作执行器抽象
class ActionActuator : public QObject
{
    Q_OBJECT
public:
    explicit ActionActuator(QObject* parent = nullptr) : QObject(parent) {}
    ~ActionActuator() override = default;

    // 初始化硬件或 SDK。
    virtual bool Init(const RobotConfig& config) = 0;
    // 注入视觉服务；真实执行器处理 VisionDetectByProduct 时通过它发送/接收 ROS2 视觉消息。
    virtual void SetVisionService(VisionService* visionService) { Q_UNUSED(visionService); }
    // 轮询所有设备状态。
    virtual void LoopDev() = 0;
    // 轮询并推进单个动作状态机。
    virtual void LoopAction(RobotAction& action) = 0;
    // 是否存在执行器错误。
    virtual bool IsErr() const = 0;
    // 错误说明。
    virtual QString ErrStr() const = 0;

signals:
    // 日志信号：替代回调，统一由 Controller 转发到前端。
    void LogMessage(const QString& message);
};

} // namespace asd_retail
