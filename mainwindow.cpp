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

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    RobotController& controller = RobotController::instance();
    connect(&controller, &RobotController::logMessage, this, &MainWindow::appendLog);
    connect(&controller, &RobotController::stateChanged, this, &MainWindow::onControllerStateChanged);
    connect(ui->sendDebugOrderButton, &QPushButton::clicked, this, &MainWindow::onSendDebugOrderClicked);

    QString error;
    if (!controller.init(resolveConfigDir(), &error))
    {
        appendLog("[ERROR] Controller 初始化失败: " + error);
        ui->sendDebugOrderButton->setEnabled(false);
        return;
    }

    ui->productComboBox->addItems(controller.productIds());
    appendLog("[INFO] 前端初始化完成，可发送调试订单");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendDebugOrderClicked()
{
    const QString productId = ui->productComboBox->currentText();
    appendLog("[UI] 发送调试订单: " + productId);
    RobotController::instance().submitDebugOrder(productId);
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
