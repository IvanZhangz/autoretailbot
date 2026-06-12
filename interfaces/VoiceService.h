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
    explicit VoiceService(QObject* parent = nullptr);

    bool start(const InterfaceConfig& config);
    void loop();
    bool hasOrder() const;
    Order takeOrder();
    void reportRobotState(const QString& state);
    void reportTaskResult(const TaskResult& result);

signals:
    void logMessage(const QString& message);

private:
    QString m_endpoint;
    bool m_running = false;
    QQueue<Order> m_orders;
};
