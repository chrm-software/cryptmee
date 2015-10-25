import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1
import ImControlThread 1.0

Page {
    id: otrChatWindow
    tools: commonTools

    /////////////////////////////////
    property alias contactName: contactNameLabel.text
    property alias contactFingerprint: fingerprintTextOutput.text
    /////////////////////////////////

    //////////////////////////////////////// Functions ////////////////////////////////
    function sendMessage(message) {
        var contactJID = contactName;

        if(resource.text != "")
            contactJID = contactJID + "/" + resource.text;

        console.debug("QML: OTR chat: Sending message to " + contactJID + ", from: " + otrConfigPage.selectedAccountName);

        if(otrConfigPage.imControlThread.sendOTRMessage(otrConfigPage.selectedAccountName, contactJID, message)) {
            // Add on success
            addMessage(message, false, Qt.formatTime(new Date(),"hh:mm"), false);
            typeMsgField.text = "";
            otrConfigPage.imControlThread.addChatMessage(contactJID, message, false, false);
        } else {
            addMessage(qsTr("Error sending message: " + message), false, Qt.formatTime(new Date(),"hh:mm"), true);
        }
    }

    function addMessage(text, remote, date, system) {
        chatListView.model.insert(0, { chatEntry: text, remoteText: remote, date: date, systemInfo: system });
    }

    function showPage(page) {
        if(page === "CHAT_PAGE") {
            chatRect.stateVisible = true;
            propsRect.stateVisible = false;
            buttonRow.checkedButton = b1;
            getAllMessages();
        } else {
            chatRect.stateVisible = false;
            propsRect.stateVisible = true;
            buttonRow.checkedButton = b2;
        }
    }

    function getBoolForInt(intValue) {
        if(intValue === "1")
            return true;
        else
            return false;
    }

    function getAllMessages() {
        chatListView.model.clear();

        var size = otrConfigPage.imControlThread.getChatHistorySizeFor(contactName);

        for(var i=0; i<size; i++) {
            var values = otrConfigPage.imControlThread.getChatHistoryMessageFor(contactName, i).split("|");
            addMessage(values[0], getBoolForInt(values[1]), values[2], getBoolForInt(values[3]));
        }
    }

    function getMessageFor(_contactName) {

        // Do not draw message for other users
        if(_contactName !== contactName)
            return;

        var values = otrConfigPage.imControlThread.getNewestChatMessageFor(_contactName).split("|");
        addMessage(values[0], getBoolForInt(values[1]), values[2], getBoolForInt(values[3]));
    }

    function getContactNameHeader() {
        return "CryptMee <font size='-5'>" + contactName + "</font>" ;
    }

    function chatBackground(_color) {
        chatRect.color = _color;
    }
    ///////////////////////////////////////////////////////////////////////////////////

    onStatusChanged: {
        if(status === DialogStatus.Open){
            getAllMessages();
            showPage("CHAT_PAGE");
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

        ButtonRow {
            id: buttonRow
            checkedButton: b1

            Button {
                id: b1
                iconSource: "image://theme/icon-m-toolbar-new-chat"
                onClicked: otrChatWindow.showPage("CHAT_PAGE")
            }

            Button {
                id: b2
                iconSource: "image://theme/icon-m-toolbar-addressbook"
                onClicked: otrChatWindow.showPage("PROPS_PAGE")
            }
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }

    Menu {
        id: myMenu
        visualParent: otrChatWindow
        MenuLayout {
            MenuItem { text: qsTr("Black background color")
                onClicked: {
                    chatBackground("black");
                }
            }
            MenuItem { text: qsTr("Normal background color")
                onClicked: {
                    chatBackground("transparent");
                }
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
            text: { return getContactNameHeader(); }
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    Rectangle {
        id: propsRect
        width: parent.width
        height: parent.height - topDecoartion.height
        visible: true
        anchors.top: topDecoartion.bottom
        color: "transparent"

        property bool stateVisible: true

        states: [
            State { when: propsRect.stateVisible;
                PropertyChanges {   target: propsRect; opacity: 1.0    }
            },
            State { when: !propsRect.stateVisible;
                PropertyChanges {   target: propsRect; opacity: 0.0    }
            }
        ]

        transitions: Transition {
            NumberAnimation {
                property: "opacity"; duration: 300

                onCompleted: {
                    propsRect.visible = propsRect.stateVisible;
                }
            }
        }

        Grid {
            id: settingsGrid
            columns: 1
            rows: 12
            spacing: 5
            anchors.fill: parent
            width: parent.width
            height: 350

            GroupSeparator {
                title: qsTr("Contact JID (set resource if needed)")
            }

            Row {
                width: parent.width;
                spacing: 5

                Text {
                    id: jidLabel
                    height: 50
                    text: " JID: ";
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22
                }

                Text {
                    id: contactNameLabel
                    height: 50
                    text: "";
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22

                    onTextChanged: {
                        if(contactNameLabel.text.indexOf("/") > -1) {
                            var jidParts = contactNameLabel.text.split("/");
                            contactNameLabel.text = jidParts[0];
                            resource.text = jidParts[1];
                        } else {
                            resource.text = "";
                        }
                    }
                }

                Text {
                    id: jitSeparator
                    height: 50
                    text: "/";
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22
                }

                TextField {
                    id: resource;
                    text: ""
                    height: 50
                    width: parent.width - jidLabel.width - contactNameLabel.width - jitSeparator.width - 16
                }
            }

            GroupSeparator {
                title: qsTr("Fingerprint")
            }

            TextArea {
                id: fingerprintTextOutput;
                text: "EMPTY"
                width: parent.width
                readOnly: true
                font.family: "Courier"
                font.pixelSize: 21
                font.bold: true
            }

            GroupSeparator {
                title: qsTr("Trust Fingerprint")
            }

            Row {
                width: parent.width

                Button {
                    id: trustFP
                    width: parent.width/2
                    text: "Manually"
                }
                Button {
                    id: trustFPSMP
                    width: parent.width/2
                    text: "SMP Verification"
                    enabled: false
                }
            }

            GroupSeparator {
                title: qsTr("Remove Fingerprint")
            }

            Button {
                id: removeFP
                width: parent.width
                text: "Remove Fingerprint"
            }

        }
    }

    Rectangle {
        id: chatRect
        width: parent.width
        height: parent.height - topDecoartion.height
        visible: true
        anchors.top: topDecoartion.bottom
        color: "black"

        property bool stateVisible: true

        states: [
            State { when: chatRect.stateVisible;
                PropertyChanges {   target: chatRect; opacity: 1.0    }
            },
            State { when: !chatRect.stateVisible;
                PropertyChanges {   target: chatRect; opacity: 0.0    }
            }
        ]

        transitions: Transition {
            NumberAnimation {
                property: "opacity"; duration: 300

                onCompleted: {
                    chatRect.visible = chatRect.stateVisible;
                }
            }
        }


        Flickable {
            id: flickAreaChat
            width: parent.width
            height: parent.height - 50
            flickableDirection: Flickable.VerticalFlick

            ListView {
                id: chatListView
                width: parent.width; height: parent.height

                model: ListModel {
                }

                rotation: 180
                delegate: chatDelegate
            }

            ScrollDecorator {
                flickableItem: chatListView
                rotation: 180
            }
        }



        Rectangle {
            anchors.top: flickAreaChat.bottom
            height: 50
            width: parent.width
            color: "white"

            Image {
                id: logoImage
                source: "image://theme/icon-m-common-locked"
                width: 48
                height: 48
            }

            TextField {
                id: typeMsgField
                anchors.left: logoImage.right
                width: parent.width - 150
                height: 50
                placeholderText: qsTr("Type your message here")
            }

            Button {
                id: sendMsgButton
                anchors.left: typeMsgField.right
                text: "Send"
                width: 100
                height: 40
                y: 5

                platformStyle: ButtonStyle {
                    background: "image://theme/meegotouch-button-inverted-background"
                    pressedBackground: "image://theme/color7-meegotouch-button-accent-background-pressed"
                    textColor: "white"
                }

                onClicked: {
                    //addMessage(typeMsgField.text, true, "01.01.1970", false);
                    sendMessage(typeMsgField.text);
                    chatListView.positionViewAtBeginning();
                }
            }
        }

        Component {
            id: chatDelegate

            Item {

                ListView.onAdd: SequentialAnimation {
                    PropertyAction { target: textButton; property: "height"; value: 0 }
                    NumberAnimation { target: textButton; property: "height"; to: (systemInfo) ? (my_text.height + 10) : (my_text.height + 40); duration: 250; easing.type: Easing.InOutQuad }

                    onCompleted: {
                        my_text.visible = true
                    }

                    onStarted: {
                        my_text.visible = false
                    }
                }

                height: {
                    return textButton.height + 10;
                }

                width: chatListView.width

                Rectangle {
                    height: textButton.height + 10;
                    width: parent.width
                    color: "transparent"
                    rotation: 180

                    Button {
                        id: textButton
                        width: {
                            if(systemInfo)
                                return parent.width/2;

                            return parent.width/1.3;
                        }

                        y: {
                            if(remoteText) return 0;
                            else return 5
                        }

                        height: {
                            if(systemInfo)
                                return my_text.height + 10;
                            else
                                return my_text.height + 40;
                        }

                        x: {
                            if(systemInfo)
                                return parent.width/4;

                            if(remoteText)
                                return parent.width/4.8;
                            else
                                return 12;
                        }

                        opacity: {
                            if(systemInfo)
                                return 0.7;
                            else
                                return 1.0;
                        }

                        platformStyle: ButtonStyle {
                            background: {
                                if(systemInfo)
                                    return "image://theme/color16-meegotouch-button-background-selected";

                                if(remoteText)
                                    return "qrc:/images/pix/bubble-out.png";
                                else
                                    return "qrc:/images/pix/bubble-in.png";
                            }
                            pressedBackground: {
                                if(systemInfo)
                                    return "image://theme/color16-meegotouch-button-background-selected";

                                if(remoteText)
                                    return "qrc:/images/pix/bubble-out.png";
                                else
                                    return "qrc:/images/pix/bubble-in.png";
                            }
                        }

                        Text {
                            id: my_text
                            anchors {centerIn: parent }
                            font.pixelSize: {
                                if(systemInfo)
                                    return 17;
                                else
                                    return 22;
                            }
                            font.bold: false
                            horizontalAlignment: {
                                if(systemInfo)
                                    return Text.AlignHCenter;

                                if(remoteText)
                                    return Text.AlignRight;
                                else
                                    return Text.AlignLeft;
                            }

                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

                            width: parent.width - 30
                            x: 15

                            y: {
                                if(remoteText)
                                    return 3;
                                else
                                    return 0;
                            }

                            color: {
                                if(systemInfo)
                                    return "white";

                                if(remoteText)
                                    return "white";
                                else
                                    return "black";
                            }

                            text: {
                                var content = "";

                                content += "" + chatEntry + "<br>";
                                content += "<font size='-2'>" + date + "</font>";

                                return content;
                            }
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
                    topDecoartion.visible = true;
                    if(chatRect.stateVisible) {
                        topDecoartion.visible = false;
                        return 0;
                    } else
                        return 45;
                }
            }
        },
        State {
            name: "portrait"
            PropertyChanges { target: label2; height: {
                    topDecoartion.visible = true;
                    return 65;
                }
            }
        }
    ]
}

