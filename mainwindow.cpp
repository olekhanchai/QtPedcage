#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtimer.h"
#include <string>
#include <QDateTime>
//#include <wiringSerial.h>
//#include <wiringPi.h>
#include <qdebug.h>
#include <QSettings>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <qfile.h>
#include <qlineseries.h>
#include <QtCharts>
#include <QSqlQuery>
#include <QSqlError>

//int MainWindow::fd = 0;
const int BUFFERLENGTH = 127;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    wiringPiSetup();
//    pinMode(0, OUTPUT);
//    pinMode(1, OUTPUT);
//    digitalWrite(0, 0);
//    digitalWrite(1, 0);
    mTimer.start(1000);
    cameraRunning = false;

    QTimer::singleShot(0, this, SLOT(showFullScreen()));
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(showClock()));

    CO2VisibleGroup(false);
    TempVisibleGroup(false);
    O2VisibleGroup(false);
    HumidityVisibleGroup(false);

    QTimer::connect(ui->lblOpenCam, SIGNAL(clicked()), this, SLOT(on_btnCamOn_clicked()));
    QTimer::connect(ui->lblBlueLight, SIGNAL(clicked()), this, SLOT(on_btnBlueLamp_clicked()));
    QTimer::connect(ui->lblNeoPixel, SIGNAL(clicked()),this, SLOT(on_btnNeoPixel_clicked()));
    QTimer::connect(ui->lblPeltierCooler, SIGNAL(clicked()),this, SLOT(on_btnPeltierCooler_clicked()));
    QTimer::connect(ui->lblSystemFan, SIGNAL(clicked()), this, SLOT(on_btnSystemFan_clicked()));
    QTimer::connect(ui->lblPlug, SIGNAL(clicked()), this, SLOT(on_btnPlug_clicked()));
    QTimer::connect(ui->lblTempIcon, SIGNAL(clicked()), this, SLOT(on_btnRedCtl_clicked()));

    QTimer::connect(ui->lblBack, SIGNAL(clicked()), this, SLOT(on_btnBack_clicked()));


    // Control temp
    //QTimer::connect(ui->lblTempCtl, SIGNAL(clicked()), this, SLOT(on_btnRedCtl_clicked()));
    //QTimer::connect(ui->lblTempCtlDown, SIGNAL(released()), this, SLOT(on_btnRedDownCtl_released()));
    //QTimer::connect(ui->lblTempCtlUp, SIGNAL(clicked()), this, SLOT(on_btnRedUpCtl_clicked()));
    //QTimer::connect(ui->lblTempCtlUp, SIGNAL(released()), this, SLOT(on_btnRedUpCtl_released()));

    // Control oxygen
    //QTimer::connect(ui->lblO2CtlDown, SIGNAL(clicked()), this, SLOT(on_btnGreenDownCtl_clicked()));
    //QTimer::connect(ui->lblO2CtlDown, SIGNAL(released()), this, SLOT(on_btnGreenDownCtl_released()));
    //QTimer::connect(ui->lblO2CtlUp, SIGNAL(clicked()), this, SLOT(on_btnGreenUpCtl_clicked()));
    //QTimer::connect(ui->lblO2CtlUp, SIGNAL(released()), this, SLOT(on_btnGreenUpCtl_released()));

    //Decrease value
//    QTimer::connect(ui->lblRedDown, SIGNAL(clicked()), this, SLOT(on_btnRedDown_clicked()));
//    QTimer::connect(ui->lblRedDown, SIGNAL(released()), this, SLOT(on_btnRedDown_released()));
//    QTimer::connect(ui->lblBlueDown, SIGNAL(clicked()), this, SLOT(on_btnBlueDown_clicked()));
//    QTimer::connect(ui->lblBlueDown, SIGNAL(released()), this, SLOT(on_btnBlueDown_released()));
//    QTimer::connect(ui->lblGreyDown, SIGNAL(clicked()), this, SLOT(on_btnGreyDown_clicked()));
//    QTimer::connect(ui->lblGreyDown, SIGNAL(released()), this, SLOT(on_btnGreyDown_released()));
//    QTimer::connect(ui->lblGreenDown, SIGNAL(clicked()), this, SLOT(on_btnGreenDown_clicked()));
//    QTimer::connect(ui->lblGreenDown, SIGNAL(released()), this, SLOT(on_btnGreenDown_released()));

//    //Increase value
//    QTimer::connect(ui->lblRedUp, SIGNAL(clicked()), this,   SLOT(on_btnRedUp_clicked()));
//    QTimer::connect(ui->lblRedUp, SIGNAL(released()), this,  SLOT(on_btnRedUp_released()));
//    QTimer::connect(ui->lblBlueUp, SIGNAL(clicked()), this,  SLOT(on_btnBlueUp_clicked()));
//    QTimer::connect(ui->lblBlueUp, SIGNAL(released()), this, SLOT(on_btnBlueUp_released()));
//    QTimer::connect(ui->lblGreyUp, SIGNAL(clicked()), this,  SLOT(on_btnGreyUp_clicked()));
//    QTimer::connect(ui->lblGreyUp, SIGNAL(released()), this, SLOT(on_btnGreyUp_released()));
//    QTimer::connect(ui->lblGreenUp, SIGNAL(clicked()), this, SLOT(on_btnGreenUp_clicked()));
//    QTimer::connect(ui->lblGreenUp, SIGNAL(released()), this,SLOT(on_btnGreenUp_released()));


    // Setup serial port
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort.setStopBits(QSerialPort::OneStop);


    m_serialPort.setPortName("/dev/ttyACM0");
    m_serialPort.setBaudRate(115200);


    connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(onSerialPortReadyRead()), Qt::QueuedConnection);
    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialPortError(QSerialPort::SerialPortError)));

    connect(&m_timerConnection, SIGNAL(timeout()), this, SLOT(onTimerConnection()));
    connect(&m_timerStateQuery, SIGNAL(timeout()), this, SLOT(onTimerStateQuery()));
    connect(&m_timerLogData, SIGNAL(timeout()), this, SLOT(onTimerLogData()));

    m_timerConnection.start(1000);
    m_timerStateQuery.start(2000);
    m_timerLogData.start(60000);
    m_statusReceived = true;

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("allsensors.db");

    if (!m_db.open())
    {
       qDebug() << "Error: connection with database failed";
    }
    else
    {
       qDebug() << "Database: connection ok";
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS oxygen(sensor_time datetime primary key, data float);");
    query.exec("CREATE TABLE IF NOT EXISTS carbondioxide(sensor_time datetime primary key, data float);");
    query.exec("CREATE TABLE IF NOT EXISTS temperature(sensor_time datetime primary key, data float);");
    query.exec("CREATE TABLE IF NOT EXISTS humidity(sensor_time datetime primary key, data float);");
}

MainWindow::~MainWindow()
{
//    serialClose(fd);
    delete ui;
}

bool MainWindow::saveOxygenValues()
{
   bool success = false;
   // you should check if args are ok first...
   QSqlQuery query;

   query.prepare("INSERT INTO oxygen values(datetime('now', 'localtime'), :o2)");
   query.bindValue(":o2", oxygenVal);
   if(query.exec())
   {
       qDebug() << "add oxygen value ok";
       success = true;
   }
   else
   {
        qDebug() << "add oxygen value error:"
                 << query.lastError();
   }
   return success;
}

bool MainWindow::saveCarbondioxideValues()
{
   bool success = false;
   // you should check if args are ok first...
   QSqlQuery query;

   query.prepare("INSERT INTO carbondioxide values(datetime('now', 'localtime'), :co2)");
   query.bindValue(":co2", co2Val);
   if(query.exec())
   {
       qDebug() << "add carbondioxide value ok";
       success = true;
   }
   else
   {
        qDebug() << "add carbondioxide value error:"
                 << query.lastError();
   }
   return success;
}

bool MainWindow::saveTemperatureValues()
{
   bool success = false;
   // you should check if args are ok first...
   QSqlQuery query;

   query.prepare("INSERT INTO temperature values(datetime('now', 'localtime'), :temp)");
   query.bindValue(":temp", tempVal);
   if(query.exec())
   {
       qDebug() << "add temperature value ok";
       success = true;
   }
   else
   {
        qDebug() << "add temperature value error:"
                 << query.lastError();
   }
   return success;
}

bool MainWindow::saveHumidityValues()
{
   bool success = false;
   // you should check if args are ok first...
   QSqlQuery query;

   query.prepare("INSERT INTO humidity values(datetime('now', 'localtime'), :humid)");
   query.bindValue(":humid", humidityVal);
   if(query.exec())
   {
       qDebug() << "add humidity value ok";
       success = true;
   }
   else
   {
        qDebug() << "add humidity value error:"
                 << query.lastError();
   }
   return success;
}

void MainWindow::updateDisplay(QString group)
{
    if (group.toUpper() == "UPPER") {
        ui->lblTempDisp->setText(QString::number(tempUpperVal,'f',2) + QString::fromUtf8(" °C"));
        ui->lblHumidityDisp->setText(QString::number(humidityUpperVal,'f',2) + " %RH");
        ui->lblCo2Disp->setText(QString::number(co2UpperVal,'f',2) + " PPM");
        ui->lblOxigenDisp->setText(QString::number(oxygenUpperVal,'f',2) + " %");
    } else if (group.toUpper() == "LOWER") {
        ui->lblTempDisp->setText(QString::number(tempLowerVal,'f',2) + QString::fromUtf8(" °C"));
        ui->lblHumidityDisp->setText(QString::number(humidityLowerVal,'f',2) + " %RH");
        ui->lblCo2Disp->setText(QString::number(co2LowerVal,'f',2) + " PPM");
        ui->lblOxigenDisp->setText(QString::number(oxygenLowerVal,'f',2) + " %");
    } else {
        ui->lblTempDisp->setText(QString::number(tempVal,'f',2) + QString::fromUtf8(" °C"));
        ui->lblHumidityDisp->setText(QString::number(humidityVal,'f',2) + " %RH");
        ui->lblCo2Disp->setText(QString::number(co2Val) + " PPM");
        ui->lblOxigenDisp->setText(QString::number(oxygenVal,'f',2) + " %");
    }
}

void MainWindow::CO2VisibleGroup(bool visible)
{
    ui->lblGreyGear->setVisible(visible);
    ui->lblGreyUp->setVisible(visible);
    ui->lblGreyDown->setVisible(visible);
}

void MainWindow::TempVisibleGroup(bool visible)
{
    ui->lblRedGear->setVisible(visible);
    ui->lblRedUp->setVisible(visible);
    ui->lblRedDown->setVisible(visible);
}

void MainWindow::O2VisibleGroup(bool visible)
{
    ui->lblGreenGear->setVisible(visible);
    ui->lblGreenUp->setVisible(visible);
    ui->lblGreenDown->setVisible(visible);
}

void MainWindow::HumidityVisibleGroup(bool visible)
{
    ui->lblBlueGear->setVisible(visible);
    ui->lblBlueUp->setVisible(visible);
    ui->lblBlueDown->setVisible(visible);
}

void MainWindow::togglePin(int pin, bool condition)
{
    QString cmd = QString("P10 R%1 S0").arg(pin);
    if (!condition) {
        cmd = QString("P10 R%1 S1").arg(pin);
    }
    sendCommand(cmd);
}

void MainWindow::on_btnUpperBound_clicked()
{

}

void MainWindow::on_btnMiddleBound_clicked()
{

}

void MainWindow::on_btnLowerBound_clicked()
{

}

void MainWindow::on_btnBlueLamp_clicked()
{
    ui->lblBlueLight->setStyleSheet(mapBlueLight[ui->lblBlueLight->isClicked()]);
    togglePin(mapRelay["UV"], ui->lblBlueLight->isClicked());
}

void MainWindow::on_btnNeoPixel_clicked()
{
    ui->lblNeoPixel->setStyleSheet(mapNeoPixel[ui->lblNeoPixel->isClicked()]);
    if (ui->lblNeoPixel->isClicked()) {
        sendCommand("P60 R255 G255 B255");
    } else {
        sendCommand("P60 R0 G0 B0");
    }
}

void MainWindow::on_btnPeltierCooler_clicked()
{
    ui->lblPeltierCooler->setStyleSheet(mapPeltier[ui->lblPeltierCooler->isClicked()]);
    togglePin(mapRelay["PeltierFan"], ui->lblPeltierCooler->isClicked());
}

void MainWindow::on_btnSystemFan_clicked()
{
    ui->lblSystemFan->setStyleSheet(mapSystemFan[ui->lblSystemFan->isClicked()]);
    togglePin(mapRelay["DownFan"], ui->lblSystemFan->isClicked());
}

void MainWindow::on_btnPlug_clicked()
{
    ui->lblPlug->setStyleSheet(mapPlug[ui->lblPlug->isClicked()]);
    togglePin(mapRelay["Plug"], ui->lblPlug->isClicked());
}

void MainWindow::on_btnCamOn_clicked()
{
    // Initialize the thread and worker
    workerThread = new QThread;
    worker = new CameraWorker;

    // Check to make sure the camera is not already running
    // If it is, return without doing anything.
    if (cameraRunning) {
        return;
    }

    // Setup the thread
    worker->moveToThread(workerThread);

    // Connect signals to slots
    connect(workerThread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), workerThread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
    connect(worker, SIGNAL(finished()), this, SLOT(cameraFinished()));
    connect(worker, SIGNAL(handleImage(QImage &)), this, SLOT(handleImage(QImage &)));
    connect(ui->lblHumidityIcon, SIGNAL(clicked()), worker, SLOT(stopWork()));
    workerThread->start();

    // Update the running flag
    cameraRunning = true;
}

void MainWindow::handleImage(QImage &image)
{
    // Update the image shown
    ui->lblOxigenBackground->setPixmap(QPixmap::fromImage(image));

    // Force an update of the UI so that the image is shown immediately.
    QApplication::processEvents();
    this->repaint();
}

void MainWindow::cameraFinished()
{
    // Update running flag
    cameraRunning = false;

    // Reset the image label
    ui->lblOxigenBackground->setText("Camera Disabled");
}

void MainWindow::on_btnRedDown_clicked()
{
    ui->lblRedDown->setStyleSheet(mapPush[btnColor::red]);
    switch (tempMode) {
        case 0: {
            ui->lblTempDisp->setText(QString::number(--tempLowerVal) + QString::fromUtf8(" °C"));
        };break;
        case 1: {
            ui->lblTempDisp->setText(QString::number(--tempVal) + QString::fromUtf8(" °C"));
        };break;
        case 2: {
            ui->lblTempDisp->setText(QString::number(--tempUpperVal) + QString::fromUtf8(" °C"));
        }break;
    }
}

void MainWindow::on_btnRedDown_released()
{
    ui->lblRedDown->setStyleSheet(mapRelease[btnColor::red]);
}

void MainWindow::on_btnRedDownCtl_clicked()
{
    //ui->lblTempCtlDown->setStyleSheet(mapPush[btnColor::red]);
}

void MainWindow::on_btnRedDownCtl_released()
{
    //ui->lblTempCtlDown->setStyleSheet(mapRelease[btnColor::red]);
}

void MainWindow::on_btnRedUpCtl_clicked()
{
    //ui->lblTempCtlUp->setStyleSheet(mapPushUp[btnColor::red]);
}

void MainWindow::on_btnRedUpCtl_released()
{
    //ui->lblTempCtlUp->setStyleSheet(mapReleaseUp[btnColor::red]);
}

void MainWindow::on_btnGreenDownCtl_clicked()
{
    //ui->lblO2CtlDown->setStyleSheet(mapPush[btnColor::green]);
}

void MainWindow::on_btnGreenDownCtl_released()
{
    //ui->lblO2CtlDown->setStyleSheet(mapRelease[btnColor::green]);
}

void MainWindow::on_btnGreenUpCtl_clicked()
{
    //ui->lblO2CtlUp->setStyleSheet(mapPushUp[btnColor::green]);
}

void MainWindow::on_btnGreenUpCtl_released()
{
    //ui->lblO2CtlUp->setStyleSheet(mapReleaseUp[btnColor::green]);
}

void MainWindow::on_btnRedCtl_clicked()
{
    ui->stackedWidget->widget(1)->show();
    ui->stackedWidget->widget(1)->resize(800, 480);
    ui->stackedWidget->widget(0)->hide();
    QLineSeries *series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");
    ui->gView = new QChartView(chart);
    ui->gView->setRenderHint(QPainter::Antialiasing);
    ui->gView->repaint();
}

void MainWindow::on_btnBlueDown_clicked()
{
    ui->lblBlueDown->setStyleSheet(mapPush[btnColor::blue]);
    switch (humidityMode) {
        case 0: {
            ui->lblHumidityDisp->setText(QString::number(--humidityLowerVal) + " %RH");
        };break;
        case 1: {
            ui->lblHumidityDisp->setText(QString::number(--humidityVal) + " %RH");
        };break;
        case 2: {
            ui->lblHumidityDisp->setText(QString::number(--humidityUpperVal) + " %RH");
        }break;
    }
}

void MainWindow::on_btnBlueDown_released()
{
    ui->lblBlueDown->setStyleSheet(mapRelease[btnColor::blue]);
}

void MainWindow::on_btnGreyDown_clicked()
{
    //ui->lblGreyDown->setStyleSheet(mapPush[btnColor::grey]);
    switch (co2Mode) {
        case 0: {
            ui->lblCo2Disp->setText(QString::number(--co2LowerVal) + " PPM");
        };break;
        case 1: {
            ui->lblCo2Disp->setText(QString::number(--co2Val) + " PPM");
        };break;
        case 2: {
            ui->lblCo2Disp->setText(QString::number(--co2UpperVal) + " PPM");
        }break;
    }
}

void MainWindow::on_btnGreyDown_released()
{
    //ui->lblGreyDown->setStyleSheet(mapRelease[btnColor::grey]);
}

void MainWindow::on_btnGreenDown_clicked()
{
    ui->lblGreenDown->setStyleSheet(mapPush[btnColor::green]);
    switch (oxygenMode) {
        case 0: {
            //ui->lblO2Disp->setText(QString::number(--oxygenLowerVal) + " %");
        };break;
        case 1: {
            //ui->lblO2Disp->setText(QString::number(--oxygenVal) + " %");
        };break;
        case 2: {
            //ui->lblO2Disp->setText(QString::number(--oxygenUpperVal) + " %");
        }break;
    }
}

void MainWindow::on_btnGreenDown_released()
{
    ui->lblGreenDown->setStyleSheet(mapRelease[btnColor::green]);
}

void MainWindow::on_btnRedUp_clicked()
{
    ui->lblRedUp->setStyleSheet(mapPushUp[btnColor::red]);
    switch (tempMode) {
        case 0: {
            ui->lblTempDisp->setText(QString::number(++tempLowerVal) + QString::fromUtf8(" °C"));
        };break;
        case 1: {
            ui->lblTempDisp->setText(QString::number(++tempVal) + QString::fromUtf8(" °C"));
        };break;
        case 2: {
            ui->lblTempDisp->setText(QString::number(++tempUpperVal) + QString::fromUtf8(" °C"));
        }break;
    }

}

void MainWindow::on_btnRedUp_released()
{
    ui->lblRedUp->setStyleSheet(mapReleaseUp[btnColor::red]);
}

void MainWindow::on_btnBlueUp_clicked()
{
    ui->lblBlueUp->setStyleSheet(mapPushUp[btnColor::blue]);
    switch (humidityMode) {
        case 0: {
            ui->lblHumidityDisp->setText(QString::number(++humidityLowerVal) + " %RH");
        };break;
        case 1: {
            ui->lblHumidityDisp->setText(QString::number(++humidityVal) + " %RH");
        };break;
        case 2: {
            ui->lblHumidityDisp->setText(QString::number(++humidityUpperVal) + " %RH");
        }break;
    }
}

void MainWindow::on_btnBlueUp_released()
{
    ui->lblBlueUp->setStyleSheet(mapReleaseUp[btnColor::blue]);
}

void MainWindow::on_btnGreyUp_clicked()
{
    //ui->lblGreyUp->setStyleSheet(mapPushUp[btnColor::grey]);
    switch (co2Mode) {
        case 0: {
            ui->lblCo2Disp->setText(QString::number(++co2LowerVal) + " PPM");
        };break;
        case 1: {
            ui->lblCo2Disp->setText(QString::number(++co2Val) + " PPM");
        };break;
        case 2: {
            ui->lblCo2Disp->setText(QString::number(++co2UpperVal) + " PPM");
        }break;
    }
}

void MainWindow::on_btnGreyUp_released()
{
    //ui->lblGreyUp->setStyleSheet(mapReleaseUp[btnColor::grey]);
}

void MainWindow::on_btnGreenUp_clicked()
{
    ui->lblGreenUp->setStyleSheet(mapPushUp[btnColor::green]);
    switch (oxygenMode) {
        case 0: {
            //ui->lblO2Disp->setText(QString::number(++oxygenLowerVal) + " %");
        };break;
        case 1: {
            //ui->lblO2Disp->setText(QString::number(++oxygenVal) + " %");
        };break;
        case 2: {
            //ui->lblO2Disp->setText(QString::number(++oxygenUpperVal) + " %");
        }break;
    }

}

void MainWindow::on_btnGreenUp_released()
{
    ui->lblGreenUp->setStyleSheet(mapReleaseUp[btnColor::green]);
}

void MainWindow::showClock()
{
    //Show sensor value with clock timer
    updateDisplay();
    ui->lblDateTime->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss"));
}

void MainWindow::onSerialPortReadyRead()
{
    while (m_serialPort.canReadLine()) {
        QString data = m_serialPort.readLine().trimmed();

        if (data.length() > 0) {

            // Processed commands
            if (m_commands.length() > 0) {

                static QString response; // Full response string

                if ((m_commands[0].command != "[CTRL+X]" && dataIsEnd(data))) {

                    response.append(data);

                    // Take command from buffer
                    CommandAttributes ca = m_commands.takeFirst();

                    // Check queue
                    if (m_queue.length() > 0) {
                        CommandQueue cq = m_queue.takeFirst();
                        while ((bufferLength() + cq.command.length() + 1) <= BUFFERLENGTH) {
                            sendCommand(cq.command, cq.tableIndex, cq.showInConsole);
                            if (m_queue.isEmpty()) break; else cq = m_queue.takeFirst();
                        }
                    }

                    if (ca.command.contains("P60") && response.contains("ok")) {

                    }
                    m_statusReceived = true;
                    response.clear();
                } else {
                    response.append(data + "; ");
                }

            } else {
                // get current sensor value every 1 second.
                QStringList params = data.split(" ");
                foreach (QString vals, params) {
                    QStringList val = vals.split(":");
                    if (val.count() == 2) {
                        QString unit = val.at(0);
                        QString value = val.at(1);
                        if (unit == "H") {
                            humidityVal = value.toFloat();
                        }
                        if (unit == "T") {
                            tempVal = value.toFloat();
                        }
                        if (unit == "C") {
                            co2Val = value.toFloat();
                        }
                        if (unit == "Z") {
                            co2Val = value.toFloat();
                        }
                        if (unit == "O") {
                            oxygenVal = value.toFloat();
                        }
                    }
                }
            }
            m_statusReceived = true;
        }
    }
}

void MainWindow::onTimerConnection()
{
    if (!m_serialPort.isOpen()) {
        openPort();
    }
}

void MainWindow::onTimerStateQuery()
{
    if (m_serialPort.isOpen() && m_resetCompleted && m_statusReceived) {
        m_serialPort.write("P40\r\n");
        m_serialPort.write("P50\r\n");
        //m_statusReceived = false;
    }
}

void MainWindow::onTimerLogData()
{
    saveOxygenValues();
    saveCarbondioxideValues();
    saveTemperatureValues();
    saveHumidityValues();
}

void MainWindow::openPort()
{
    if (m_serialPort.open(QIODevice::ReadWrite)) {
//        ui->txtStatus->setText(tr("Port opened"));
//        ui->txtStatus->setStyleSheet(QString("background-color: palette(button); color: palette(text);"));
////        updateControlsState();
//        grblReset();
    }
}

int MainWindow::bufferLength()
{
    int length = 0;

    foreach (CommandAttributes ca, m_commands) {
        length += ca.length;
    }

    return length;
}

void MainWindow::sendCommand(QString command, int tableIndex, bool showInConsole)
{
    if (!m_serialPort.isOpen()/* || !m_resetCompleted*/) return;

    command = command.toUpper();

    // Commands queue
   if ((bufferLength() + command.length() + 1) > BUFFERLENGTH) {
        qDebug() << "queue:" << command;

        CommandQueue cq;

        cq.command = command;
        cq.tableIndex = tableIndex;
        cq.showInConsole = showInConsole;

        m_queue.append(cq);
        return;
    }

    CommandAttributes ca;

    ca.command = command;
    ca.length = command.length() + 1;
    ca.tableIndex = tableIndex;

    m_commands.append(ca);

    m_serialPort.write((command + "\r").toLatin1());
}

bool MainWindow::dataIsEnd(QString data) {
    QStringList ends;

    ends << "ok";
    ends << "error";
//    ends << "Reset to continue";
//    ends << "'$' for help";
//    ends << "'$H'|'$X' to unlock";
//    ends << "Caution: Unlocked";
//    ends << "Enabled";
//    ends << "Disabled";
//    ends << "Check Door";
//    ends << "Pgm End";

    foreach (QString str, ends) {
        if (data.contains(str)) return true;
    }

    return false;
}

void MainWindow::onSerialPortError(QSerialPort::SerialPortError){};

void MainWindow::on_btnBack_clicked() {
    ui->stackedWidget->widget(0)->show();
    ui->stackedWidget->widget(0)->resize(MainWindow::width(), MainWindow::height());
    ui->stackedWidget->widget(1)->hide();
}
