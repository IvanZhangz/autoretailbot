// 文件说明：声明 Fake 动作执行器。
// Fake 执行器不依赖真实硬件，用定时状态机模拟动作 Begin -> Running -> Finished。
#pragma once

#include "hardware/HardwareInterfaces.h"

#include <QElapsedTimer>
#include <memory>

// Fake 简单设备：只保存状态文本。
class FakeSimpleDevice : public ISimpleDevice, public IChassisDevice, public IArmDevice
{
public:
    // name 用于日志/状态文本中标识设备，例如 chassis 或 right_arm。
    explicit FakeSimpleDevice(QString name);
    // 保留真实硬件轮询入口；Fake 版本不做实际通讯。
    void loop() override;
    // 返回“设备名:状态”形式的文本，方便调试。
    QString statusText() const override;
    // 更新 Fake 设备状态文本。
    void setStatus(const QString& text);

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
    bool init(const RobotConfig& config) override;
    // 轮询所有 Fake 设备。
    void loopDev() override;
    // 推进单个 RobotAction 的模拟状态机。
    void loopAction(RobotAction& action) override;
    // Fake 执行器是否处于错误状态。
    bool isErr() const override;
    // Fake 执行器错误文本。
    QString errStr() const override;

private:
    // 每个动作模拟耗时，当前统一返回 150ms。
    int durationMs(const RobotAction& action) const;
    // 动作完成收尾；视觉动作会额外写入 detected_pose。
    void finishAction(RobotAction& action);
    // 把 Pose 转成 QVariantMap，写入 mission.args。
    QVariantMap poseToMap(const Pose& pose) const;

    RobotConfig m_config; // 缓存配置，Fake 视觉需要读取行高等信息。
    QElapsedTimer m_clock; // 模拟动作耗时的单调时钟。
    bool m_error = false; // 是否发生模拟错误。
    QString m_errorText;  // 模拟错误说明。

    std::unique_ptr<FakeSimpleDevice> m_chassis;      // 底盘 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_leftArm;      // 左臂 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_rightArm;     // 右臂 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_lift;         // 升降 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_waist;        // 腰部 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_neck;         // 颈部/头部 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_leftGripper;  // 左夹爪 Fake 设备。
    std::unique_ptr<FakeSimpleDevice> m_rightGripper; // 右夹爪 Fake 设备。
};
