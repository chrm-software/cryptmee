import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1
import ImControlThread 1.0

Page {
    tools: commonTools
    id: otrChatWindow

    /////////////////////////////////
    property alias contactName: contactNameLabel.text
    property alias contactFingerprint: fingerprintTextOutput.text
    /////////////////////////////////

    function sendMessage() {
        var contactJID = contactName;

        if(resource.text != "")
            contactJID = contactJID + "/" + resource.text;

        console.debug("QML: OTR chat: Sending message to " + contactJID + ", from: " + otrConfigPage.selectedAccountName);

        if(otrConfigPage.imControlThread.sendOTRMessage(otrConfigPage.selectedAccountName, contactJID, chatText.text)) {
            chatText.text = "";
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

        gradient: Gradient {
            GradientStop {color: "#0093dd"; position: 0.0}
            GradientStop {color: "#0069a0"; position: 0.9}
        }

        Label {
            id: label2
            x: 10
            y: 0
            width: parent.width - 40
            height: 65
            color: "#ffffff"
            text: "CryptMee <font size='-5'>" + qsTr("OTR Chat") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    Rectangle {
        id: chatArea
        height: parent.height - topDecoartion.height
        width: parent.width
        color: "transparent"
        border.color: "transparent"
        anchors.top: topDecoartion.bottom
        //anchors.bottom: parent.bottom


        Flickable {
            id: flickAreaChat
            width: parent.width
            height: parent.height
            contentWidth: parent.width - 2
            contentHeight: settingsGrid.height
            flickableDirection: Flickable.VerticalFlick
            clip: true


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
                    title: qsTr("Send encrypted message")
                }

                TextArea {
                    id: chatText;
                    text: ""
                    width: parent.width
                }

                Button {
                    id: buttonSend
                    width: parent.width
                    height: 55
                    text: qsTr("Send Message")

                    onClicked: {
                        sendMessage();
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
                }
            }
        }
    }
}
