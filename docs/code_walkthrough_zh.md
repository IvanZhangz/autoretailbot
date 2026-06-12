# 代码讲解（从 `main.cpp` 开始）

本文面向第一次阅读该项目的人，按程序启动后的真实调用顺序解释每个模块做什么、关键函数和关键变量分别有什么用途。

## 1. `main.cpp`：程序入口

1. `QApplication app(argc, argv);` 创建 Qt GUI 应用对象，后续窗口显示、信号槽和定时器都依赖它。
2. `RobotController& controller = RobotController::instance();` 取得后端控制器单例。这里提前创建 Controller，是为了让机器人后端和前端窗口解耦：窗口只负责显示和按钮，不负责机器人核心逻辑。
3. `Q_UNUSED(controller);` 告诉编译器这个变量是有意暂时不用，避免未使用警告。
4. `MainWindow w; w.show();` 创建并显示调试窗口。`MainWindow` 构造函数内部会连接 Controller 信号并调用 Controller 初始化。
5. `return app.exec();` 进入 Qt 事件循环。进入事件循环后，按钮点击、日志刷新、Controller 的 `timerEvent()` 都由 Qt 分发。

## 2. `MainWindow`：调试前端

`MainWindow` 是薄前端，只做三件事：

- 初始化界面控件和 Controller。
- 把 Controller 日志显示到日志框。
- 把“发送调试订单”按钮转换成 Controller 的调试订单。

关键流程：

1. `ui->setupUi(this)` 根据 `mainwindow.ui` 创建商品下拉框、按钮、日志框和状态栏。
2. `RobotController::instance()` 获取同一个后端单例。
3. `connect(...)` 把后端信号接到前端槽函数：
   - `logMessage -> appendLog`：日志进入 `QTextEdit`。
   - `stateChanged -> onControllerStateChanged`：状态显示到状态栏。
   - `sendDebugOrderButton.clicked -> onSendDebugOrderClicked`：按钮触发调试订单。
4. `controller.init(resolveConfigDir(), &error)` 初始化后端。
5. `productComboBox->addItems(controller.productIds())` 把配置里的商品 ID 加入下拉框。

`resolveConfigDir()` 的优先级是：程序所在目录的 `config`、当前工作目录的 `config`、源码目录的 `config`、最后回退到 Qt 资源 `:/config`。

## 3. `RobotController`：后端控制核心

`RobotController` 是机器人业务核心，采用单例模式，主要变量如下：

- `m_config`：加载后的总配置，包括货架、库存、商品、home、接口等。
- `m_initialized`：后端是否初始化成功，未成功时不接单。
- `m_timerId`：Qt 定时器 ID。
- `m_currStep`：当前执行到第几个 `RobotBeat`。
- `m_state`：机器人当前状态文本。
- `m_orderQueue`：平板、语音、调试订单汇总后的统一队列。
- `m_mission`：当前正在执行或刚执行完的任务。
- `m_actuator`：动作执行器，当前是 `FakeRetailActuator`。
- `m_tabletService`、`m_voiceService`：平板和语音服务。

### 3.1 初始化 `init()`

初始化顺序如下：

1. 用 `ConfigLoader` 读取配置目录下的多个 JSON 文件，填入 `m_config`。
2. 用 `ActuatorFactory::create(m_config.factories)` 创建动作执行器。
3. 初始化执行器 `m_actuator->init(m_config)`。
4. 创建 `TabletService` 和 `VoiceService`，连接它们的日志信号。
5. 启动平板和语音服务。
6. 构造一个启动回 home 任务，让机器人先回安全姿态。
7. `startTimer(m_config.taskPolicy.loopIntervalMs)` 启动主循环定时器。

### 3.2 主循环 `loop()`

`timerEvent()` 每隔 `loop_interval_ms` 调用一次 `loop()`。`loop()` 的顺序是固定的：

1. `loopDevices()`：轮询硬件设备。
2. `loopServices()`：轮询平板和语音服务。
3. `acceptPendingOrders()`：把服务内部队列中的订单搬到 `m_orderQueue`。
4. `startNextMissionIfIdle()`：如果当前任务结束且有订单，则构建新任务。
5. `loop_workSM()`：推进当前任务状态机。

### 3.3 任务状态机 `loop_workSM()`

任务模型是：`RobotMission -> RobotBeat -> RobotAction`。

- 一个 `Mission` 是完整订单任务。
- 一个 `Beat` 是一组可以并行推进的动作。
- 一个 `Action` 是最小动作，例如 AGV 到站点、升降到高度、右臂移动、视觉识别、夹爪打开。

执行规则：

1. `m_currStep` 指向当前 `Beat`。
2. 遍历当前 `Beat` 的所有 `Action`。
3. 每个 Action 先绑定 `m_mission.args`，这样视觉动作写入的数据可以被后续动作读取。
4. 调用 `m_actuator->loopAction(action)` 推进动作。
5. 只要有一个 Action 没完成，本 Beat 就没完成。
6. 当前 Beat 全部完成后，`m_currStep++` 进入下一 Beat。
7. 所有 Beat 完成后调用 `finishMission(true, "任务完成")`。

## 4. 订单如何变成任务

`buildMissionFromOrder()` 做转换：

1. 检查订单是否为空。
2. 取当前 MVP 的第一个商品。
3. 在商品目录中确认商品存在。
4. 在库存中查找该商品的可用货位。
5. 根据货位查货架配置和停车点。
6. 根据商品包装类型判断使用双臂还是右臂单手：`boxed` 使用双臂，否则右臂。
7. 生成一串 Beat：
   - 到达货架并准备观察/抓取。
   - 视觉识别。
   - 打开夹爪。
   - 移动到视觉抓取位。
   - 闭合夹爪。
   - 进入携物姿态。
   - 返回售卖点并准备交付。
   - 柜台放置。
   - 释放商品。
   - 回 home。

## 5. 配置加载 `ConfigLoader`

`ConfigLoader::load()` 一次性读取所有配置分片：

- `store.json`：门店、机器人配置名、坐标系。
- `factories.json`：使用 fake 还是真实硬件/服务工厂。
- `interfaces.json`：平板、语音、视觉接口配置。
- `task_policy.json`：主循环周期和重试策略。
- `shelf_layout.json`：货架行列、货位、停车点、预抓取姿态。
- `product_catalog.json`：商品 ID、展示名、包装类型。
- `inventory.json`：商品和货位的库存关系。
- `home.json`：售卖等待点、柜台放置位、空闲姿态。

加载时先写入临时变量 `loaded`，所有文件都成功后才赋值给外部 `config`，这样可以避免加载一半失败后留下半成品配置。

## 6. 硬件抽象和 Fake 执行器

`ActionActuator` 是硬件抽象接口，Controller 只认识这个接口，不关心具体厂家 SDK。

当前实现是 `FakeRetailActuator`：

- `init()` 创建底盘、左右臂、升降、腰、颈、左右夹爪的 Fake 设备。
- `loopDev()` 轮询所有 Fake 设备。
- `loopAction()` 模拟动作状态机：`ActBegin -> ActRunning -> ActFinished`。
- `durationMs()` 当前统一模拟每个动作 150ms。
- `finishAction()` 在动作完成时做收尾；视觉动作会写入 `vision_success`、`detected_pose` 和 `confidence` 到 `missionArgs`。

## 7. 平板服务和语音服务

`TabletService` 和 `VoiceService` 都是占位实现，保留了真实接入需要的结构：

- `start()` 读取 endpoint 并标记运行。
- `loop()` 保留事件轮询入口。
- `hasOrder()` / `takeOrder()` 给 Controller 取订单。
- `reportRobotState()` 上报机器人状态。
- `reportTaskResult()` 上报任务结果。

目前调试界面的按钮会调用 `TabletService::enqueueDebugOrder()`，相当于模拟平板收到一笔订单。

## 8. 一次调试订单的完整链路

1. 用户在窗口下拉框选择商品。
2. 点击“发送调试订单”。
3. `MainWindow::onSendDebugOrderClicked()` 调用 `RobotController::submitDebugOrder(productId)`。
4. Controller 创建 `Order`，放入 `TabletService` 内部队列。
5. 下一轮 `loop()` 中，`acceptPendingOrders()` 把订单搬到 `m_orderQueue`。
6. 如果机器人空闲，`startNextMissionIfIdle()` 取出订单并调用 `buildMissionFromOrder()`。
7. 任务被写入 `m_mission`，`m_currStep` 置 0。
8. 后续每轮 `loop_workSM()` 推进一个或多个 Action。
9. 所有 Beat 完成后 `finishMission()` 上报结果并把状态切回空闲。
