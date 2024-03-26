/********************************************************************************
** Form generated from reading UI file 'VoiceCallWnd.ui'
**
** Created by: Qt User Interface Compiler version 5.14.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VOICECALLWND_H
#define UI_VOICECALLWND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VoiceCallWnd
{
public:
    QHBoxLayout *horizontalLayout;
    QWidget *voiceLabel;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *VoiceCallWnd)
    {
        if (VoiceCallWnd->objectName().isEmpty())
            VoiceCallWnd->setObjectName(QString::fromUtf8("VoiceCallWnd"));
        VoiceCallWnd->resize(1216, 32);
        VoiceCallWnd->setMinimumSize(QSize(0, 32));
        VoiceCallWnd->setMaximumSize(QSize(1920, 32));
        horizontalLayout = new QHBoxLayout(VoiceCallWnd);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        voiceLabel = new QWidget(VoiceCallWnd);
        voiceLabel->setObjectName(QString::fromUtf8("voiceLabel"));
        voiceLabel->setMinimumSize(QSize(0, 32));
        voiceLabel->setMaximumSize(QSize(16777215, 32));
        horizontalLayout_2 = new QHBoxLayout(voiceLabel);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(8);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));

        horizontalLayout_2->addLayout(horizontalLayout_3);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        horizontalLayout->addWidget(voiceLabel);


        retranslateUi(VoiceCallWnd);

        QMetaObject::connectSlotsByName(VoiceCallWnd);
    } // setupUi

    void retranslateUi(QWidget *VoiceCallWnd)
    {
        VoiceCallWnd->setWindowTitle(QCoreApplication::translate("VoiceCallWnd", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VoiceCallWnd: public Ui_VoiceCallWnd {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VOICECALLWND_H
