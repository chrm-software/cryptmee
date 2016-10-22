# Add more folders to ship with the application, here
folder_01.source = qml/CryptMee
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

folder_splash.source = pix/splash
folder_splash.target = splash
DEPLOYMENTFOLDERS += folder_splash

# Deploy otrlibs
#folder_otrlibs.source = lib
#folder_otrlibs.target = /usr
#DEPLOYMENTFOLDERS += folder_otrlibs

libs.path = /usr/lib
libs.files = lib/libotr.so.2.2.0
INSTALLS += libs

ldd.path = /etc/ld.so.conf.d
ldd.files = lib/kdepim.conf
INSTALLS += ldd

#libs2.path = /opt/CryptMee/bin
#libs2.files = lib/libgpg-error.so.0.8.0
#INSTALLS += libs2

#libs3.path = /opt/CryptMee/bin
#libs3.files = lib/libgcrypt.so.11.1.1
#INSTALLS += libs3


# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xE4AB2BF6

# Smart Installer package's UID
# This UID is from the protected range and therefore the package will
# fail to install if self-signed. By default qmake uses the unprotected
# range value if unprotected UID is defined for the application and
# 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
QT += dbus
#CONFIG += mobility
#MOBILITY += messaging

# DB
QT += sql

# Emojis
QT += xml

# Picture upload
QT += network

# Tracker
# CONFIG+=qtsparql

# Other libs
CONFIG += link_pkgconfig
PKGCONFIG += libotr
LIBS += -lotr -lgcrypt -lgpg-error


# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
CONFIG += qdeclarative-boostable

# Add dependency to Symbian components
# CONFIG += qt-components

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    mailreaderthread.cpp \
    mailreader.cpp \
    mailobject.cpp \
    keyreader.cpp \
    keyobject.cpp \
    gnupgconnector.cpp \
    imcontrolthread.cpp \
    otrlconnector.cpp \
    trackeraccess.cpp \
    emojimanager.cpp \
    pictureuploader.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/postinst \
    qtc_packaging/debian_harmattan/changelog \
    pix/splash.jpg \
    qml/CryptMee/OTRChatWindow.qml

HEADERS += \
    mailreaderthread.h \
    mailreader.h \
    mailobject.h \
    keyreader.h \
    keyobject.h \
    gnupgconnector.h \
    configuration.h \
    imcontrolthread.h \
    otrlconnector.h \
    trackeraccess.h \
    emojimanager.h \
    pictureuploader.h

RESOURCES += \
    cryptmeeressource.qrc

lupdate_only{
    SOURCES += \
        qml/CryptMee/GpgHistoryViewPage.qml \
        qml/CryptMee/GroupSeparator.qml \
        qml/CryptMee/KeyPage.qml \
        qml/CryptMee/KeySearchResultSelectionPage.qml \
        qml/CryptMee/MailPage.qml \
        qml/CryptMee/MailViewer.qml \
        qml/CryptMee/main.qml \
        qml/CryptMee/MainPage.qml \
        qml/CryptMee/OTRConfigPage.qml \
        qml/CryptMee/PasswordInputPage.qml \
        qml/CryptMee/PubKeySelectionPage.qml \
        qml/CryptMee/SettingsPage.qml \
        qml/CryptMee/StartPage.qml \
        qml/CryptMee/OTRChatWindow.qml \
        qml/CryptMee/ChatContacts.qml \
        qml/CryptMee/PictureSelectionPage.qml
}

TRANSLATIONS += languages/lang_de_DE.ts \
                languages/lang_ru_RU.ts

# Install splashscreen
#splash.path = /opt/CryptMee
#splash.files = pix/splash.png
#INSTALLS += splash

