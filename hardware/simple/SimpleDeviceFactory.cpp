#include "SimpleDeviceFactory.h"

#include "hardware/simple/VendorSimpleDevice.h"

namespace asd_retail
{

std::unique_ptr<ISimpleDevice>
SimpleDeviceFactory::Create(
    const SimpleDeviceConfig& config)
{
    if (!config.m_enabled)
        return nullptr;

    if (config.m_vendor == "modbus_lift" ||
        config.m_vendor == "can_servo" ||
        config.m_vendor == "realman_gripper")
    {
        return std::make_unique<VendorSimpleDevice>(
            config);
    }

    return nullptr;
}

} // namespace asd_retail