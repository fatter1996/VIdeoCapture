include("TCPSocket/TCPSocket.pri")
include("UDPSocket/UDPSocket.pri")
include("VoiceDialog/VoiceDialog.pri")

HEADERS += \
    $$PWD/CallButton.h \
    $$PWD/VoiceCallWnd.h

SOURCES += \
    $$PWD/CallButton.cpp \
    $$PWD/VoiceCallWnd.cpp

FORMS += \
    $$PWD/VoiceCallWnd.ui

RESOURCES +=
