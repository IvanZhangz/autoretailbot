// 文件说明：实现语音服务工厂。
#include "VoiceServiceFactory.h"

#include "FakeVoiceService.h"
#include "Ros2VoiceService.h"


namespace asd_retail
{
std::unique_ptr<VoiceService> VoiceServiceFactory::Create(const FactoryConfig& factories)
{
    if (factories.m_voice_factory == "ros2")
        return std::make_unique<Ros2VoiceService>();
    return std::make_unique<FakeVoiceService>();
}

} // namespace asd_retail
