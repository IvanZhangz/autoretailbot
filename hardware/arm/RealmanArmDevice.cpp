#include "RealmanArmDevice.h"

namespace asd_retail
{

RealmanArmDevice::RealmanArmDevice(
    const ArmDeviceConfig& config)
    : m_config(config)
{
    // 临时示例。更推荐以后从独立姿态配置读取。
    m_named_poses["home"] =
        QVector<double>(m_config.m_dof, 0.0);
}

RealmanArmDevice::~RealmanArmDevice()
{
    Stop();
    Disable();
    Disconnect();
}

bool RealmanArmDevice::Connect()
{
    if (m_state.m_connected)
        return true;

    // TODO：调用 Realman SDK。
    //
    // m_handle = rm_create_robot_arm(
    //     m_config.m_ip.toStdString().c_str(),
    //     m_config.m_port);

    const bool success = true;

    if (!success)
    {
        m_state.m_error = true;
        m_state.m_error_text =
            m_config.m_name + " 连接失败";
        return false;
    }

    m_state.m_connected = true;
    m_state.m_error = false;
    m_state.m_error_text.clear();

    return true;
}

bool RealmanArmDevice::Disconnect()
{
    if (!m_state.m_connected)
        return true;

    // TODO：关闭 SDK handle。

    m_state.m_connected = false;
    m_state.m_enabled = false;
    m_state.m_moving = false;
    m_state.m_reached = false;
    m_command_active = false;

    return true;
}