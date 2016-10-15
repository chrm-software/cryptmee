#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Release
#define RELEASE "0.3.5"

// Set Build target
#define MEEGO_BUILD
//#define WIN_BUILD

// GnuPG binary
#ifdef WIN_BUILD
    #define GPGBIN QString("\"C:\\Program Files (x86)\\GNU\\GnuPG\\gpg2.exe\"")
    #define TRACKER_BINARY QString ("NOT_AVAILABLE")
    #define MCTOOL_BINARY QString ("NOT_AVAILABLE")
#else
    #define GPGBIN QString("/usr/bin/gpg")
    #define TRACKER_BINARY QString ("/usr/bin/tracker-sparql")
    #define MCTOOL_BINARY QString ("/usr/bin/mc-tool")
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
    #define MAIL_PATH QDir::homePath() + "/.qmf/mail/"
    #define MAIL_DB QDir::homePath() + "/.qmf/database/qmailstore.db"
    #define TMP_DIR QDir::homePath() + "/MyDocs/.CryptMee/"
#endif

// Import file prefix
#ifdef WIN_BUILD
    #define MYDOCS_PATH QString("D:/chrm-profile/Projects/QtProjects/CryptMee/tmpData/")
#else
    #define MYDOCS_PATH QString(QDir::homePath() + "/MyDocs/")
#endif


// OTR
#define OTR_DIR QDir::homePath() + "/.otr/"
#define OTR_PROTOCOL_STRING "prpl-jabber"
#define OTR_FINGERPRINTS_FILE QDir::homePath() + "/.otr/otr.fingerprints"
#define OTR_KEYS_FILE QDir::homePath() + "/.otr/otr.keys"
#define OTR_PRIVKEY_FILE QDir::homePath() + "/.otr/otr_priv_key"

#define ENCRYPT_SYMBOL QString::fromUtf8("\u2611")

// Emojis
#define EMOJI_DEF "/home/user/.emojis/Emojis-1.0/icondef.xml"

#endif // CONFIGURATION_H
