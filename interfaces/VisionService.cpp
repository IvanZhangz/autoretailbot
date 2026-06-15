// 文件说明：实现视觉服务抽象基类的结果缓存公共逻辑。
#include "VisionService.h"


namespace asd_retail
{
VisionService::VisionService(QObject* parent)
    : QObject(parent)
{
}

bool VisionService::HasResult(const QString& request_id) const
{
    return m_results.contains(request_id);
}

VisionResult VisionService::TakeResult(const QString& request_id)
{
    return m_results.take(request_id);
}

void VisionService::StoreResult(const VisionResult& result)
{
    m_results[result.m_request_id] = result;
}

} // namespace asd_retail
