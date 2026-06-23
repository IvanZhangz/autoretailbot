// 文件说明：实现配置加载器，把多文件 JSON 配置转换为运行期 RobotConfig。
#include "ConfigLoader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>


namespace asd_retail
{
namespace
{
// 工具函数：打开一个 JSON 文件并要求根节点必须是对象。
QJsonObject ReadObject(const QString& path, QString* error_message)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        if (error_message) *error_message = "无法打开配置文件: " + path;
        return {};
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
    {
        if (error_message) *error_message = "配置文件不是 JSON 对象: " + path;
        return {};
    }
    return doc.object();
}

// 工具函数：把 JSON 中的 x/y/z/roll/pitch/yaw 转成 Pose。
Pose ParsePose(const QJsonObject& object)
{
    Pose pose;
    pose.m_x = object.value("x").toDouble();
    pose.m_y = object.value("y").toDouble();
    pose.m_z = object.value("z").toDouble();
    pose.m_roll = object.value("roll").toDouble();
    pose.m_pitch = object.value("pitch").toDouble();
    pose.m_yaw = object.value("yaw").toDouble();
    return pose;
}

// 工具函数：解析 AGV 站点；兼容 pose 和 chassis_pose 两种字段名。
ColumnStation ParseStation(const QJsonObject& object)
{
    ColumnStation station;
    station.m_col = object.value("col").toInt();
    station.m_station_id = object.value("station_id").toString();
    station.m_pose = ParsePose(object.value("pose").toObject(object.value("chassis_pose").toObject()));
    return station;
}

// 工具函数：解析arm
ArmDeviceConfig ParseArmDevice(const QJsonObject& object, const QString& default_name)
{
    ArmDeviceConfig config;
    config.m_name = object.value("name").toString(default_name);
    config.m_vendor = object.value("vendor").toString(config.m_vendor);
    config.m_ip = object.value("ip").toString();
    config.m_port = object.value("port").toInt();
    config.m_dof = object.value("dof").toInt(config.m_dof);
    config.m_enabled = object.value("enabled").toBool(true);
    return config;
}

// 工具函数：解析底盘设备
ChassisDeviceConfig ParseChassisDevice(const QJsonObject& object)
{
    ChassisDeviceConfig config;
    config.m_name = object.value("name").toString(config.m_name);
    config.m_vendor = object.value("vendor").toString();
    config.m_ip = object.value("ip").toString();
    config.m_port = object.value("port").toInt();
    config.m_slave_id = object.value("slave_id").toInt(config.m_slave_id);
    config.m_enabled = object.value("enabled").toBool(true);
    return config;
}

// 工具函数：解析简单设备
SimpleDeviceConfig ParseSimpleDevice(const QJsonObject& object, const QString& default_name, const QString& default_type)
{
    SimpleDeviceConfig config;
    config.m_name = object.value("name").toString(default_name);
    config.m_type = object.value("type").toString(default_type);
    config.m_vendor = object.value("vendor").toString();
    config.m_ip = object.value("ip").toString();
    config.m_port = object.value("port").toInt();
    config.m_device_id = object.value("device_id").toInt();
    config.m_enabled = object.value("enabled").toBool(true);
    config.m_position_tolerance = object.value("position_tolerance").toDouble(config.m_position_tolerance);
    return config;
}
}

bool ConfigLoader::Load(const QString& config_dir, RobotConfig& config, QString* error_message) const
{
    // 先加载到临时对象 loaded，避免中途失败时污染外部 config。
    RobotConfig loaded;
    if (!LoadStore(config_dir + "/store.json", loaded.m_store, error_message)) return false;
    if (!LoadFactories(config_dir + "/factories.json", loaded.m_factories, error_message)) return false;
    if (!LoadHardware(config_dir + "/hardware.json", loaded.m_hardware, error_message)) return false;
    if (!LoadInterfaces(config_dir + "/interfaces.json", loaded.m_interfaces, error_message)) return false;
    if (!LoadTaskPolicy(config_dir + "/task_policy.json", loaded.m_task_policy, error_message)) return false;
    if (!LoadShelfLayout(config_dir + "/shelf_layout.json", loaded.m_shelf_layout, error_message)) return false;
    if (!LoadProducts(config_dir + "/product_catalog.json", loaded.m_products, error_message)) return false;
    if (!LoadInventory(config_dir + "/inventory.json", loaded.m_inventory, error_message)) return false;
    if (!LoadHome(config_dir + "/home.json", loaded.m_home, error_message)) return false;
    config = loaded;
    return true;
}

bool ConfigLoader::LoadStore(const QString& path, StoreConfig& store, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    store.m_store_id = root.value("store_id").toString(store.m_store_id);
    store.m_robot_profile = root.value("robot_profile").toString(store.m_robot_profile);
    store.m_map_frame = root.value("map_frame").toString(store.m_map_frame);
    store.m_shelf_frame = root.value("shelf_frame").toString(store.m_shelf_frame);
    return true;
}

bool ConfigLoader::LoadFactories(const QString& path, FactoryConfig& factories, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    factories.m_actuator_factory = root.value("actuator_factory").toString(factories.m_actuator_factory);
    factories.m_tablet_factory = root.value("tablet_factory").toString(factories.m_tablet_factory);
    factories.m_voice_factory = root.value("voice_factory").toString(factories.m_voice_factory);
    factories.m_vision_factory = root.value("vision_factory").toString(factories.m_vision_factory);
    return true;
}

bool ConfigLoader::LoadHardware(const QString& path, HardwareConfig& hardware, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;

    const QJsonObject arms = root.value("arms").toObject();
    hardware.m_left_arm = ParseArmDevice(arms.value("left").toObject(), "left_arm");
    hardware.m_right_arm = ParseArmDevice(arms.value("right").toObject(), "right_arm");
    hardware.m_chassis = ParseChassisDevice(root.value("chassis").toObject());
    
    const QJsonObject simple = root.value("simple_devices").toObject();
    hardware.m_lift = ParseSimpleDevice(simple.value("lift").toObject(), "lift", "lift");
    hardware.m_waist = ParseSimpleDevice(simple.value("waist").toObject(), "waist", "waist");
    hardware.m_neck = ParseSimpleDevice(simple.value("neck").toObject(), "neck", "neck");
    hardware.m_left_gripper = ParseSimpleDevice(simple.value("left_gripper").toObject(), "left_gripper", "gripper");
    hardware.m_right_gripper = ParseSimpleDevice(simple.value("right_gripper").toObject(), "right_gripper", "gripper");
    
    auto validateArm =
        [error_message, &path](const ArmDeviceConfig& config) -> bool
    {
        if (!config.m_enabled) return true;
        if (config.m_vendor.isEmpty())
        {
            if (error_message)
                *error_message = "机械臂 vendor 为空: " + config.m_name + ", file=" + path;
            return false;
        }
        if (config.m_ip.isEmpty())
        {
            if (error_message)
                *error_message = "机械臂 ip 为空: " + config.m_name + ", file=" + path;
            return false;
        }
        if (config.m_dof <= 0)
        {
            if (error_message)
                *error_message = "机械臂 dof 非法: " + config.m_name + ", file=" + path;
            return false;
        }
        return true;
    };

    if (!validateArm(hardware.m_left_arm)) return false;
    if (!validateArm(hardware.m_right_arm)) return false;
    if (hardware.m_chassis.m_enabled)
    {
        if (hardware.m_chassis.m_vendor.isEmpty())
        {
            if (error_message)
                *error_message = "底盘 vendor 为空: " + path;
            return false;
        }
        if (hardware.m_chassis.m_ip.isEmpty())
        {
            if (error_message)
                *error_message = "底盘 ip 为空: " + path;
            return false;
        }
    }

    const QList<SimpleDeviceConfig> simpleDevices = {hardware.m_lift, hardware.m_waist, hardware.m_neck, hardware.m_left_gripper, hardware.m_right_gripper};

    for (const SimpleDeviceConfig& config : simpleDevices)
    {
        if (!config.m_enabled) continue;

        if (config.m_type.isEmpty())
        {
            if (error_message)
                *error_message = "简单设备 type 为空: " + config.m_name + ", file=" + path;
            return false;
        }
        if (config.m_vendor.isEmpty())
        {
            if (error_message)
                *error_message = "简单设备 vendor 为空: " + config.m_name + ", file=" + path;
            return false;
        }
    }

    return true;
}

bool ConfigLoader::LoadInterfaces(const QString& path, InterfaceConfig& interfaces, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    interfaces.m_ros_node_name = root.value("ros_node_name").toString(interfaces.m_ros_node_name);

    // 平板真实接入走 gRPC：机器人控制系统监听 tablet_grpc_listen，平板作为客户端提交订单/查询状态。
    interfaces.m_tablet.m_grpc_listen = root.value("tablet_grpc_listen").toString(interfaces.m_tablet.m_grpc_listen);

    // 语音真实接入走 ROS2：语音订单输入、机器人状态输出、任务结果输出分别使用独立 topic。
    interfaces.m_voice.m_order_topic = root.value("voice_order_topic").toString();
    interfaces.m_voice.m_state_topic = root.value("voice_state_topic").toString();
    interfaces.m_voice.m_task_result_topic = root.value("voice_task_result_topic").toString();

    // 视觉真实接入走 ROS2：控制系统发布识别请求，视觉软件返回识别结果和置信度。
    interfaces.m_vision.m_request_topic = root.value("vision_request_topic").toString();
    interfaces.m_vision.m_result_topic = root.value("vision_result_topic").toString();
    interfaces.m_vision.m_timeout_ms = root.value("vision_timeout_ms").toInt(interfaces.m_vision.m_timeout_ms);
    return true;
}

bool ConfigLoader::LoadTaskPolicy(const QString& path, TaskPolicyConfig& policy, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    policy.m_max_vision_attempts = root.value("max_vision_attempts").toInt(policy.m_max_vision_attempts);
    policy.m_max_pick_attempts = root.value("max_pick_attempts").toInt(policy.m_max_pick_attempts);
    policy.m_loop_interval_ms = root.value("loop_interval_ms").toInt(policy.m_loop_interval_ms);
    return true;
}

bool ConfigLoader::LoadShelfLayout(const QString& path, ShelfLayoutConfig& layout, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    layout.m_shelf_id = root.value("shelf_id").toString();
    layout.m_rows = root.value("rows").toInt();
    layout.m_cols = root.value("cols").toInt();
    // row_heights 是 JSON 对象，key 是行号字符串，value 是该行升降高度。
    const QJsonObject heights = root.value("row_heights").toObject();
    for (auto it = heights.begin(); it != heights.end(); ++it)
        layout.m_row_heights[it.key().toInt()] = it.value().toDouble();
    // column_stations 保存每列的标准停车点。
    for (const QJsonValue& value : root.value("column_stations").toArray())
    {
        const ColumnStation station = ParseStation(value.toObject());
        if (station.m_col > 0 && !station.m_station_id.isEmpty())
            layout.m_column_stations[station.m_col] = station;
    }
    // slots 保存每个具体货位的行列、观察配置和预抓取姿态。
    for (const QJsonValue& value : root.value("slots").toArray())
    {
        const QJsonObject object = value.toObject();
        ShelfSlot slot;
        slot.m_slot_id = object.value("slot_id").toString();
        slot.m_row = object.value("row").toInt();
        slot.m_col = object.value("col").toInt();
        slot.m_observe_profile = object.value("observe_profile").toString();
        slot.m_right_pre_pick_pose = ParsePose(object.value("right_pre_pick_pose").toObject());
        slot.m_dual_pre_pick_pose = ParsePose(object.value("dual_pre_pick_pose").toObject());
        // dedicated_station 是可选字段；有它时该货位使用专用停车点。
        if (object.contains("dedicated_station"))
        {
            slot.m_has_dedicated_station = true;
            slot.m_dedicated_station = ParseStation(object.value("dedicated_station").toObject());
            slot.m_dedicated_station.m_col = slot.m_col;
        }
        if (!slot.m_slot_id.isEmpty()) layout.m_shelf_slots[slot.m_slot_id] = slot;
    }
    if (layout.m_rows <= 0 || layout.m_cols <= 0 || layout.m_shelf_slots.isEmpty())
    {
        if (error_message) *error_message = "货架配置缺少 rows/cols/slots: " + path;
        return false;
    }
    return true;
}

bool ConfigLoader::LoadProducts(const QString& path, QMap<QString, ProductConfig>& products, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    // 商品用 product_id 做 key，便于订单快速查找。
    for (const QJsonValue& value : root.value("products").toArray())
    {
        const QJsonObject object = value.toObject();
        ProductConfig product;
        product.m_product_id = object.value("product_id").toString();
        product.m_display_name = object.value("display_name").toString();
        product.m_package_type = object.value("package_type").toString();
        if (!product.m_product_id.isEmpty()) products[product.m_product_id] = product;
    }
    if (products.isEmpty())
    {
        if (error_message) *error_message = "商品配置为空: " + path;
        return false;
    }
    return true;
}

bool ConfigLoader::LoadInventory(const QString& path, QList<InventoryItem>& inventory, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    // 库存 item 把 product_id 绑定到 slot_id，Controller 会从这里找可用货位。
    for (const QJsonValue& value : root.value("items").toArray())
    {
        const QJsonObject object = value.toObject();
        InventoryItem item;
        item.m_slot_id = object.value("slot_id").toString();
        item.m_product_id = object.value("product_id").toString();
        item.m_quantity = object.value("quantity").toInt();
        item.m_enabled = object.value("enabled").toBool(true);
        if (!item.m_slot_id.isEmpty() && !item.m_product_id.isEmpty()) inventory.append(item);
    }
    return !inventory.isEmpty();
}

bool ConfigLoader::LoadHome(const QString& path, HomeConfig& home, QString* error_message) const
{
    const QJsonObject root = ReadObject(path, error_message);
    if (root.isEmpty()) return false;
    home.m_selling_home = ParseStation(root.value("selling_home").toObject());
    home.m_counter_place_pose = ParsePose(root.value("counter_place_pose").toObject());
    // idle_posture 描述任务结束后的空闲姿态。
    const QJsonObject idle = root.value("idle_posture").toObject();
    home.m_left_arm_home = idle.value("left_arm_home").toString(home.m_left_arm_home);
    home.m_right_arm_home = idle.value("right_arm_home").toString(home.m_right_arm_home);
    home.m_waist_home_yaw = idle.value("waist_home_yaw").toDouble();
    home.m_neck_home_yaw = idle.value("neck_home_yaw").toDouble();
    home.m_neck_home_pitch = idle.value("neck_home_pitch").toDouble();
    home.m_lift_home_height = idle.value("lift_home_height").toDouble();
    home.m_left_gripper_open = idle.value("left_gripper_open").toBool(true);
    home.m_right_gripper_open = idle.value("right_gripper_open").toBool(true);
    return !home.m_selling_home.m_station_id.isEmpty();
}

} // namespace asd_retail
