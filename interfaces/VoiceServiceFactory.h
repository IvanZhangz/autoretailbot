// 文件说明：声明语音服务工厂。
// 根据 m_factories.m_voice_factory 创建 fake 或 ros2 语音服务。
#pragma once

#include "interfaces/VoiceService.h"

#include <memory>


namespace asd_retail
{
class VoiceServiceFactory
{
public:
    // factory 值为 ros2 时创建 Ros2VoiceService，其它值默认创建 FakeVoiceService。
    static std::unique_ptr<VoiceService> Create(const FactoryConfig& factories);
};

} // namespace asd_retail
