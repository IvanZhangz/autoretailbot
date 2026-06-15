// 文件说明：实现 Qt 调试主窗口。
// 前端通过 signal/slot 与 RobotController 通讯，不持有硬件、服务或任务状态机。
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "controller/RobotController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QTextEdit>

#define STRINGIFY_DETAIL(m_x) #m_x
#define STRINGIFY(m_x) STRINGIFY_DETAIL(m_x)

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // setupUi 会根据 mainwindow.ui 创建控件并绑定到 ui 指针。
    ui->setupUi(this);

    // MainWindow 不创建后端对象，只取 Controller 单例并连接信号。
    asd_retail::RobotController& controller = asd_retail::RobotController::Instance();
    connect(&controller, &asd_retail::RobotController::LogMessage, this, &MainWindow::appendLog);
    connect(&controller, &asd_retail::RobotController::StateChanged, this, &MainWindow::onControllerStateChanged);
    connect(ui->sendDebugOrderButton, &QPushButton::clicked, this, &MainWindow::onSendDebugOrderClicked);

    // 初始化 Controller；失败时禁用调试按钮，避免继续发送订单。
    QString error;
    if (!controller.Init(resolveConfigDir(), &error))
    {
        appendLog("[ERROR] Controller 初始化失败: " + error);
        ui->sendDebugOrderButton->setEnabled(false);
        return;
    }

    // 商品下拉框的数据来自 product_catalog.json 解析后的配置。
    ui->productComboBox->addItems(controller.ProductIds());
    appendLog("[INFO] 前端初始化完成，可发送调试订单");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendDebugOrderClicked()
{
    const QString product_id = ui->productComboBox->currentText();
    appendLog("[UI] 发送调试订单: " + product_id);
    asd_retail::RobotController::Instance().SubmitDebugOrder(product_id);
}

void MainWindow::appendLog(const QString& message)
{
    if (ui && ui->logEdit)
        ui->logEdit->append(message);
}

void MainWindow::onControllerStateChanged(const QString& state)
{
    if (statusBar())
        statusBar()->showMessage(state);
}

// 运行时优先使用外部 config，方便部署改配置；都找不到时回退到 Qt 资源。
QString MainWindow::resolveConfigDir() const
{
    const QString appConfigDir = QCoreApplication::applicationDirPath() + "/config";
    if (QFileInfo::exists(appConfigDir + "/store.json")) return appConfigDir;

    const QString cwdConfigDir = QDir::currentPath() + "/config";
    if (QFileInfo::exists(cwdConfigDir + "/store.json")) return cwdConfigDir;

#ifdef PROJECT_SOURCE_DIR
    const QString sourceConfigDir = QStringLiteral(STRINGIFY(PROJECT_SOURCE_DIR)) + "/config";
    if (QFileInfo::exists(sourceConfigDir + "/store.json")) return sourceConfigDir;
#endif

    return ":/config";
}
