#pragma once

#include "hardware/arm/IArmDevice.h"
#include "model/RobotTypes.h"

#include <QMap>

namespace asd_retail
{

class RealmanArmDevice final : public IArmDevice
{
public:
    explicit RealmanArmDevice(const ArmDeviceConfig& config);

    ~RealmanArmDevice() override;

    bool Connect() override;
    bool Disconnect() override;

    bool Enable() override {return true;}
    bool Disable() override {return true;}

    void Loop() override {return;}

    bool MoveJ(const QVector<double>& joints, double speed) override {return true;}

    bool MoveL(const Pose& pose, double speed) override {return true;}

    bool MoveJNamed(const QString& pose_name, double speed) override {return true;}

    bool Stop() override {return true;}
    bool ClearError() override {return true;}

    bool IsConnected() const override {return true;}
    bool IsEnabled() const override {return true;}
    bool IsMoving() const override {return true;}
    bool IsReached() const override {return true;}
    bool HasError() const override {return false;}

    QString ErrorText() const override {return QString();}
    QString StatusText() const override {return QString();}

    ArmState State() const override {return m_state;}

private:
    ArmDeviceConfig m_config;
    ArmState m_state;

    // 替换为真实 SDK handle。
    // rm_robot_handle* m_handle = nullptr;

};

} // namespace asd_retail