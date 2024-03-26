#include "VoiceCallWnd.h"
#include "ui_VoiceCallWnd.h"
#include "Universal.h"
#include "CallButton.h"
#include <qevent.h>

VoiceCallWnd::VoiceCallWnd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VoiceCallWnd)
{
    GlobalData::m_pCallConfigInfo = new CallConfigInfo;
    GlobalData::m_pCallConfigInfo->init();

    m_pVoice = new VoiceDialog;
    connect(m_pVoice, &VoiceDialog::NewMember, this, &VoiceCallWnd::onNewMember);
    connect(m_pVoice, &VoiceDialog::MemberQuit, this, &VoiceCallWnd::onMemberQuit);

    ui->setupUi(this);
}

VoiceCallWnd::~VoiceCallWnd()
{
    delete ui;
}

void VoiceCallWnd::onNewMember(QString ip, uint port, QString name)
{
    CallButton* btn = new CallButton(ip, port, name);
    ui->horizontalLayout_3->addWidget(btn);
    connect(btn, &CallButton::Call, m_pVoice, &VoiceDialog::onCall);
}

void VoiceCallWnd::onMemberQuit(QString ip, uint port)
{
    CallButton* btn = findChild<CallButton*>(QString("%1:%2").arg(ip).arg(port));
    if(btn)
    {
        ui->horizontalLayout_3->removeWidget(btn);
        btn->close();
    }
}
