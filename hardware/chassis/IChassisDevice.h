#pragma once

#include "model/RobotTypes.h"

#include <QString>

namespace asd_retail
{

struct ChassisState
{
    bool m_connected = false;
    bool m_moving = false;
    bool m_reached = false;
    bool m_error = false;

    QString m_error_text;

    QString m_current_station_id;
    Pose m_current_pose;
};

class IChassisDevice
{
public:
    virtual ~IChassisDevice() = default;

    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;

    virtual void Loop() = 0;

    virtual bool MoveToStation(
        const QString& station_id) = 0;

    virtual bool MoveToPose(
        const Pose& pose) = 0;

    virtual bool Stop() = 0;
    virtual bool ClearError() = 0;

    virtual bool IsConnected() const = 0;
    virtual bool IsMoving() const = 0;
    virtual bool IsReached() const = 0;
    virtual bool HasError() const = 0;

    virtual QString ErrorText() const = 0;
    virtual QString StatusText() const = 0;

    virtual ChassisState State() const = 0;
};

} // namespace asd_retail