/********************************************************************************
** Form generated from reading UI file 'VoiceDialogWnd.ui'
**
** Created by: Qt User Interface Compiler version 5.14.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VOICEDIALOGWND_H
#define UI_VOICEDIALOGWND_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VoiceDialogWnd
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *closeBtn_2;
    QSpacerItem *verticalSpacer;
    QLabel *label;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *agreeBtn;
    QPushButton *disAgreeBtn;
    QPushButton *closeBtn;

    void setupUi(QWidget *VoiceDialogWnd)
    {
        if (VoiceDialogWnd->objectName().isEmpty())
            VoiceDialogWnd->setObjectName(QString::fromUtf8("VoiceDialogWnd"));
        VoiceDialogWnd->resize(260, 149);
        VoiceDialogWnd->setStyleSheet(QString::fromUtf8("alternate-background-color: rgb(222, 226, 229);"));
        verticalLayout = new QVBoxLayout(VoiceDialogWnd);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        widget = new QWidget(VoiceDialogWnd);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setStyleSheet(QString::fromUtf8("background-color: rgb(16, 16, 16);\n"
""));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        closeBtn_2 = new QPushButton(widget);
        closeBtn_2->setObjectName(QString::fromUtf8("closeBtn_2"));
        closeBtn_2->setMinimumSize(QSize(32, 32));
        closeBtn_2->setMaximumSize(QSize(32, 32));
        closeBtn_2->setStyleSheet(QString::fromUtf8("QPushButton\n"
"{\n"
"	background:url(:/res/res/CloseWndM1.png);\n"
"	background-color:transparent;\n"
"	border: none;\n"
"}\n"
"QPushButton:hover\n"
"{\n"
"	background:url(:/res/res/CloseWndM2.png);\n"
"	background-color:transparent;\n"
"}\n"
"QPushButton:pressed\n"
"{\n"
"	background:url(:/res/res/CloseWndM3.png);\n"
"	background-color:transparent;\n"
"}\n"
"\n"
""));

        horizontalLayout->addWidget(closeBtn_2);


        verticalLayout_2->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(0, 32));
        label->setMaximumSize(QSize(16777215, 32));
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(11);
        label->setFont(font);
        label->setStyleSheet(QString::fromUtf8("color: rgb(250, 250, 250);"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(24);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(12, -1, 12, 12);
        agreeBtn = new QPushButton(widget);
        agreeBtn->setObjectName(QString::fromUtf8("agreeBtn"));
        agreeBtn->setMinimumSize(QSize(60, 70));
        agreeBtn->setMaximumSize(QSize(60, 70));
        agreeBtn->setStyleSheet(QString::fromUtf8("border-image: url(:/res/res/AgreeBtn.png);"));

        horizontalLayout_2->addWidget(agreeBtn);

        disAgreeBtn = new QPushButton(widget);
        disAgreeBtn->setObjectName(QString::fromUtf8("disAgreeBtn"));
        disAgreeBtn->setMinimumSize(QSize(60, 70));
        disAgreeBtn->setMaximumSize(QSize(60, 70));
        disAgreeBtn->setStyleSheet(QString::fromUtf8("border-image: url(:/res/res/DisAgreeBtn.png);"));

        horizontalLayout_2->addWidget(disAgreeBtn);

        closeBtn = new QPushButton(widget);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));
        closeBtn->setMinimumSize(QSize(60, 70));
        closeBtn->setMaximumSize(QSize(60, 70));
        closeBtn->setStyleSheet(QString::fromUtf8("border-image: url(:/new/prefix1/\350\257\255\351\237\263\351\200\232\350\257\235\345\217\212\350\247\206\351\242\221\351\200\232\350\257\235UI/\350\257\255\351\237\263\351\200\232\350\257\235-\346\214\202\346\226\255.png);\n"
"border-image: url(:/res/res/HangUpBtn.png);"));

        horizontalLayout_2->addWidget(closeBtn);


        verticalLayout_2->addLayout(horizontalLayout_2);


        verticalLayout->addWidget(widget);


        retranslateUi(VoiceDialogWnd);

        QMetaObject::connectSlotsByName(VoiceDialogWnd);
    } // setupUi

    void retranslateUi(QWidget *VoiceDialogWnd)
    {
        VoiceDialogWnd->setWindowTitle(QCoreApplication::translate("VoiceDialogWnd", "Form", nullptr));
        closeBtn_2->setText(QString());
        label->setText(QCoreApplication::translate("VoiceDialogWnd", "XXX\350\257\267\346\261\202\351\200\232\350\257\235", nullptr));
        agreeBtn->setText(QString());
        disAgreeBtn->setText(QString());
        closeBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class VoiceDialogWnd: public Ui_VoiceDialogWnd {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VOICEDIALOGWND_H
