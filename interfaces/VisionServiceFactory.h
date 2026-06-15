// 文件说明：声明视觉服务工厂。
// 根据 m_factories.m_vision_factory 创建 fake 或 ros2 视觉服务。
#pragma once

#include "interfaces/VisionService.h"

#include <memory>


namespace asd_retail
{
class VisionServiceFactory
{
public:
    // factory 值为 ros2 时创建 Ros2VisionService，其它值默认创建 FakeVisionService。
    static std::unique_ptr<VisionService> Create(const FactoryConfig& factories);
};

} // namespace asd_retail
