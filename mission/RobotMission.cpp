// 文件说明：实现 Mission/Beat/Action 的基础方法。
#include "RobotMission.h"

RobotAction::RobotAction(ActionType actionType, const QString& actionName, const QVariantMap& actionArgs)
    : type(actionType)
    , name(actionName)
    , args(actionArgs)
{
}

void RobotAction::bindMissionArgs(QVariantMap* argsPtr)
{
    missionArgs = argsPtr;
}

void RobotMission::clear()
{
    isFinished = true;
    isEnd = true;
    orderId.clear();
    errStr.clear();
    beats.clear();
    args.clear();
}
