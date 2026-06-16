#pragma once

#include "ISimpleDevice.h"
#include "RobotTypes.h"

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