#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class GraphWindow; }
QT_END_NAMESPACE

class GraphWindow : public QMainWindow
{
    Q_OBJECT
private:
    enum btnColor { red = 0, blue, grey, green };
public:
    GraphWindow(QWidget *parent = nullptr);
    ~GraphWindow();

private slots:
    void showClock();
    void initialSensor();
    void displaySensorValue();
    void togglePin(int pin, bool condition);
    void on_btnBack_clicked();

private:
    Ui::GraphWindow *ui;

    int tempVal = 30;
    int humidityVal = 30;
    int co2Val = 30;
    int oxygenVal = 30;

    int tempLowerVal = 20;
    int humidityLowerVal = 50;
    int co2LowerVal = 200;
    int oxygenLowerVal = 25;

    int tempUpperVal = 40;
    int humidityUpperVal = 100;
    int co2UpperVal = 999;
    int oxygenUpperVal = 100;

    int tempMode = 0;
    int humidityMode = 0;
    int co2Mode = 0;
    int oxygenMode = 0;

    QTimer mTimer;
    QMap<btnColor, QString> mapPush = {{red, "image: url(:/reddownpush.png);"},{blue,"image: url(:/bluedownpush.png);"},{grey,"image: url(:/greydownpush.png);"},{green, "image: url(:/greendownpush.png);"}};
    QMap<btnColor, QString> mapRelease = {{red, "image: url(:/reddown.png);"},{blue,"image: url(:/bluedown.png);"},{grey,"image: url(:/greydown.png);"},{green, "image: url(:/greendown.png);"}};
    QMap<btnColor, QString> mapPushUp = {{red, "image: url(:/reduppush.png);"},{blue,"image: url(:/blueuppush.png);"},{grey,"image: url(:/greyuppush.png);"},{green, "image: url(:/greenuppush.png);"}};
    QMap<btnColor, QString> mapReleaseUp = {{red, "image: url(:/redup.png);"},{blue,"image: url(:/blueup.png);"},{grey,"image: url(:/greyup.png);"},{green, "image: url(:/greenup.png);"}};
    QMap<bool, QString> mapBlueLight = {{true, "image: url(:/bluelamp.png);"}, {false, "image: url(:/offlamp.png);"}};
    QMap<bool, QString> mapYellowLight = {{true, "image: url(:/yellolamp.png);"}, {false, "image: url(:/offlamp.png);"}};
    QMap<bool, QString> mapOpenCam = {{true, "image: url(:/camera.png);"},{false, "image: url(:/offcam.png);"}};
    QMap<bool, QString> mapStart = {{true, "image: url(:/green.png);"},{false, "image: url(:/gear.png);"}};
    QMap<bool, QString> mapStartText = {{true, "Normal Running State"},{false, "System Settings"}};
    QString strHumidity = "";
    QString strTemp = "";
    QString strCO2 = "";
    int fd;
};
#endif // GRAPHWINDOW_H
