#include <QtGui/QApplication>
#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>
#include <qdeclarative.h>
#include <qdeclarativeitem.h>
#include <qdeclarativeview.h>

#include <QTranslator>
#include <QTextCodec>
#include <QLocale>
#include <QDir>

#include "qmlapplicationviewer.h"
#include "gnupgconnector.h"
#include "mailreader.h"
#include "configuration.h"
#include "imcontrolthread.h"
#include "pictureuploader.h"


Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    // i18n stuff
    qDebug() << "I18N: local:" << QLocale::system().name();
    QString locale = QLocale::system().name();

    if(locale.startsWith("de_"))
        locale = "de_DE";
    else if(locale.startsWith("ru_"))
        locale = "ru_RU";

    QString filename = QString("languages/languages/lang_") + locale;

    // Data for Settings
    QCoreApplication::setOrganizationName("chrmSoft");
    QCoreApplication::setOrganizationDomain("chrm.info");
    QCoreApplication::setApplicationName("CryptMee");


    static QTranslator translator;
    if( translator.load(filename, ":/") ){
        app->installTranslator(&translator);
        QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
        qDebug() << "I18N: Translation file loaded" << filename;
    } else
        qDebug() << "I18N: Translation file not loaded:" << filename;


    // Register C++ classes
    qmlRegisterType<GnuPGConnector>("GnuPGConnector", 1, 0, "GnuPGConnector");
    qmlRegisterType<MailReader>("MailReader", 1, 0, "MailReader");
    qmlRegisterType<ImControlThread>("ImControlThread", 1, 0, "ImControlThread");
    qmlRegisterType<PictureUploader>("PictureUploader", 1, 0, "PictureUploader");

    QmlApplicationViewer viewer;
    QDeclarativeContext *ctxt = viewer.rootContext();
    ctxt->setContextProperty("Release_Version", RELEASE);
    ctxt->setContextProperty("GnuPG_PATH", GPGBIN);
    ctxt->setContextProperty("GnuPG_TMPFILE", TMPFILE);
    ctxt->setContextProperty("GnuPG_KEYSERVER", KEYSERVER);
    ctxt->setContextProperty("MAIL_PATH", MAIL_PATH);
    ctxt->setContextProperty("MAIL_DB", MAIL_DB);
    ctxt->setContextProperty("TMP_DIR", TMP_DIR);

    // Install avatar image provider
    QDeclarativeEngine *engine = ctxt->engine();
    engine->addImageProvider(QLatin1String("avatars"), new AvatarImageProvider);

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/CryptMee/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
