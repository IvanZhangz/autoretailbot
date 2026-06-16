#pragma once

#include "IChassisDevice.h"
#include "RobotTypes.h"

#include <memory>

namespace asd_retail
{

class ChassisDeviceFactory
{
public:
    static std::unique_ptr<IChassisDevice> Create(
        const ChassisDeviceConfig& config);
};

} // namespace asd_retail