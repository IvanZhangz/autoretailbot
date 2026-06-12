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
    explicit FakeSimpleDevice(QString name);
    void loop() override;
    QString statusText() const override;
    void setStatus(const QString& text);

private:
    QString m_name;
    QString m_status;
};

// FakeRetailActuator：把 RobotAction 转换成模拟硬件动作。
class FakeRetailActuator : public ActionActuator
{
    Q_OBJECT
public:
    explicit FakeRetailActuator(QObject* parent = nullptr);

    bool init(const RobotConfig& config) override;
    void loopDev() override;
    void loopAction(RobotAction& action) override;
    bool isErr() const override;
    QString errStr() const override;

private:
    int durationMs(const RobotAction& action) const;
    void finishAction(RobotAction& action);
    QVariantMap poseToMap(const Pose& pose) const;

    RobotConfig m_config;
    QElapsedTimer m_clock;
    bool m_error = false;
    QString m_errorText;

    std::unique_ptr<FakeSimpleDevice> m_chassis;
    std::unique_ptr<FakeSimpleDevice> m_leftArm;
    std::unique_ptr<FakeSimpleDevice> m_rightArm;
    std::unique_ptr<FakeSimpleDevice> m_lift;
    std::unique_ptr<FakeSimpleDevice> m_waist;
    std::unique_ptr<FakeSimpleDevice> m_neck;
    std::unique_ptr<FakeSimpleDevice> m_leftGripper;
    std::unique_ptr<FakeSimpleDevice> m_rightGripper;
};
