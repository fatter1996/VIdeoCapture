#ifndef VOICECALLWND_H
#define VOICECALLWND_H

#include <QWidget>
#include "VoiceDialog/VoiceDialog.h"

namespace Ui {
class VoiceCallWnd;
}

class VoiceCallWnd : public QWidget
{
    Q_OBJECT

public:
    explicit VoiceCallWnd(QWidget *parent = nullptr);
    ~VoiceCallWnd();

private slots:
    void onNewMember(QString, uint, QString);
    void onMemberQuit(QString, uint);

private:
    Ui::VoiceCallWnd *ui;
    VoiceDialog* m_pVoice = nullptr;
};

#endif // VOICECALLWND_H
