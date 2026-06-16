#pragma once

#include "hardware/arm/IArmDevice.h"
#include "model/RobotTypes.h"

#include <memory>

namespace asd_retail
{

class ArmDeviceFactory
{
public:
    static std::unique_ptr<IArmDevice> Create(
        const ArmDeviceConfig& config);
};

} // namespace asd_retail