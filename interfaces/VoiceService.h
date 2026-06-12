// 文件说明：声明语音服务接口。
// 语音服务与平板服务一样拥有独立 start/loop，识别出的订单放入自身队列，由 Controller 轮询获取。
#pragma once

#include "model/RobotTypes.h"

#include <QObject>
#include <QQueue>

class VoiceService : public QObject
{
    Q_OBJECT
public:
    // 创建语音服务对象；真正的网络/SDK 初始化放在 start()。
    explicit VoiceService(QObject* parent = nullptr);

    // 启动语音服务，读取配置中的 endpoint。
    bool start(const InterfaceConfig& config);
    // 周期轮询语音服务；真实实现会在这里处理输入事件。
    void loop();
    // 是否有待 Controller 取走的订单。
    bool hasOrder() const;
    // 取出队头订单；调用前应先 hasOrder()。
    Order takeOrder();
    // 向语音端上报机器人状态。
    void reportRobotState(const QString& state);
    // 向语音端上报任务结果。
    void reportTaskResult(const TaskResult& result);

signals:
    // 语音服务日志，由 Controller 转发到前端。
    void logMessage(const QString& message);

private:
    QString m_endpoint;       // 语音服务端点/监听地址。
    bool m_running = false;   // start() 成功后为 true。
    QQueue<Order> m_orders;   // 语音识别或接收到的订单队列。
};
