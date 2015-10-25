#ifndef OTRLCONNECTOR_H
#define OTRLCONNECTOR_H

#include "imcontrolthread.h"
#include <QHash>

extern "C" {
    #include <libotr/proto.h>
    #include <libotr/userstate.h>
    #include <libotr/message.h>
    #include <libotr/privkey.h>
    #ifndef OTRL_PRIVKEY_FPRINT_HUMAN_LEN
        #define OTRL_PRIVKEY_FPRINT_HUMAN_LEN 45
    #endif
}

class ImControlThread;

class OTRLConnector
{
public:
    OTRLConnector(ImControlThread* _imControl);
    ~OTRLConnector();

    void setAccountName(QString _account);
    bool setUpKeys();
    void startSession(const QString& _account, const QString& _contact);
    QString decryptMessage(const QString& _account, const QString& _sender, const QString& _message);
    QString encryptMessage(const QString& _account, const QString& _contact,
                                          const QString& _message);
    bool hasPrivKeyForAccount(QString _account);
    QString getFingerprintForAccount(QString _account);
    void expireSession(const QString& _account, const QString& _contact);
    void endSession(const QString& _account, const QString& _contact);
    QStringList getFingerprints();
    QString getLibOTRVersion();

private:
    QString otrAccountName;
    ImControlThread* myImControl;
    OtrlUserState otrUserState;
    OtrlMessageAppOps ui_ops;
    QString libOTRVersion;

    bool isGeneratingPrivKey;

    OtrlUserState get_userstate(const char* username);
    QString humanFingerprint(const unsigned char* fingerprint);

    // Static libotr callbacks
    static OtrlPolicy policy_cb(void *opdata, ConnContext *context);
    static void create_privkey_cb(void *opdata, const char *accountname,
                                  const char *protocol);
    static const char *protocol_name_cb(void *opdata, const char *protocol);
    static void protocol_name_free_cb(void* opdata, const char* protocol_name);
    static void new_fingerprint_cb(void *opdata, OtrlUserState us,
                                   const char *accountname, const char *protocol, const char *username,
                                   unsigned char fingerprint[20]);
    static void write_fingerprints_cb(void *opdata);
    static int display_otr_message_cb(void *opdata, const char *accountname,
                                      const char *protocol, const char *username, const char *msg);
    static int is_online_cb(void *opdata, const char *accountname,
                            const char *protocol, const char *recipient);
    static void inject_message_cb(void *opdata, const char *accountname,
                                  const char *protocol, const char *recipient, const char *message);
    static void notify_cb(void *opdata, OtrlNotifyLevel level,
                          const char *accountname, const char *protocol, const char *username,
                          const char *title, const char *primary, const char *secondary);
    static void gone_secure_cb(void* opdata, ConnContext* context);
    static void gone_insecure_cb(void* opdata, ConnContext* context);
    static const char* account_name_cb(void* opdata, const char* account, const char* protocol);
    static void account_name_free_cb(void* opdata, const char* account_name);

    // Callbacks
    //OtrlPolicy policy(ConnContext* context);
    void create_privkey(const char* accountname, const char* protocol);
    void inject_message(const char* accountname, const char* protocol,
                        const char* recipient, const char* message);
    void write_fingerprints();
    void new_fingerprint(const char *username, unsigned char fingerprint[20]);
    void gone_secure(ConnContext* context);
    void gone_insecure(ConnContext* context);
    void notify(OtrlNotifyLevel level,
                const char *accountname, const char *protocol, const char *username,
                const char *title, const char *primary, const char *secondary);
    int display_otr_message(const char *msg);
    const char* account_name(const char* account, const char* protocol);
    void account_name_free(const char* account_name);

};

#endif // OTRLCONNECTOR_H
