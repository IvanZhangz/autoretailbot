// 文件说明：实现 Fake 动作执行器。
#include "FakeRetailActuator.h"

#include <QVariant>

FakeSimpleDevice::FakeSimpleDevice(QString name)
    : m_name(std::move(name))
    , m_status("idle")
{
}

void FakeSimpleDevice::loop()
{
    // Fake 设备暂无底层通讯，保留 loop 入口以匹配真实设备轮询结构。
}

QString FakeSimpleDevice::statusText() const
{
    return m_name + ":" + m_status;
}

void FakeSimpleDevice::setStatus(const QString& text)
{
    m_status = text;
}

FakeRetailActuator::FakeRetailActuator(QObject* parent)
    : ActionActuator(parent)
{
}

bool FakeRetailActuator::init(const RobotConfig& config)
{
    m_config = config;
    m_clock.start();
    m_chassis = std::make_unique<FakeSimpleDevice>("chassis");
    m_leftArm = std::make_unique<FakeSimpleDevice>("left_arm");
    m_rightArm = std::make_unique<FakeSimpleDevice>("right_arm");
    m_lift = std::make_unique<FakeSimpleDevice>("lift");
    m_waist = std::make_unique<FakeSimpleDevice>("waist");
    m_neck = std::make_unique<FakeSimpleDevice>("neck");
    m_leftGripper = std::make_unique<FakeSimpleDevice>("left_gripper");
    m_rightGripper = std::make_unique<FakeSimpleDevice>("right_gripper");
    emit logMessage("FakeRetailActuator 初始化完成");
    return true;
}

void FakeRetailActuator::loopDev()
{
    m_chassis->loop();
    m_leftArm->loop();
    m_rightArm->loop();
    m_lift->loop();
    m_waist->loop();
    m_neck->loop();
    m_leftGripper->loop();
    m_rightGripper->loop();
}

// Fake 动作状态机：Begin 记录开始时间，Running 等待耗时结束，然后 Finished。
void FakeRetailActuator::loopAction(RobotAction& action)
{
    if (action.sta == RobotAction::ActFinished || action.sta == RobotAction::ActFailure)
        return;

    if (action.sta == RobotAction::ActBegin)
    {
        action.bindMissionArgs(action.missionArgs);
        action.startedAtMs = m_clock.elapsed();
        action.sta = RobotAction::ActRunning;
        emit logMessage("动作启动: " + action.name);
        return;
    }

    if (action.sta != RobotAction::ActRunning)
        return;

    if (m_clock.elapsed() - action.startedAtMs < durationMs(action))
        return;

    finishAction(action);
}

bool FakeRetailActuator::isErr() const
{
    return m_error;
}

QString FakeRetailActuator::errStr() const
{
    return m_errorText;
}

int FakeRetailActuator::durationMs(const RobotAction& action) const
{
    Q_UNUSED(action);
    return 150;
}

QVariantMap FakeRetailActuator::poseToMap(const Pose& pose) const
{
    return {{"x", pose.x}, {"y", pose.y}, {"z", pose.z}, {"roll", pose.roll}, {"pitch", pose.pitch}, {"yaw", pose.yaw}};
}

// 根据动作类型做完成后的副作用；大多数动作只需要标记完成。
void FakeRetailActuator::finishAction(RobotAction& action)
{
    switch (action.type)
    {
    case RobotAction::VisionDetectByProduct:
    {
        const int row = action.args.value("row").toInt();
        const int col = action.args.value("col").toInt();
        // Fake 视觉：根据货架行高和列号生成一个固定的识别位姿。
        Pose detected;
        detected.x = 0.35;
        detected.y = 0.02 * col;
        detected.z = m_config.shelfLayout.rowHeights.value(row, 0.25);
        if (action.missionArgs)
        {
            (*action.missionArgs)["vision_success"] = true;
            (*action.missionArgs)["detected_pose"] = poseToMap(detected);
            (*action.missionArgs)["confidence"] = 0.98;
        }
        break;
    }
    default:
        break;
    }
    action.sta = RobotAction::ActFinished;
    emit logMessage("动作完成: " + action.name);
}
