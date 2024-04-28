#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <map>
#include <QFile>
#include <QCoreApplication>

class WeatherTool{
public:
    WeatherTool(){
        //获取当前程序运行的路径
        QString fileName = QCoreApplication::applicationDirPath();
        //创建错误信息收集的对象
        QJsonParseError err;
        //构建文件所在的路径。因为citycode-2019-08-23.json，在项目文件夹下，也可以将它复制到
        //项目运行的目录下，就不需要这么先返回两级目录
        fileName+="/citycode-2019-08-23.json";
        QFile file(fileName);
        //打开文件
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QByteArray json = file.readAll();
        file.close();
        //读取Json数据
        QJsonDocument jsonDoc = QJsonDocument::fromJson(json,&err);
        //获取城市列表数组
        QJsonArray citys = jsonDoc.array();
        for(int i=0;i<citys.size();i++)
        {
            QString code = citys.at(i).toObject().value("city_code").toString();
            QString city = citys.at(i).toObject().value("city_name").toString();
            //省份的code是空的，所以城市代码长度大于0的都可以添加的map中
            if(code.size()>0){
                m_mapCity2Code.insert(std::pair<QString,QString>(city,code));
            }
        }
    }

    QString operator[](const QString& city){
        //为了很好的匹配，设置了直接搜索城市名字
        std::map<QString,QString>::iterator it = m_mapCity2Code.find(city);
        if(it==m_mapCity2Code.end()){
            //搜索城市后带市字
            it = m_mapCity2Code.find(city+u8"市");
        }
        if(it==m_mapCity2Code.end()){
            //搜索城市名后带县字
            it = m_mapCity2Code.find(city+u8"县");
        }
        if(it!=m_mapCity2Code.end()){
            //找到直接返回城市代码
            return it->second;
        }
        //没找到返回9个0
        return "000000000";
}

private:
    std::map<QString, QString> m_mapCity2Code;
};

#endif // WEATHERTOOL_H
