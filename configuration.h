#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Release
#define RELEASE "0.2.3"

// Set Build target
#define MEEGO_BUILD
//#define WIN_BUILD

// GnuPG binary
#ifdef WIN_BUILD
    #define GPGBIN QString("\"C:\\Program Files (x86)\\GNU\\GnuPG\\gpg2.exe\"")
#else
    #define GPGBIN QString("/usr/bin/gpg")
#endif

// Temporary files
#define TMPFILE QDir::homePath() + "/.cryptmee.tmp"

// Default key server
#define KEYSERVER "pgp.mit.edu"

// eMails storage
#ifdef WIN_BUILD
    #define MAIL_PATH "D:/chrm-profile/Projects/QtProjects/CryptMee/tmpData/.qmf/mail/"
    #define MAIL_DB "D:/chrm-profile/Projects/QtProjects/CryptMee/tmpData/.qmf/database/qmailstore.db"
    #define TMP_DIR "D:/chrm-profile/Projects/QtProjects/CryptMee/tmpData/.CryptMee/"
#else
    #define MAIL_PATH QDir::homePath()+"/.qmf/mail/"
    #define MAIL_DB QDir::homePath()+"/.qmf/database/qmailstore.db"
    #define TMP_DIR "/home/user/MyDocs/.CryptMee/"
#endif

// Import file prefix
#ifdef WIN_BUILD
    #define MYDOCS_PATH QString("D:/chrm-profile/Projects/QtProjects/CryptMee/tmpData/")
#else
    #define MYDOCS_PATH QString("/home/user/MyDocs/")
#endif



#endif // CONFIGURATION_H
