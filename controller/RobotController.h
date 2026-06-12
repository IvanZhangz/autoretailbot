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
    // 返回全局唯一 Controller；全项目共享同一个机器人后端状态。
    static RobotController& instance();

    // 初始化配置、硬件执行器、外部服务，并启动 Qt 定时器主循环。
    bool init(const QString& configDir, QString* errorMessage = nullptr);
    // 给前端下拉框使用：返回配置中所有商品 ID。
    QStringList productIds() const;
    // 调试入口：把前端选择的商品包装成订单，放进平板服务队列。
    void submitDebugOrder(const QString& productId);

signals:
    // 日志信号：Controller、服务、硬件统一转发到 MainWindow 日志框。
    void logMessage(const QString& message);
    // 状态信号：用于更新窗口状态栏，也可接入平板/语音状态上报。
    void stateChanged(const QString& state);

protected:
    // Qt 定时器回调：每隔 task_policy.loop_interval_ms 触发一次 loop()。
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RobotController(QObject* parent = nullptr);
    Q_DISABLE_COPY_MOVE(RobotController)

    // 单次主循环：轮询设备、服务、订单队列和任务状态机。
    void loop();
    // 轮询底盘、机械臂、夹爪等硬件设备。
    void loopDevices();
    // 轮询平板服务和语音服务，真实接入时会处理网络/SDK事件。
    void loopServices();
    // 工作状态机：推进当前 Mission 的 Beat/Action。
    void loop_workSM();
    // 从各服务内部队列取出订单，汇总到 Controller 的统一订单队列。
    void acceptPendingOrders();
    // 机器人空闲且有订单时，构建并启动下一个 Mission。
    void startNextMissionIfIdle();
    // 把订单转换成可执行的 Mission/Beat/Action 序列。
    bool buildMissionFromOrder(const Order& order, RobotMission& mission, QString* errorMessage);
    // 给任务末尾追加回 home 节拍；启动时也用它做一次归位。
    void appendHomeBeat(RobotMission& mission);
    // 根据货位选择停车点：货位专用点优先，否则使用所在列标准点。
    ColumnStation stationForSlot(const ShelfSlot& slot) const;
    // 把 Pose 结构转成动作参数表，供 ActionActuator 读取。
    QVariantMap poseArgs(const Pose& pose) const;
    // 把 ColumnStation 转成动作参数表，供 AGV 动作读取。
    QVariantMap stationArgs(const ColumnStation& station) const;
    // 标记任务完成/失败，并把结果上报给外部服务。
    void finishMission(bool success, const QString& message);
    // 更新 Controller 状态，避免重复广播同一个状态。
    void setState(const QString& state);

    RobotConfig m_config;          // 已加载的全局配置。
    bool m_initialized = false;    // init() 成功后才允许接单。
    int m_timerId = 0;             // Qt startTimer 返回的定时器 ID。
    int m_currStep = 0;            // 当前正在执行的 beat 下标。
    QString m_state = "未初始化"; // 当前机器人状态文本。
    QQueue<Order> m_orderQueue;    // 平板/语音订单汇总后的待执行队列。
    RobotMission m_mission;        // 当前正在执行或刚结束的任务。

    std::unique_ptr<ActionActuator> m_actuator;          // 硬件动作执行器。
    std::unique_ptr<TabletService> m_tabletService;      // 平板服务实例。
    std::unique_ptr<VoiceService> m_voiceService;        // 语音服务实例。
};
