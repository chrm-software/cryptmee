// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

Page {
    tools: commonTools
    id: keyPage

    property string currentKeyID: ""
    property alias importKeyFile: textFieldKeyImportFile.text
    property int buttonGap: 10;

    onStatusChanged: {
        if(status === DialogStatus.Open){
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
                // Close all menues before exit
                createKeyPairArea.visible = false;
                createKeyPairArea.height = 0;
                fadeInKeyGen.start();

                editKeyArea.visible = false;
                editKeyArea.height = 0;
                fadeInKeyEdit.start();

                importKeyArea.visible = false;
                importKeyArea.height = 0;
                fadeInKeyImport.start();

                buttonGap = 10;

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
        visualParent: keyPage
        MenuLayout {
            MenuItem { text: qsTr("Show GnuPG log")
                onClicked: {
                    pageStack.push(gpgHistoryPage);
                }
            }
        }
    }

    InfoBanner {
        id: infoBanner
        z: 5
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
        visible: false
        z: 1
    }

    function createIDText() {
        idText.text = nameT.text + " (" + commentT.text + ") <" + mailT.text + ">";
        generateKeyPair.enabled = true;
    }

    QueryDialog  {
        id: askKeyGenDialog
        titleText: qsTr("Generate Key Pair")
        message:  qsTr("Creating cryptographic key pairs takes some time. You can reduce the time by using your phone more intense. Start a game or use the camera!");
        acceptButtonText: qsTr("Generate!")
        rejectButtonText: qsTr("Cancel")
        onAccepted: {
            genKeyPair();
        }
    }

    QueryDialog  {
        id: askKeyDelDialog
        titleText: qsTr("Delete Key")
        message:  {
            var keyID = currentKeyID.split("|")[0];
            var keyType = currentKeyID.split("|")[1];

            if(keyType === "0") {
                return qsTr("Are you sure you want to delete this key: ") + keyID;
            } else {
                return qsTr("This is a secret key! Are you sure you want to delete this key: ") + keyID;
            }
        }
        acceptButtonText: qsTr("Delete!")
        rejectButtonText: qsTr("Cancel")
        onAccepted: {
            deleteKey();
        }
    }

    SelectionDialog {
        id: trustLevelDialog
        titleText: qsTr("Set user trust:")
        selectedIndex: 3

        model: ListModel {
            ListElement { value: "1"; name: "I don't know" }
            ListElement { value: "2"; name: "I do NOT trust" }
            ListElement { value: "3"; name: "I trust marginally" }
            ListElement { value: "4"; name: "I trust fully" }
            ListElement { value: "6"; name: "I trust ultimately" }
        }

        onAccepted: {
            var trust = trustLevelDialog.model.get(trustLevelDialog.selectedIndex).value;
            setOwnerTrust(trust);
        }
    }

    /////////////////////////////// GPG functions ////////////////////////
    function showOneKey() {
        busyIndicator.running = false;
        busyIndicator.visible = false
        editKeyArea.height = 300;

        currentKeyID = keyDialog.selectedKeyID;
        var keyData = startPage.gpgConnector.getKeyByID(currentKeyID.split("|")[0]);

        labelEditKeyInfo.text = keyData;

    }

    function importKeysFromFile() {
        startPage.currentState = "IMPORTKEYS";
        startPage.gpgConnector.importKeysFromFile(importKeyFile);
    }

    function importKeysFromClipboard() {
        startPage.currentState = "IMPORTKEYS";
        startPage.gpgConnector.importKeysFromClipboard();
    }

    function searchOnKeyServer(_keyword) {

        if(_keyword === "")
            return;

        busyIndicator.running = true;
        busyIndicator.visible = true;

        startPage.currentState = "KEYSERVER_SEARCH";
        startPage.gpgConnector.searchKeysOnKeyserver(_keyword);
    }

    function setOwnerTrust(_trust) {
        startPage.currentState = "TRUSTKEYS";
        currentKeyID = keyDialog.selectedKeyID;

        startPage.gpgConnector.setOwnerTrust(currentKeyID.split("|")[0], _trust);
    }

    function deleteKey() {
        startPage.currentState = "DELETEKEY";
        currentKeyID = keyDialog.selectedKeyID;
        startPage.gpgConnector.deleteKey(currentKeyID);
    }

    function genKeyPair() {
        busyIndicator.running = true;
        busyIndicator.visible = true;
        generateKeyPair.enabled = false;
        startPage.currentState = "GENKEYS";
        if(!startPage.gpgConnector.generateKeyPair(nameT.text, commentT.text, mailT.text, passphraseT.text))
            keyPairGenerated(false);
    }

    function importKeysFromServer(_keyListString) {
        startPage.gpgConnector.importKeysFromKeyserver(_keyListString);
    }

    /////////////////////////////// Results from GnuPG ////////////////////
    function keyPairGenerated(_result) {
        busyIndicator.running = false;
        busyIndicator.visible = false

        if(_result) {
            // Success
            infoBanner.text = qsTr("Keypair successfully created!");
            idText.text = "<b><font color='green'>" + qsTr("Successfull generated!") + "</font></b>";

        } else {
            // Fails
            infoBanner.text = qsTr("Error occures!");
            idText.text = "<b><font color='red'>" + qsTr("Error occures!") + "</font></b>";
        }

        if(createKeyPairArea.visible == true) {
            createKeyPairArea.visible = false;
            createKeyPairArea.height = 0;
            fadeInKeyGen.start();
        }

        infoBanner.show();
    }

    function keyserverSearched(_result) {
        busyIndicator.running = false;
        busyIndicator.visible = false

        if(_result) {
            // Success
            var numOfKeysFound = startPage.gpgConnector.getNumOfPubKeys(1);
            if(numOfKeysFound == 0)
                infoBanner.text = qsTr("No keys found. Try again!");
            else {
                infoBanner.text = qsTr("Found keys!") + " (" + numOfKeysFound + ")";
                fillSearchKeysModel();
                pageStack.push(searchKeyDialog);
            }
        } else {
            // Fails
            infoBanner.text = qsTr("Error occures!") + "\n" + startPage.dataErrOutput;
        }

        infoBanner.show();
    }

    function keyImported(_result) {
        busyIndicator.running = false;
        busyIndicator.visible = false

        if(_result) {
            // Success
            infoBanner.text = qsTr("Keys imported.");
        } else {
            // Fails
            infoBanner.text = qsTr("Error occures!") + "\n" + startPage.dataErrOutput;
        }

        infoBanner.show();
    }

    function keyEdited(_result) {
        busyIndicator.running = false;
        busyIndicator.visible = false

        if(_result) {
            // Success
            infoBanner.text = qsTr("Key successfull edited.");
        } else {
            // Fails
            infoBanner.text = qsTr("Unable to edit this key!");
        }

        infoBanner.show();

        // Close edit area
        editKeyArea.visible = false;
        editKeyArea.height = 0;
        fadeInKeyEdit.start();
        labelEditKeyInfo.text = "";
        buttonGap = 10;
    }

    function keyDeleted(_result) {
        if(_result) {
            // Success
            infoBanner.text = qsTr("Key successfull deleted.");
        } else {
            // Fails
            infoBanner.text = qsTr("Unable to delete this key!");
        }

        infoBanner.show();

        // Close edit area
        editKeyArea.visible = false;
        editKeyArea.height = 0;
        fadeInKeyEdit.start();
        labelEditKeyInfo.text = "";
        buttonGap = 10;
    }

    // Create model for key list view
    function fillSearchKeysModel() {
        var tmpEntryID = "";
        var tmpEntryNames = "";

        searchKeyDialog.allKeyList.clear();

        for(var i=0; i < startPage.gpgConnector.getNumOfPubKeys(1); i++) {
            tmpEntryID = startPage.gpgConnector.getKey(i, 1).split("|")[0];
            tmpEntryNames = startPage.gpgConnector.getKey(i, 1).split("|")[1];

            console.debug("Key from search: " + tmpEntryID);
            searchKeyDialog.allKeyList.append({ name: tmpEntryID, ids: tmpEntryNames, selected_param: false, isVisible: true });
        }
    }

    ///////////////////////////////////////

    ParallelAnimation {
        id: fadeInKeyGen
        running: false
        NumberAnimation { target: editKey; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: importKey; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: exportKey; property: "opacity"; to: 1.0; duration: 800}
    }

    ParallelAnimation {
        id: fadeOutKeyGen
        running: false
        NumberAnimation { target: editKey; property: "opacity"; to: 0.0; duration: 500}
        NumberAnimation { target: importKey; property: "opacity"; to: 0.0; duration: 500}
        NumberAnimation { target: exportKey; property: "opacity"; to: 0.0; duration: 500}
    }

    ParallelAnimation {
        id: fadeInKeyEdit
        running: false
        NumberAnimation { target: createKeyPair; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: importKey; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: exportKey; property: "opacity"; to: 1.0; duration: 800}

        onStarted: {
            createKeyPair.visible = true;
            createKeyPair.height = 65;
            importKey.visible = true;
            importKey.height = 65;
            exportKey.visible = true;
            exportKey.height = 65;
        }
    }

    ParallelAnimation {
        id: fadeOutKeyEdit
        running: false
        NumberAnimation { target: createKeyPair; property: "opacity"; to: 0.0; duration: 500}
        NumberAnimation { target: importKey; property: "opacity"; to: 0.0; duration: 500}
        NumberAnimation { target: exportKey; property: "opacity"; to: 0.0; duration: 500}

        onCompleted: {
            createKeyPair.visible = false;
            createKeyPair.height = 0;
            importKey.visible = false;
            importKey.height = 0;
            exportKey.visible = false;
            exportKey.height = 0;
        }
    }

    ParallelAnimation {
        id: fadeInKeyImport
        running: false
        NumberAnimation { target: createKeyPair; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: editKey; property: "opacity"; to: 1.0; duration: 800}
        NumberAnimation { target: exportKey; property: "opacity"; to: 1.0; duration: 800}

        onStarted: {
            createKeyPair.visible = true;
            createKeyPair.height = 65;
            editKey.visible = true;
            editKey.height = 65;
            exportKey.visible = true;
            exportKey.height = 65;
        }
    }

    ParallelAnimation {
        id: fadeOutKeyImport
        running: false
        NumberAnimation { target: createKeyPair; property: "opacity"; to: 0.0; duration: 500}
        NumberAnimation { target: editKey; property: "opacity"; to: 0.0; duration: 500}
        NumberAnimation { target: exportKey; property: "opacity"; to: 0.0; duration: 500}

        onCompleted: {
            createKeyPair.visible = false;
            createKeyPair.height = 0;
            editKey.visible = false;
            editKey.height = 0;
            exportKey.visible = false;
            exportKey.height = 0;

            buttonGap = 0;
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
            text: "CryptMee <font size='-5'>" + qsTr("Key Management") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    ScrollDecorator {
        flickableItem: flickArea
    }

    Flickable {
        id: flickArea
        flickableDirection: Flickable.VerticalFlick
        clip: true
        width: parent.width
        height: parent.height - topDecoartion.height
        y: topDecoartion.height + 10
        contentHeight: 700

        Button {
            id: createKeyPair
            height: 70
            iconSource: "qrc:/images/pix/private_keys.png"
            text: qsTr("Generate Key Pair")
            x: 5
            width: parent.width-10

            onClicked: {
                if(createKeyPairArea.visible == false) {
                    createKeyPairArea.visible = true;
                    createKeyPairArea.height = nameT.height*5 + idText.height + generateKeyPair.height + 14;
                    fadeOutKeyGen.start();

                } else {
                    createKeyPairArea.visible = false;
                    createKeyPairArea.height = 0;
                    fadeInKeyGen.start();
                }
            }
        }

        Rectangle {
            id: createKeyPairArea
            height: 0
            width: parent.width
            visible: false
            color: "transparent"
            y: createKeyPair.y + createKeyPair.height

            Grid {
                columns: 1
                rows: 7
                spacing: 2
                anchors.fill: parent

                TextField {
                    id: nameT;
                    width: parent.width
                    text: ""
                    placeholderText: qsTr("Enter Your Name")
                    inputMethodHints: Qt.ImhNoPredictiveText;
                    onTextChanged: {
                        createIDText()
                    }
                }
                TextField {
                    id: mailT;
                    width: parent.width
                    text: ""
                    placeholderText: qsTr("Enter Your eMail Address")
                    inputMethodHints: Qt.ImhNoPredictiveText;
                    onTextChanged: {
                        createIDText()
                    }
                }
                TextField {
                    id: commentT;
                    width: parent.width
                    text: ""
                    placeholderText: qsTr("Enter Comment")
                    inputMethodHints: Qt.ImhNoPredictiveText;
                    onTextChanged: {
                        createIDText()
                    }
                }
                TextField {
                    id: passphraseT;
                    width: parent.width
                    text: ""
                    echoMode: TextInput.Password
                    placeholderText: qsTr("Enter Passphrase")
                    inputMethodHints: Qt.ImhNoPredictiveText;
                }
                TextField {
                    id: passphraseT2;
                    width: parent.width
                    text: ""
                    echoMode: TextInput.Password
                    placeholderText: qsTr("Repeat Passphrase")
                    inputMethodHints: Qt.ImhNoPredictiveText;
                }

                Text {
                    id: idText
                    height: 35
                    width: parent.width
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 15
                }

                Button {
                    id: generateKeyPair
                    width: parent.width
                    text: qsTr("Generate!")
                    iconSource: "image://theme/icon-m-toolbar-add"
                    enabled: false

                    onClicked: {
                        if(passphraseT.text == "" || passphraseT.text !== passphraseT2.text) {
                            infoBanner.text = qsTr("Passphrases empty or not equal!");
                            infoBanner.show();
                        } else if(nameT.text == "" || mailT.text == "") {
                            infoBanner.text = qsTr("Enter name and email!");
                            infoBanner.show();
                        } else if(mailT.text.indexOf("@") === -1) {
                            infoBanner.text = qsTr("Incorrect email address!");
                            infoBanner.show();
                        } else {
                            // Show info about key generation
                            askKeyGenDialog.open();
                        }
                    }
                }
            }
        }

        Button {
            id: editKey
            height: 70
            y: createKeyPairArea.y + createKeyPairArea.height + buttonGap
            iconSource: "qrc:/images/pix/public_keys_edit.png"
            text: qsTr("Edit Keys")
            x: 5
            width: parent.width-10

            onClicked: {
                if(!editKeyArea.visible) {
                    editKeyArea.visible = true;
                    editKeyArea.height = 300;
                    fadeOutKeyEdit.start();
                    buttonGap = 0;

                    keyDialog.stateAfterExit = "SHOWKEY";
                    pageStack.push(keyDialog);
                } else {
                    editKeyArea.visible = false;
                    editKeyArea.height = 0;
                    fadeInKeyEdit.start();

                    labelEditKeyInfo.text = "";
                    buttonGap = 10;
                }
            }
        }

        Rectangle {
            id: editKeyArea
            height: 0
            width: parent.width
            visible: false
            color: "white"
            border.color: "black"
            border.width: 2
            anchors.margins: 3

            y: editKey.y + editKey.height

            Text {
                id: labelEditKeyInfo
                width: parent.width
                height: parent.height
                text: ""
                font.pixelSize: 21
                anchors.margins: 4
            }

            Button {
                id: trustKeyButton
                anchors.margins: 3
                width: parent.width
                anchors.top: labelEditKeyInfo.bottom
                iconSource: "image://theme/icon-m-toolbar-frequent-used"
                text: qsTr("Trust Key")

                onClicked: {
                    trustLevelDialog.open();
                }
            }

            Button {
                id: signKeyButton
                anchors.margins: 3
                width: parent.width
                anchors.top: trustKeyButton.bottom
                iconSource: "image://theme/icon-m-toolbar-edit"
                text: qsTr("Sign Key")

                onClicked: {
                }
            }

            Button {
                id: deleteKeyButton
                anchors.margins: 3
                width: parent.width
                anchors.top: signKeyButton.bottom
                iconSource: "image://theme/icon-m-toolbar-delete"
                text: qsTr("Remove Key")

                onClicked: {
                    askKeyDelDialog.open();
                }
            }
        }

        Button {
            id: importKey
            height: 70
            y: editKeyArea.y + editKeyArea.height + buttonGap
            iconSource: "qrc:/images/pix/public_keys_import.png"
            text: qsTr("Import Key")
            x: 5
            width: parent.width-10

            onClicked: {
                if(!importKeyArea.visible) {
                    importKeyArea.visible = true;
                    importKeyArea.height = 300;
                    fadeOutKeyImport.start();
                } else {
                    importKeyArea.visible = false;
                    importKeyArea.height = 0;
                    fadeInKeyImport.start();
                    buttonGap = 10;
                }
            }
        }

        Rectangle {
            id: importKeyArea
            height: 0
            width: parent.width
            visible: false
            color: "transparent"
            y: importKey.y + importKey.height

            Grid {
                columns: 1
                rows: 8
                spacing: 2
                anchors.fill: parent

                Label {
                    text: qsTr("Import from clipboard:")
                    width: parent.width
                }

                Button {
                    id: importKeyCliboardButton
                    anchors.margins: 3
                    width: parent.width
                    iconSource: "qrc:/images/pix/clipboard_from.png"
                    text: qsTr("Import")

                    onClicked: {
                        importKeysFromClipboard();
                    }
                }

                Label {
                    text: qsTr("Import from file:") + " /home/user/MyDocs/"
                    width: parent.width
                }

                TextField {
                    id: textFieldKeyImportFile;
                    width: parent.width
                    text: "keyring.asc"
                    placeholderText: qsTr("File name")
                    inputMethodHints: Qt.ImhNoPredictiveText;
                    onTextChanged: {

                    }
                }

                Button {
                    id: importKeyFileButton
                    anchors.margins: 3
                    width: parent.width
                    iconSource: "image://theme/icon-m-toolbar-directory-move-to"
                    text: qsTr("Import")

                    onClicked: {
                        importKeysFromFile();
                    }
                }

                Label {
                    text: qsTr("Import from key server:") + " " +  startPage.gpgConnector.settingsGetValue("SETTINGS_GPGKEYSERVER");
                    width: parent.width
                }

                TextField {
                    id: textFieldKeyImportServer;
                    width: parent.width
                    text: ""
                    placeholderText: qsTr("Search keyword...")
                    inputMethodHints: Qt.ImhNoPredictiveText;

                    onTextChanged: {
                        if(textFieldKeyImportServer.text.length < 4)
                            importKeyServerButton.enabled = false;
                        else
                            importKeyServerButton.enabled = true;
                    }
                }

                Button {
                    id: importKeyServerButton
                    anchors.margins: 3
                    width: parent.width
                    iconSource: "image://theme/icon-m-toolbar-search"
                    text: qsTr("Search")
                    enabled: false

                    onClicked: {
                        searchOnKeyServer(textFieldKeyImportServer.text);
                    }
                }
            }

        }

        Button {
            id: exportKey
            height: 70
            y: importKeyArea.y + importKeyArea.height + buttonGap
            iconSource: "qrc:/images/pix/public_keys_export.png"
            text: qsTr("Export Key")
            x: 5
            width: parent.width-10

            onClicked: {
                if(!exportKeyArea.visible) {
                    exportKeyArea.visible = true;
                    exportKeyArea.height = 300;
                    //fadeOutKeyImport.start();
                    createKeyPair.visible = false;
                    importKey.visible = false;
                    editKey.visible = false;
                    buttonGap = 0;
                } else {
                    exportKeyArea.visible = false;
                    exportKeyArea.height = 0;
                    //fadeInKeyImport.start();
                    createKeyPair.visible = true;
                    importKey.visible = true;
                    editKey.visible = true;
                    buttonGap = 10;
                }
            }
        }

        Rectangle {
            id: exportKeyArea
            height: 0
            width: parent.width
            visible: false
            color: "transparent"
            y: exportKey.y + exportKey.height
        }
    }
}
