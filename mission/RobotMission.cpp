// 文件说明：实现 Mission/Beat/Action 的基础方法。
#include "RobotMission.h"


namespace asd_retail
{
RobotAction::RobotAction(ActionType actionType, const QString& actionName, const QVariantMap& actionArgs)
    : m_type(actionType)
    , m_name(actionName)
    , m_args(actionArgs)
{
}

void RobotAction::BindMissionArgs(QVariantMap* argsPtr)
{
    m_mission_args = argsPtr;
}

void RobotMission::Clear()
{
    m_is_finished = true;
    m_is_end = true;
    m_order_id.clear();
    ErrStr.clear();
    m_beats.clear();
    m_args.clear();
}

} // namespace asd_retail
