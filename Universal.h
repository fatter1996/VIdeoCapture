#ifndef UNIVERSAL_H
#define UNIVERSAL_H

#include <QObject>

#define MODE_CAPTURE
//#define MODE_RECORD

#define WORKTYPE_SERVER     1
#define WORKTYPE_CLIENT     2
#define WORKTYPE    WORKTYPE_CLIENT

typedef struct {
    time_t lasttime;
    bool connected;
}Runner;

struct ConfigInfo
{
    //TCP-IP
    QString serverIp;
    uint serverPORT;
    //推流信息
    QString name; //设备名称
    QString outUrl; //推流地址

    //桌面录制
    int frameRate = 0; //帧率
    int videoBitRate = 0; //比特率
    int desktopRange_x = 0; //录制区域的起始坐标-x
    int desktopRange_y = 0; //录制区域的起始坐标-y
    int desktopWidth = 0; //图像宽度
    int desktopHeight = 0; //图像高度

    //摄像头录制
    QString cameraName;
    int cameraIndex = 0;
    int cameraWidth = 0; //图像宽度
    int cameraHeight = 0; //图像高度
    int cameraPos_x = 0; //显示区域的起始坐标-x
    int cameraPos_y = 0; //显示区域的起始坐标-y

    //麦克风录制
    QString MICName; //设备名称
    int MICIndex = 0; //设备编号
    int sampleRate = 0; //帧率
    int audioBitRate = 0; //比特率

    //车站信息
    QString stationName; //站名
    QString sreverIP; //共享服务器名称
    QString savePath; //共享文件夹路径

public:
    void init();
};

struct CallConfigInfo
{
    //TCP-IP
    QString serverIp;
    uint serverPORT;
    QString localIp_Tcp;
    uint localPORT_Tcp;
    //UDP-IP
    QString localIp;
    uint localPORT;

    //语音
    int sampleRate = 0; //采样率
    int channelCount = 0; //声道数
    int sampleSize = 0; //样本数

    //车站信息
    QString name;
public:
    void init();
}; //配置文件读取,设为全局变量

class GlobalData
{
public:
    static ConfigInfo* m_pConfigInfo;
    static CallConfigInfo* m_pCallConfigInfo;
    static int getBigEndian_2Bit(QByteArray bigArray);
    static QByteArray getLittleEndian_2Bit(int bigArray);
    static QDateTime getDateTimeByArray(QByteArray data);

    static QByteArray getDateTimeArray(QDateTime time);
    static QByteArray getIPeArray(QString ip);
};



#endif // UNIVERSAL_H
