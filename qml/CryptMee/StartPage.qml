// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1
import GnuPGConnector 1.0


Page {
    tools: commonTools
    id: startPage

    // Some properties ///////////////////////////////////
    property string currentState: "IDLE"
    property alias gpgConnector: myGPGConnector
    property string currentGpgVersion: ""

    property string dataOutput: ""
    property string dataErrOutput: ""

    property alias otrIcon: iconOTRState.iconSource
    //////////////////////////////////////////////////////


    function showAboutDialog() {
        aboutDialog.open();
    }

    function hasNewMsgs(yes) {
        if(yes) {
            newMsgIndicator.visible = true;
        } else
            newMsgIndicator.visible = false;
    }

    ToolBarLayout {
        id: commonTools
        visible: true

        ToolIcon {
            id: iconOTRState
            iconSource: "qrc:/images/pix/otr_inactive_toolbar.png"
            anchors.left: (parent === undefined) ? undefined : parent.left
            onClicked: {
                pageStack.push(otrConfigPage);
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
        visualParent: startPage
        MenuLayout {

            MenuItem { text: qsTr("Settings")
                onClicked: {
                    pageStack.push(settingsPage);
                }
            }

            MenuItem { text: qsTr("OTR Settings")
                onClicked: {
                    pageStack.push(otrConfigPage);
                }
            }

            MenuItem { text: qsTr("About")
                onClicked: {
                    showAboutDialog();
                }
            }
        }
    }

    Dialog {
        id: aboutDialog
        opacity: 0.8
        anchors.fill: parent

        title: Rectangle {
            id: titleField
            height: logoImage.height

            Image {
                id: logoImage
                anchors.left: parent.left
                source: "qrc:/images/pix/CryptMee.png"
            }

            Text {
                id: aboutText
                anchors.left: logoImage.right
                anchors.margins: 15
                font.pixelSize: 32
                height: logoImage.height
                color: "white"
                text: "CryptMee <i>" + Release_Version + "</i>"
                verticalAlignment: Text.AlignVCenter
            }
        }

        content: Item {
            id: aboutItem
            height: 350
            width: parent.width

            Label {
                id: aboutInfos
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 10
                font.pixelSize: 20
                height: 65
                width: parent.width
                color: "white"
                text: qsTr("Enrypt/decrypt text and email or manage your OpenPGP keys!")
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Label {
                id: aboutCopy
                anchors.top: aboutInfos.bottom
                anchors.left: parent.left
                anchors.margins: 10
                font.pixelSize: 20
                height: 65
                width: parent.width
                color: "white"
                text: qsTr("Copyright &copy; 2015 by Christoph Maciejewski (chrm@users.sf.net) <br><br> \
 CryptMee is free software licensed under the terms of the GNU General Public License as published by \
the Free Software Foundation; you can redistribute it and/or modify it under the terms of the GNU General \
Public License either version 3 of the license, or (at your option) any later version (GPLv3+)") + "<br><br>" +
                      qsTr("Using GnuPG: ") + currentGpgVersion + "<br><br>" + otrConfigPage.imControlThread.getLibOTRVersion();
                horizontalAlignment: Text.AlignLeft
            }
        }
    }

    InfoBanner {
        id: infoBanner
        z: 5
    }

    onStatusChanged: {
        if(status === DialogStatus.Open){
            fadeIn.start();

            // Check gpg version
            currentGpgVersion = myGPGConnector.getGPGVersionString();
            if(currentGpgVersion === "-1") {
                // No gpg found. Show Error
                infoBanner.text = "Error: no GnuPG found. Check settings!"
                infoBanner.show();
            }
        }
    }

    state: (screen.currentOrientation === Screen.Portrait) ? "portrait" : "landscape"

    states: [
        State {
            name: "landscape"
            PropertyChanges { target: buttonsGrid; columns: 2; rows: 2 }
            PropertyChanges { target: encDecTextButton; text: "<html>"+qsTr("Encrypt Or Decrypt Text")+"</html>"; width: parent.width/2-2; height: 150; /*x: 100*/ }
            PropertyChanges { target: decMailsButton; text: "<html>"+qsTr("Read Encrypted Emails")+"</html>"; width: parent.width/2-2; height: 150; /*x: 100*/ }
            PropertyChanges { target: keyManagementButton; text: "<html>"+qsTr("PGP Key Management")+"</html>"; width: parent.width/2-2; height: 150; /*x: 100*/ }
            PropertyChanges { target: otrChatButton; text: "<html>"+qsTr("Secure Conversations")+"</html>"; width: parent.width/2-2; height: 150; /*x: 100*/ }
        },
        State {
            name: "portrait"
            PropertyChanges { target: buttonsGrid; columns: 1; rows: 4 }
            PropertyChanges { target: encDecTextButton; text: "<html>"+qsTr("Encrypt Text")+"</html>"; width: parent.width; height: 155; /*x: 5*/ }
            PropertyChanges { target: decMailsButton; text: "<html>"+qsTr("Read Mails")+"</html>"; width: parent.width; height: 155; /*x: 5*/ }
            PropertyChanges { target: keyManagementButton; text: "<html>"+qsTr("Key Management")+"</html>"; width: parent.width; height: 155; /*x: 5*/ }
            PropertyChanges { target: otrChatButton; text: "<html>"+qsTr("Secure Conversations")+"</html>"; width: parent.width; height: 155; /*x: 5*/ }
        }
    ]

    ParallelAnimation {
        id: fadeIn
        running: false
        NumberAnimation { target: encDecTextButton; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: decMailsButton; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: keyManagementButton; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: otrChatButton; property: "opacity"; to: 1.0; duration: 800}
    }

    ParallelAnimation {
        id: fadeOut1
        running: false
        NumberAnimation { target: decMailsButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: keyManagementButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: otrChatButton; property: "opacity"; to: 0.0; duration: 300}
        onCompleted: pageStack.push(mainPage)
    }

    ParallelAnimation {
        id: fadeOut2
        running: false
        NumberAnimation { target: encDecTextButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: keyManagementButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: otrChatButton; property: "opacity"; to: 0.0; duration: 300}
        onCompleted: pageStack.push(mailPage)
    }

    ParallelAnimation {
        id: fadeOut3
        running: false
        NumberAnimation { target: encDecTextButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: decMailsButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: otrChatButton; property: "opacity"; to: 0.0; duration: 300}
        onCompleted: pageStack.push(keyPage)
    }

    ParallelAnimation {
        id: fadeOut4
        running: false
        NumberAnimation { target: encDecTextButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: decMailsButton; property: "opacity"; to: 0.0; duration: 300}
        NumberAnimation { target: keyManagementButton; property: "opacity"; to: 0.0; duration: 300}        
        onCompleted: {
            if(otrConfigPage.isInitialized === 1)
                pageStack.push(chatContacts)
            else
                pageStack.push(otrConfigPage)
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
            text: "CryptMee <font size='-5'>"+qsTr("Privacy Software")+"</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    Grid {
        id: buttonsGrid
        columns: 1
        rows: 4
        y: topDecoartion.height + 10
        x: 5
        width: parent.width - 10
        spacing: 5

        Button {
            id: encDecTextButton
            height: 90
            width: parent.width/2
            iconSource: "qrc:/images/pix/gpg-text.png"
            opacity: 0.0

            onPressedChanged: fadeOut1.start()

        }

        Button {
            id: decMailsButton
            height: 90
            width: parent.width/2
            iconSource: "qrc:/images/pix/gpg-mail.png"
            opacity: 0.0

            onPressedChanged: fadeOut2.start()
        }

        Button {
            id: keyManagementButton
            height: 90
            width: parent.width/2
            iconSource: "qrc:/images/pix/gpg-keys.png"
            opacity: 0.0

            onPressedChanged: fadeOut3.start()
        }

        Button {
            id: otrChatButton
            height: 90
            width: parent.width/2
            iconSource: "qrc:/images/pix/gpg-chat.png"
            opacity: 0.0

            onPressedChanged: fadeOut4.start()

            Image {
                id: newMsgIndicator
                visible: false
                x: parent.width - 48
                width: 48
                height: 48
                source: "image://theme/icon-m-common-blue"

                Text {
                    width: parent.width
                    height: parent.height
                    x: 18
                    y: 10
                    color: "#ffffff"
                    text: "1"
                    font.bold: true
                    font.pointSize: 14
                    styleColor: "#000000"
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////// GPG Functions /////////////////////////////////////////////

    // States: IDLE, TXT_ENCRYPT, TXT_DECRYPT, MAIL_DECRYPT, GENKEYS, SHOWKEYS

    // Init Connector
    GnuPGConnector {
        id: myGPGConnector

        onReady: {
            console.debug("---> READY Signal from C++ (gpgconnector)! State: " + currentState);

            dataErrOutput = myGPGConnector.getData(true);
            dataOutput = myGPGConnector.getData(false);

            if(dataErrOutput !== "No Errors") {
                // Error occured reset PGP passphrase
                // TODO: check for right error.
                passwordDialog.prop_passwd = "";
                mainPage.errorField = dataErrOutput;
            }

            if(currentState === "GETKEYS") {
                // Read secret keys now
                updateSecretKeys();

            } else if(currentState === "GET_SECRETKEYS") {
                fillPublicKeysModel();

            } else if(currentState === "TXT_ENCRYPT") {
                // coming from key selection dialog - show enrypted text
                mainPage.textField = dataOutput;

            } else if(currentState === "TXT_DECRYPT") {
                // Show decrypted text
                mainPage.textField = dataOutput;

            } else if(currentState === "MAIL_DECRYPT") {
                // Show decrypted eMail
                mailViewPage.mailDecrypted(dataOutput, true);

            } else if(currentState === "GENKEYS") {
                // Keypair successfull generated
                keyPage.keyPairGenerated(true);

            } else if(currentState === "SHOWKEY") {
                // Load one key
                keyPage.showOneKey();

            } else if(currentState === "TRUSTKEYS") {
                // Set ownertrust OK
                keyPage.keyEdited(true);

            } else if(currentState === "GETVERSION") {
                // Got gpg version
                settingsPage.gpgRetValVersion(true, dataOutput);
                currentGpgVersion = myGPGConnector.getGPGVersionString();

            } else if(currentState === "IMPORTKEYS") {
                // Keys imported
                keyPage.keyImported(true);

            } else if(currentState === "DELETEKEY") {
                // Keys deleted
                keyPage.keyDeleted(true);

            } else if(currentState === "SIGNKEY") {
                // Keys signed
                keyPage.keySigned(true);

            } else if(currentState === "EXPORTKEYS") {
                // Keys export
                keyPage.keyExported(true);

            } else if(currentState === "KEYSERVER_SEARCH") {
                // Keys imported
                keyPage.keyserverSearched(true);

            } else {
                currentGpgVersion = myGPGConnector.getGPGVersionString();
            }
        }

        onErrorOccured: {
            console.debug("---> ERROR Signal from C++ (gpgconnector)! State:" + currentState);

            dataErrOutput = myGPGConnector.getData(true);
            dataOutput = myGPGConnector.getData(false);

            if(dataErrOutput !== "No Errors") {
                // Error occured reset PGP passphrase
                passwordDialog.prop_passwd = "";
            }

            if(currentState === "TXT_ENCRYPT" || currentState === "TXT_DECRYPT") {
                mainPage.errorField = dataErrOutput;

            } else if(currentState === "MAIL_DECRYPT") {
                mailViewPage.setErrorMessage("ERROR: " + dataErrOutput);
                console.debug("[MainPage] onErrorOccured: send error to mailView." + dataErrOutput);

                // Since there are a lot of possible warnings, just try
                // to decrypt message anyway
                console.debug("[MainPage] onErrorOccured: send output to mailView anyway.");
                mailViewPage.mailDecrypted(dataOutput, false);

            } else if(currentState === "GETKEYS") {
                // TODO

            } else if(currentState === "GENKEYS") {
                // Error while generating keys
                keyPage.keyPairGenerated(false);

            } else if(currentState === "SHOWKEY") {
                // Trigger showing key - keyID is saved in the
                // selection dialog
                keyPage.showOneKey();

            } else if(currentState === "TRUSTKEYS") {
                // Set ownertrust failed
                keyPage.keyEdited(false);

            } else if(currentState === "GETVERSION") {
                // Got gpg version
                settingsPage.gpgRetValVersion(false, dataErrOutput);
                currentGpgVersion = myGPGConnector.getGPGVersionString();

            } else if(currentState === "IMPORTKEYS") {
                // Keys imported
                keyPage.keyImported(false);

            } else if(currentState === "DELETEKEY") {
                // Keys deleted
                keyPage.keyDeleted(false);

            } else if(currentState === "SIGNKEY") {
                // Keys signed
                keyPage.keySigned(false);

            } else if(currentState === "EXPORTKEYS") {
                // Keys export
                keyPage.keyExported(false);

            } else if(currentState === "KEYSERVER_SEARCH") {
                // Keys imported
                keyPage.keyserverSearched(false);
            }
        }
    }

    // Start encryption process
    function pgpEncrypt(_pubKey, _state) {
        console.debug("pubKey:" + _pubKey);

        // Clear error output
        mainPage.errorField = "";

        console.debug("[MainPage]pgpEncrypt(): text to encrypt: " + mainPage.textField);

        if(_pubKey === undefined) {
            updatePublicKeys();
            keyDialog.stateAfterExit = "TXT_ENCRYPT";
            pageStack.push(keyDialog);

        } else {
            currentState = _state;
            myGPGConnector.encrypt(mainPage.textField, _pubKey);
        }
    }

    // Start decryption process
    function pgpDecrypt(_content, _state) {
        // Clear error output
        mainPage.errorField = "";

        if(_content === undefined)
            _content = mainPage.textField;
        if(_state === undefined)
            _state = "TXT_DECRYPT";

        if(passwordDialog.prop_passwd === "") {
            passwordDialog.prop_content = _content;
            passwordDialog.prop_state = _state;
            pageStack.push(passwordDialog);

        } else {
            // GPGConnector starts here asynchronus. Wait for ready signal
            currentState = _state;
            mainPage.textField = myGPGConnector.decrypt(_content, passwordDialog.prop_passwd);
        }
    }

    // Load keys
    function updatePublicKeys() {
        //keyDialog.allKeyList.clear();
        currentState = "GETKEYS";
        myGPGConnector.showKeys();
    }

    // Load secret keys
    function updateSecretKeys() {
        keyDialog.allKeyList.clear();
        currentState = "GET_SECRETKEYS";
        myGPGConnector.showSecretKeys();
    }

    // Get Version
    function getPGPVersion(_path) {
        currentState = "GETVERSION";
        myGPGConnector.checkGPGVersion(_path);
    }

    // Create model for key list view
    function fillPublicKeysModel() {
        var tmpEntryID = "";
        var tmpEntryNames = "";
        var tmpEntryTrust = "";
        var tmpEntryIsSecret = "";

        var filterString = keyDialog.filterKeys;
        keyDialog.allKeyList.clear();

        console.debug("fillPublicKeysModel(): filter keys: " + filterString);

        for(var i=0; i<myGPGConnector.getNumOfPubKeys(); i++) {
            tmpEntryTrust = myGPGConnector.getKey(i).split("|")[0];
            tmpEntryID = myGPGConnector.getKey(i).split("|")[1];
            tmpEntryNames = myGPGConnector.getKey(i).split("|")[2];
            tmpEntryIsSecret = myGPGConnector.getKey(i).split("|")[3];

            console.debug("Key: " + tmpEntryID + ", isSecret: " + tmpEntryIsSecret);

            if(filterString === undefined || (tmpEntryID.indexOf(filterString) > -1 || tmpEntryNames.indexOf(filterString) > -1))
                keyDialog.allKeyList.append({ name: tmpEntryID, ids: tmpEntryNames, trust: tmpEntryTrust, isVisible: true, isSecret: tmpEntryIsSecret });
            else
                keyDialog.allKeyList.append({ name: tmpEntryID, ids: tmpEntryNames, trust: tmpEntryTrust, isVisible: false, isSecret: tmpEntryIsSecret });
        }
    }
}
