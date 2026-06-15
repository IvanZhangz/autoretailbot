// 文件说明：定义任务 Mission、节拍 Beat、动作 Action。
#pragma once

#include <QList>
#include <QString>
#include <QVariantMap>

// 机器人动作定义。

namespace asd_retail
{
struct RobotAction
{
    // 动作类型：覆盖 AGV、手臂、夹爪、升降、视觉等 MVP 必要动作。
    enum ActionType
    {
        Uninitialized = 0,
        AgvMoveToStation,
        LiftMoveToHeight,
        WaistRotateTo,
        NeckMoveTo,
        LeftArmMoveNamed,
        RightArmMoveNamed,
        RightArmMoveL,
        RightArmMoveJNamed,
        DualArmMoveL,
        DualArmMoveCarry,
        DualArmMovePlace,
        LeftGripperOpen,
        RightGripperOpen,
        BothGrippersOpen,
        LeftGripperClose,
        RightGripperClose,
        BothGrippersClose,
        VisionDetectByProduct
    };

    // 动作状态：Begin/Wait/Initialized/Running/Failure/Finished
    enum ActionSta
    {
        ActBegin = 0,
        ActWait,
        ActInitialized,
        ActHalt,
        ActRunning,
        ActFailure,
        ActFinished,
        ActEnd
    };

    RobotAction() = default;
    RobotAction(ActionType actionType, const QString& actionName, const QVariantMap& actionArgs = {});

    // 绑定任务公共参数区，视觉输出和后续动作输入通过该参数区传递。
    void BindMissionArgs(QVariantMap* m_args);

    ActionType m_type = Uninitialized; // 动作类型。
    ActionSta m_sta = ActBegin;        // 动作状态。
    QString m_name;                    // 动作名，用于日志。
    QVariantMap m_args;                // 动作静态参数。
    QVariantMap* m_mission_args = nullptr; // 任务运行期共享参数。
    qint64 m_started_at_ms = 0;          // Fake 执行器用于模拟动作耗时。
};

// 一个节拍：包含多个同时启动的动作。
struct RobotBeat
{
    QString m_name;                // 节拍名。
    QList<RobotAction> m_actions;  // 节拍内动作列表。
};

// 一个完整任务：包含多个顺序执行的节拍。
class RobotMission
{
public:
    void Clear();

    bool m_is_finished = true;  // 任务流程是否已完成。
    bool m_is_end = true;       // 任务是否已结束，失败也算结束。
    QString m_order_id;         // 关联订单号。
    QString ErrStr;          // 失败原因。
    QList<RobotBeat> m_beats;  // 节拍列表。
    QVariantMap m_args;        // 任务公共参数区。
};

} // namespace asd_retail
