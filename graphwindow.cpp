#include "graphwindow.h"
#include "ui_graphwindow.h"
#include "qtimer.h"
#include <string>
#include <QDateTime>
//#include <wiringSerial.h>
//#include <wiringPi.h>
#include <qdebug.h>
#include <QSettings>
#include <QSplineSeries>
#include <QBarSet>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChartView>

using namespace QtCharts;
//int GraphWindow::fd = 0;

GraphWindow::GraphWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GraphWindow)
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

    QTimer::connect(ui->lblBack, SIGNAL(clicked()), this, SLOT(on_btnBack_clicked()));

    QSplineSeries *series = new QSplineSeries();
    series->setName("temperature");

    *series << QPointF(0, 11.9) << QPointF(1, 12.8) << QPointF(2, 18.5) << QPointF(3, 26.5) << QPointF(4, 32.0) << QPointF(5, 34.8)
         << QPointF(6, 38.2) << QPointF(7, 34.8) << QPointF(8, 29.8) << QPointF(9, 20.4) << QPointF(10, 15.1) << QPointF(11, 11.8);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Temperature records in celcius");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories = {
          "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
      };

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Month");
    chart->addAxis(axisX, Qt::AlignBottom);
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(-52, 52);
    axisY->setTitleText("Temperature [&deg;C]");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    //      QChartView *chartView = new QChartView(chart, ui->gView);
    //      chartView->setRenderHint(QPainter::Antialiasing);

    QChartView *chartView = new QChartView(ui->gView);
    chartView->resize(741, 271); // swap?
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

}

GraphWindow::~GraphWindow()
{
//    serialClose(fd);
    delete ui;
}

void GraphWindow::togglePin(int pin, bool condition)
{
    if (condition) {
//        digitalWrite(pin, 0);
    } else {
//        digitalWrite(pin, 1);
    }
}

void GraphWindow::on_btnBack_clicked()
{
    this->hide();
}

void GraphWindow::initialSensor()
{
//    fd = serialOpen("/dev/ttyS0", 9600);
    if (fd != -1) {
//        serialPuts(fd, "G\r\n");
//        serialFlush(fd);
//        while (serialDataAvail(fd)) {
//            QChar temp = QChar(serialGetchar(fd));
//            qDebug() << "This is a G" << temp << "\n";
//        }
//        serialPuts(fd, "M 04164\r\n");
//        serialFlush(fd);
//        while (serialDataAvail(fd)) {
//            QChar temp = QChar(serialGetchar(fd));
//            qDebug() << "This is a M" << temp << "\n";
//        }
//        serialPuts(fd, "K 2\r\n");
//        serialFlush(fd);
//        while (serialDataAvail(fd)) {
//            QChar temp = QChar(serialGetchar(fd));
//            qDebug() << "This is a K" << temp << "\n";
//        }
    }
//    serialClose(fd);
}

void GraphWindow::displaySensorValue()
{
//    int fd = serialOpen("/dev/ttyS0", 9600);
//    serialFlush(fd);
//    if (fd != -1) {
//            serialPuts(fd, "Q\r\n");
//            while (serialDataAvail(fd)) {
//                QChar temp = QChar(serialGetchar(fd));
//                qDebug() << "This is a cat" << temp << "\n";
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
//                if (temp == "\uffff") {
//                    serialFlush(fd);
//                    break;
//                }
//            }
//            double humidity = strHumidity.toInt() / 10.0;
//            double temp1 = (strTemp.toInt() - 1000) / 10.0;
//            int Co2 = strCO2.toInt() * 10;
//            ui->lblHumidityDisp->setText(QString::number(humidity) + QString::fromUtf8(" %RH"));
//            ui->lblTempDisp->setText(QString::number(temp1) + QString::fromUtf8(" °C"));
//            ui->lblCo2Disp->setText(QString::number(Co2) + QString::fromUtf8(" ppm"));
//    }
    //serialClose(fd);
}

void GraphWindow::showClock()
{
    //Show sensor value with clock timer
    displaySensorValue();
    ui->lblDateTime->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss"));
}
