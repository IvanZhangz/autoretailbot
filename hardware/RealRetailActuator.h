// 文件说明：声明真实硬件动作执行器骨架。
// LoopAction 先按 ActionType 分发，再在每个 Loop_ActSM_xxx 中按 ActionSta 轮询状态机。
#pragma once

#include "hardware/HardwareInterfaces.h"
#include "interfaces/VisionService.h"

#include <QElapsedTimer>


namespace asd_retail
{
class RealRetailActuator : public ActionActuator
{
    Q_OBJECT
public:
    explicit RealRetailActuator(QObject* parent = nullptr);

    // TODO: 初始化真实底盘、机械臂、升降、腰、头、夹爪等 SDK/通讯对象。
    bool Init(const RobotConfig& config) override;
    // 注入视觉服务；VisionDetectByProduct 状态机通过该服务与 ROS2 视觉软件交互。
    void SetVisionService(VisionService* visionService) override;
    // TODO: 周期轮询真实设备状态、错误码和反馈缓存。
    void LoopDev() override;
    // 按 ActionType 分发到对应动作状态机。
    void LoopAction(RobotAction& action) override;
    // 真实执行器是否处于错误状态。
    bool IsErr() const override;
    // 真实执行器错误文本。
    QString ErrStr() const override;

private:
    void Loop_ActSM_AgvMoveToStation(RobotAction& action);     // AGV 到站状态机。
    void Loop_ActSM_LiftMoveToHeight(RobotAction& action);     // 升降到高度状态机。
    void Loop_ActSM_WaistRotateTo(RobotAction& action);        // 腰部旋转状态机。
    void Loop_ActSM_NeckMoveTo(RobotAction& action);           // 头/颈运动状态机。
    void Loop_ActSM_LeftArmMoveNamed(RobotAction& action);     // 左臂命名姿态状态机。
    void Loop_ActSM_RightArmMoveNamed(RobotAction& action);    // 右臂命名姿态状态机。
    void Loop_ActSM_RightArmMoveL(RobotAction& action);        // 右臂直线运动状态机。
    void Loop_ActSM_RightArmMoveJNamed(RobotAction& action);   // 右臂关节命名姿态状态机。
    void Loop_ActSM_DualArmMoveL(RobotAction& action);         // 双臂直线运动状态机。
    void Loop_ActSM_DualArmMoveCarry(RobotAction& action);     // 双臂携物姿态状态机。
    void Loop_ActSM_DualArmMovePlace(RobotAction& action);     // 双臂放置姿态状态机。
    void Loop_ActSM_Gripper(RobotAction& action);              // 单/双夹爪开闭状态机。
    void Loop_ActSM_VisionDetectByProduct(RobotAction& action);// 视觉识别状态机。

    // 状态切换工具：集中处理日志和状态赋值，真实接入时可扩展 Entry/Exit 动作。
    void Swit2Sta_ActSM_VisionDetectByProduct(RobotAction& action, RobotAction::ActionSta nextSta, const QString& reason);
  // TODO：按照 loop_ActSM_Xxx 和 swit2Sta_ActSM_Xxx 的格式，为每个动作实现对应的状态机。

    // 把 Pose 转成 QVariantMap，写入 mission.m_args。
    QVariantMap PoseToMap(const Pose& pose) const;

    RobotConfig m_config;                 // 缓存配置，供超时和接口参数使用。
    VisionService* m_vision_service = nullptr; // 视觉服务指针，不拥有对象生命周期。
    QElapsedTimer m_clock;                // 用于 TODO 超时/请求 ID 生成。
    bool m_error = false;                 // 真实执行器错误标记。
    QString m_error_text;                  // 真实执行器错误说明。
};

} // namespace asd_retail
