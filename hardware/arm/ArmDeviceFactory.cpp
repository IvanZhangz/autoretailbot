#include "ArmDeviceFactory.h"

#include "hardware/arm/RealmanArmDevice.h"

namespace asd_retail
{

std::unique_ptr<IArmDevice> ArmDeviceFactory::Create(
    const ArmDeviceConfig& config)
{
    if (!config.m_enabled)
        return nullptr;

    if (config.m_vendor == "realman")
        return std::make_unique<RealmanArmDevice>(config);

    // 后续扩展：
    //
    // if (config.m_vendor == "aubo")
    //     return std::make_unique<AuboArmDevice>(config);
    //
    // if (config.m_vendor == "jaka")
    //     return std::make_unique<JakaArmDevice>(config);

    return nullptr;
}

} // namespace asd_retail