import QtQuick 1.1
import com.nokia.meego 1.0
//import QtMobility.feedback 1.1

Page {
    tools: commonTools
    id: chatContacts

    /////////////////////////////////
    property string searchWord: ""
    /////////////////////////////////

    onStatusChanged: {
        if(status === DialogStatus.Open){
            if(searchWord !== "")
                searchWord = "";
            contactsListView.model.clear();
            // Add static search widget
            addContactEntry("--SEARCH--", "---", "--SEARCH--", "", "", "", "", "");
            fillContacts();
        }
    }

    onSearchWordChanged: {
        console.debug("QML: search word: " + searchWord);
        fillContacts();
    }

    function addContactEntry(name, fp, jid, icon, verified, newMsg, isOnline, lastMsgDate) {
        // Check first for search filter
        if(searchWord !== "" && name !== "--SEARCH--" && jid !== "--SEPARATOR--") {
            if(name.toLowerCase().indexOf(searchWord.toLowerCase()) < 0 && jid.indexOf(searchWord.toLowerCase()) < 0) {
                return;
            }
        }

        // Add entry
        //console.debug("QML: addContactEntry(): [" + name + "], verified: [" + verified + "], presence: [" + isOnline + "], fp: " + fp + ", hasNewMsg: [" + newMsg + "]");
        contactsListView.model.append({ contactName: name, fingerprint: fp, contactJID: jid, contactIcon: icon, contactVerified: verified, hasNewMsg: newMsg, onlineState: isOnline, lastDate: lastMsgDate });
    }

    function fillContacts() {
        var sizeModel = contactsListView.model.count;
        for(var j=1; j<sizeModel; j++) {
            contactsListView.model.remove(1);
        }

        var size = otrConfigPage.imControlThread.getNumOfAllContacts();
        var verifiedList = true;

        startPage.hasNewMsgs(false);

        for(var i=0; i<size; i++) {
            var tmpFP = otrConfigPage.imControlThread.getContact(i).split("|");

            // Insert separator before untrusted contacts
            /*if(verifiedList && tmpFP[3] === "zunknown") {
                addContactEntry(qsTr("Unknown Fingerprints"), "", "--SEPARATOR--", "--ICON--", "", "", "", "");
                verifiedList = false;
            }*/

            /*
              0: FP
              1: JID
              2: open-session-state
              3: verified
              4: has-new-msg
              5: name
              6: presence
              7: photo
              8: last-msg-date
              */

            addContactEntry(tmpFP[5], tmpFP[0], tmpFP[1], tmpFP[7], tmpFP[3], tmpFP[4], tmpFP[6], tmpFP[8]);

            if(tmpFP[4] === "1")
                startPage.hasNewMsgs(true);

            // Update chat view for user (new fp, new photo, ...)
            if(otrChatWindow.contactName === tmpFP[1]) {
                otrChatWindow.contactFingerprint = tmpFP[0];
                if(tmpFP[7].indexOf("file://") >= 0)
                    otrChatWindow.contactAvatar = "image://avatars/"+tmpFP[7];
            }
        }
    }

    ToolBarLayout {
        id: commonTools
        visible: true

        ToolIcon {
            id: backButton
            platformIconId: "toolbar-back";
            anchors.left: (parent === undefined) ? undefined : parent.left
            enabled: true

            onClicked: {
                pageStack.pop();
            }
        }
    }

    Rectangle {
        id: topDecoartion
        color: "#0000b0"
        width: parent.width
        height: childrenRect.height
        z: 1

        gradient: Gradient {
            GradientStop {color: "#0093dd"; position: 0.0}
            GradientStop {color: "#0069a0"; position: 0.9}
        }

        Label {
            id: label2
            x: 10
            y: 0
            width: parent.width
            height: 65
            color: "#ffffff"
            text: "CryptMee <font size='-5'>" + qsTr("Contacts") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    Rectangle {
        id: contactsRect
        width: parent.width
        height: parent.height - topDecoartion.height
        visible: true
        anchors.top: topDecoartion.bottom
        color: "transparent"

        Flickable {
            id: flickAreaContacts
            width: parent.width
            height: parent.height
            flickableDirection: Flickable.VerticalFlick

            onFlickStarted: {
                // TODO
                console.debug(flickAreaContacts.contentY);
            }

            ListView {
                id: contactsListView
                width: parent.width; height: parent.height

                model: ListModel {
                }

                delegate: contactsDelegate
            }

            ScrollDecorator {
                flickableItem: contactsListView
            }
        }
    }

    Component {
        id: searchBoxComponent

        TextField {
            id: textFieldSearch
            width: contactsListView.width - 20
            y: 10
            x: 10
            text: ""
            placeholderText: qsTr("Search...")
            platformStyle: TextFieldStyle { paddingRight: clearText.width }
            inputMethodHints: Qt.ImhNoPredictiveText;

            onTextChanged: {
                console.debug("QML: textFieldSearch: " + textFieldSearch.text);
                chatContacts.searchWord = textFieldSearch.text;
            }

            Image {
                id: clearText

                anchors {
                    top: parent.top;
                    right: parent.right;
                    margins: 3;
                }

                //fillMode: Image.PreserveAspectFit
                smooth: true; visible: textFieldSearch.text
                source: "image://theme/icon-m-input-clear"

                MouseArea {
                    id: clear
                    anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                    height: textFieldSearch.height; width: textFieldSearch.height
                    onClicked: {
                        textFieldSearch.text = ""
                    }
                }
            }
        }
    }

    Component {
        id: emptyComponent
        Rectangle {
            height: 0
            width: 0
            visible: false
        }
    }

    Component {
        id: contactsDelegate

        Item {

            height: {
                if(contactJID === "--SEPARATOR--")
                    return 45;
                else
                    return contactNamesRect.height + 4;
            }

            width: contactsListView.width

            Rectangle {
                height: parent.height
                width: parent.width
                color: "transparent"

                MouseArea {
                    width: parent.width;
                    height: parent.height

                    onClicked: {
                        contactNamesRect.color = "red";

                        console.debug("QML: ChatContacts: opening chat for: " + contactJID + ", " + fingerprint);

                        if(contactJID !== "--SEARCH--" && contactJID !== "--SEPARATOR--") {
                            otrChatWindow.contactName = contactJID;
                            otrChatWindow.contactFingerprint = fingerprint;
                            otrChatWindow.contactAvatar = avatarImage.source;
                            pageStack.push(otrChatWindow);
                        }

                        //chatContacts.searchWord = "";
                    }
                }

                GroupSeparator {
                    visible: {
                        if(contactJID === "--SEPARATOR--")
                            return true;
                        else
                            return false;
                    }

                    title: { return contactName; }
                }

                Loader {
                    sourceComponent: {
                        if(contactJID === "--SEARCH--")
                            return searchBoxComponent;
                        else
                            return emptyComponent;
                    }
                }

                Rectangle {
                    id: newMsgRect
                    height: contactNamesRect.height - 3
                    width: 15
                    color: "#0093dd"
                    visible: {
                        if(hasNewMsg === "1")
                            return true;
                        else
                            return false;
                    }
                }

                Rectangle {
                    id: spacer
                    anchors.left: newMsgRect.right
                    height: contactNamesRect.height
                    width: 15
                    color: "transparent"
                }

                Text {
                    id: contactNamesRect
                    anchors.left: spacer.right
                    font.pixelSize: 25
                    width: parent.width - 115
                    text: { return "<b>" + contactName + "</b><br><font size='-2'>" + contactJID + "</font>" + "<br><font size='-2' color='#0093dd'>" + lastDate + "</font>"; }
                    visible: {
                        if(contactJID !== "--SEARCH--" && contactJID !== "--SEPARATOR--")
                            return true;
                        else
                            return false;
                    }
                }

                Image {
                    id: avatarImage
                    anchors.left: contactNamesRect.right
                    width: 64
                    height: 64
                    source: {
                        if(contactIcon.indexOf("file://") < 0)
                            return "image://theme/icon-m-content-avatar-placeholder";
                        else
                            return "image://avatars/"+contactIcon;
                    }

                    visible: {
                        if(contactJID !== "--SEARCH--" && contactJID !== "--SEPARATOR--")
                            return true;
                        else
                            return false;
                    }

                    Image {
                        x: parent.width - 18
                        width: 18
                        height: 18
                        source: {
                            if(onlineState === "presence-status-available")
                                return "image://theme/icon-s-common-presence-online"
                            else if(onlineState === "presence-status-away")
                                return "image://theme/icon-s-common-presence-away"
                            else if(onlineState === "presence-status-extended-away")
                                return "image://theme/icon-s-common-presence-away"
                            else if(onlineState === "presence-status-busy")
                                return "image://theme/icon-s-common-presence-busy"
                            else if(onlineState === "presence-status-offline")
                                return "image://theme/icon-s-common-presence-offline"
                            else
                                return "image://theme/icon-s-common-presence-unknown"
                        }
                    }

                    Image {
                        x: parent.width - 20
                        y: parent.height - 20
                        width: 20
                        height: 20
                        visible: {
                            if(contactVerified === "zunknown")
                                return false;
                            else
                                return true;
                        }

                        source: {
                            if(contactVerified === "smp")
                                return "qrc:/images/pix/otr_veryfied.png"
                            else if(contactVerified === "verified")
                                return "qrc:/images/pix/otr_veryfied.png"
                            else
                                return "qrc:/images/pix/otr_unknown.png"
                        }
                    }
                }
            }
        }
    }

    state: (screen.currentOrientation === Screen.Portrait) ? "portrait" : "landscape"

    states: [
        State {
            name: "landscape"
            PropertyChanges { target: label2; height: {
                    return 45;
                }
            }
        },
        State {
            name: "portrait"
            PropertyChanges { target: label2; height: {
                    return 65;
                }
            }
        }
    ]

}
