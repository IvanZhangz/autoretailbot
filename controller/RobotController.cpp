// 文件说明：实现机器人控制器单例和主轮询状态机。
#include "RobotController.h"

#include <QDateTime>
#include <QTimerEvent>

RobotController& RobotController::instance()
{
    static RobotController controller;
    return controller;
}

RobotController::RobotController(QObject* parent)
    : QObject(parent)
{
    m_mission.clear();
}

bool RobotController::init(const QString& configDir, QString* errorMessage)
{
    emit logMessage("Controller 初始化，配置目录: " + configDir);
    ConfigLoader loader;
    if (!loader.load(configDir, m_config, errorMessage))
    {
        setState("错误");
        return false;
    }

    m_actuator = ActuatorFactory::create(m_config.factories);
    connect(m_actuator.get(), &ActionActuator::logMessage, this, &RobotController::logMessage);
    if (!m_actuator->init(m_config))
    {
        if (errorMessage) *errorMessage = m_actuator->errStr();
        setState("错误");
        return false;
    }

    m_tabletService = std::make_unique<TabletService>();
    m_voiceService = std::make_unique<VoiceService>();
    connect(m_tabletService.get(), &TabletService::logMessage, this, &RobotController::logMessage);
    connect(m_voiceService.get(), &VoiceService::logMessage, this, &RobotController::logMessage);
    m_tabletService->start(m_config.interfaces);
    m_voiceService->start(m_config.interfaces);

    RobotMission homeMission;
    appendHomeBeat(homeMission);
    homeMission.orderId = "startup_home";
    homeMission.isFinished = false;
    homeMission.isEnd = false;
    m_mission = homeMission;
    m_currStep = 0;

    if (m_timerId == 0)
        m_timerId = startTimer(m_config.taskPolicy.loopIntervalMs);
    m_initialized = true;
    setState("初始化中");
    return true;
}

QStringList RobotController::productIds() const
{
    return m_config.products.keys();
}

void RobotController::submitDebugOrder(const QString& productId)
{
    if (!m_tabletService) return;
    Order order;
    order.orderId = "debug_" + QDateTime::currentDateTimeUtc().toString("yyyyMMdd_hhmmss_zzz");
    order.source = "qt_debug";
    order.items.append({productId, 1});
    m_tabletService->enqueueDebugOrder(order);
}

void RobotController::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);
    loop();
}

void RobotController::loop()
{
    loopDevices();
    loopServices();
    acceptPendingOrders();
    startNextMissionIfIdle();
    loop_workSM();
}

void RobotController::loopDevices()
{
    if (m_actuator)
        m_actuator->loopDev();
}

void RobotController::loopServices()
{
    if (m_tabletService) m_tabletService->loop();
    if (m_voiceService) m_voiceService->loop();
}

void RobotController::acceptPendingOrders()
{
    while (m_tabletService && m_tabletService->hasOrder())
        m_orderQueue.enqueue(m_tabletService->takeOrder());
    while (m_voiceService && m_voiceService->hasOrder())
        m_orderQueue.enqueue(m_voiceService->takeOrder());
}

void RobotController::startNextMissionIfIdle()
{
    if (!m_initialized || !m_mission.isFinished || !m_mission.isEnd || m_orderQueue.isEmpty())
        return;

    const Order order = m_orderQueue.dequeue();
    QString error;
    RobotMission mission;
    if (!buildMissionFromOrder(order, mission, &error))
    {
        TaskResult result{false, order.orderId, "MISSION_BUILD_FAILED", error};
        m_tabletService->reportTaskResult(result);
        m_voiceService->reportTaskResult(result);
        return;
    }
    m_mission = mission;
    m_currStep = 0;
    setState("执行订单 " + order.orderId);
}

void RobotController::loop_workSM()
{
    if (m_mission.isFinished || m_mission.isEnd || !m_actuator)
        return;

    if (m_currStep < m_mission.beats.size())
    {
        RobotBeat& beat = m_mission.beats[m_currStep];
        bool stepFinished = true;
        for (RobotAction& action : beat.actions)
        {
            action.bindMissionArgs(&m_mission.args);
            m_actuator->loopAction(action);
            if (action.sta == RobotAction::ActFailure)
            {
                m_mission.errStr = "动作失败: " + action.name;
                finishMission(false, m_mission.errStr);
                return;
            }
            if (action.sta != RobotAction::ActFinished)
                stepFinished = false;
        }
        if (stepFinished)
        {
            emit logMessage("Beat完成: " + beat.name);
            ++m_currStep;
        }
        return;
    }

    finishMission(true, "任务完成");
}

ColumnStation RobotController::stationForSlot(const ShelfSlot& slot) const
{
    if (slot.hasDedicatedStation)
        return slot.dedicatedStation;
    return m_config.shelfLayout.columnStations.value(slot.col);
}

QVariantMap RobotController::poseArgs(const Pose& pose) const
{
    return {{"x", pose.x}, {"y", pose.y}, {"z", pose.z}, {"roll", pose.roll}, {"pitch", pose.pitch}, {"yaw", pose.yaw}};
}

QVariantMap RobotController::stationArgs(const ColumnStation& station) const
{
    return {{"station_id", station.stationId}, {"x", station.pose.x}, {"y", station.pose.y}, {"z", station.pose.z}, {"yaw", station.pose.yaw}};
}

bool RobotController::buildMissionFromOrder(const Order& order, RobotMission& mission, QString* errorMessage)
{
    if (order.items.isEmpty())
    {
        if (errorMessage) *errorMessage = "订单为空";
        return false;
    }
    const QString productId = order.items.first().productId;
    if (!m_config.products.contains(productId))
    {
        if (errorMessage) *errorMessage = "未知商品: " + productId;
        return false;
    }

    InventoryItem inventory;
    bool found = false;
    for (const InventoryItem& item : m_config.inventory)
    {
        if (item.enabled && item.quantity > 0 && item.productId == productId)
        {
            inventory = item;
            found = true;
            break;
        }
    }
    if (!found || !m_config.shelfLayout.shelfSlots.contains(inventory.slotId))
    {
        if (errorMessage) *errorMessage = "没有可用货位: " + productId;
        return false;
    }

    const ProductConfig product = m_config.products.value(productId);
    const ShelfSlot slot = m_config.shelfLayout.shelfSlots.value(inventory.slotId);
    const ColumnStation station = stationForSlot(slot);
    const bool dualArm = product.packageType == "boxed";

    mission.clear();
    mission.isFinished = false;
    mission.isEnd = false;
    mission.orderId = order.orderId;
    mission.args["product_id"] = productId;
    mission.args["slot_id"] = slot.slotId;

    RobotBeat prepare;
    prepare.name = "到达货架并准备观察/抓取";
    prepare.actions.append({RobotAction::AgvMoveToStation, "AGV到站点", stationArgs(station)});
    prepare.actions.append({RobotAction::LiftMoveToHeight, "升降到行高", {{"height", m_config.shelfLayout.rowHeights.value(slot.row)}}});
    prepare.actions.append({RobotAction::WaistRotateTo, "腰部转向货架", {{"yaw", 0.0}}});
    prepare.actions.append({RobotAction::NeckMoveTo, "头部观察货位", {{"profile", slot.observeProfile}}});
    prepare.actions.append({dualArm ? RobotAction::DualArmMoveL : RobotAction::RightArmMoveL,
                            dualArm ? "双臂到预抓取" : "右臂到预抓取",
                            poseArgs(dualArm ? slot.dualPrePickPose : slot.rightPrePickPose)});
    mission.beats.append(prepare);

    RobotBeat vision;
    vision.name = "视觉识别";
    vision.actions.append({RobotAction::VisionDetectByProduct, "识别目标商品", {{"product_id", productId}, {"row", slot.row}, {"col", slot.col}}});
    mission.beats.append(vision);

    RobotBeat open;
    open.name = "打开夹爪";
    open.actions.append({dualArm ? RobotAction::BothGrippersOpen : RobotAction::RightGripperOpen, dualArm ? "左右夹爪打开" : "右夹爪打开"});
    mission.beats.append(open);

    RobotBeat movePick;
    movePick.name = "移动到视觉抓取位";
    movePick.actions.append({dualArm ? RobotAction::DualArmMoveL : RobotAction::RightArmMoveL, dualArm ? "双臂到抓取位" : "右臂到抓取位", {{"pose_key", "detected_pose"}}});
    mission.beats.append(movePick);

    RobotBeat close;
    close.name = "闭合夹爪";
    close.actions.append({dualArm ? RobotAction::BothGrippersClose : RobotAction::RightGripperClose, dualArm ? "左右夹爪闭合" : "右夹爪闭合"});
    mission.beats.append(close);

    RobotBeat carry;
    carry.name = "进入携物姿态";
    carry.actions.append({dualArm ? RobotAction::DualArmMoveCarry : RobotAction::RightArmMoveJNamed, dualArm ? "双臂携物" : "右臂携物", {{"pose_name", "right_carry"}}});
    mission.beats.append(carry);

    RobotBeat back;
    back.name = "返回售卖点并准备交付";
    back.actions.append({RobotAction::AgvMoveToStation, "AGV返回售卖点", stationArgs(m_config.home.sellingHome)});
    back.actions.append({RobotAction::LiftMoveToHeight, "升降到柜台高度", {{"height", m_config.home.counterPlacePose.z}}});
    back.actions.append({RobotAction::WaistRotateTo, "腰部转向柜台", {{"yaw", m_config.home.counterPlacePose.yaw}}});
    back.actions.append({RobotAction::NeckMoveTo, "头部回交付观察", {{"yaw", 0.0}, {"pitch", 0.0}}});
    mission.beats.append(back);

    RobotBeat place;
    place.name = "柜台放置";
    place.actions.append({dualArm ? RobotAction::DualArmMovePlace : RobotAction::RightArmMoveL, dualArm ? "双臂到放置位" : "右臂到放置位", poseArgs(m_config.home.counterPlacePose)});
    mission.beats.append(place);

    RobotBeat release;
    release.name = "释放商品";
    release.actions.append({dualArm ? RobotAction::BothGrippersOpen : RobotAction::RightGripperOpen, dualArm ? "左右夹爪释放" : "右夹爪释放"});
    mission.beats.append(release);

    appendHomeBeat(mission);
    return true;
}

void RobotController::appendHomeBeat(RobotMission& mission)
{
    RobotBeat home;
    home.name = "回home并恢复空闲姿态";
    home.actions.append({RobotAction::AgvMoveToStation, "AGV回home", stationArgs(m_config.home.sellingHome)});
    home.actions.append({RobotAction::LeftArmMoveNamed, "左臂home", {{"pose_name", m_config.home.leftArmHome}}});
    home.actions.append({RobotAction::RightArmMoveNamed, "右臂home", {{"pose_name", m_config.home.rightArmHome}}});
    home.actions.append({RobotAction::WaistRotateTo, "腰部home", {{"yaw", m_config.home.waistHomeYaw}}});
    home.actions.append({RobotAction::NeckMoveTo, "颈部home", {{"yaw", m_config.home.neckHomeYaw}, {"pitch", m_config.home.neckHomePitch}}});
    home.actions.append({RobotAction::LiftMoveToHeight, "升降home", {{"height", m_config.home.liftHomeHeight}}});
    home.actions.append({m_config.home.leftGripperOpen ? RobotAction::LeftGripperOpen : RobotAction::LeftGripperClose, "左夹爪home"});
    home.actions.append({m_config.home.rightGripperOpen ? RobotAction::RightGripperOpen : RobotAction::RightGripperClose, "右夹爪home"});
    mission.beats.append(home);
}

void RobotController::finishMission(bool success, const QString& message)
{
    m_mission.isFinished = true;
    m_mission.isEnd = true;
    TaskResult result{success, m_mission.orderId, success ? QString() : "MISSION_FAILED", message};
    if (m_tabletService) m_tabletService->reportTaskResult(result);
    if (m_voiceService) m_voiceService->reportTaskResult(result);
    setState(success ? "空闲" : "错误");
}

void RobotController::setState(const QString& state)
{
    if (m_state == state) return;
    m_state = state;
    if (m_tabletService) m_tabletService->reportRobotState(state);
    if (m_voiceService) m_voiceService->reportRobotState(state);
    emit stateChanged(state);
    emit logMessage("状态切换: " + state);
}
