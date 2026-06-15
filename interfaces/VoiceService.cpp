// 文件说明：实现语音服务抽象基类的订单队列公共逻辑。
#include "VoiceService.h"


namespace asd_retail
{
VoiceService::VoiceService(QObject* parent)
    : QObject(parent)
{
}

bool VoiceService::HasOrder() const
{
    return !m_orders.isEmpty();
}

Order VoiceService::TakeOrder()
{
    return m_orders.dequeue();
}

void VoiceService::EnqueueOrder(const Order& order)
{
    m_orders.enqueue(order);
}

} // namespace asd_retail
