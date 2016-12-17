import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

Page {
    tools: commonTools
    id: mainPage

    property alias textField: textarea1.text
    property alias errorField: textarea2.text
    property string fileToAttach: ""
    property string currentKeyID: ""
    property string selectedFile: ""

    function setTargetKeyID(_name, _ids) {
        currentKeyID = _name;

        // _ids contais creation date as first array value. Ignore it and start with the 2nd.
        if(_ids.length > 2) {
            // Ask first for identity
            dialogSelectPGPIdentity.model.clear();

            for(var i=1; i<_ids.length; i++) {
                dialogSelectPGPIdentity.model.append({ name: _ids[i] });
            }

            dialogSelectPGPIdentity.selectedIndex = 0;

            console.debug("QML: setTargetKeyID: more then one identity found, ask for the right one: "+_ids.length);
            dialogSelectPGPIdentity.open();

        } else {
            setIdentity(_ids[1]);
        }
    }

    function setIdentity(_identity) {
        console.debug("QML: setting identity to: "+_identity);
        textarea3.text = _identity.replace("&lt;", "<");
        mainPage.state = "contentEncrypted";
    }

    function attachEncryptedFile(_path) {
        selectedFile = _path;
        textarea4.text = qsTr("File to encrypt: ") + _path;
    }

    function textEncrypted(_content) {
        textField = _content;

        // Hide error box
        textarea2.visible = errorMessage.visible = false;
        textarea2.height = 0;

        // Is file selected? If yes, encrypt it too
        if(selectedFile != "") {
            if(currentKeyID != "") {
                busyIndicator.running = true;
                busyIndicator.visible = true;

                textarea4.text = qsTr("Encrypting file. Please wait... " + _path);
                startPage.pgpEncryptFile(selectedFile, currentKeyID, "FILE_ENCRYPT");

            } else {
                textarea4.text = qsTr("Select public key first!");
            }
        }
    }

    function attachmentEncrypted(_filename) {
        fileToAttach = _filename;
        console.debug("QML: file encrypted: "+_filename);
        textarea4.text = qsTr("Encrypted file name: ") + _filename;

        busyIndicator.running = false;
        busyIndicator.visible = false;
    }

    function errorOccured(_output) {
        errorField = _output;

        // Show error box
        textarea2.visible = errorMessage.visible = true;
        textarea2.height = 200;

        busyIndicator.running = false;
        busyIndicator.visible = false;
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
        visible: false
        z: 1
    }

    InfoBanner {
        id: infoBanner
        z: 5
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
            id: copyToClipboard
            //anchors.left: backButton.right
            iconSource: "qrc:/images/pix/clipboard_to.png"

            onClicked: {
                startPage.gpgConnector.setToClipboard(textarea1.text);
                infoBanner.text = qsTr("Copied text to clipboard");
                infoBanner.show();
            }
        }

        ToolIcon {
            id: sendEMail
            //anchors.left: copyToClipboard.right
            iconSource: "image://theme/icon-m-toolbar-send-email"
            enabled: false

            onClicked: {
                console.debug("Open Mail Client.");
                infoBanner.text = qsTr("Sending eMail...");
                infoBanner.show();

                var tmpStr = textarea3.text;
                var emailOnly = tmpStr.substring(tmpStr.lastIndexOf("<")+1,tmpStr.lastIndexOf(">"));

                console.debug("QML: compose email to: "+emailOnly);

                if(fileToAttach == "")
                    Qt.openUrlExternally("mailto:"+emailOnly+"?subject=Encrypted eMail&body="+textarea1.text);
                else
                    Qt.openUrlExternally("mailto:"+emailOnly+"?subject=Encrypted eMail&body="+textarea1.text+"&attach="+fileToAttach);
            }
        }

        ToolIcon {
            id: clearText            
            iconSource: "image://theme/icon-m-toolbar-backspace"

            onClicked: {
                textarea1.text = "";
                textarea3.text = "";
                textarea4.text = qsTr("Encrypt file");
                fileToAttach = "";
                currentKeyID = "";
                selectedFile = "";
                mainPage.state = "contentPlain";

                textarea2.visible = errorMessage.visible = false;
                textarea2.height = 0;
            }
        }

        ToolIcon {
            id: showErrors
            //anchors.left: clearText.right
            iconSource: "image://theme/icon-s-error"

            onClicked: {
                if(textarea2.visible) {
                    textarea2.visible = errorMessage.visible = false;
                    textarea2.height = 0;
                } else {
                    textarea2.visible = errorMessage.visible = true;
                    textarea2.height = 200;
                }
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
        visualParent: mainPage
        MenuLayout {
            MenuItem { text: qsTr("Decrypt")
                onClicked: {
                    startPage.pgpDecrypt();                    
                    textarea3.text = "";
                    textarea4.text = "";
                    fileToAttach = "";
                    currentKeyID = "";
                    selectedFile = "";
                    mainPage.state = "contentPlain";
                }
                id: menuItemDecrypt
            }

            MenuItem { text: qsTr("Encrypt")
                onClicked: {
                    if(currentKeyID != "")
                        startPage.pgpEncrypt(currentKeyID, "TXT_ENCRYPT");
                    else
                        startPage.pgpEncrypt();

                    sendEMail.enabled = true;
                }
                id: menuItemEncrypt
                enabled: true
            }

            MenuItem { text: qsTr("Show GnuPG log")
                onClicked: {
                    pageStack.push(gpgHistoryPage);
                }
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
            width: parent.width
            height: 65
            color: "#ffffff"
            text: "CryptMee <font size='-5'>" + qsTr("Encrypt Or Decrypt Text") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }


    Component.onCompleted: {
        textarea1.text = startPage.gpgConnector.getFromClipboard();
    }


    Rectangle {
        id: errorMessage
        color: "#ff0000"
        width: parent.width
        height: textarea2.height
        anchors.top: topDecoartion.bottom
        visible: false

        gradient: Gradient {
            GradientStop {color: "#ff0000"; position: 0.0}
            GradientStop {color: "#000000"; position: 0.9}
        }

        Label {
            id: textarea2
            width: parent.width
            visible: false;
            height: 0
            color: "#ffffff"
            text: qsTr("No Errors")
            font.pixelSize: 18
            font.bold: false
        }
    }

    Rectangle {
        id: targetIDRect
        color: "#000000"
        width: parent.width
        height: textarea3.height
        anchors.top: errorMessage.bottom
        visible: false

        Label {
            id: textarea3
            width: parent.width
            visible: true;
            height: 0
            color: "#ffffff"
            text: qsTr("")
            //textFormat: Text.PlainText
            font.pixelSize: 18
            font.bold: false
        }
    }

    Rectangle {
        anchors.top: targetIDRect.bottom        
        width: parent.width
        height: parent.height - errorMessage.height - targetIDRect.height - showErrors.height - fileRectangle.height
        color: "transparent"
        id: textRectangle

        Flickable {
            id: flickAreaTxt
            anchors.fill: parent
            contentWidth: parent.width
            contentHeight: textarea1.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            TextArea {
                id: textarea1
                width: parent.width
                anchors.topMargin: 0
                placeholderText: qsTr("Enter Text...")

                //onTextChanged: {
                    // TODO: check if pasted content is encrypted
                //}

                onActiveFocusChanged: {
                    if(textarea1.focus) {
                        textarea4.height = 0;
                        fileRectangle.visible = false;
                        attachEncryptedPicButton.visible = false;
                        attachEncryptedFileButton.visible = false;
                    } else {
                        textarea4.height = 100;
                        fileRectangle.visible = true;
                        attachEncryptedPicButton.visible = true;
                        attachEncryptedFileButton.visible = true;
                    }
                }
            }
        }
    }

    Rectangle {
        id: fileRectangle
        color: "#000000"
        anchors.top: textRectangle.bottom
        width: parent.width
        height: textarea4.height

        Label {
            id: textarea4
            width: parent.width
            visible: true;
            height: 100
            color: "#ffffff"
            text: qsTr("Encrypt file")
            font.pixelSize: 18
            font.bold: false
        }

    }

    Button {
        id: attachEncryptedPicButton
        width: 80
        height: 40
        x: parent.width/2 - 90
        y: parent.height - 45

        iconSource: "image://theme/icon-m-toolbar-gallery"

        onClicked: {
            pictureSelectionPage.action_prop = "MAIL";
            pageStack.push(pictureSelectionPage);
        }

    }

    Button {
        id: attachEncryptedFileButton
        width: 80
        height: 40
        x: parent.width/2 + 10
        y: parent.height - 45

        iconSource: "image://theme/icon-m-toolbar-directory"

        onClicked: {
            fileSelectionPage.action_prop = "MAIL";
            pageStack.push(fileSelectionPage);
        }

    }

    SelectionDialog {
        id: dialogSelectPGPIdentity
        titleText: qsTr("Select identity:")
        selectedIndex: 0
        model: ListModel { }
        onAccepted: {
            setIdentity(dialogSelectPGPIdentity.model.get(dialogSelectPGPIdentity.selectedIndex).name);
        }

        onRejected: {
            // Content is already encrypted, so leave with valid value (first identity)
            setIdentity(dialogSelectPGPIdentity.model.get(0).name);
        }
    }

    state: (screen.currentOrientation === Screen.Portrait) ? "portrait" : "landscape"

    states: [
        State {
            name: "landscape"
            PropertyChanges { target: label2; height: {
                        return 45
                }
            }
        },
        State {
            name: "portrait"
            PropertyChanges { target: label2; height: {
                        return 65
                }
            }
        },
        State {
            name: "contentEncrypted"
            PropertyChanges { target: textarea3; height: { return 50; }}
            PropertyChanges { target: targetIDRect; visible: { return true; }}
            PropertyChanges { target: sendEMail; enabled: { return true; }}
            PropertyChanges { target: menuItemEncrypt; enabled: { return false; }}
            PropertyChanges { target: menuItemDecrypt; enabled: { return true; }}
            PropertyChanges { target: textarea1; readOnly: { return true; }}
        },
        State {
            name: "contentPlain"
            PropertyChanges { target: textarea3; height: { return 0; }}
            PropertyChanges { target: targetIDRect; visible: { return false; }}
            PropertyChanges { target: sendEMail; enabled: { return false; }}
            PropertyChanges { target: menuItemEncrypt; enabled: { return true; }}
            PropertyChanges { target: menuItemDecrypt; enabled: { return false; }}
            PropertyChanges { target: textarea1; readOnly: { return false; }}
        }
    ]
}
