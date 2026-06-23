// 文件说明：定义无人零售机器人框架的基础数据结构。
// 本文件只保存跨模块共享的数据模型，具体控制逻辑分别放在 controller、mission、hardware、interfaces 中。
#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QVariantMap>

// 三维位姿：用于底盘站点、机械臂 TCP、柜台放置位、视觉识别结果等。

namespace asd_retail
{
struct Pose
{
    double m_x = 0.0;      // X 坐标。
    double m_y = 0.0;      // Y 坐标。
    double m_z = 0.0;      // Z 坐标或升降高度。
    double m_roll = 0.0;   // 绕 X 轴旋转角。
    double m_pitch = 0.0;  // 绕 Y 轴旋转角。
    double m_yaw = 0.0;    // 绕 Z 轴旋转角。
};

// 货架列标准停车点：MVP 中每列货架只配置一个列标准 AGV 站点。
struct ColumnStation
{
    int m_col = 0;         // 货架列号。
    QString m_station_id;   // 站点编号。
    Pose m_pose;           // 站点底盘位姿。
};

// 货位配置：描述某个货位属于哪一行/列，以及是否有部署期验证好的货位专用停车点。
struct ShelfSlot
{
    QString m_slot_id;              // 货位编号，例如 R1C1。
    int m_row = 0;                 // 行号。
    int m_col = 0;                 // 列号。
    QString m_observe_profile;      // 视觉/头部观察配置名。
    bool m_has_dedicated_station = false; // 是否配置货位专用停车点。
    ColumnStation m_dedicated_station;   // 货位专用停车点，有则优先于列标准点。
    Pose m_right_pre_pick_pose;       // 右手预抓取位姿。
    Pose m_dual_pre_pick_pose;        // 双臂预抓取位姿。
};

// 货架布局：按列保存标准停车点，按货位 ID 保存货位详情。
struct ShelfLayoutConfig
{
    QString m_shelf_id;                         // 货架 ID。
    int m_rows = 0;                            // 总行数。
    int m_cols = 0;                            // 总列数。
    QMap<int, double> m_row_heights;            // 行号到腿部/升降高度映射。
    QMap<int, ColumnStation> m_column_stations; // 列号到标准停车点映射。
    QMap<QString, ShelfSlot> m_shelf_slots;     // 货位 ID 到货位配置映射。
};

// 商品配置：用于把订单商品映射到抓取策略。
struct ProductConfig
{
    QString m_product_id;    // 商品 ID。
    QString m_display_name;  // 展示名称。
    QString m_package_type;  // 包装类型：naked 或 boxed。
};

// 库存配置：用于从商品 ID 找到可用货位。
struct InventoryItem
{
    QString m_slot_id;     // 货位 ID。
    QString m_product_id;  // 商品 ID。
    int m_quantity = 0;   // 可用数量。
    bool m_enabled = true;// 是否启用该库存位置。
};

// Home 配置：描述售卖等待点、柜台放置位和机器人空闲姿态。
struct HomeConfig
{
    ColumnStation m_selling_home; // 售卖等待点。
    Pose m_counter_place_pose;     // 柜台放置位姿。
    QString m_left_arm_home = "home";  // 左臂 home 姿态名。
    QString m_right_arm_home = "home"; // 右臂 home 姿态名。
    double m_waist_home_yaw = 0.0;      // 腰部 home 角。
    double m_neck_home_yaw = 0.0;       // 颈部 yaw。
    double m_neck_home_pitch = 0.0;     // 颈部 pitch。
    double m_lift_home_height = 0.0;    // 升降 home 高度。
    bool m_left_gripper_open = true;    // 左夹爪空闲时是否打开。
    bool m_right_gripper_open = true;   // 右夹爪空闲时是否打开。
};

// 任务策略：部署时配置
struct TaskPolicyConfig
{
    int m_max_vision_attempts = 1; // 视觉最大尝试次数，MVP 默认 1。
    int m_max_pick_attempts = 1;   // 抓取最大尝试次数，MVP 默认 1。
    int m_loop_interval_ms = 20;   // 主控制循环周期。
};

// 单个机械臂配置。
/*
    当前任务生成逻辑会根据商品package_type == "boxed" 决定是否使用双臂，
    没有针对左臂disabled时禁止生成双臂任务的完整保护逻辑。
*/
struct ArmDeviceConfig
{
    QString m_name;                 // left_arm / right_arm
    QString m_vendor = "realman";   // realman / aubo / jaka / fake
    QString m_ip;                   // 机械臂控制器的网络 IP 地址。
    int m_port = 0;                 // 机械臂控制器的网络端口号，0 表示未配置或由具体 SDK 使用默认端口。
    int m_dof = 6;                  // 机械臂自由度。
    bool m_enabled = true;          // 是否启用该机械臂设备。
};

// 底盘配置。
struct ChassisDeviceConfig
{
    QString m_name = "chassis";     // 底盘在系统中的逻辑名称。
    QString m_vendor;               // 具体厂家，例如 slamware、agv_vendor
    QString m_ip;                   // 底盘控制器的网络 IP 地址。
    int m_port = 0;                 // 底盘控制器的网络端口号。
    int m_slave_id = 1;             // 如果底盘使用 Modbus，可配置设备地址或从站地址，1 表示 Modbus Server Address。
    bool m_enabled = true;          // 是否启用底盘设备。
};

// 简单设备配置。
struct SimpleDeviceConfig
{
    QString m_name;                 // 设备在系统中的逻辑名称：lift / waist / neck / left_gripper...
    QString m_type;                 // 设备功能类型：lift / waist / neck / gripper
    QString m_vendor;               // 设备厂家或驱动实现名称，该字段决定底层调用哪个SDK或通信协议。
    QString m_ip;                   // 设备 IP 地址。
    int m_port = 0;                 // 设备通信端口。
    int m_device_id = 0;            // 设备地址或设备编号。
    bool m_enabled = true;          // 是否启用该设备。

    double m_position_tolerance = 0.01; // 位置到位容差，必须确保配置单位与设备接口中的位置单位一致。
};

// 全部真实硬件配置。
struct HardwareConfig
{
    ArmDeviceConfig m_left_arm;             // 左机械臂配置。
    ArmDeviceConfig m_right_arm;            // 右机械臂配置。

    ChassisDeviceConfig m_chassis;          // 移动底盘配置。

    SimpleDeviceConfig m_lift;              // 升降设备配置。
    SimpleDeviceConfig m_waist;             // 腰部设备配置。
    SimpleDeviceConfig m_neck;              // 颈部设备配置。
    SimpleDeviceConfig m_left_gripper;      // 左夹爪设备配置。
    SimpleDeviceConfig m_right_gripper;     // 右夹爪设备配置。
};

// 工厂配置：部署时选择 fake、真实硬件、ROS2 或 gRPC 适配器。
struct FactoryConfig
{
    QString m_actuator_factory = "fake"; // 动作执行器/硬件工厂：fake 或 real。
    QString m_tablet_factory = "fake";   // 平板服务工厂：fake 或 grpc。
    QString m_voice_factory = "fake";    // 语音服务工厂：fake 或 ros2。
    QString m_vision_factory = "fake";   // 视觉服务工厂：fake 或 ros2。
};

// 平板接口配置：真实部署时机器人控制系统作为 gRPC 服务端监听该地址。
struct TabletInterfaceConfig
{
    QString m_grpc_listen = "0.0.0.0:18080"; // 平板 gRPC 监听地址。
};

// 语音接口配置：语音软件通过 ROS2 topic 与机器人控制系统交互。
struct VoiceInterfaceConfig
{
    QString m_order_topic;      // 语音软件发布订单/意图的 topic。
    QString m_state_topic;      // 机器人控制系统向语音软件发布状态的 topic。
    QString m_task_result_topic; // 机器人控制系统向语音软件发布任务结果的 topic。
};

// 视觉接口配置：感知视觉软件通过 ROS2 请求/结果 topic 与机器人控制系统交互。
struct VisionInterfaceConfig
{
    QString m_request_topic;     // 机器人控制系统发布视觉识别请求的 topic。
    QString m_result_topic;      // 视觉软件发布识别结果的 topic。
    int m_timeout_ms = 3000;     // 等待单次视觉识别结果的超时时间。
};

// 外部接口配置：集中保存平板 gRPC、语音 ROS2、视觉 ROS2 的通讯参数。
struct InterfaceConfig
{
    QString m_ros_node_name = "autoretailbot_controller"; // 本控制系统在 ROS2 中使用的节点名。
    TabletInterfaceConfig m_tablet; // 平板 gRPC 配置。
    VoiceInterfaceConfig m_voice;   // 语音 ROS2 配置。
    VisionInterfaceConfig m_vision; // 视觉 ROS2 配置。
};

// 门店配置：使同一套代码能部署到多个门店。
struct StoreConfig
{
    QString m_store_id = "default_store";       // 门店 ID。
    QString m_robot_profile = "default_robot";  // 机器人配置名。
    QString m_map_frame = "map";                // 地图坐标系。
    QString m_shelf_frame = "shelf";            // 货架坐标系。
};

// 总配置：Controller 初始化后持有。
struct RobotConfig
{
    StoreConfig m_store;                   // 门店基础配置。
    FactoryConfig m_factories;             // fake/real/ros2/grpc 工厂选择。
    InterfaceConfig m_interfaces;          // 外部通讯接口配置。
    TaskPolicyConfig m_task_policy;         // 任务循环和重试策略。
    ShelfLayoutConfig m_shelf_layout;       // 货架布局、行高和货位。
    QMap<QString, ProductConfig> m_products; // 商品目录，key 为 product_id。
    QList<InventoryItem> m_inventory;      // 库存货位绑定。
    HomeConfig m_home;                     // 售卖点、放置点和空闲姿态。
    HardwareConfig m_hardware;             // 真实硬件配置。
};

// 订单明细：当前 MVP 一次处理第一个商品。
struct OrderItem
{
    QString m_product_id; // 商品 ID。
    int m_quantity = 1;  // 数量，当前任务构建只使用第一个商品。
};

// 订单：平板、语音、调试界面都会转换成这个结构。
struct Order
{
    QString m_order_id;        // 订单号。
    QString m_source;         // 来源：qt_debug、tablet_grpc、voice_ros2 等。
    QList<OrderItem> m_items; // 订单商品列表。
};

// 任务结果：用于日志和外部接口上报。
struct TaskResult
{
    bool m_success = false; // 是否成功。
    QString m_order_id;     // 对应订单号。
    QString m_error_code;   // 失败错误码，成功时为空。
    QString m_message;     // 人可读结果说明。
};

} // namespace asd_retail
