#pragma once

#include "hardware/simple/ISimpleDevice.h"
#include "model/RobotTypes.h"

#include <memory>

namespace asd_retail
{

class SimpleDeviceFactory
{
public:
    static std::unique_ptr<ISimpleDevice> Create(
        const SimpleDeviceConfig& config);
};

} // namespace asd_retail