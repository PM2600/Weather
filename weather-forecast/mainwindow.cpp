﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>

#define SPAN_INDEX 3 // 温度曲线间隔指数
#define ORIGIN_SIZE 3 // 温度曲线原点大小
#define TEMPERATURE_STARTING_COORDINATE 45

// 日出日落底线
const QPoint MainWindow::sun[2] = {
    QPoint(20, 75),
    QPoint(130, 75)
};

// 日出日落时间
const QRect MainWindow::sunRizeSet[2] = {
    QRect(0, 80, 50, 20),
    QRect(100, 80, 50, 20)
};

// 日出日落圆弧
const QRect MainWindow::rect[2] = {
    QRect(25, 25, 100, 100), // 虚线圆弧
    QRect(50, 80, 50, 20) // “日出日落”文本
};


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //设置窗口属性为没有边框的窗口
    setWindowFlag(Qt::FramelessWindowHint);
    //固定窗口大小
    setFixedSize(width(), height());
    exitMenu = new QMenu(this);
    exitAct = new QAction;
    exitAct->setText(u8"退出");
    exitAct->setIcon(QIcon(":/weatherIco/close.ico"));
    exitMenu->addAction(exitAct);
    connect(exitAct, SIGNAL(triggered(bool)), this, SLOT(slot_exitApp()));

    //UI初始化
    forecast_week_list << ui->week0Lb << ui->week1Lb << ui->week2Lb << ui->week3Lb << ui->week4Lb << ui->week5Lb;
    forecast_date_list << ui->date0Lb << ui->date1Lb << ui->date2Lb << ui->date3Lb << ui->date4Lb << ui->date5Lb;
    forecast_aqi_list << ui->quality0Lb << ui->quality1Lb << ui->quality2Lb << ui->quality3Lb << ui->quality4Lb << ui->quality5Lb;
    forecast_type_list << ui->type0Lb << ui->type1Lb << ui->type2Lb << ui->type3Lb << ui->type4Lb << ui->type5Lb;
    forecast_typeIco_list << ui->typeIco0Lb << ui->typeIco1Lb << ui->typeIco2Lb << ui->typeIco3Lb << ui->typeIco4Lb << ui->typeIco5Lb;
    forecast_high_list << ui->high0Lb << ui->high1Lb << ui->high2Lb << ui->high3Lb << ui->high4Lb << ui->high5Lb;
    forecast_low_list << ui->low0Lb << ui->low1Lb << ui->low2Lb << ui->low3Lb << ui->low4Lb << ui->low5Lb;

    // dateLb和WeekLb样式表设置
    for (int i = 0; i < 6; i++)
    {
        forecast_date_list[i]->setStyleSheet("background-color: rgba(0, 255, 255, 100);");
        forecast_week_list[i]->setStyleSheet("background-color: rgba(0, 255, 255, 100);");
    }
    ui->cityLineEdit->setStyleSheet("QLineEdit{border: 1px solid gray; border-radius: 4px; background:argb(47, 47, 47, 130); color:rgb(255, 255, 255);} QLineEdit:hover{border-color:rgb(101, 255, 106); }");

    // 请求天气API信息
    url = "http://t.weather.itboy.net/api/weather/city/";
    city = u8"北京";
    cityTmp = city;
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replayFinished(QNetworkReply*)));
    getWeatherInfo(manager);

    ui->sunRiseSetLb->installEventFilter(this);
    ui->curveLb->installEventFilter(this);
    ui->cityLineEdit->installEventFilter(this);

    sunTimer = new QTimer(ui->sunRiseSetLb);
    connect(sunTimer, SIGNAL(timeout()), ui->sunRiseSetLb, SLOT(update()));
    sunTimer->start(1000);
}

void MainWindow::getWeatherInfo(QNetworkAccessManager* manager){
    QString citycode = tool[city];
    if(citycode=="000000000"){
        QMessageBox::warning(this, u8"错误", u8"天气：指定城市不存在！", QMessageBox::Ok);
        return;
    }
    QUrl jsonUrl(url + citycode);
    manager->get( QNetworkRequest(jsonUrl) );
}

void MainWindow::replayFinished(QNetworkReply* reply){
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(reply->error() != QNetworkReply::NoError || status_code != 200)
    {
        QMessageBox::warning(this, u8"错误", u8"天气：请求数据错误，检查网络连接！", QMessageBox::Ok);
        return;
    }

    QByteArray bytes = reply->readAll();
    //qDebug() << bytes;
    parseJson(bytes);
}

void MainWindow::parseJson(QByteArray& bytes){
    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes, &err); // 检测json格式
    if (err.error != QJsonParseError::NoError) // Json格式错误
    {
        return;
    }
    QJsonObject jsObj = jsonDoc.object();
//    qDebug() << jsObj;
//    qDebug() << "=========================\r\n";
//    qDebug() << jsObj.value("message").toString();
    QString message = jsObj.value("message").toString();

    if (message.contains("success")==false)
    {
        QMessageBox::information(this, tr("The information of Json_desc"), u8"天气：城市错误！", QMessageBox::Ok );
        city = cityTmp;
        return;
    }
    today = jsObj;
    // 解析data中的yesterday
    QJsonObject dataObj = jsObj.value("data").toObject();
    forecast[0] = dataObj.value("yesterday").toObject();
    // 解析data中的forecast
    QJsonArray forecastArr = dataObj.value("forecast").toArray();
    int j = 0;
    for (int i = 1; i < 6; i++)
    {
        forecast[i] = forecastArr.at(j).toObject();
        j++;
    }
    setLabelContent();
}

void MainWindow::setLabelContent(){
    //更新ui数据
    ui->dateLb->setText(today.date);
    ui->temLb->setText(today.wendu);
    ui->cityLb->setText(today.city);
    ui->typeLb->setText(today.type);
    ui->noticeLb->setText(today.notice);
    ui->shiduLb->setText(today.shidu);
    ui->pm25Lb->setText(today.pm25);
    ui->fxLb->setText(today.fx);
    ui->flLb->setText(today.fl);
    ui->ganmaoBrowser->setText(today.ganmao);

    // 六天数据
    for (int i = 0; i < 6; i++)
    {
        forecast_week_list[i]->setText(forecast[i].week.right(3));
        forecast_date_list[i]->setText(forecast[i].date.left(3));
        forecast_type_list[i]->setText(forecast[i].type);
        forecast_high_list[i]->setText(forecast[i].high.split(" ").at(1));
        forecast_low_list[i]->setText(forecast[i].low.split(" ").at(1));
        forecast_typeIco_list[i]->setStyleSheet( tr("image: url(:/day/day/%1.png);").arg(forecast[i].type) );

        if (forecast[i].aqi.toInt() >= 0 && forecast[i].aqi.toInt() <= 50)
        {
            forecast_aqi_list[i]->setText(u8"优质");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(0, 255, 0);");
        }
        else if (forecast[i].aqi.toInt() > 50 && forecast[i].aqi.toInt() <= 100)
        {
            forecast_aqi_list[i]->setText(u8"良好");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(255, 255, 0);");
        }
        else if (forecast[i].aqi.toInt() > 100 && forecast[i].aqi.toInt() <= 150)
        {
            forecast_aqi_list[i]->setText(u8"轻度污染");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(255, 170, 0);");
        }
        else if (forecast[i].aqi.toInt() > 150 && forecast[i].aqi.toInt() <= 200)
        {
            forecast_aqi_list[i]->setText(u8"重度污染");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(255, 0, 0);");
        }
        else
        {
            forecast_aqi_list[i]->setText(u8"严重污染");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(170, 0, 0);");
        }
    }
    ui->week0Lb->setText( u8"昨天" );
    ui->week1Lb->setText( u8"今天" );

    ui->curveLb->update();
}

void MainWindow::paintSunRiseSet(){
    QPainter painter(ui->sunRiseSetLb);
    //反锯齿
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.save();
    QPen pen = painter.pen();
    pen.setWidthF(0.5);
    pen.setColor(Qt::yellow);
    painter.setPen(pen);
    painter.drawLine(sun[0], sun[1]);
    painter.restore();

    painter.save();
    painter.setFont( QFont("Microsoft Yahei", 8, QFont::Normal) ); // 字体、大小、正常粗细
    painter.setPen(Qt::white);

    if (today.sunrise != "" && today.sunset != "")
    {
       //在指定区域内绘制文字
       painter.drawText(sunRizeSet[0], Qt::AlignHCenter, today.sunrise);
       painter.drawText(sunRizeSet[1], Qt::AlignHCenter, today.sunset);
    }
    painter.drawText(rect[1], Qt::AlignHCenter, u8"日出日落");//在指定区域绘制文字
    painter.restore();

    // 绘制圆弧
    painter.save();
    pen.setWidthF(0.5);         //设置线条的宽度0.5
    pen.setStyle(Qt::DotLine);  //虚线
    pen.setColor(Qt::green);    //设置颜色
    painter.setPen(pen);
    painter.drawArc(rect[0], 0 * 16, 180 * 16); //绘制圆弧
    painter.restore();

    // 绘制日出日落占比
    if (today.sunrise != "" && today.sunset != "")
    {
        painter.setPen(Qt::NoPen); //关闭笔
        painter.setBrush(QColor(255, 85, 0, 100));//设置画刷

        int startAngle, spanAngle;
        QString sunsetTime = today.date + " " + today.sunset;

        if (QDateTime::currentDateTime() > QDateTime::fromString(sunsetTime, "yyyy-MM-dd hh:mm"))
        {
            startAngle = 0 * 16;
            spanAngle = 180 * 16;
        }
        else
        {
            // 计算起始角度和跨越角度
            static QStringList sunSetTime = today.sunset.split(":");
            static QStringList sunRiseTime = today.sunrise.split(":");

            static QString sunsetHour = sunSetTime.at(0);
            static QString sunsetMint = sunSetTime.at(1);
            static QString sunriseHour = sunRiseTime.at(0);
            static QString sunriseMint = sunRiseTime.at(1);

            static int sunrise = sunriseHour.toInt() * 60 + sunriseMint.toInt();
            static int sunset = sunsetHour.toInt() * 60 + sunsetMint.toInt();
            int now = QTime::currentTime().hour() * 60 + QTime::currentTime().minute();

            startAngle = ( (double)(sunset - now) / (sunset - sunrise) ) * 180 * 16;
            spanAngle = ( (double)(now - sunrise) / (sunset - sunrise) ) * 180 * 16;
        }

        if (startAngle >= 0 && spanAngle >= 0)
        {
            painter.drawPie(rect[0], startAngle, spanAngle); // 扇形绘制
            //painter.drawPie(rect[0], startAngle, 180 * 16);
        }
    }
}

void MainWindow::paintCurve(){
    QPainter painter(ui->curveLb);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int tempTotal = 0;
    int high[6] = {};
    int low[6] = {};

    QString h, l;
    for (int i = 0; i < 6; i++)
    {
        h = forecast[i].high.split(" ").at(1);
        h = h.left(h.length() - 1);
        high[i] = (int)(h.toDouble());
        tempTotal += high[i];

        l = forecast[i].low.split(" ").at(1);
        l = l.left(h.length() - 1);
        low[i] = (int)(l.toDouble());
    }
    int tempAverage = (int)(tempTotal / 6); // 最高温平均值

    // 算出温度对应坐标
    int pointX[6] = {35, 103, 172, 241, 310, 379}; // 点的X坐标
    int pointHY[6] = {0};
    int pointLY[6] = {0};
    for (int i = 0; i < 6; i++)
    {
       pointHY[i] = TEMPERATURE_STARTING_COORDINATE - ((high[i] - tempAverage) * SPAN_INDEX);
       pointLY[i] = TEMPERATURE_STARTING_COORDINATE + ((tempAverage - low[i]) * SPAN_INDEX);
    }

    QPen pen = painter.pen();
    pen.setWidth(1);		//设置笔的宽度为1
    // 高温曲线绘制
    painter.save();
    pen.setColor(QColor(255, 170, 0));	//设置颜色
    pen.setStyle(Qt::DotLine);	//虚线
    painter.setPen(pen);
    painter.setBrush(QColor(255, 170, 0));		//设置画刷颜色

    painter.drawEllipse(QPoint(pointX[0], pointHY[0]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawEllipse(QPoint(pointX[1], pointHY[1]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawLine(pointX[0], pointHY[0], pointX[1], pointHY[1]);

    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    painter.setPen(pen);

    for (int i = 1; i < 5; i++)
    {
        painter.drawEllipse(QPoint(pointX[i+1], pointHY[i+1]), ORIGIN_SIZE, ORIGIN_SIZE);
        painter.drawLine(pointX[i], pointHY[i], pointX[i+1], pointHY[i+1]);
    }
    painter.restore();

    // 低温曲线绘制
    pen.setColor(QColor(0, 255, 255));
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    painter.setBrush(QColor(0, 255, 255));
    painter.drawEllipse(QPoint(pointX[0], pointLY[0]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawEllipse(QPoint(pointX[1], pointLY[1]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawLine(pointX[0], pointLY[0], pointX[1], pointLY[1]);

    pen.setColor(QColor(0, 255, 255));
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    for (int i = 1; i < 5; i++)
    {
       painter.drawEllipse(QPoint(pointX[i+1], pointLY[i+1]), ORIGIN_SIZE, ORIGIN_SIZE);
       painter.drawLine(pointX[i], pointLY[i], pointX[i+1], pointLY[i+1]);
    }
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event){
    exitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::slot_exitApp(){
    qApp->exit(0);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event){
    move(event->globalPos() - mPos);
}

void MainWindow::mousePressEvent(QMouseEvent* event){
    mPos = event->globalPos() - this->pos();
}


bool MainWindow::eventFilter(QObject* watched, QEvent* event){
    if(watched == ui->sunRiseSetLb && event->type() == QEvent::Paint)
    {
        paintSunRiseSet();
    }
    else if(watched == ui->curveLb && event->type() == QEvent::Paint){
        paintCurve();
    }
    return QWidget::eventFilter(watched, event);
}

void MainWindow::on_refreshBt_clicked()
{
    getWeatherInfo(manager);
    ui->curveLb->update();
}

void MainWindow::on_searchButton_clicked()
{
    cityTmp = city;
    city = ui->cityLineEdit->text();
    getWeatherInfo(manager);
}
