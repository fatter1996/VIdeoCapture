#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <windows.h>
#include <QtConcurrent>
#include "Universal.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    GlobalData::m_pConfigInfo = new ConfigInfo;
    GlobalData::m_pConfigInfo->init();
    textAnalysis = new TextAnalysis;
#ifdef MODE_RECORD

    tcpSocket  = new ServerSide;
    connect(tcpSocket, &ServerSide::Receive, this, &MainWindow::onRecive);
#elif defined MODE_CAPTURE
    //tcpClient = new ClientSide;
    CaptureStart();
#endif
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    if(m_pCapture)
    {
        m_pCapture->CaptureStop();
        delete m_pCapture;
    }
    if(ass)
        delete ass;
    delete ui;
}

void MainWindow::CaptureStart()
{
    m_pCapture = new Capture;
    connect(m_pCapture, &Capture::Error, this, &MainWindow::onCaptureError);
    m_pCapture->Init();
    m_pCapture->start();
}

void MainWindow::onCaptureError()
{
    if(m_pCapture)
        videoPath = m_pCapture->CaptureStop();
    RecodeStop();
    //推拉流模式下时意外关闭后重启
#ifdef MODE_CAPTURE
    CaptureStart();
#endif
}

//仅用于录制模式下
void MainWindow::onRecive(QByteArray reciveArray)
{
    TextFrame* data = textAnalysis->TextFrameUnpack(reciveArray);

    if(data->captureState == 0xaa)
    {
        Dialogue* event = textAnalysis->TextFrameToDialogue(data);
        ass->AddEvents(event);
    }
    else if(data->captureState == 0xa5) //开始录屏
    {
        CaptureStart();
        RecodeStart();
    }
    else if(data->captureState == 0x5a)
    {
        if(!m_pCapture)
            return;
        videoPath = m_pCapture->CaptureStop();
        RecodeStop();
    }
    delete data;
}

void MainWindow::RecodeStart()
{
    ass = new ASSCaptions;
    assPath = ass->Create();
    ass->AddScriptInfo(1920,1032);
    ass->StylesInit();
    ass->AddStyles(CaptionsStyle());
    ass->EventsInit();
    textAnalysis->setStratTime();
}

void MainWindow::RecodeStop()
{
    if(m_pCapture)
    {
        delete m_pCapture;
        m_pCapture = nullptr;
    }

    if(ass)
    {
        ass->WriteEvents();
        delete ass;
        ass = nullptr;

        addCaptions(videoPath, assPath);
    }
}

void MainWindow::addCaptions(QString videoPath, QString assPath)
{
    QString outpath = QCoreApplication::applicationDirPath() + QString("/video/video_%1.mp4").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    std::wstring cmd = QString(" -i %1 -vf subtitles=%2 -vcodec libx264 %3")
            .arg(videoPath).arg(assPath).arg(outpath).toStdWString();

    std::wstring strexeContent  = L"ffmpeg.exe ";

    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = strexeContent.c_str();
    ShExecInfo.lpParameters = cmd.c_str();
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    //关闭该exe程序
    if( ShExecInfo.hProcess != NULL)
    {
        //等待程序运行完毕
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
        //关闭程序
        TerminateProcess(ShExecInfo.hProcess,0);
        ShExecInfo.hProcess = NULL;
    }

    QFile video(videoPath);
    video.remove();
    QFile ass(assPath);
    ass.remove();
}

