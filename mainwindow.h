#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    enum btnColor { red = 0, blue, grey, green };
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    struct CommandAttributes {
        int length;
        int consoleIndex;
        int tableIndex;
        QString command;
    };

    struct CommandQueue {
        QString command;
        int tableIndex;
        bool showInConsole;
    };

private slots:

    void onSerialPortReadyRead();
    void onSerialPortError(QSerialPort::SerialPortError);
    void openPort();
    void sendCommand(QString command, int tableIndex = -1, bool showInConsole = true);
    void onTimerConnection();
    void onTimerStateQuery();
    void onTimerLogData();

    void on_btnNeoPixel_clicked();
    void on_btnPeltierCooler_clicked();
    void on_btnSystemFan_clicked();
    void on_btnPlug_clicked();
    void on_btnBlueLamp_clicked();
    void on_btnCamOn_clicked();

    void on_btnUpperBound_clicked();
    void on_btnMiddleBound_clicked();
    void on_btnLowerBound_clicked();

    void on_btnRedDown_clicked();
    void on_btnRedDown_released();
    void on_btnBlueDown_clicked();
    void on_btnBlueDown_released();
    void on_btnGreyDown_clicked();
    void on_btnGreyDown_released();
    void on_btnGreenDown_clicked();
    void on_btnGreenDown_released();

    void on_btnRedUp_clicked();
    void on_btnRedUp_released();
    void on_btnBlueUp_clicked();
    void on_btnBlueUp_released();
    void on_btnGreyUp_clicked();
    void on_btnGreyUp_released();
    void on_btnGreenUp_clicked();
    void on_btnGreenUp_released();

    void on_btnRedDownCtl_clicked();
    void on_btnRedDownCtl_released();
    void on_btnRedUpCtl_clicked();
    void on_btnRedUpCtl_released();

    void on_btnGreenDownCtl_clicked();
    void on_btnGreenDownCtl_released();
    void on_btnGreenUpCtl_clicked();
    void on_btnGreenUpCtl_released();

    void on_btnRedCtl_clicked();
    void on_btnBack_clicked();

    void CO2VisibleGroup(bool visible);
    void TempVisibleGroup(bool visible);
    void O2VisibleGroup(bool visible);
    void HumidityVisibleGroup(bool visible);

    void showClock();
    void togglePin(int pin, bool condition);
    int bufferLength();
    bool dataIsEnd(QString data);

private:
    Ui::MainWindow *ui;
    QSerialPort m_serialPort;

    bool m_reseting = false;
    bool m_resetCompleted = true;
    bool m_aborting = false;
    bool m_statusReceived = false;

    QStringList m_status;
    QStringList m_statusCaptions;
    QStringList m_statusBackColors;
    QStringList m_statusForeColors;

    QList<CommandAttributes> m_commands;
    QList<CommandQueue> m_queue;

    QTime m_startTime;
    QTimer m_timerConnection;
    QTimer m_timerStateQuery;
    QTimer m_timerLogData;

    void updateDisplay(QString group = "");

    double tempVal = 0;
    double humidityVal = 0;
    double co2Val = 0;
    double oxygenVal = 0;

    double tempLowerVal = 20;
    double humidityLowerVal = 50;
    double co2LowerVal = 200;
    double oxygenLowerVal = 25;

    double tempUpperVal = 40;
    double humidityUpperVal = 100;
    double co2UpperVal = 999;
    double oxygenUpperVal = 100;

    int tempMode = 0;
    int humidityMode = 0;
    int co2Mode = 0;
    int oxygenMode = 0;

    QTimer mTimer;

    QMap<QString, int> mapRelay = {{"PeltierFan",1}, {"DownFan",2}, {"Heater",4}, {"Plug",6}, {"Nebulizer",7}, {"UV", 8}};

    QMap<btnColor, QString> mapPush = {{red, "image: url(:/reddownpush.png);"},{blue,"image: url(:/bluedownpush.png);"},{grey,"image: url(:/greydownpush.png);"},{green, "image: url(:/greendownpush.png);"}};
    QMap<btnColor, QString> mapRelease = {{red, "image: url(:/reddown.png);"},{blue,"image: url(:/bluedown.png);"},{grey,"image: url(:/greydown.png);"},{green, "image: url(:/greendown.png);"}};
    QMap<btnColor, QString> mapPushUp = {{red, "image: url(:/reduppush.png);"},{blue,"image: url(:/blueuppush.png);"},{grey,"image: url(:/greyuppush.png);"},{green, "image: url(:/greenuppush.png);"}};
    QMap<btnColor, QString> mapReleaseUp = {{red, "image: url(:/redup.png);"},{blue,"image: url(:/blueup.png);"},{grey,"image: url(:/greyup.png);"},{green, "image: url(:/greenup.png);"}};
    QMap<bool, QString> mapBlueLight = {{true, "image: url(:/bluelamp.png);"}, {false, "image: url(:/offlamp.png);"}};
    QMap<bool, QString> mapNeoPixel = {{true, "image: url(:/neopixel.png);"}, {false, "image: url(:/offneopixel.png);"}};
    QMap<bool, QString> mapPeltier = {{true, "image: url(:/greenice.png);"}, {false, "image: url(:/offgreenice.png);"}};
    QMap<bool, QString> mapSystemFan = {{true, "image: url(:/greenfan.png);"}, {false, "image: url(:/offgreenfan.png);"}};
    QMap<bool, QString> mapPlug = {{true, "image: url(:/plug.png);"}, {false, "image: url(:/offplug.png);"}};
    QMap<bool, QString> mapOpenCam = {{true, "image: url(:/camera.png);"},{false, "image: url(:/offcam.png);"}};
    QString strHumidity = "";
    QString strTemp = "";
    QString strCO2 = "";
    int fd;

    QSqlDatabase m_db;

    bool saveOxygenValues();
    bool saveCarbondioxideValues();
    bool saveTemperatureValues();
    bool saveHumidityValues();
};
#endif // MAINWINDOW_H
