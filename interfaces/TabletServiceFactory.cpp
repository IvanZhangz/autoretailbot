// 文件说明：实现平板服务工厂。
#include "TabletServiceFactory.h"

#include "FakeTabletService.h"
#include "GrpcTabletService.h"


namespace asd_retail
{
std::unique_ptr<TabletService> TabletServiceFactory::Create(const FactoryConfig& factories)
{
    if (factories.m_tablet_factory == "grpc")
        return std::make_unique<GrpcTabletService>();
    return std::make_unique<FakeTabletService>();
}

} // namespace asd_retail
