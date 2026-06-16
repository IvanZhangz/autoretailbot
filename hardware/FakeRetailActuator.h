// 文件说明：声明 Fake 动作执行器。
// Fake 执行器不依赖真实硬件，但仍按 ActionType 分发到各动作状态机，结构与真实执行器保持一致。
#pragma once

#include "hardware/HardwareInterfaces.h"
#include "interfaces/VisionService.h"

#include <QElapsedTimer>
#include <memory>

// Fake 简单设备：只保存状态文本，用于模拟底盘、手臂、升降、腰、头和夹爪等设备对象。

namespace asd_retail
{
class FakeSimpleDevice
{
public:
    // m_name 用于日志/状态文本中标识设备，例如 chassis 或 right_arm。
    explicit FakeSimpleDevice(QString m_name);
    // 保留真实硬件轮询入口；Fake 版本不做实际通讯。
    void Loop();
    // 返回“设备名:状态”形式的文本，方便调试。
    QString StatusText() const;
    // 更新 Fake 设备状态文本。
    void SetStatus(const QString& text);

private:
    QString m_name;   // 设备名称。
    QString m_status; // 当前模拟状态。
};

// FakeRetailActuator：把 RobotAction 转换成模拟硬件动作。
class FakeRetailActuator : public ActionActuator
{
    Q_OBJECT
public:
    explicit FakeRetailActuator(QObject* parent = nullptr);

    // 创建所有 Fake 设备并启动内部计时器。
    bool Init(const RobotConfig& config) override;
    // 注入视觉服务；Fake 中 VisionDetectByProduct 也通过 FakeVisionService 生成 detected_pose。
    void SetVisionService(VisionService* visionService) override;
    // 轮询所有 Fake 设备。
    void LoopDev() override;
    // 按 ActionType 分发并推进单个 RobotAction 的模拟状态机。
    void LoopAction(RobotAction& action) override;
    // Fake 执行器是否处于错误状态。
    bool IsErr() const override;
    // Fake 执行器错误文本。
    QString ErrStr() const override;

private:
    // 通用 Fake 动作状态机：Begin 记录开始时间，Running 等待耗时结束，然后 Finished。
    void LoopTimedAction(RobotAction& action, const QString& deviceName);
    // Fake 视觉动作状态机：通过 VisionService 发送请求、轮询结果并写入 m_mission_args。
    void LoopVisionDetectByProduct(RobotAction& action);
    // 状态切换工具：统一记录日志，便于观察 Begin/Running/Finished 等状态变化。
    void SwitchActionState(RobotAction& action, RobotAction::ActionSta nextSta, const QString& reason);
    // 每个动作模拟耗时，当前统一返回 150ms。
    int DurationMs(const RobotAction& action) const;
    // 把 Pose 转成 QVariantMap，写入 mission.m_args。
    QVariantMap PoseToMap(const Pose& pose) const;
    // 根据动作类型更新对应 Fake 设备状态文本。
    void SetDeviceStatus(const QString& deviceName, const QString& status);

    RobotConfig m_config;        // 缓存配置，Fake 视觉需要读取货位等信息。
    VisionService* m_vision_service = nullptr; // 视觉服务，Fake/ROS2 均通过该抽象交互。
    QElapsedTimer m_clock;       // 模拟动作耗时的单调时钟。
    bool m_error = false;        // 是否发生模拟错误。
    QString m_error_text;         // 模拟错误说明。

    std::unique_ptr<FakeSimpleDevice> m_chassis;      // 底盘 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_left_arm;      // 左臂 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_right_arm;     // 右臂 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_lift;         // 升降 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_waist;        // 腰部 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_neck;         // 颈部/头部 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_left_gripper;  // 左夹爪 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_right_gripper; // 右夹爪 Fake 设备。
};

} // namespace asd_retail
