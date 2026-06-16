// 文件说明：声明平板服务工厂。
// 根据 m_factories.m_tablet_factory 创建 fake 或 grpc 平板服务。
#pragma once

#include "TabletService.h"

#include <memory>


namespace asd_retail
{
class TabletServiceFactory
{
public:
    // factory 值为 fake 时创建 FakeTabletService，值为 grpc 时创建 GrpcTabletService。
    static std::unique_ptr<TabletService> Create(const FactoryConfig& factories);
};

} // namespace asd_retail
