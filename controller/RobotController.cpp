// 文件说明：实现机器人控制器单例和主轮询状态机
#include "RobotController.h"

#include <QDateTime>
#include <QTimerEvent>

// 使用函数内 static，C++ 会保证线程安全初始化，避免全局对象初始化顺序问题。

namespace asd_retail
{
RobotController& RobotController::Instance()
{
    static RobotController controller;
    return controller;
}

RobotController::RobotController(QObject* parent)
    : QObject(parent)
{
    m_mission.Clear();
}

// 初始化顺序很重要：先读配置，再按配置创建硬件和服务，最后启动定时器。
bool RobotController::Init(const QString& config_dir, QString* error_message)
{
    emit LogMessage("Controller 初始化，配置目录: " + config_dir);
    // 1) 从 config_dir 读取所有 JSON 配置，填充 m_config。
    ConfigLoader loader;
    if (!loader.Load(config_dir, m_config, error_message))
    {
        SetState("错误");
        return false;
    }

    // 2) 通过工厂创建视觉/平板/语音服务：视觉和语音真实接入走 ROS2，平板真实接入走 gRPC。
    m_vision_service = VisionServiceFactory::Create(m_config.m_factories);
    m_tablet_service = TabletServiceFactory::Create(m_config.m_factories);
    m_voice_service = VoiceServiceFactory::Create(m_config.m_factories);
    connect(m_vision_service.get(), &VisionService::LogMessage, this, &RobotController::LogMessage);
    connect(m_tablet_service.get(), &TabletService::LogMessage, this, &RobotController::LogMessage);
    connect(m_voice_service.get(), &VoiceService::LogMessage, this, &RobotController::LogMessage);
    if (!m_vision_service->Start(m_config.m_interfaces) || !m_tablet_service->Start(m_config.m_interfaces) || !m_voice_service->Start(m_config.m_interfaces))
    {
        if (error_message) *error_message = "外部服务初始化失败";
        SetState("错误");
        return false;
    }

    // 3) 通过工厂创建动作执行器；真实执行器会通过注入的 VisionService 请求 ROS2 视觉结果。
    m_actuator = ActuatorFactory::Create(m_config.m_factories);
    m_actuator->SetVisionService(m_vision_service.get());
    connect(m_actuator.get(), &ActionActuator::LogMessage, this, &RobotController::LogMessage);
    if (!m_actuator->Init(m_config))
    {
        if (error_message) *error_message = m_actuator->ErrStr();
        SetState("错误");
        return false;
    }

    // 4) 启动后先执行一次回 home 任务，确保机器人从安全姿态开始接单。
    RobotMission homeMission;
    AppendHomeBeat(homeMission);
    homeMission.m_order_id = "startup_home";
    homeMission.m_is_finished = false;
    homeMission.m_is_end = false;
    m_mission = homeMission;
    m_curr_step = 0;

    // 5) 启动 Qt 定时器；后续 timerEvent 会周期调用 Loop()。
    if (m_timer_id == 0)
        m_timer_id = startTimer(m_config.m_task_policy.m_loop_interval_ms);
    m_initialized = true;
    SetState("初始化中");
    return true;
}

QStringList RobotController::ProductIds() const
{
    return m_config.m_products.keys();
}

void RobotController::SubmitDebugOrder(const QString& product_id)
{
    if (!m_tablet_service) return;
    Order order;
    order.m_order_id = "debug_" + QDateTime::currentDateTimeUtc().toString("yyyyMMdd_hhmmss_zzz");
    order.m_source = "qt_debug";
    order.m_items.append({product_id, 1});
    m_tablet_service->EnqueueDebugOrder(order);
}

void RobotController::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);
    Loop();
}

// 主循环保持短小，便于理解每一轮先做什么、后做什么。
void RobotController::Loop()
{
    LoopDevices();
    LoopServices();
    AcceptPendingOrders();
    StartNextMissionIfIdle();
    LoopWorkSM();
}

void RobotController::LoopDevices()
{
    if (m_actuator)
        m_actuator->LoopDev();
}

void RobotController::LoopServices()
{
    if (m_vision_service) m_vision_service->Loop();
    if (m_tablet_service) m_tablet_service->Loop();
    if (m_voice_service) m_voice_service->Loop();
}

// 外部服务各自维护输入队列；这里统一搬运到 Controller 订单队列。
void RobotController::AcceptPendingOrders()
{
    while (m_tablet_service && m_tablet_service->HasOrder())
        m_order_queue.enqueue(m_tablet_service->TakeOrder());
    while (m_voice_service && m_voice_service->HasOrder())
        m_order_queue.enqueue(m_voice_service->TakeOrder());
}

// 只有当前任务已经结束并且队列非空，才会取出一个订单开始执行。
void RobotController::StartNextMissionIfIdle()
{
    if (!m_initialized || !m_mission.m_is_finished || !m_mission.m_is_end || m_order_queue.isEmpty())
        return;

    const Order order = m_order_queue.dequeue();
    QString error;
    RobotMission mission;
    if (!BuildMissionFromOrder(order, mission, &error))
    {
        TaskResult result{false, order.m_order_id, "MISSION_BUILD_FAILED", error};
        m_tablet_service->ReportTaskResult(result);
        m_voice_service->ReportTaskResult(result);
        return;
    }
    m_mission = mission;
    m_curr_step = 0;
    SetState("执行订单 " + order.m_order_id);
}

// 工作状态机：beat 顺序执行，beat 内 action 并行推进。
void RobotController::LoopWorkSM()
{
    if (m_mission.m_is_finished || m_mission.m_is_end || !m_actuator)
        return;

    if (m_curr_step < m_mission.m_beats.size())
    {
        // 当前 beat 中所有 action 都完成后，m_curr_step 才会加一进入下一个 beat。
        RobotBeat& beat = m_mission.m_beats[m_curr_step];
        bool stepFinished = true;
        for (RobotAction& action : beat.m_actions)
        {
            // mission.m_args 是同一任务内动作共享的数据区，例如视觉动作写入 detected_pose。
            action.BindMissionArgs(&m_mission.m_args);
            m_actuator->LoopAction(action);
            if (action.m_sta == RobotAction::ActFailure)
            {
                m_mission.ErrStr = "动作失败: " + action.m_name;
                if (m_recovering_from_failure)
                    FinishMission(false, m_recovery_failure_message + "；回home失败: " + action.m_name);
                else
                    BeginFailureRecovery(m_mission.ErrStr);
                return;
            }
            if (action.m_sta != RobotAction::ActFinished)
                stepFinished = false;
        }
        if (stepFinished)
        {
            emit LogMessage("Beat完成: " + beat.m_name);
            ++m_curr_step;
        }
        return;
    }

    if (m_recovering_from_failure)
        FinishMission(false, m_recovery_failure_message);
    else
        FinishMission(true, "任务完成");
}

ColumnStation RobotController::StationForSlot(const ShelfSlot& slot) const
{
    if (slot.m_has_dedicated_station)
        return slot.m_dedicated_station;
    return m_config.m_shelf_layout.m_column_stations.value(slot.m_col);
}

QVariantMap RobotController::PoseArgs(const Pose& pose) const
{
    return {{"x", pose.m_x}, {"y", pose.m_y}, {"z", pose.m_z}, {"roll", pose.m_roll}, {"pitch", pose.m_pitch}, {"yaw", pose.m_yaw}};
}

QVariantMap RobotController::StationArgs(const ColumnStation& station) const
{
    return {{"station_id", station.m_station_id}, {"x", station.m_pose.m_x}, {"y", station.m_pose.m_y}, {"z", station.m_pose.m_z}, {"yaw", station.m_pose.m_yaw}};
}

// 订单转任务：校验商品 -> 查库存货位 -> 选择站点/抓取方式 -> 生成节拍。
bool RobotController::BuildMissionFromOrder(const Order& order, RobotMission& mission, QString* error_message)
{
    if (order.m_items.isEmpty())
    {
        if (error_message) *error_message = "订单为空";
        return false;
    }
    const QString product_id = order.m_items.first().m_product_id;
    if (!m_config.m_products.contains(product_id))
    {
        if (error_message) *error_message = "未知商品: " + product_id;
        return false;
    }

    InventoryItem inventory;
    bool found = false;
    for (const InventoryItem& item : m_config.m_inventory)
    {
        if (item.m_enabled && item.m_quantity > 0 && item.m_product_id == product_id)
        {
            inventory = item;
            found = true;
            break;
        }
    }
    if (!found || !m_config.m_shelf_layout.m_shelf_slots.contains(inventory.m_slot_id))
    {
        if (error_message) *error_message = "没有可用货位: " + product_id;
        return false;
    }

    const ProductConfig product = m_config.m_products.value(product_id);
    const ShelfSlot slot = m_config.m_shelf_layout.m_shelf_slots.value(inventory.m_slot_id);
    const ColumnStation station = StationForSlot(slot);
    // boxed 包装默认使用双臂；naked 等非盒装商品默认右臂单手抓取。
    const bool dualArm = product.m_package_type == "boxed";

    mission.Clear();
    mission.m_is_finished = false;
    mission.m_is_end = false;
    mission.m_order_id = order.m_order_id;
    mission.m_args["product_id"] = product_id;
    mission.m_args["slot_id"] = slot.m_slot_id;

    // Beat 1：移动到货架列/货位，并把身体、头部和手臂移动到预抓取姿态。
    RobotBeat prepare;
    prepare.m_name = "到达货架并准备观察/抓取";
    prepare.m_actions.append({RobotAction::AgvMoveToStation, "AGV到站点", StationArgs(station)});
    prepare.m_actions.append({RobotAction::LiftMoveToHeight, "升降到行高", {{"height", m_config.m_shelf_layout.m_row_heights.value(slot.m_row)}}});
    prepare.m_actions.append({RobotAction::WaistRotateTo, "腰部转向货架", {{"yaw", 0.0}}});
    prepare.m_actions.append({RobotAction::NeckMoveTo, "头部观察货位", {{"profile", slot.m_observe_profile}}});
    prepare.m_actions.append({dualArm ? RobotAction::DualArmMoveL : RobotAction::RightArmMoveL,
                            dualArm ? "双臂到预抓取" : "右臂到预抓取",
                            PoseArgs(dualArm ? slot.m_dual_pre_pick_pose : slot.m_right_pre_pick_pose)});
    // 夹爪打开不依赖视觉结果，可与到站、升降、头部观察、手臂预抓取并行执行。
    prepare.m_actions.append({dualArm ? RobotAction::BothGrippersOpen : RobotAction::RightGripperOpen, dualArm ? "左右夹爪打开" : "右夹爪打开"});
    mission.m_beats.append(prepare);

    // Beat 2：视觉识别目标，Fake 执行器会把 detected_pose 写入 mission.m_args。
    RobotBeat vision;
    vision.m_name = "视觉识别";
    vision.m_actions.append({RobotAction::VisionDetectByProduct, "识别目标商品", {{"product_id", product_id}, {"row", slot.m_row}, {"col", slot.m_col}}});
    mission.m_beats.append(vision);

    // Beat 3：移动到视觉识别出的抓取位。
    RobotBeat movePick;
    movePick.m_name = "移动到视觉抓取位";
    movePick.m_actions.append({dualArm ? RobotAction::DualArmMoveL : RobotAction::RightArmMoveL, dualArm ? "双臂到抓取位" : "右臂到抓取位", {{"pose_key", "detected_pose"}}});
    mission.m_beats.append(movePick);

    // Beat 4：闭合夹爪夹住商品。
    RobotBeat close;
    close.m_name = "闭合夹爪";
    close.m_actions.append({dualArm ? RobotAction::BothGrippersClose : RobotAction::RightGripperClose, dualArm ? "左右夹爪闭合" : "右夹爪闭合"});
    mission.m_beats.append(close);

    // Beat 5：把商品收回到携物安全姿态。
    RobotBeat carry;
    carry.m_name = "进入携物姿态";
    carry.m_actions.append({dualArm ? RobotAction::DualArmMoveCarry : RobotAction::RightArmMoveJNamed, dualArm ? "双臂携物" : "右臂携物", {{"pose_name", "right_carry"}}});
    mission.m_beats.append(carry);

    // Beat 6：AGV 回到售卖/交付点，并调整身体准备放置。
    RobotBeat back;
    back.m_name = "返回售卖点并准备交付";
    back.m_actions.append({RobotAction::AgvMoveToStation, "AGV返回售卖点", StationArgs(m_config.m_home.m_selling_home)});
    back.m_actions.append({RobotAction::LiftMoveToHeight, "升降到柜台高度", {{"height", m_config.m_home.m_counter_place_pose.m_z}}});
    back.m_actions.append({RobotAction::WaistRotateTo, "腰部转向柜台", {{"yaw", m_config.m_home.m_counter_place_pose.m_yaw}}});
    back.m_actions.append({RobotAction::NeckMoveTo, "头部回交付观察", {{"yaw", 0.0}, {"pitch", 0.0}}});
    mission.m_beats.append(back);

    // Beat 7：移动到柜台放置位。
    RobotBeat place;
    place.m_name = "柜台放置";
    place.m_actions.append({dualArm ? RobotAction::DualArmMovePlace : RobotAction::RightArmMoveL, dualArm ? "双臂到放置位" : "右臂到放置位", PoseArgs(m_config.m_home.m_counter_place_pose)});
    mission.m_beats.append(place);

    // Beat 8：打开夹爪释放商品。
    RobotBeat release;
    release.m_name = "释放商品";
    release.m_actions.append({dualArm ? RobotAction::BothGrippersOpen : RobotAction::RightGripperOpen, dualArm ? "左右夹爪释放" : "右夹爪释放"});
    mission.m_beats.append(release);

    AppendHomeBeat(mission);
    return true;
}

// 归位节拍会同时让底盘、左右臂、腰、颈、升降和夹爪回到配置中的 home。
void RobotController::AppendHomeBeat(RobotMission& mission)
{
    RobotBeat home;
    home.m_name = "回home并恢复空闲姿态";
    home.m_actions.append({RobotAction::AgvMoveToStation, "AGV回home", StationArgs(m_config.m_home.m_selling_home)});
    home.m_actions.append({RobotAction::LeftArmMoveNamed, "左臂home", {{"pose_name", m_config.m_home.m_left_arm_home}}});
    home.m_actions.append({RobotAction::RightArmMoveNamed, "右臂home", {{"pose_name", m_config.m_home.m_right_arm_home}}});
    home.m_actions.append({RobotAction::WaistRotateTo, "腰部home", {{"yaw", m_config.m_home.m_waist_home_yaw}}});
    home.m_actions.append({RobotAction::NeckMoveTo, "颈部home", {{"yaw", m_config.m_home.m_neck_home_yaw}, {"pitch", m_config.m_home.m_neck_home_pitch}}});
    home.m_actions.append({RobotAction::LiftMoveToHeight, "升降home", {{"height", m_config.m_home.m_lift_home_height}}});
    home.m_actions.append({m_config.m_home.m_left_gripper_open ? RobotAction::LeftGripperOpen : RobotAction::LeftGripperClose, "左夹爪home"});
    home.m_actions.append({m_config.m_home.m_right_gripper_open ? RobotAction::RightGripperOpen : RobotAction::RightGripperClose, "右夹爪home"});
    mission.m_beats.append(home);
}

// 任务结束后统一上报结果，并把状态切回“空闲”或“错误”。
void RobotController::FinishMission(bool success, const QString& message)
{
    if (success)
        ConsumeMissionInventory();

    m_recovering_from_failure = false;
    m_recovery_failure_message.clear();
    m_mission.m_is_finished = true;
    m_mission.m_is_end = true;
    TaskResult result{success, m_mission.m_order_id, success ? QString() : "MISSION_FAILED", message};
    if (m_tablet_service) m_tablet_service->ReportTaskResult(result);
    if (m_voice_service) m_voice_service->ReportTaskResult(result);
    SetState(success ? "空闲" : "错误");
}

void RobotController::ConsumeMissionInventory()
{
    const QString productId = m_mission.m_args.value("product_id").toString();
    const QString slotId = m_mission.m_args.value("slot_id").toString();
    if (productId.isEmpty() || slotId.isEmpty())
        return;

    for (InventoryItem& item : m_config.m_inventory)
    {
        if (item.m_enabled && item.m_product_id == productId && item.m_slot_id == slotId)
        {
            if (item.m_quantity <= 0)
            {
                emit LogMessage(QString("库存扣减跳过: product=%1 slot=%2 quantity=%3")
                                    .arg(productId, slotId)
                                    .arg(item.m_quantity));
                return;
            }

            --item.m_quantity;
            emit LogMessage(QString("库存已扣减: product=%1 slot=%2 remaining=%3")
                                .arg(productId, slotId)
                                .arg(item.m_quantity));
            return;
        }
    }

    emit LogMessage(QString("库存扣减失败: 未找到 product=%1 slot=%2")
                        .arg(productId, slotId));
}

void RobotController::BeginFailureRecovery(const QString& failure_message)
{
    m_recovering_from_failure = true;
    m_recovery_failure_message = failure_message;

    RobotMission recoveryMission;
    recoveryMission.m_order_id = m_mission.m_order_id;
    recoveryMission.m_is_finished = false;
    recoveryMission.m_is_end = false;
    AppendHomeBeat(recoveryMission);

    m_mission = recoveryMission;
    m_curr_step = 0;
    emit LogMessage("任务失败，开始回home恢复: " + failure_message);
    SetState("故障恢复回home");
}

void RobotController::SetState(const QString& state)
{
    if (m_state == state) return;
    m_state = state;
    if (m_tablet_service) m_tablet_service->ReportRobotState(state);
    if (m_voice_service) m_voice_service->ReportRobotState(state);
    emit StateChanged(state);
    emit LogMessage("状态切换: " + state);
}

} // namespace asd_retail
