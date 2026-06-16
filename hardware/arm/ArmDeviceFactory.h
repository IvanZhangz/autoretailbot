#pragma once

#include "IArmDevice.h"
#include "RobotTypes.h"

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