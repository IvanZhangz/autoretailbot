// 文件说明：实现配置加载器，把多文件 JSON 配置转换为运行期 RobotConfig。
#include "ConfigLoader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
// 工具函数：打开一个 JSON 文件并要求根节点必须是对象。
QJsonObject readObject(const QString& path, QString* errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        if (errorMessage) *errorMessage = "无法打开配置文件: " + path;
        return {};
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
    {
        if (errorMessage) *errorMessage = "配置文件不是 JSON 对象: " + path;
        return {};
    }
    return doc.object();
}

// 工具函数：把 JSON 中的 x/y/z/roll/pitch/yaw 转成 Pose。
Pose parsePose(const QJsonObject& object)
{
    Pose pose;
    pose.x = object.value("x").toDouble();
    pose.y = object.value("y").toDouble();
    pose.z = object.value("z").toDouble();
    pose.roll = object.value("roll").toDouble();
    pose.pitch = object.value("pitch").toDouble();
    pose.yaw = object.value("yaw").toDouble();
    return pose;
}

// 工具函数：解析 AGV 站点；兼容 pose 和 chassis_pose 两种字段名。
ColumnStation parseStation(const QJsonObject& object)
{
    ColumnStation station;
    station.col = object.value("col").toInt();
    station.stationId = object.value("station_id").toString();
    station.pose = parsePose(object.value("pose").toObject(object.value("chassis_pose").toObject()));
    return station;
}
}

bool ConfigLoader::load(const QString& configDir, RobotConfig& config, QString* errorMessage) const
{
    // 先加载到临时对象 loaded，避免中途失败时污染外部 config。
    RobotConfig loaded;
    if (!loadStore(configDir + "/store.json", loaded.store, errorMessage)) return false;
    if (!loadFactories(configDir + "/factories.json", loaded.factories, errorMessage)) return false;
    if (!loadInterfaces(configDir + "/interfaces.json", loaded.interfaces, errorMessage)) return false;
    if (!loadTaskPolicy(configDir + "/task_policy.json", loaded.taskPolicy, errorMessage)) return false;
    if (!loadShelfLayout(configDir + "/shelf_layout.json", loaded.shelfLayout, errorMessage)) return false;
    if (!loadProducts(configDir + "/product_catalog.json", loaded.products, errorMessage)) return false;
    if (!loadInventory(configDir + "/inventory.json", loaded.inventory, errorMessage)) return false;
    if (!loadHome(configDir + "/home.json", loaded.home, errorMessage)) return false;
    config = loaded;
    return true;
}

bool ConfigLoader::loadStore(const QString& path, StoreConfig& store, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    store.storeId = root.value("store_id").toString(store.storeId);
    store.robotProfile = root.value("robot_profile").toString(store.robotProfile);
    store.mapFrame = root.value("map_frame").toString(store.mapFrame);
    store.shelfFrame = root.value("shelf_frame").toString(store.shelfFrame);
    return true;
}

bool ConfigLoader::loadFactories(const QString& path, FactoryConfig& factories, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    factories.actuatorFactory = root.value("actuator_factory").toString(factories.actuatorFactory);
    factories.tabletFactory = root.value("tablet_factory").toString(factories.tabletFactory);
    factories.voiceFactory = root.value("voice_factory").toString(factories.voiceFactory);
    return true;
}

bool ConfigLoader::loadInterfaces(const QString& path, InterfaceConfig& interfaces, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    interfaces.tabletEndpoint = root.value("tablet_endpoint").toString();
    interfaces.voiceEndpoint = root.value("voice_endpoint").toString();
    interfaces.visionRequestTopic = root.value("vision_request_topic").toString();
    interfaces.visionResultTopic = root.value("vision_result_topic").toString();
    return true;
}

bool ConfigLoader::loadTaskPolicy(const QString& path, TaskPolicyConfig& policy, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    policy.maxVisionAttempts = root.value("max_vision_attempts").toInt(policy.maxVisionAttempts);
    policy.maxPickAttempts = root.value("max_pick_attempts").toInt(policy.maxPickAttempts);
    policy.loopIntervalMs = root.value("loop_interval_ms").toInt(policy.loopIntervalMs);
    return true;
}

bool ConfigLoader::loadShelfLayout(const QString& path, ShelfLayoutConfig& layout, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    layout.shelfId = root.value("shelf_id").toString();
    layout.rows = root.value("rows").toInt();
    layout.cols = root.value("cols").toInt();
    // row_heights 是 JSON 对象，key 是行号字符串，value 是该行升降高度。
    const QJsonObject heights = root.value("row_heights").toObject();
    for (auto it = heights.begin(); it != heights.end(); ++it)
        layout.rowHeights[it.key().toInt()] = it.value().toDouble();
    // column_stations 保存每列的标准停车点。
    for (const QJsonValue& value : root.value("column_stations").toArray())
    {
        const ColumnStation station = parseStation(value.toObject());
        if (station.col > 0 && !station.stationId.isEmpty())
            layout.columnStations[station.col] = station;
    }
    // slots 保存每个具体货位的行列、观察配置和预抓取姿态。
    for (const QJsonValue& value : root.value("slots").toArray())
    {
        const QJsonObject object = value.toObject();
        ShelfSlot slot;
        slot.slotId = object.value("slot_id").toString();
        slot.row = object.value("row").toInt();
        slot.col = object.value("col").toInt();
        slot.observeProfile = object.value("observe_profile").toString();
        slot.rightPrePickPose = parsePose(object.value("right_pre_pick_pose").toObject());
        slot.dualPrePickPose = parsePose(object.value("dual_pre_pick_pose").toObject());
        // dedicated_station 是可选字段；有它时该货位使用专用停车点。
        if (object.contains("dedicated_station"))
        {
            slot.hasDedicatedStation = true;
            slot.dedicatedStation = parseStation(object.value("dedicated_station").toObject());
            slot.dedicatedStation.col = slot.col;
        }
        if (!slot.slotId.isEmpty()) layout.shelfSlots[slot.slotId] = slot;
    }
    if (layout.rows <= 0 || layout.cols <= 0 || layout.shelfSlots.isEmpty())
    {
        if (errorMessage) *errorMessage = "货架配置缺少 rows/cols/slots: " + path;
        return false;
    }
    return true;
}

bool ConfigLoader::loadProducts(const QString& path, QMap<QString, ProductConfig>& products, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    // 商品用 product_id 做 key，便于订单快速查找。
    for (const QJsonValue& value : root.value("products").toArray())
    {
        const QJsonObject object = value.toObject();
        ProductConfig product;
        product.productId = object.value("product_id").toString();
        product.displayName = object.value("display_name").toString();
        product.packageType = object.value("package_type").toString();
        if (!product.productId.isEmpty()) products[product.productId] = product;
    }
    if (products.isEmpty())
    {
        if (errorMessage) *errorMessage = "商品配置为空: " + path;
        return false;
    }
    return true;
}

bool ConfigLoader::loadInventory(const QString& path, QList<InventoryItem>& inventory, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    // 库存 item 把 product_id 绑定到 slot_id，Controller 会从这里找可用货位。
    for (const QJsonValue& value : root.value("items").toArray())
    {
        const QJsonObject object = value.toObject();
        InventoryItem item;
        item.slotId = object.value("slot_id").toString();
        item.productId = object.value("product_id").toString();
        item.quantity = object.value("quantity").toInt();
        item.enabled = object.value("enabled").toBool(true);
        if (!item.slotId.isEmpty() && !item.productId.isEmpty()) inventory.append(item);
    }
    return !inventory.isEmpty();
}

bool ConfigLoader::loadHome(const QString& path, HomeConfig& home, QString* errorMessage) const
{
    const QJsonObject root = readObject(path, errorMessage);
    if (root.isEmpty()) return false;
    home.sellingHome = parseStation(root.value("selling_home").toObject());
    home.counterPlacePose = parsePose(root.value("counter_place_pose").toObject());
    // idle_posture 描述任务结束后的空闲姿态。
    const QJsonObject idle = root.value("idle_posture").toObject();
    home.leftArmHome = idle.value("left_arm_home").toString(home.leftArmHome);
    home.rightArmHome = idle.value("right_arm_home").toString(home.rightArmHome);
    home.waistHomeYaw = idle.value("waist_home_yaw").toDouble();
    home.neckHomeYaw = idle.value("neck_home_yaw").toDouble();
    home.neckHomePitch = idle.value("neck_home_pitch").toDouble();
    home.liftHomeHeight = idle.value("lift_home_height").toDouble();
    home.leftGripperOpen = idle.value("left_gripper_open").toBool(true);
    home.rightGripperOpen = idle.value("right_gripper_open").toBool(true);
    return !home.sellingHome.stationId.isEmpty();
}
