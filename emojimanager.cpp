#include <QFileInfo>
#include "emojimanager.h"

EmojiManager::EmojiManager(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
{
}

// Parse XML here
bool EmojiManager::initEmojis(QString _descriptionFile)
{
    QFileInfo fi(_descriptionFile);
    QFile file(_descriptionFile);

    qDebug() << "EmojiManager::initEmojis(" + fi.fileName() + ")";

    if(fi.exists()) {
        qDebug() << "EmojiManager::initEmojis()...";
        this->emojiPath = fi.canonicalPath();
        return this->read(&file);
    }

    qWarning() << "EmojiManager::initEmojis(): unable to init emojis. File does not exists.";
    return false;
}

QString EmojiManager::replaceEmojisInMsg(QString _msg)
{
    QString retVal = _msg;

    QHashIterator<QString, QString> i(this->emojiHash);
    while (i.hasNext()) {
        i.next();
        if(_msg.contains(i.key())) {
            retVal.replace(i.key(), this->getEmojiImgForCode(i.key()));
        }
    }

    return retVal;
}

// Find emoji and return html img-code
QString EmojiManager::getEmojiImgForCode(QString _code)
{
    if(!this->emojiHash.keys().contains(_code))
        return _code;

    return "<img src='file:/" + this->emojiPath + "/" + this->emojiHash.value(_code) + "'>";
}

bool EmojiManager::read(QIODevice *device)
{
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                &errorColumn)) {
        // Error occured
        qWarning() << "EmojiManager::read(): Unable to parse XML: " << errorStr << " at line: " << errorLine;
        return false;
    }

    qDebug() << "EmojiManager::read()...";

    QDomElement root = domDocument.documentElement();

    if (root.tagName() != "icondef") {
        qWarning() << "EmojiManager::read(): Unable to parse XML: no <icondef> root element found";
        return false;
    }

    QDomElement child = root.firstChildElement("icon");
    QDomNodeList innerList;
    QDomElement element;
    QString currentKey = "";

    while (!child.isNull()) {
        innerList = child.childNodes();

        currentKey = "";
        for(int i=0; i<innerList.size(); i++) {
            element = innerList.at(i).toElement();

            if(!element.isNull()) {
                if(element.tagName() == "text") {
                    currentKey += (element.text() + "_#_");
                } else if(element.tagName() == "object" && currentKey != "") {
                    for(int j=0; j<currentKey.split("_#_").size(); j++)
                        this->emojiHash[currentKey.split("_#_").at(j)] = element.text();
                }
            }
        }
        child = child.nextSiblingElement("icon");
    }

    this->emojiHash.remove("");
    return true;
}
