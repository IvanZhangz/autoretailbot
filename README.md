# Mvp_Cup 无人零售机器人框架

无人零售机器人控制框架核心原则是：

1. `main.cpp`创建Qt应用、后端`RobotController`单例和前端`MainWindow`。
2. `MainWindow`只做前端调试显示，不创建硬件、不执行任务。
3. `RobotController`是后端核心，周期调用`loop()`。
4. `loop()`按顺序轮询设备、轮询外部服务、接收订单、启动任务、轮询任务状态机。
5. 任务模型为`RobotMission -> RobotBeat -> RobotAction`：Mission包含多个Beat；Beat顺序执行；同一Beat内Action同时开始，全部完成后进入下一Beat。
6. 硬件由`ActuatorFactory`根据配置创建，通过`ActionActuator`抽象接口执行动作。
7. 平板和语音服务分别在相应文件中`start()`并在`loop()`中监听/处理。

## 目录说明

- `controller/`：机器人后端控制器单例，包含主循环和`loop_workSM()`。
- `mission/`：任务、节拍、动作的数据结构。
- `hardware/`：硬件抽象接口、执行器工厂和 Fake 执行器。
- `interfaces/`：平板服务和语音服务占位实现。
- `config/`：分文件配置，JSON中使用`_comment`字段说明字段作用。
- `model/`：跨模块共享数据结构。
- `mainwindow.*`：Qt调试前端。

## 构建

```bash
qmake6 Mvp_Cup.pro -o Makefile
make -j2
```

## 运行

```bash
./Mvp_Cup
```

无显示环境可使用：

```bash
QT_QPA_PLATFORM=offscreen ./Mvp_Cup
```
