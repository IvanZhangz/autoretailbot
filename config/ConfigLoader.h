// 文件说明：声明配置加载器。
// 配置按门店、工厂、接口、任务策略、货架、商品、库存、home 分文件保存，便于多门店复用和部署替换。
#pragma once

#include "model/RobotTypes.h"

#include <QString>

class ConfigLoader
{
public:
    // 总入口：一次性加载所有配置分片，全部成功才写回输出 config。
    bool load(const QString& configDir, RobotConfig& config, QString* errorMessage = nullptr) const;

private:
    // 读取门店基础信息。
    bool loadStore(const QString& path, StoreConfig& store, QString* errorMessage) const;
    // 读取工厂选择，例如 fake/真实硬件。
    bool loadFactories(const QString& path, FactoryConfig& factories, QString* errorMessage) const;
    // 读取外部服务端点和主题名。
    bool loadInterfaces(const QString& path, InterfaceConfig& interfaces, QString* errorMessage) const;
    // 读取任务循环周期、视觉/抓取重试等策略。
    bool loadTaskPolicy(const QString& path, TaskPolicyConfig& policy, QString* errorMessage) const;
    // 读取货架行列、货位和停车点。
    bool loadShelfLayout(const QString& path, ShelfLayoutConfig& layout, QString* errorMessage) const;
    // 读取商品目录。
    bool loadProducts(const QString& path, QMap<QString, ProductConfig>& products, QString* errorMessage) const;
    // 读取库存与货位绑定关系。
    bool loadInventory(const QString& path, QList<InventoryItem>& inventory, QString* errorMessage) const;
    // 读取售卖点、柜台放置位和空闲姿态。
    bool loadHome(const QString& path, HomeConfig& home, QString* errorMessage) const;
};
