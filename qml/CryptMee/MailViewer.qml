// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    tools: commonTools
    id: mailViewPage

    property alias prop_eMailHeader: labelHeader.text
    property alias prop_eMailContent: textareaContent.text
    property int prop_idx: -1
    property string rawMailData: ""

    function setAttachments() {
        dialogAttach.model.clear();

        var size = mailPage.prop_mailReader.getAttachmentListCount();

        for(var i=0; i<size; i++) {
            var tmpAttachment = mailPage.prop_mailReader.getAttachment(i);
            dialogAttach.model.append({ name: tmpAttachment });
        }

        dialogAttach.selectedIndex = 0;
    }

    function mailDecrypted(_content, _result) {
        if(_result)
            delayDecryptTimer.stop();

        rawMailData = _content;
        prop_eMailContent = mailPage.prop_mailReader.parseMailContent(_content);
        setAttachments();

        busyIndicator.running = false;
        busyIndicator.visible = false
    }

    onStatusChanged: {
        if(status === DialogStatus.Open){
            console.debug("[mailViewPage]: onOpen, call pgpDecrypt")
            delayDecryptTimer.start();
            setErrorMessage("");
            prop_eMailContent = "[Loading...]";
            busyIndicator.running = true;
            busyIndicator.visible = true
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
        visible: false
        z: 1
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
                myMenu.close();
                pageStack.pop();
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
        visualParent: mailViewPage
        MenuLayout {
            MenuItem { text: qsTr("Show attachments")
                onClicked: {
                    dialogAttach.open();
                }
            }

            MenuItem { text: qsTr("Show raw data")
                onClicked: {
                    prop_eMailContent = rawMailData;
                }
            }

            MenuItem { text: qsTr("Show GnuPG log")
                onClicked: {
                    pageStack.push(gpgHistoryPage);
                }
            }
        }
    }

    SelectionDialog {
        id: dialogAttach
        titleText: qsTr("Open attachment:")
        selectedIndex: 0

        model: ListModel {}

        onAccepted: {
            var filename = dialogAttach.model.get(dialogAttach.selectedIndex).name;
            Qt.openUrlExternally("file://" + TMP_DIR + filename);
        }
    }

    Timer {
        id: delayDecryptTimer
        interval: 1000
        repeat: false
        running: false

        onTriggered: {
            startPage.pgpDecrypt(mailPage.prop_mailReader.getContent(prop_idx), "MAIL_DECRYPT");
        }
    }

    /*Rectangle {
        id: topDecoartion
        color: "#0000b0"
        width: parent.width
        height: childrenRect.height
        anchors.top: parent.top

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
            text: "CryptMee <font size='-5'>eMail</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }*/

    Rectangle {
        id: errorMessage
        color: "#ff0000"
        width: parent.width
        height: labelError.height
        anchors.top: parent.top
        visible: false

        gradient: Gradient {
            GradientStop {color: "#ff0000"; position: 0.0}
            GradientStop {color: "#000000"; position: 0.9}
        }
        Flickable {
            id: flickAreaError
            anchors.fill: parent
            contentWidth: labelError.width
            contentHeight: labelError.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            Label {
                id: labelError
                width: errorMessage.width
                height: 0
                color: "#ffffff"
                text: "No errors"
                font.pixelSize: 18
                font.bold: false
                visible: false
            }
        }
    }


    Rectangle {
        id: mailHeader
        width: parent.width
        height: labelHeader.height
        anchors.top: errorMessage.bottom
        color: "#dddddd"
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#DDDDDD" }
            GradientStop { position: 1.0; color: "#AAAAAA" }
        }

        Label {
            id: headerIcon
            width: 50
            x: parent.width - 120
            height: labelHeader.height

            Image {
                id: iconHeader
                width: 120
                height: 110
                source: "qrc:/images/pix/gpg-mail.png"
                anchors.margins: 5
                fillMode: Image.PreserveAspectFit
                smooth: true
            }
        }

        Text {
            id: labelHeader
            x: 5
            width: parent.width
            height: 110
            text: ""
            font.pixelSize: 20
        }

        Rectangle {
            x: 0
            width:  labelHeader.width
            height: labelHeader.width
            rotation: 270

            gradient: Gradient {

                GradientStop {
                position: 0.0
                color: "#00DDDDDD"
                }
                GradientStop {
                position: 1.0
                color: "#EEDDDDDD"
                }
            }
        }
    }

    Rectangle {
        id: mailContent
        width: parent.width
        anchors.top: mailHeader.bottom
        anchors.bottom: parent.bottom

        Flickable {
            id: flickAreaContent
            anchors.fill: parent
            contentWidth: textareaContent.width
            contentHeight: textareaContent.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            TextArea {
                id: textareaContent
                readOnly: true
                text: "Content"
                width:mailContent.width
            }
        }

        ScrollDecorator {
            flickableItem: flickAreaContent
        }
    }

    function setErrorMessage(_msg) {
        if(_msg !== "") {
            errorMessage.visible = true;
            labelError.visible = true;
            labelError.text = _msg;
            labelError.height = 150;
        } else {
            errorMessage.visible = false;
            labelError.visible = false;
            labelError.text = _msg;
            labelError.height = 0;
        }
    }



}
