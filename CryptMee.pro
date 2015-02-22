# Add more folders to ship with the application, here
folder_01.source = qml/CryptMee
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

folder_splash.source = pix/splash
folder_splash.target = splash
DEPLOYMENTFOLDERS += folder_splash

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
# CONFIG += mobility
# MOBILITY +=

# DB
QT += sql

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
    gnupgconnector.cpp

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
    qtc_packaging/debian_harmattan/changelog \
    pix/splash.jpg \

HEADERS += \
    mailreaderthread.h \
    mailreader.h \
    mailobject.h \
    keyreader.h \
    keyobject.h \
    gnupgconnector.h \
    configuration.h

RESOURCES += \
    cryptmeeressource.qrc

TRANSLATIONS += languages/lang_de_DE.ts

# Install splashscreen
#splash.path = /opt/CryptMee
#splash.files = pix/splash.png
#INSTALLS += splash

