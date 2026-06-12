// 文件说明：实现动作执行器工厂。
#include "ActuatorFactory.h"
#include "FakeRetailActuator.h"

std::unique_ptr<ActionActuator> ActuatorFactory::create(const FactoryConfig& config)
{
    Q_UNUSED(config);
    // 当前已实现 fake；真实硬件接入时在这里根据 config.actuatorFactory 分支创建。
    return std::make_unique<FakeRetailActuator>();
}
