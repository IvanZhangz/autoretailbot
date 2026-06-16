// 文件说明：实现 Fake 动作执行器。
#include "FakeRetailActuator.h"

#include <QVariant>


namespace asd_retail
{
FakeSimpleDevice::FakeSimpleDevice(QString m_name)
    : m_name(std::move(m_name))
    , m_status("idle")
{
}

void FakeSimpleDevice::Loop()
{
    // Fake 设备暂无底层通讯，保留 Loop 入口以匹配真实设备轮询结构。
}

QString FakeSimpleDevice::StatusText() const
{
    return m_name + ":" + m_status;
}

void FakeSimpleDevice::SetStatus(const QString& text)
{
    m_status = text;
}

FakeRetailActuator::FakeRetailActuator(QObject* parent)
    : ActionActuator(parent)
{
}

bool FakeRetailActuator::Init(const RobotConfig& config)
{
    m_config = config;
    m_clock.start();
    m_chassis = std::make_unique<FakeSimpleDevice>("chassis");
    m_left_arm = std::make_unique<FakeSimpleDevice>("left_arm");
    m_right_arm = std::make_unique<FakeSimpleDevice>("right_arm");
    m_lift = std::make_unique<FakeSimpleDevice>("lift");
    m_waist = std::make_unique<FakeSimpleDevice>("waist");
    m_neck = std::make_unique<FakeSimpleDevice>("neck");
    m_left_gripper = std::make_unique<FakeSimpleDevice>("left_gripper");
    m_right_gripper = std::make_unique<FakeSimpleDevice>("right_gripper");
    emit LogMessage("FakeRetailActuator 初始化完成");
    return true;
}

void FakeRetailActuator::SetVisionService(VisionService* visionService)
{
    m_vision_service = visionService;
}

void FakeRetailActuator::LoopDev()
{
    m_chassis->Loop();
    m_left_arm->Loop();
    m_right_arm->Loop();
    m_lift->Loop();
    m_waist->Loop();
    m_neck->Loop();
    m_left_gripper->Loop();
    m_right_gripper->Loop();
}

void FakeRetailActuator::LoopAction(RobotAction& action)
{
    switch (action.m_type)
    {
    case RobotAction::AgvMoveToStation:
        LoopTimedAction(action, "chassis");
        break;
    case RobotAction::LiftMoveToHeight:
        LoopTimedAction(action, "lift");
        break;
    case RobotAction::WaistRotateTo:
        LoopTimedAction(action, "waist");
        break;
    case RobotAction::NeckMoveTo:
        LoopTimedAction(action, "neck");
        break;
    case RobotAction::LeftArmMoveNamed:
        LoopTimedAction(action, "left_arm");
        break;
    case RobotAction::RightArmMoveNamed:
    case RobotAction::RightArmMoveL:
    case RobotAction::RightArmMoveJNamed:
        LoopTimedAction(action, "right_arm");
        break;
    case RobotAction::DualArmMoveL:
    case RobotAction::DualArmMoveCarry:
    case RobotAction::DualArmMovePlace:
        LoopTimedAction(action, "dual_arm");
        break;
    case RobotAction::LeftGripperOpen:
    case RobotAction::LeftGripperClose:
        LoopTimedAction(action, "left_gripper");
        break;
    case RobotAction::RightGripperOpen:
    case RobotAction::RightGripperClose:
        LoopTimedAction(action, "right_gripper");
        break;
    case RobotAction::BothGrippersOpen:
    case RobotAction::BothGrippersClose:
        LoopTimedAction(action, "both_grippers");
        break;
    case RobotAction::VisionDetectByProduct:
        LoopVisionDetectByProduct(action);
        break;
    default:
        m_error = true;
        m_error_text = "Fake 不支持动作类型: " + action.m_name;
        SwitchActionState(action, RobotAction::ActFailure, m_error_text);
        break;
    }
}

bool FakeRetailActuator::IsErr() const
{
    return m_error;
}

QString FakeRetailActuator::ErrStr() const
{
    return m_error_text;
}

void FakeRetailActuator::LoopTimedAction(RobotAction& action, const QString& deviceName)
{
    if (action.m_sta == RobotAction::ActFinished || action.m_sta == RobotAction::ActFailure)
        return;

    if (action.m_sta == RobotAction::ActBegin)
    {
        action.m_started_at_ms = m_clock.elapsed();
        SetDeviceStatus(deviceName, "running:" + action.m_name);
        SwitchActionState(action, RobotAction::ActRunning, "Fake 定时动作启动");
        return;
    }

    if (action.m_sta != RobotAction::ActRunning)
        return;

    if (m_clock.elapsed() - action.m_started_at_ms < DurationMs(action))
        return;

    SetDeviceStatus(deviceName, "idle");
    SwitchActionState(action, RobotAction::ActFinished, "Fake 定时动作完成");
}

void FakeRetailActuator::LoopVisionDetectByProduct(RobotAction& action)
{
    if (action.m_sta == RobotAction::ActFinished || action.m_sta == RobotAction::ActFailure)
        return;

    if (!m_vision_service)
    {
        SwitchActionState(action, RobotAction::ActFailure, "未注入视觉服务");
        return;
    }

    if (action.m_sta == RobotAction::ActBegin)
    {
        VisionRequest request;
        request.m_request_id = action.m_args.value("request_id").toString();
        if (request.m_request_id.isEmpty())
            request.m_request_id = action.m_name + QString::number(m_clock.elapsed());
        request.m_product_id = action.m_args.value("product_id").toString();
        request.m_slot_id = action.m_mission_args ? action.m_mission_args->value("slot_id").toString() : QString();
        request.m_row = action.m_args.value("row").toInt();
        request.m_col = action.m_args.value("col").toInt();
        request.m_observe_profile = action.m_args.value("observe_profile").toString();
        action.m_args["request_id"] = request.m_request_id;
        action.m_started_at_ms = m_clock.elapsed();
        if (!m_vision_service->RequestDetection(request))
        {
            SwitchActionState(action, RobotAction::ActFailure, "视觉请求发送失败");
            return;
        }
        SwitchActionState(action, RobotAction::ActRunning, "等待视觉结果");
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
            SwitchActionState(action, RobotAction::ActFailure, "视觉识别失败: " + result.error_message);
            return;
        }
        if (action.m_mission_args)
        {
            (*action.m_mission_args)["vision_success"] = true;
            (*action.m_mission_args)["detected_pose"] = PoseToMap(result.m_detected_pose);
            (*action.m_mission_args)["confidence"] = result.m_confidence;
        }
        SwitchActionState(action, RobotAction::ActFinished, "视觉识别完成");
        return;
    }

    if (m_clock.elapsed() - action.m_started_at_ms > m_config.m_interfaces.m_vision.m_timeout_ms)
        SwitchActionState(action, RobotAction::ActFailure, "视觉识别超时");
}

void FakeRetailActuator::SwitchActionState(RobotAction& action, RobotAction::ActionSta nextSta, const QString& reason)
{
    action.m_sta = nextSta;
    emit LogMessage(QString("Fake 动作状态切换: %1 -> %2，原因: %3")
                        .arg(action.m_name)
                        .arg(nextSta)
                        .arg(reason));
}

int FakeRetailActuator::DurationMs(const RobotAction& action) const
{
    Q_UNUSED(action);
    return 150;
}

QVariantMap FakeRetailActuator::PoseToMap(const Pose& pose) const
{
    return {{"x", pose.m_x}, {"y", pose.m_y}, {"z", pose.m_z}, {"roll", pose.m_roll}, {"pitch", pose.m_pitch}, {"yaw", pose.m_yaw}};
}

void FakeRetailActuator::SetDeviceStatus(const QString& deviceName, const QString& status)
{
    if (deviceName == "chassis") m_chassis->SetStatus(status);
    else if (deviceName == "lift") m_lift->SetStatus(status);
    else if (deviceName == "waist") m_waist->SetStatus(status);
    else if (deviceName == "neck") m_neck->SetStatus(status);
    else if (deviceName == "left_arm") m_left_arm->SetStatus(status);
    else if (deviceName == "right_arm") m_right_arm->SetStatus(status);
    else if (deviceName == "dual_arm") { m_left_arm->SetStatus(status); m_right_arm->SetStatus(status); }
    else if (deviceName == "left_gripper") m_left_gripper->SetStatus(status);
    else if (deviceName == "right_gripper") m_right_gripper->SetStatus(status);
    else if (deviceName == "both_grippers") { m_left_gripper->SetStatus(status); m_right_gripper->SetStatus(status); }
}

} // namespace asd_retail
