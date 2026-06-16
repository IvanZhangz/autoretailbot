#include "ChassisDeviceFactory.h"

#include "VendorChassisDevice.h"

namespace asd_retail
{

std::unique_ptr<IChassisDevice>
ChassisDeviceFactory::Create(
    const ChassisDeviceConfig& config)
{
    if (!config.m_enabled)
        return nullptr;

    if (config.m_vendor.compare(
            "agv_vendor",
            Qt::CaseInsensitive) == 0)
    {
        return std::make_unique<VendorChassisDevice>(
            config);
    }

    return nullptr;
}

} // namespace asd_retail