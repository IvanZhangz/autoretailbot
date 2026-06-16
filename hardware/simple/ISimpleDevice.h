#pragma once

#include <QString>

namespace asd_retail
{

struct SimpleDeviceState
{
    bool m_connected = false;
    bool m_enabled = false;
    bool m_running = false;
    bool m_reached = false;
    bool m_error = false;

    double m_current_position = 0.0;
    double m_target_position = 0.0;

    QString m_last_command;
    QString m_error_text;
};

class ISimpleDevice
{
public:
    virtual ~ISimpleDevice() = default;

    virtual bool Connect() = 0;
    virtual bool Disconnect() = 0;

    virtual bool Enable() = 0;
    virtual bool Disable() = 0;

    virtual void Loop() = 0;

    // 升降、腰、颈等连续位置设备。
    virtual bool MoveTo(double target) = 0;

    // 夹爪等离散命令设备。
    virtual bool ExecuteCommand(
        const QString& command) = 0;

    virtual bool Stop() = 0;
    virtual bool ClearError() = 0;

    virtual bool IsConnected() const = 0;
    virtual bool IsEnabled() const = 0;
    virtual bool IsRunning() const = 0;
    virtual bool IsReached() const = 0;
    virtual bool HasError() const = 0;

    virtual double CurrentPosition() const = 0;

    virtual QString ErrorText() const = 0;
    virtual QString StatusText() const = 0;

    virtual SimpleDeviceState State() const = 0;
};

} // namespace asd_retail