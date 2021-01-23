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

//int MainWindow::fd = 0;
const int BUFFERLENGTH = 127;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initialSensor();
//    wiringPiSetup();
//    pinMode(0, OUTPUT);
//    pinMode(1, OUTPUT);
//    digitalWrite(0, 0);
//    digitalWrite(1, 0);
    mTimer.start(1000);

    QTimer::singleShot(0, this, SLOT(showFullScreen()));
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(showClock()));

    SetupVisibleGroup(false);
    CO2VisibleGroup(false);
    TempVisibleGroup(false);
    O2VisibleGroup(false);
    HumidityVisibleGroup(false);

    QTimer::connect(ui->lblStart, SIGNAL(clicked()), this, SLOT(on_btnStart_clicked()));
    QTimer::connect(ui->lblOpenCam, SIGNAL(clicked()), this, SLOT(on_btnCamOn_clicked()));
    QTimer::connect(ui->lblBlueLight, SIGNAL(clicked()), this, SLOT(on_btnBlueLamp_clicked()));
    QTimer::connect(ui->lblYellowLight, SIGNAL(clicked()), this, SLOT(on_btnYellowLamp_clicked()));

    QTimer::connect(ui->lblUpperBound, SIGNAL(clicked()), this, SLOT(on_btnUpperBound_clicked()));
    QTimer::connect(ui->lblMiddleBound, SIGNAL(clicked()), this, SLOT(on_btnMiddleBound_clicked()));
    QTimer::connect(ui->lblLowerBound, SIGNAL(clicked()), this, SLOT(on_btnLowerBound_clicked()));

    // Control temp
    QTimer::connect(ui->lblTempCtl, SIGNAL(clicked()), this, SLOT(on_btnRedCtl_clicked()));
    //QTimer::connect(ui->lblTempCtlDown, SIGNAL(released()), this, SLOT(on_btnRedDownCtl_released()));
    //QTimer::connect(ui->lblTempCtlUp, SIGNAL(clicked()), this, SLOT(on_btnRedUpCtl_clicked()));
    //QTimer::connect(ui->lblTempCtlUp, SIGNAL(released()), this, SLOT(on_btnRedUpCtl_released()));

    // Control oxygen
    //QTimer::connect(ui->lblO2CtlDown, SIGNAL(clicked()), this, SLOT(on_btnGreenDownCtl_clicked()));
    //QTimer::connect(ui->lblO2CtlDown, SIGNAL(released()), this, SLOT(on_btnGreenDownCtl_released()));
    //QTimer::connect(ui->lblO2CtlUp, SIGNAL(clicked()), this, SLOT(on_btnGreenUpCtl_clicked()));
    //QTimer::connect(ui->lblO2CtlUp, SIGNAL(released()), this, SLOT(on_btnGreenUpCtl_released()));

    //Decrease value
    QTimer::connect(ui->lblRedDown, SIGNAL(clicked()), this, SLOT(on_btnRedDown_clicked()));
    QTimer::connect(ui->lblRedDown, SIGNAL(released()), this, SLOT(on_btnRedDown_released()));
    QTimer::connect(ui->lblBlueDown, SIGNAL(clicked()), this, SLOT(on_btnBlueDown_clicked()));
    QTimer::connect(ui->lblBlueDown, SIGNAL(released()), this, SLOT(on_btnBlueDown_released()));
    QTimer::connect(ui->lblGreyDown, SIGNAL(clicked()), this, SLOT(on_btnGreyDown_clicked()));
    QTimer::connect(ui->lblGreyDown, SIGNAL(released()), this, SLOT(on_btnGreyDown_released()));
    QTimer::connect(ui->lblGreenDown, SIGNAL(clicked()), this, SLOT(on_btnGreenDown_clicked()));
    QTimer::connect(ui->lblGreenDown, SIGNAL(released()), this, SLOT(on_btnGreenDown_released()));

    //Increase value
    QTimer::connect(ui->lblRedUp, SIGNAL(clicked()), this,   SLOT(on_btnRedUp_clicked()));
    QTimer::connect(ui->lblRedUp, SIGNAL(released()), this,  SLOT(on_btnRedUp_released()));
    QTimer::connect(ui->lblBlueUp, SIGNAL(clicked()), this,  SLOT(on_btnBlueUp_clicked()));
    QTimer::connect(ui->lblBlueUp, SIGNAL(released()), this, SLOT(on_btnBlueUp_released()));
    QTimer::connect(ui->lblGreyUp, SIGNAL(clicked()), this,  SLOT(on_btnGreyUp_clicked()));
    QTimer::connect(ui->lblGreyUp, SIGNAL(released()), this, SLOT(on_btnGreyUp_released()));
    QTimer::connect(ui->lblGreenUp, SIGNAL(clicked()), this, SLOT(on_btnGreenUp_clicked()));
    QTimer::connect(ui->lblGreenUp, SIGNAL(released()), this,SLOT(on_btnGreenUp_released()));


    // Setup serial port
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort.setStopBits(QSerialPort::OneStop);


    m_serialPort.setPortName("COM11");
    m_serialPort.setBaudRate(115200);


    connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(onSerialPortReadyRead()), Qt::QueuedConnection);
    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialPortError(QSerialPort::SerialPortError)));

    connect(&m_timerConnection, SIGNAL(timeout()), this, SLOT(onTimerConnection()));
    connect(&m_timerStateQuery, SIGNAL(timeout()), this, SLOT(onTimerStateQuery()));
    m_timerConnection.start(1000);
    m_timerStateQuery.start(2000);

    //Start pet live time
    ui->btnLowerBound->setVisible(true);
    ui->lblLowerBound->setVisible(true);
    ui->lblLowerBound->setText("Start Treat");
    ui->lblStartDateTime->setStyleSheet("background-color: rgb(50, 50, 200);");
    ui->lblStartDateTime->setText("<html><head/><body><p><span style=\" color:#ffffff;\">Start Date/time</span></p></body></html>");
    m_statusReceived = true;
}

MainWindow::~MainWindow()
{
//    serialClose(fd);
    delete ui;
}

void MainWindow::SetupVisibleGroup(bool visible)
{
        ui->lblUpperBound->setVisible(visible);
        ui->lblLowerBound->setVisible(visible);
        ui->lblMiddleBound->setVisible(visible);
        ui->btnMiddleBound->setVisible(false);
        ui->btnUpperBound->setVisible(false);
        ui->btnLowerBound->setVisible(visible);

        ui->lblGreyGear->setVisible(false);
        ui->lblGreenGear->setVisible(false);
        ui->lblBlueGear->setVisible(false);
        ui->lblRedGear->setVisible(false);
}

void MainWindow::ToggleVisibleGroup(int button)
{

        ui->lblGreyGear->setVisible(true);
        ui->lblGreenGear->setVisible(true);
        ui->lblBlueGear->setVisible(true);
        ui->lblRedGear->setVisible(true);

        ui->lblUpperBound->setVisible(true);
        ui->lblMiddleBound->setVisible(true);
        ui->lblLowerBound->setVisible(true);
        switch (button) {
            case 1: {
                ui->btnLowerBound->setVisible(true);
                ui->btnMiddleBound->setVisible(false);
                ui->btnUpperBound->setVisible(false);
                tempMode = 0;
                humidityMode = 0;
                co2Mode = 0;
                oxygenMode = 0;
                ui->lblTempDisp->setText(QString::number(tempLowerVal) + QString::fromUtf8(" °C"));
                ui->lblHumidityDisp->setText(QString::number(humidityLowerVal) + " %RH");
                ui->lblCo2Disp->setText(QString::number(co2LowerVal) + " PPM");
                ui->lblO2Disp->setText(QString::number(oxygenLowerVal) + " %");
            };break;
            case 2: {
                ui->btnLowerBound->setVisible(false);
                ui->btnMiddleBound->setVisible(true);
                ui->btnUpperBound->setVisible(false);
                tempMode = 1;
                humidityMode = 1;
                co2Mode = 1;
                oxygenMode = 1;
                ui->lblTempDisp->setText(QString::number(tempVal) + QString::fromUtf8(" °C"));
                ui->lblHumidityDisp->setText(QString::number(humidityVal) + " %RH");
                ui->lblCo2Disp->setText(QString::number(co2Val) + " PPM");
                ui->lblO2Disp->setText(QString::number(oxygenVal) + " %");
            };break;
            case 3: {
                ui->btnLowerBound->setVisible(false);
                ui->btnMiddleBound->setVisible(false);
                ui->btnUpperBound->setVisible(true);
                tempMode = 2;
                humidityMode = 2;
                co2Mode = 2;
                oxygenMode = 2;
                ui->lblTempDisp->setText(QString::number(tempUpperVal) + QString::fromUtf8(" °C"));
                ui->lblHumidityDisp->setText(QString::number(humidityUpperVal) + " %RH");
                ui->lblCo2Disp->setText(QString::number(co2UpperVal) + " PPM");
                ui->lblO2Disp->setText(QString::number(oxygenUpperVal) + " %");
            };break;
        }
}

void MainWindow::CO2VisibleGroup(bool visible)
{
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

void MainWindow::on_btnStart_clicked()
{
    ui->lblStart->setStyleSheet(mapStart[ui->lblStart->isClicked()]);
    SetupVisibleGroup(!ui->lblStart->isClicked());
    CO2VisibleGroup(!ui->lblStart->isClicked());
    TempVisibleGroup(!ui->lblStart->isClicked());
    O2VisibleGroup(!ui->lblStart->isClicked());
    HumidityVisibleGroup(!ui->lblStart->isClicked());
    ui->lblStatus->setText(mapStartText[ui->lblStart->isClicked()]);
    QString path = "configuration.ini";
    if (ui->lblStart->isClicked()) {
        ToggleVisibleGroup(2);
        SetupVisibleGroup(false);
        QSettings *settings = new QSettings(path, QSettings::IniFormat);

        qDebug() << "is this write?" << settings->isWritable();

        settings->beginGroup("LowerBounds");
        settings->setValue("temp", tempLowerVal);
        settings->setValue("humidity", humidityLowerVal);
        settings->setValue("O2", oxygenLowerVal);
        settings->setValue("CO2", co2LowerVal);
        settings->endGroup();

        settings->beginGroup("DefaultValue");
        settings->setValue("temp", tempVal);
        settings->setValue("humidity", humidityVal);
        settings->setValue("O2", oxygenVal);
        settings->setValue("CO2", co2Val);
        settings->endGroup();

        settings->beginGroup("UpperBounds");
        settings->setValue("temp", tempUpperVal);
        settings->setValue("humidity", humidityUpperVal);
        settings->setValue("O2", oxygenUpperVal);
        settings->setValue("CO2", co2UpperVal);
        settings->endGroup();

        settings->beginGroup("SettingsValue");
        settings->setValue("bluelamp", ui->lblBlueLight->isClicked());
        settings->setValue("yellowlamp", ui->lblYellowLight->isClicked());
        settings->setValue("start_date", QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
        settings->setValue("sensor_date", QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
        settings->endGroup();

    } else {
        ToggleVisibleGroup(1);

        QSettings *settings = new QSettings(path, QSettings::IniFormat);

        settings->beginGroup("LowerBounds");
        settings->value("temp", 20);
        settings->value("humidity", 20);
        settings->value("O2", 10);
        settings->value("CO2", 100);
        settings->endGroup();

        settings->beginGroup("DefaultValue");
        settings->value("temp", 25);
        settings->value("humidity", 60);
        settings->value("O2", 30);
        settings->value("CO2", 440);
        settings->endGroup();

        settings->beginGroup("UpperBounds");
        settings->value("temp", 40);
        settings->value("humidity", 80);
        settings->value("O2", 80);
        settings->value("CO2", 990);
        settings->endGroup();

        settings->beginGroup("SettingsValue");
        settings->value("bluelamp", false);
        settings->value("yellolamp", false);
        settings->value("start_date", QDate::currentDate());
        settings->value("sensor_date", QDate::currentDate());
        settings->endGroup();

    }
    sendCommand("P60 R255 G255 B255 N16\r\n");
//    digitalWrite(0, !digitalRead(0));
}

void MainWindow::togglePin(int pin, bool condition)
{
    if (condition) {
        sendCommand("P10 R1 S0\r\n");
    } else {
        sendCommand("P10 R1 S1\r\n");
    }
}

void MainWindow::on_btnUpperBound_clicked()
{
    ToggleVisibleGroup(3);
}

void MainWindow::on_btnMiddleBound_clicked()
{
    ToggleVisibleGroup(2);
}

void MainWindow::on_btnLowerBound_clicked()
{
    ToggleVisibleGroup(1);
}

void MainWindow::on_btnYellowLamp_clicked()
{
     ui->lblYellowLight->setStyleSheet(mapYellowLight[ui->lblYellowLight->isClicked()]);
    togglePin(0, ui->lblYellowLight->isClicked());
}

void MainWindow::on_btnBlueLamp_clicked()
{
    ui->lblBlueLight->setStyleSheet(mapBlueLight[ui->lblBlueLight->isClicked()]);
    togglePin(1, ui->lblBlueLight->isClicked());
}

void MainWindow::on_btnCamOn_clicked()
{
    ui->lblOpenCam->setStyleSheet(mapOpenCam[ui->lblOpenCam->isClicked()]);
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
    ui->lblGreyDown->setStyleSheet(mapPush[btnColor::grey]);
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
    ui->lblGreyDown->setStyleSheet(mapRelease[btnColor::grey]);
}

void MainWindow::on_btnGreenDown_clicked()
{
    ui->lblGreenDown->setStyleSheet(mapPush[btnColor::green]);
    switch (oxygenMode) {
        case 0: {
            ui->lblO2Disp->setText(QString::number(--oxygenLowerVal) + " %");
        };break;
        case 1: {
            ui->lblO2Disp->setText(QString::number(--oxygenVal) + " %");
        };break;
        case 2: {
            ui->lblO2Disp->setText(QString::number(--oxygenUpperVal) + " %");
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
    ui->lblGreyUp->setStyleSheet(mapPushUp[btnColor::grey]);
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
    ui->lblGreyUp->setStyleSheet(mapReleaseUp[btnColor::grey]);
}

void MainWindow::on_btnGreenUp_clicked()
{
    ui->lblGreenUp->setStyleSheet(mapPushUp[btnColor::green]);
    switch (oxygenMode) {
        case 0: {
            ui->lblO2Disp->setText(QString::number(++oxygenLowerVal) + " %");
        };break;
        case 1: {
            ui->lblO2Disp->setText(QString::number(++oxygenVal) + " %");
        };break;
        case 2: {
            ui->lblO2Disp->setText(QString::number(++oxygenUpperVal) + " %");
        }break;
    }

}

void MainWindow::on_btnGreenUp_released()
{
    ui->lblGreenUp->setStyleSheet(mapReleaseUp[btnColor::green]);
}

void MainWindow::initialSensor()
{
//    fd = serialOpen("/dev/ttyS0", 9600);
//    if (fd != -1) {
//        serialPuts(fd, "G\r\n");
//        serialPuts(fd, "M 04164\r\n");
//        serialPuts(fd, "K 2\r\n");
//    }
}

void MainWindow::displaySensorValue()
{
//    int fd = serialOpen("/dev/ttyS0", 9600);
//    serialFlush(fd);
//    if (fd != -1) {
//            serialPuts(fd, "Q\r\n");
//            while (serialDataAvail(fd)) {
//                QChar temp = QChar(serialGetchar(fd));
//                if (temp == 'H') {
//                    for(int i = 0; i < 6; i++){
//                        QChar temp = QChar(serialGetchar(fd));
//                        strHumidity.push_back(QString(temp));
//                    }
//                }
//                if (temp == 'T') {
//                    for(int i = 0; i < 6; i++){
//                        QChar temp = QChar(serialGetchar(fd));
//                        strTemp.push_back(QString(temp));
//                    }
//                }
//                if (temp == 'Z') {
//                    for(int i = 0; i < 6; i++){
//                        QChar temp = QChar(serialGetchar(fd));
//                        strCO2.push_back(QString(temp));
//                    }
//                    break;
//                }
////                if (temp == "\uffff") {
////                    serialFlush(fd);
////                    break;
////                }
//            }
//            double humidity = strHumidity.toInt() / 10.0;
//            double temp1 = (strTemp.toInt() - 1000) / 10.0;
//            int Co2 = strCO2.toInt() * 10;
//            ui->lblHumidityDisp->setText(QString::number(humidity) + QString::fromUtf8(" %RH"));
//            ui->lblTempDisp->setText(QString::number(temp1) + QString::fromUtf8(" °C"));
//            ui->lblCo2Disp->setText(QString::number(Co2) + QString::fromUtf8(" ppm"));
//    }
//    //serialClose(fd);
}

void MainWindow::showClock()
{
    //Show sensor value with clock timer
    displaySensorValue();
    ui->lblDateTime->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss"));
}

bool MainWindow::dataIsFloating(QString data) {
    QStringList ends;

    ends << "Reset to continue";
    ends << "'$H'|'$X' to unlock";
    ends << "ALARM: Soft limit";
    ends << "ALARM: Hard limit";
    ends << "Check Door";

    foreach (QString str, ends) {
        if (data.contains(str)) return true;
    }

    return false;
}

bool MainWindow::dataIsReset(QString data) {
    return QRegExp("^GRBL|GCARVIN\\s\\d\\.\\d.").indexIn(data.toUpper()) != -1;
}

void MainWindow::onSerialPortReadyRead()
{
    while (m_serialPort.canReadLine()) {
        QString data = m_serialPort.readLine().trimmed();

        if (data.length() > 0 && data[0] == 'o' && data[1] == 'k') {

            // Processed commands
            if (m_commands.length() > 0) {

                static QString response; // Full response string

                if ((m_commands[0].command != "[CTRL+X]" && dataIsEnd(data))
                        || (m_commands[0].command == "[CTRL+X]" && dataIsReset(data))) {

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
                    }
                }
                m_statusReceived = true;
            }
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
        m_serialPort.write("P27\r\n");
        m_statusReceived = false;
    }
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

    m_serialPort.write((command + "\r\n").toLatin1());
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

