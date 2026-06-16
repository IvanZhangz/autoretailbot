#pragma once

#include "RobotTypes.h"

#include <QString>
#include <QVector>

namespace asd_retail
{

struct ArmState
{
    bool m_connected = false;
    bool m_enabled = false;
    bool m_moving = false;
    bool m_reached = false;
    bool m_error = false;

    QString m_error_text;

    QVector<double> m_joints;
    Pose m_tcp_pose;
};

class IArmDevice
{
public:
    virtual ~IArmDevice() = default;

    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;

    virtual bool Enable() = 0;
    virtual bool Disable() = 0;

    virtual void Loop() = 0;
    
    virtual bool MoveJ(const QVector<double>& joints, double speed = 0.2) = 0;
    virtual bool MoveL(const Pose& pose, double speed = 0.1) = 0;
    virtual bool MoveJNamed(const QString& pose_name, double speed = 0.2) = 0;

    virtual bool Stop() = 0;
    virtual bool ClearError() = 0;

    virtual bool IsConnected() const = 0;
    virtual bool IsEnabled() const = 0;
    virtual bool IsMoving() const = 0;
    virtual bool IsReached() const = 0;
    virtual bool HasError() const = 0;

    virtual QString ErrorText() const = 0;
    virtual QString StatusText() const = 0;

    virtual ArmState State() const = 0;
};

} // namespace asd_retail