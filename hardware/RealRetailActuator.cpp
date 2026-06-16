// 文件说明：实现真实硬件动作执行器骨架。
// 注意：当前不引入任何厂家 SDK、ROS2 或 gRPC 依赖，只保留动作状态机 TODO 位置，保证项目可编译。
#include "RealRetailActuator.h"

#include "hardware/arm/ArmDeviceFactory.h"
#include "hardware/chassis/ChassisDeviceFactory.h"
#include "hardware/simple/SimpleDeviceFactory.h"

#include <QVariant>


namespace asd_retail
{
RealRetailActuator::RealRetailActuator(QObject* parent)
    : ActionActuator(parent)
{
}

RealRetailActuator::~RealRetailActuator()
{
}

bool RealRetailActuator::Init(const RobotConfig& config)
{
    m_config = config;
    m_clock.start();
    // TODO: 在这里初始化真实底盘、机械臂、升降、腰、头、夹爪等设备 SDK 或通讯连接。
    emit LogMessage("真实硬件执行器骨架初始化完成：等待填充设备 SDK TODO");
    return true;
}

void RealRetailActuator::SetVisionService(VisionService* visionService)
{
    m_vision_service = visionService;
}

void RealRetailActuator::LoopDev()
{
    // TODO: 周期读取真实底盘/机械臂/夹爪/升降/腰/头状态，刷新在线状态、到位状态和错误码。
}

void RealRetailActuator::LoopAction(RobotAction& action)
{
    switch (action.m_type)
    {
    case RobotAction::AgvMoveToStation: Loop_ActSM_AgvMoveToStation(action); break;
    case RobotAction::LiftMoveToHeight: Loop_ActSM_LiftMoveToHeight(action); break;
    case RobotAction::WaistRotateTo: Loop_ActSM_WaistRotateTo(action); break;
    case RobotAction::NeckMoveTo: Loop_ActSM_NeckMoveTo(action); break;
    case RobotAction::LeftArmMoveNamed: Loop_ActSM_LeftArmMoveNamed(action); break;
    case RobotAction::RightArmMoveNamed: Loop_ActSM_RightArmMoveNamed(action); break;
    case RobotAction::RightArmMoveL: Loop_ActSM_RightArmMoveL(action); break;
    case RobotAction::RightArmMoveJNamed: Loop_ActSM_RightArmMoveJNamed(action); break;
    case RobotAction::DualArmMoveL: Loop_ActSM_DualArmMoveL(action); break;
    case RobotAction::DualArmMoveCarry: Loop_ActSM_DualArmMoveCarry(action); break;
    case RobotAction::DualArmMovePlace: Loop_ActSM_DualArmMovePlace(action); break;
    case RobotAction::LeftGripperOpen:
    case RobotAction::RightGripperOpen:
    case RobotAction::BothGrippersOpen:
    case RobotAction::LeftGripperClose:
    case RobotAction::RightGripperClose:
    case RobotAction::BothGrippersClose:
        Loop_ActSM_Gripper(action);
        break;
    case RobotAction::VisionDetectByProduct:
        Loop_ActSM_VisionDetectByProduct(action);
        break;
    default:
        break;
    }
}

bool RealRetailActuator::IsErr() const
{
    return m_error;
}

QString RealRetailActuator::ErrStr() const
{
    return m_error_text;
}

void RealRetailActuator::Loop_ActSM_AgvMoveToStation(RobotAction& action)
{
    // TODO：参考Loop_ActSM_VisionDetectByProduct和Swit2Sta_ActSM_VisionDetectByProduct，后续各个动作同理。
    // 每一个Loop_ActSM_xxx，需对应一个swit2Sta_ActSM_xxx
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_LiftMoveToHeight(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_WaistRotateTo(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_NeckMoveTo(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_LeftArmMoveNamed(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_RightArmMoveNamed(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_RightArmMoveL(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_RightArmMoveJNamed(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_DualArmMoveL(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_DualArmMoveCarry(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_DualArmMovePlace(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_Gripper(RobotAction& action)
{
    Q_UNUSED(action)
}

void RealRetailActuator::Loop_ActSM_VisionDetectByProduct(RobotAction& action)
{
    if (action.m_sta == RobotAction::ActFinished || action.m_sta == RobotAction::ActFailure)
        return;

    if (!m_vision_service)
    {
        Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActFailure, "未注入视觉服务，无法请求 ROS2 视觉识别");
        return;
    }

    if (action.m_sta == RobotAction::ActBegin)
    {
        VisionRequest request;
        request.m_request_id = action.m_name + QString::number(m_clock.elapsed());
        request.m_product_id = action.m_args.value("product_id").toString();
        request.m_slot_id = action.m_mission_args ? action.m_mission_args->value("slot_id").toString() : QString();
        request.m_row = action.m_args.value("row").toInt();
        request.m_col = action.m_args.value("col").toInt();
        request.m_observe_profile = action.m_args.value("observe_profile").toString();
        action.m_args["request_id"] = request.m_request_id;
        action.m_started_at_ms = m_clock.elapsed();
        Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActInitialized, "视觉请求参数已准备");
        if (!m_vision_service->RequestDetection(request))
            Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActFailure, "ROS2 视觉请求发布失败");
        else
            Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActRunning, "ROS2 视觉请求已发布，等待结果");
        return;
    }

    if (action.m_sta != RobotAction::ActRunning)
        return;

    const QString request_id = action.m_args.value("request_id").toString();
    if (m_vision_service->HasResult(request_id))
    {
        const VisionResult result = m_vision_service->TakeResult(request_id);
        if (!result.m_success)
        {
            Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActFailure, "视觉返回失败: " + result.error_message);
            return;
        }
        if (action.m_mission_args)
        {
            (*action.m_mission_args)["vision_success"] = true;
            (*action.m_mission_args)["detected_pose"] = PoseToMap(result.m_detected_pose);
            (*action.m_mission_args)["confidence"] = result.m_confidence;
        }
        Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActFinished, "视觉结果已写入 m_mission_args");
        return;
    }

    if (m_clock.elapsed() - action.m_started_at_ms > m_config.m_interfaces.m_vision.m_timeout_ms)
        Swit2Sta_ActSM_VisionDetectByProduct(action, RobotAction::ActFailure, "等待 ROS2 视觉结果超时");
}

void RealRetailActuator::Swit2Sta_ActSM_VisionDetectByProduct(RobotAction& action, RobotAction::ActionSta nextSta, const QString& reason)
{
    emit LogMessage(QString("真实动作状态切换: %1 -> %2，原因: %3")
                        .arg(action.m_name)
                        .arg(nextSta)
                        .arg(reason));
                        
    if(action.m_sta==nextSta)
    {
        return;
    }
    //Exit动作
    switch(action.m_sta)
    {
    case RobotAction::ActBegin:
        break;
    case RobotAction::ActWait:
        break;
    case RobotAction::ActInitialized:
        break;
    case RobotAction::ActHalt:
        break;
    case RobotAction::ActRunning:
        break;
    case RobotAction::ActFailure:
        break;
    case RobotAction::ActFinished:
        break;
    case RobotAction::ActEnd:
        break;
    default:
        break;
    }
    //Entry动作
    switch(nextSta)
    {
    case RobotAction::ActBegin:
        break;
    case RobotAction::ActWait:
        break;
    case RobotAction::ActInitialized:
        break;
    case RobotAction::ActHalt:
        break;
    case RobotAction::ActRunning:
        break;
    case RobotAction::ActFailure:
        break;
    case RobotAction::ActFinished:
        break;
    case RobotAction::ActEnd:
        break;
    default:
        break;
    }
    //更新状态
    action.m_sta=nextSta;
}

QVariantMap RealRetailActuator::PoseToMap(const Pose& pose) const
{
    return {{"x", pose.m_x}, {"y", pose.m_y}, {"z", pose.m_z}, {"roll", pose.m_roll}, {"pitch", pose.m_pitch}, {"yaw", pose.m_yaw}};
}

} // namespace asd_retail
