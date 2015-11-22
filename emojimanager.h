#ifndef EMOJIMANAGER_H
#define EMOJIMANAGER_H

#include <QObject>
#include <QDeclarativeItem>
#include <QHash>
#include <QDomDocument>

class EmojiManager : public QDeclarativeItem
{
    Q_OBJECT
public:
    explicit EmojiManager(QDeclarativeItem *parent = 0);

    Q_INVOKABLE bool initEmojis(QString _descriptionFile);
    Q_INVOKABLE QString replaceEmojisInMsg(QString _msg);

private:
    QString emojiPath;
    QHash<QString, QString> emojiHash;
    QDomDocument domDocument;
    bool read(QIODevice *device);
    QString getEmojiImgForCode(QString _code);

signals:

public slots:

};

#endif // EMOJIMANAGER_H
