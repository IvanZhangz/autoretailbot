// 文件说明：定义无人零售机器人框架的基础数据结构。
// 本文件只保存跨模块共享的数据模型，具体控制逻辑分别放在 controller、mission、hardware、interfaces 中。
#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QVariantMap>

// 三维位姿：用于底盘站点、机械臂 TCP、柜台放置位等。
struct Pose
{
    double x = 0.0;      // X 坐标。
    double y = 0.0;      // Y 坐标。
    double z = 0.0;      // Z 坐标或高度。
    double roll = 0.0;   // 绕 X 轴旋转。
    double pitch = 0.0;  // 绕 Y 轴旋转。
    double yaw = 0.0;    // 绕 Z 轴旋转。
};

// 货架列标准停车点：MVP 中每列货架只配置一个列标准 AGV 站点。
struct ColumnStation
{
    int col = 0;         // 货架列号。
    QString stationId;   // 站点编号。
    Pose pose;           // 站点底盘位姿。
};

// 货位配置：描述某个货位属于哪一行/列，以及是否有部署期验证好的货位专用停车点。
struct ShelfSlot
{
    QString slotId;              // 货位编号，例如 R1C1。
    int row = 0;                 // 行号。
    int col = 0;                 // 列号。
    QString observeProfile;      // 视觉/头部观察配置名。
    bool hasDedicatedStation = false; // 是否配置货位专用停车点。
    ColumnStation dedicatedStation;   // 货位专用停车点，有则优先于列标准点。
    Pose rightPrePickPose;       // 右手预抓取位姿。
    Pose dualPrePickPose;        // 双臂预抓取位姿。
};

// 货架布局：按列保存标准停车点，按货位 ID 保存货位详情。
struct ShelfLayoutConfig
{
    QString shelfId;                         // 货架 ID。
    int rows = 0;                            // 总行数。
    int cols = 0;                            // 总列数。
    QMap<int, double> rowHeights;            // 行号到腿部/升降高度映射。
    QMap<int, ColumnStation> columnStations; // 列号到标准停车点映射。
    QMap<QString, ShelfSlot> shelfSlots;          // 货位 ID 到货位配置映射。
};

// 商品配置：用于把订单商品映射到抓取策略。
struct ProductConfig
{
    QString productId;    // 商品 ID。
    QString displayName;  // 展示名称。
    QString packageType;  // 包装类型：naked 或 boxed。
};

// 库存配置：用于从商品 ID 找到可用货位。
struct InventoryItem
{
    QString slotId;     // 货位 ID。
    QString productId;  // 商品 ID。
    int quantity = 0;   // 可用数量。
    bool enabled = true;// 是否启用该库存位置。
};

// Home 配置：描述售卖等待点、柜台放置位和机器人空闲姿态。
struct HomeConfig
{
    ColumnStation sellingHome; // 售卖等待点。
    Pose counterPlacePose;     // 柜台放置位姿。
    QString leftArmHome = "home";  // 左臂 home 姿态名。
    QString rightArmHome = "home"; // 右臂 home 姿态名。
    double waistHomeYaw = 0.0;      // 腰部 home 角。
    double neckHomeYaw = 0.0;       // 颈部 yaw。
    double neckHomePitch = 0.0;     // 颈部 pitch。
    double liftHomeHeight = 0.0;    // 升降 home 高度。
    bool leftGripperOpen = true;    // 左夹爪空闲时是否打开。
    bool rightGripperOpen = true;   // 右夹爪空闲时是否打开。
};

// 任务策略：部署时配置，不在代码里写死。
struct TaskPolicyConfig
{
    int maxVisionAttempts = 1; // 视觉最大尝试次数，MVP 默认 1。
    int maxPickAttempts = 1;   // 抓取最大尝试次数，MVP 默认 1。
    int loopIntervalMs = 50;   // 主控制循环周期。
};

// 工厂配置：部署时选择创建 fake、ros2 或真实厂家适配器。
struct FactoryConfig
{
    QString actuatorFactory = "fake"; // 动作执行器/硬件工厂。
    QString tabletFactory = "fake";   // 平板服务工厂。
    QString voiceFactory = "fake";    // 语音服务工厂。
};

// 外部接口配置：服务端点、主题名等。
struct InterfaceConfig
{
    QString tabletEndpoint;       // 平板服务监听地址。
    QString voiceEndpoint;        // 语音服务监听地址。
    QString visionRequestTopic;   // 视觉请求主题。
    QString visionResultTopic;    // 视觉结果主题。
};

// 门店配置：使同一套代码能部署到多个门店。
struct StoreConfig
{
    QString storeId = "default_store";    // 门店 ID。
    QString robotProfile = "default_robot"; // 机器人配置名。
    QString mapFrame = "map";             // 地图坐标系。
    QString shelfFrame = "shelf";         // 货架坐标系。
};

// 总配置：Controller 初始化后持有。
struct RobotConfig
{
    StoreConfig store;
    FactoryConfig factories;
    InterfaceConfig interfaces;
    TaskPolicyConfig taskPolicy;
    ShelfLayoutConfig shelfLayout;
    QMap<QString, ProductConfig> products;
    QList<InventoryItem> inventory;
    HomeConfig home;
};

// 订单明细：当前 MVP 一次处理第一个商品。
struct OrderItem
{
    QString productId;
    int quantity = 1;
};

// 订单：平板、语音、调试界面都会转换成这个结构。
struct Order
{
    QString orderId;
    QString source;
    QList<OrderItem> items;
};

// 任务结果：用于日志和外部接口上报。
struct TaskResult
{
    bool success = false;
    QString orderId;
    QString errorCode;
    QString message;
};
