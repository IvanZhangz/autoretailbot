// 文件说明：实现动作执行器工厂。
#include "ActuatorFactory.h"

#include "FakeRetailActuator.h"
#include "RealRetailActuator.h"

namespace asd_retail
{
std::unique_ptr<ActionActuator> ActuatorFactory::Create(const FactoryConfig& config)
{
    // 配置为 real 时创建真实硬件执行器骨架；具体 SDK 控制逻辑在 RealRetailActuator 的 TODO 状态机中补齐。
    if (config.m_actuator_factory == "real")
        return std::make_unique<RealRetailActuator>();

    // 默认使用 fake，保证无真实硬件、无 ROS2/gRPC 环境时仍可运行调试界面。
    return std::make_unique<FakeRetailActuator>();
}

} // namespace asd_retail
