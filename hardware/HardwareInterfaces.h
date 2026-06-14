// 文件说明：声明硬件抽象接口和动作执行器接口。
// Controller 不直接依赖具体厂家 SDK，只通过 ActionActuator 轮询设备和动作。
#pragma once

#include "mission/RobotMission.h"
#include "model/RobotTypes.h"

#include <QObject>
#include <QString>

// 底盘接口抽象。
class IChassisDevice
{
public:
    virtual ~IChassisDevice() = default;
    virtual void loop() = 0;
    virtual QString statusText() const = 0;
};

// 机械臂接口抽象。
class IArmDevice
{
public:
    virtual ~IArmDevice() = default;
    virtual void loop() = 0;
    virtual QString statusText() const = 0;
};

// 升降/腰/颈/夹爪等通用设备接口抽象。
class ISimpleDevice
{
public:
    virtual ~ISimpleDevice() = default;
    virtual void loop() = 0;
    virtual QString statusText() const = 0;
};

// 动作执行器抽象
class ActionActuator : public QObject
{
    Q_OBJECT
public:
    explicit ActionActuator(QObject* parent = nullptr) : QObject(parent) {}
    ~ActionActuator() override = default;

    // 初始化硬件或 SDK。
    virtual bool init(const RobotConfig& config) = 0;
    // 轮询所有设备状态。
    virtual void loopDev() = 0;
    // 轮询并推进单个动作状态机。
    virtual void loopAction(RobotAction& action) = 0;
    // 是否存在执行器错误。
    virtual bool isErr() const = 0;
    // 错误说明。
    virtual QString errStr() const = 0;

signals:
    // 日志信号：替代回调，统一由 Controller 转发到前端。
    void logMessage(const QString& message);
};
