// 文件说明：实现平板服务抽象基类的队列公共逻辑。
#include "TabletService.h"


namespace asd_retail
{
TabletService::TabletService(QObject* parent)
    : QObject(parent)
{
}

bool TabletService::HasOrder() const
{
    return !m_orders.isEmpty();
}

Order TabletService::TakeOrder()
{
    return m_orders.dequeue();
}

void TabletService::EnqueueDebugOrder(const Order& order)
{
    EnqueueOrder(order);
    emit LogMessage("平板调试入口收到订单: " + order.m_order_id);
}

void TabletService::EnqueueOrder(const Order& order)
{
    m_orders.enqueue(order);
}

} // namespace asd_retail
