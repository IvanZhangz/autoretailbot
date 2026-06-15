// 文件说明：实现视觉服务工厂。
#include "VisionServiceFactory.h"

#include "FakeVisionService.h"
#include "Ros2VisionService.h"


namespace asd_retail
{
std::unique_ptr<VisionService> VisionServiceFactory::Create(const FactoryConfig& factories)
{
    if (factories.m_vision_factory == "ros2")
        return std::make_unique<Ros2VisionService>();
    return std::make_unique<FakeVisionService>();
}

} // namespace asd_retail
