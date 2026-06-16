// 文件说明：声明动作执行器工厂。
// 工厂根据配置创建 Fake 或未来真实硬件执行器，Controller 不直接 new 具体类。
#pragma once

#include "HardwareInterfaces.h"

#include <memory>


namespace asd_retail
{
class ActuatorFactory
{
public:
    static std::unique_ptr<ActionActuator> Create(const FactoryConfig& config);
};

} // namespace asd_retail
