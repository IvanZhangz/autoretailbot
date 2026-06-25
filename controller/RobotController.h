// 文件说明：声明机器人控制器单例。
// MainWindow 初始化该 Controller；Controller 负责初始化部件、startTimer、timerEvent、Loop、LoopWorkSM。
#pragma once

#include "ConfigLoader.h"
#include "ActuatorFactory.h"
#include "TabletService.h"
#include "TabletServiceFactory.h"
#include "VoiceService.h"
#include "VoiceServiceFactory.h"
#include "VisionService.h"
#include "VisionServiceFactory.h"
#include "RobotMission.h"

#include <QObject>
#include <QQueue>
#include <memory>

// RobotController 是后端控制核心，前端 MainWindow 只连接信号和发送调试命令。

namespace asd_retail
{
class RobotController : public QObject
{
    Q_OBJECT
public:
    // 返回全局唯一 Controller；全项目共享同一个机器人后端状态。
    static RobotController& Instance();

    // 初始化配置、硬件执行器、外部服务，并启动 Qt 定时器主循环。
    bool Init(const QString& config_dir, QString* error_message = nullptr);
    // 给前端下拉框使用：返回配置中所有商品 ID。
    QStringList ProductIds() const;
    // 调试入口：把前端选择的商品包装成订单，放进平板服务队列。
    void SubmitDebugOrder(const QString& product_id);

signals:
    // 日志信号：Controller、服务、硬件统一转发到 MainWindow 日志框。
    void LogMessage(const QString& message);
    // 状态信号：用于更新窗口状态栏，也可接入平板/语音状态上报。
    void StateChanged(const QString& state);

protected:
    // Qt 定时器回调：每隔 task_policy.loop_interval_ms 触发一次 Loop()。
    void timerEvent(QTimerEvent* event) override;

private:
    explicit RobotController(QObject* parent = nullptr);
    Q_DISABLE_COPY_MOVE(RobotController)

    // 单次主循环：轮询设备、服务、订单队列和任务状态机。
    void Loop();
    // 轮询底盘、机械臂、夹爪等硬件设备。
    void LoopDevices();
    // 轮询平板 gRPC、语音 ROS2 和视觉 ROS2 服务，真实接入时会处理网络/ROS2事件。
    void LoopServices();
    // 工作状态机：推进当前 Mission 的 Beat/Action。
    void LoopWorkSM();
    // 从各服务内部队列取出订单，汇总到 Controller 的统一订单队列。
    void AcceptPendingOrders();
    // 机器人空闲且有订单时，构建并启动下一个 Mission。
    void StartNextMissionIfIdle();
    // 把订单转换成可执行的 Mission/Beat/Action 序列。
    bool BuildMissionFromOrder(const Order& order, RobotMission& mission, QString* error_message);
    // 给任务末尾追加回 home 节拍；启动时也用它做一次归位。
    void AppendHomeBeat(RobotMission& mission);
    // 根据货位选择停车点：货位专用点优先，否则使用所在列标准点。
    ColumnStation StationForSlot(const ShelfSlot& slot) const;
    // 把 Pose 结构转成动作参数表，供 ActionActuator 读取。
    QVariantMap PoseArgs(const Pose& pose) const;
    // 把 ColumnStation 转成动作参数表，供 AGV 动作读取。
    QVariantMap StationArgs(const ColumnStation& station) const;
    // 标记任务完成/失败，并把结果上报给外部服务。
    void FinishMission(bool success, const QString& message);
    // 任务成功后再从本进程内存库存扣减本次取出的商品。
    void ConsumeMissionInventory();
    // 普通任务动作失败时，先切换到回 home 恢复任务；恢复完成后再按原失败原因结束。
    void BeginFailureRecovery(const QString& failure_message);
    // 更新 Controller 状态，避免重复广播同一个状态。
    void SetState(const QString& state);

    RobotConfig m_config;          // 已加载的全局配置。
    bool m_initialized = false;    // Init() 成功后才允许接单。
    int m_timer_id = 0;             // Qt startTimer 返回的定时器 ID。
    int m_curr_step = 0;            // 当前正在执行的 beat 下标。
    QString m_state = "未初始化"; // 当前机器人状态文本。
    QQueue<Order> m_order_queue;    // 平板/语音订单汇总后的待执行队列。
    RobotMission m_mission;        // 当前正在执行或刚结束的任务。
    bool m_recovering_from_failure = false; // 动作失败后是否正在执行回 home 恢复任务。
    QString m_recovery_failure_message;     // 恢复任务完成后要上报的原始失败原因。

    std::unique_ptr<ActionActuator> m_actuator;          // 硬件动作执行器。
    std::unique_ptr<TabletService> m_tablet_service;      // 平板服务实例，fake 或 gRPC。
    std::unique_ptr<VoiceService> m_voice_service;        // 语音服务实例，fake 或 ROS2。
    std::unique_ptr<VisionService> m_vision_service;      // 视觉服务实例，fake 或 ROS2。
};

} // namespace asd_retail
