# autoretailbot 无人零售机器人框架

无人零售机器人控制框架核心原则是：

1. `main.cpp`只创建 Qt 应用和 `MainWindow`；`RobotController` 单例由窗口初始化，避免入口处重复取单例造成误解。
2. `MainWindow`只做前端调试显示，不创建硬件、不执行任务。
3. `RobotController`是后端核心，周期调用`Loop()`。
4. `Loop()`按顺序轮询设备、轮询外部服务、接收订单、启动任务、轮询任务状态机。
5. 任务模型为`RobotMission -> RobotBeat -> RobotAction`：Mission包含多个Beat；Beat顺序执行；同一Beat内Action同时开始，全部完成后进入下一Beat。
6. 硬件由`ActuatorFactory`根据配置创建，通过`ActionActuator`抽象接口执行动作；真实执行器按`ActionType`分发到各动作状态机。
7. 视觉是独立软件，真实接入通过 ROS2 请求/结果 topic 交互；当前提供 `FakeVisionService` 和 `Ros2VisionService` 骨架。
8. 语音软件真实接入通过 ROS2 topic 交互；平板真实接入通过 gRPC 协议交互；当前默认 fake，保证无 ROS2/gRPC 环境也能编译运行。

## 目录说明

- `controller/`：机器人后端控制器单例，包含主循环和`LoopWorkSM()`。
- `mission/`：任务、节拍、动作的数据结构。
- `hardware/`：硬件抽象接口、执行器工厂、Fake 执行器和真实执行器 TODO 骨架。
- `interfaces/`：平板 gRPC、语音 ROS2、视觉 ROS2 的抽象接口、Fake 实现和真实协议骨架。
- `config/`：分文件配置，JSON中使用`_comment`字段说明字段作用。
- `model/`：跨模块共享数据结构。
- `mainwindow.*`：Qt调试前端。

## 代码讲解

### 1. `main.cpp`：程序入口

### 2. `MainWindow`：调试前端

`MainWindow` 前端调试UI，只做三件事：

- 初始化界面控件和 Controller。
- 把 Controller 日志显示到日志框。
- 把“发送调试订单”按钮转换成 Controller 的调试订单。

关键流程：

1. `ui->setupUi(this)` 根据 `mainwindow.ui` 创建商品下拉框、按钮、日志框和状态栏。
2. `RobotController::Instance()` 获取后端单例；真正的单例对象在 `RobotController::Instance()` 内的函数内 `static` 中创建，全项目只有一个。
3. `connect(...)` 把后端信号接到前端槽函数：
   - `LogMessage -> appendLog`：日志进入 `QTextEdit`。
   - `StateChanged -> onControllerStateChanged`：状态显示到状态栏。
   - `sendDebugOrderButton.clicked -> onSendDebugOrderClicked`：按钮触发调试订单。
4. `controller.Init(resolveConfigDir(), &error)` 初始化后端。
5. `productComboBox->addItems(controller.ProductIds())` 把配置里的商品 ID 加入下拉框。

`resolveConfigDir()` 的优先级是：程序所在目录的 `config`、当前工作目录的 `config`、源码目录的 `config`、最后回退到 Qt 资源 `:/config`。

### 3. `RobotController`：后端控制核心

`RobotController` 是机器人业务核心，主要变量如下：

- `m_config`：加载后的总配置，包括货架、库存、商品、home、接口、工厂选择等。
- `m_initialized`：后端是否初始化成功，未成功时不接单。
- `m_timer_id`：Qt 定时器 ID。
- `m_curr_step`：当前执行到第几个 `RobotBeat`。
- `m_state`：机器人当前状态文本。
- `m_order_queue`：平板、语音、调试订单汇总后的统一队列。
- `m_mission`：当前正在执行或刚执行完的任务。
- `m_actuator`：动作执行器，可由配置切换为 Fake 或真实执行器骨架。
- `m_tablet_service`：平板服务，可由配置切换为 Fake 或 gRPC 骨架。
- `m_voice_service`：语音服务，可由配置切换为 Fake 或 ROS2 骨架。
- `m_vision_service`：视觉服务，可由配置切换为 Fake 或 ROS2 骨架。

#### 3.1 初始化 `Init()`

初始化顺序如下：

1. 用 `ConfigLoader` 读取配置目录下的多个 JSON 文件，填入 `m_config`。
2. 用 `VisionServiceFactory`、`TabletServiceFactory`、`VoiceServiceFactory` 创建视觉、平板、语音服务。
3. 启动视觉、平板、语音服务，并把它们的日志信号转发给 Controller。
4. 用 `ActuatorFactory::Create(m_config.m_factories)` 创建动作执行器。
5. 调用 `m_actuator->SetVisionService(m_vision_service.get())`，让真实或 Fake 执行器在处理 `VisionDetectByProduct` 时通过视觉服务交互。
6. 初始化执行器 `m_actuator->Init(m_config)`。
7. 构造一个启动回 home 任务，让机器人先回安全姿态。
8. `startTimer(m_config.m_task_policy.m_loop_interval_ms)` 启动主循环定时器。

#### 3.2 主循环 `Loop()`

`timerEvent()` 每隔 `loop_interval_ms` 调用一次 `Loop()`。`Loop()` 的顺序是固定的：

1. `LoopDevices()`：轮询硬件设备。
2. `LoopServices()`：轮询视觉 ROS2、平板 gRPC、语音 ROS2 等外部服务。
3. `AcceptPendingOrders()`：把平板/语音服务内部队列中的订单搬到 `m_order_queue`。
4. `StartNextMissionIfIdle()`：如果当前任务结束且有订单，则构建新任务。
5. `LoopWorkSM()`：推进当前任务状态机。

#### 3.3 任务状态机 `LoopWorkSM()`

任务模型是：`RobotMission -> RobotBeat -> RobotAction`。

- 一个 `Mission` 是完整订单任务。
- 一个 `Beat` 是一组可以并行推进的动作。
- 一个 `Action` 是最小动作，例如 AGV 到站点、升降到高度、右臂移动、视觉识别、夹爪打开。

执行规则：

1. `m_curr_step` 指向当前 `Beat`。
2. 遍历当前 `Beat` 的所有 `Action`。
3. 每个 Action 先绑定 `m_mission.m_args`，这样视觉动作写入的数据可以被后续动作读取。
4. 调用 `m_actuator->LoopAction(action)` 推进动作。
5. 只要有一个 Action 没完成，本 Beat 就没完成。
6. 当前 Beat 全部完成后，`m_curr_step++` 进入下一 Beat。
7. 所有 Beat 完成后调用 `FinishMission(true, "任务完成")`。

### 4. 订单如何变成任务

`BuildMissionFromOrder()` 做转换：

1. 检查订单是否为空。
2. 取当前 MVP 的第一个商品。
3. 在商品目录中确认商品存在。
4. 在库存中查找该商品的可用货位。
5. 根据货位查货架配置和停车点。
6. 根据商品包装类型判断使用双臂还是右臂单手：`boxed` 使用双臂，否则右臂。
7. 生成一串 Beat：
   - 到达货架并准备观察/抓取：AGV、升降、腰、头、手臂预抓取和夹爪打开并行执行。
   - 视觉识别：通过 `VisionService` 请求独立视觉软件，结果写入 `mission.m_args["detected_pose"]`。
   - 移动到视觉抓取位：右臂或双臂读取 `detected_pose`。
   - 闭合夹爪。
   - 进入携物姿态。
   - 返回售卖点并准备交付。
   - 柜台放置。
   - 释放商品。
   - 回 home。

### 5. 配置加载 `ConfigLoader`

`ConfigLoader::Load()` 一次性读取所有配置分片：

- `store.json`：门店、机器人配置名、坐标系。
- `factories.json`：选择 fake/real/ros2/grpc 工厂，例如 `actuator_factory`、`tablet_factory`、`voice_factory`、`vision_factory`。
- `interfaces.json`：平板 gRPC、语音 ROS2、视觉 ROS2 的接口配置。
- `task_policy.json`：主循环周期和重试策略。
- `shelf_layout.json`：货架行列、货位、停车点、预抓取姿态。
- `product_catalog.json`：商品 ID、展示名、包装类型。
- `inventory.json`：商品和货位的库存关系。
- `home.json`：售卖等待点、柜台放置位、空闲姿态。

加载时先写入临时变量 `loaded`，所有文件都成功后才赋值给外部 `config`，这样可以避免加载一半失败后留下半成品配置。

### 6. 硬件抽象、Fake 执行器和真实执行器骨架

`ActionActuator` 是硬件抽象接口，Controller 只认识这个接口，不关心具体厂家 SDK。

当前有两类执行器：

- `FakeRetailActuator`：不依赖真实硬件，但已经按 `ActionType` 分发到动作状态机；普通动作使用短时间模拟完成，视觉动作通过 `VisionService` 获取结果。
- `RealRetailActuator`：真实硬件骨架，`LoopAction()` 按 `ActionType` 分发到 `loop_actSM_xxx()`，每个动作内部按 `ActBegin/ActWait/ActInitialized/ActRunning/ActFailure/ActFinished` 留出 TODO。

真实硬件后续应在 `RealRetailActuator` 中补齐：

- AGV 到站：解析站点，调用底盘导航接口，轮询到站反馈。
- 升降：解析目标高度，调用升降接口，轮询高度反馈。
- 腰/头：解析角度或观察 profile，调用对应设备接口。
- 机械臂：解析命名姿态、m_pose 或 `pose_key=detected_pose`，调用 MoveJ/MoveL 等接口。
- 夹爪：根据 ActionType 打开/闭合左、右或双夹爪。
- 视觉：通过 `VisionService` 发请求、等结果、写入 `mission.m_args`。

### 7. 视觉、语音和平板服务

#### 7.1 视觉服务

视觉是独立软件，真实接入走 ROS2：

- `VisionService` 是抽象接口。
- `FakeVisionService` 同步生成模拟 `detected_pose`。
- `Ros2VisionService` 是 ROS2 骨架，TODO 位置包括创建 node、发布 request topic、订阅 result topic、把结果写入缓存。

#### 7.2 语音服务

语音软件真实接入走 ROS2：

- `VoiceService` 是抽象接口。
- `FakeVoiceService` 只打印日志。
- `Ros2VoiceService` 是 ROS2 骨架，TODO 位置包括订阅语音订单 topic、发布机器人状态 topic、发布任务结果 topic。

#### 7.3 平板服务

平板真实接入走 gRPC：

- `TabletService` 是抽象接口。
- `FakeTabletService` 保留 Qt 调试按钮的订单队列。
- `GrpcTabletService` 是 gRPC 骨架，TODO 位置包括启动 gRPC Server、实现 SubmitOrder/GetState/TaskEvent 等接口。

目前调试界面的按钮会调用 `TabletService::EnqueueDebugOrder()`，相当于模拟平板收到一笔订单；真实平板订单后续应由 `GrpcTabletService` 收到 gRPC 请求后写入同一个订单队列。

### 8. 一次调试订单的完整链路

1. 用户在窗口下拉框选择商品。
2. 点击“发送调试订单”。
3. `MainWindow::onSendDebugOrderClicked()` 调用 `RobotController::SubmitDebugOrder(product_id)`。
4. Controller 创建 `Order`，放入 `TabletService` 内部队列。
5. 下一轮 `Loop()` 中，`AcceptPendingOrders()` 把订单搬到 `m_order_queue`。
6. 如果机器人空闲，`StartNextMissionIfIdle()` 取出订单并调用 `BuildMissionFromOrder()`。
7. 任务被写入 `m_mission`，`m_curr_step` 置 0。
8. 后续每轮 `LoopWorkSM()` 推进一个或多个 Action。
9. 视觉 Action 通过 `VisionService` 得到 `detected_pose`，后续机械臂抓取 Action 通过 `pose_key` 读取该结果。
10. 所有 Beat 完成后 `FinishMission()` 上报结果并把状态切回空闲。


## 构建

```bash
qmake6 autoretailbot.pro -o Makefile
make -j2
```

## 运行

```bash
./autoretailbot
```

无显示环境可使用：

```bash
QT_QPA_PLATFORM=offscreen ./autoretailbot
```
