// 文件说明：声明配置加载器。
// 配置按门店、工厂、接口、任务策略、货架、商品、库存、home 分文件保存，便于多门店复用和部署替换。
#pragma once

#include "model/RobotTypes.h"

#include <QString>

class ConfigLoader
{
public:
    bool load(const QString& configDir, RobotConfig& config, QString* errorMessage = nullptr) const;

private:
    bool loadStore(const QString& path, StoreConfig& store, QString* errorMessage) const;
    bool loadFactories(const QString& path, FactoryConfig& factories, QString* errorMessage) const;
    bool loadInterfaces(const QString& path, InterfaceConfig& interfaces, QString* errorMessage) const;
    bool loadTaskPolicy(const QString& path, TaskPolicyConfig& policy, QString* errorMessage) const;
    bool loadShelfLayout(const QString& path, ShelfLayoutConfig& layout, QString* errorMessage) const;
    bool loadProducts(const QString& path, QMap<QString, ProductConfig>& products, QString* errorMessage) const;
    bool loadInventory(const QString& path, QList<InventoryItem>& inventory, QString* errorMessage) const;
    bool loadHome(const QString& path, HomeConfig& home, QString* errorMessage) const;
};
