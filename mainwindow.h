#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Capture/Capture.h"
#include "ASSFileCreate/ASSCaptions.h"
#include "TextAnalysis/TextAnalysis.h"
//#include "TCPSocket/ClientSide.h"
//#include "TCPSocket/ServerSide.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void CaptureStart();
    void addCaptions(QString videoPat, QString assPath);

    void RecodeStart();
    void RecodeStop();
public slots:
    void onCaptureError();
    void onRecive(QByteArray);

private:
    Ui::MainWindow *ui;

    ASSCaptions* ass = NULL;
    Capture* m_pCapture = NULL;

    //ServerSide* tcpSocket  = nullptr;
    //ClientSide* tcpClient  = nullptr;
    TextAnalysis* textAnalysis = nullptr;

    QString videoPath;
    QString assPath;
};
#endif // MAINWINDOW_H
