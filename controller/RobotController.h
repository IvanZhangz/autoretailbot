// 文件说明：声明机器人控制器单例。
// main 创建 MainWindow 的同时会启动该 Controller；Controller 负责初始化部件、startTimer、timerEvent、loop、loop_workSM。
#pragma once

#include "config/ConfigLoader.h"
#include "hardware/ActuatorFactory.h"
#include "interfaces/TabletService.h"
#include "interfaces/VoiceService.h"
#include "mission/RobotMission.h"

#include <QObject>
#include <QQueue>
#include <memory>

// RobotController 是后端控制核心，前端 MainWindow 只连接信号和发送调试命令。
class RobotController : public QObject
{
    Q_OBJECT
public:
    static RobotController& instance();

    bool init(const QString& configDir, QString* errorMessage = nullptr);
    QStringList productIds() const;
    void submitDebugOrder(const QString& productId);

signals:
    void logMessage(const QString& message);
    void stateChanged(const QString& state);

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RobotController(QObject* parent = nullptr);
    Q_DISABLE_COPY_MOVE(RobotController)

    void loop();
    void loopDevices();
    void loopServices();
    void loop_workSM();
    void acceptPendingOrders();
    void startNextMissionIfIdle();
    bool buildMissionFromOrder(const Order& order, RobotMission& mission, QString* errorMessage);
    void appendHomeBeat(RobotMission& mission);
    ColumnStation stationForSlot(const ShelfSlot& slot) const;
    QVariantMap poseArgs(const Pose& pose) const;
    QVariantMap stationArgs(const ColumnStation& station) const;
    void finishMission(bool success, const QString& message);
    void setState(const QString& state);

    RobotConfig m_config;
    bool m_initialized = false;
    int m_timerId = 0;
    int m_currStep = 0;
    QString m_state = "未初始化";
    QQueue<Order> m_orderQueue;
    RobotMission m_mission;

    std::unique_ptr<ActionActuator> m_actuator;
    std::unique_ptr<TabletService> m_tabletService;
    std::unique_ptr<VoiceService> m_voiceService;
};
