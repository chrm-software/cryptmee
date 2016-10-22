import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1
import ImControlThread 1.0
import QtMobility.feedback 1.1
import PictureUploader 1.0

Page {
    id: otrChatWindow
    tools: commonTools

    /////////////////////////////////
    property alias contactName: contactNameLabel.text
    property alias contactFingerprint: fingerprintTextOutput.text
    property alias contactAvatar: avatarImage.source
    property bool chatSendEncrypted: true
    property string currentChatEntry: ""
    property int animDurationIncoming: 250
    property bool reloadChatContent: true
    /////////////////////////////////

    //////////////////////////////////////// Functions ////////////////////////////////
    function sendMessage(message) {
        if(message === "")
            return;

        var contactJID = contactName;

        // TODO: change JID if resource is in use!
        // Otherwise the message will not be visible - (user@server/res != user@server).
        if(resource.text != "")
            contactJID = contactJID + "/" + resource.text;

        console.debug("QML: OTR chat: Sending message to " + contactJID + ", from: " + otrConfigPage.selectedAccountName);

        if(chatSendEncrypted) {
            if(otrConfigPage.imControlThread.sendOTRMessage(otrConfigPage.selectedAccountName, contactJID, message)) {
                typeMsgField.text = "";
                otrConfigPage.imControlThread.addChatMessage(contactJID, message, false, false, chatSendEncrypted);
            } else {
                addMessage("*** " + qsTr("Error sending message: ") + message, false, Qt.formatTime(new Date(),"hh:mm"), true, false);
            }
        } else {
            console.debug("QML: OTR chat: Sending message UNENCRYPTED!");
            if(otrConfigPage.imControlThread.sendPlainTextMessage(otrConfigPage.selectedAccountName, contactJID, message)) {
                typeMsgField.text = "";
                otrConfigPage.imControlThread.addChatMessage(contactJID, message, false, false, chatSendEncrypted);
            }
        }
    }

    function addMessage(text, remote, date, system, encrypted) {
        chatListView.model.insert(0, { chatEntry: text, remoteText: remote, date: date, systemInfo: system, isEncrypted: encrypted });
    }

    function showPage(page, reloadContacts) {
        if(page === "CHAT_PAGE") {
            chatRect.stateVisible = true;
            propsRect.stateVisible = false;
            buttonRow.checkedButton = b1;

            if(reloadContacts)
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
        // Load last pending message only
        getMessageFor(contactName);

        // Load all other messages later, so GUI will switch fast to this chat view
        loadAllMessagesLaterTimer.start();
    }

    function getAllMessagesLater() {
        chatListView.model.clear();

        animDurationIncoming = 0;
        var size = otrConfigPage.imControlThread.getChatHistorySizeFor(contactName);

        for(var i=0; i<size; i++) {
            var values = otrConfigPage.imControlThread.getChatHistoryMessageFor(contactName, i).split("|");
            if(values[0] !== "")
                addMessage(values[0], getBoolForInt(values[1]), values[2], getBoolForInt(values[3]), getBoolForInt(values[4]));
        }

        animDurationIncoming = 250;
    }

    function getMessageFor(_contactName) {
        // Do not draw message for other users
        if(_contactName !== contactName)
            return;

        // Get latest unread message
        var values = otrConfigPage.imControlThread.getNewestChatMessageFor(_contactName).split("|");

        if(values[0] !== "")
            addMessage(values[0], getBoolForInt(values[1]), values[2], getBoolForInt(values[3]), getBoolForInt(values[4]));

        checkIfChatVerified();
    }

    function getContactNameHeader() {
        return contactName;
    }

    function chatBackground(_color) {
        chatRect.color = _color;
    }

    function verifyFingerprint() {
        var contactJID = contactName;

        if(resource.text != "")
            contactJID = contactJID + "/" + resource.text;

        otrConfigPage.imControlThread.verifyFingerprint(otrConfigPage.selectedAccountName, contactJID, contactFingerprint, true);
        checkIfChatVerified();
    }

    function deleteFingerprint() {
        var contactJID = contactName;

        if(resource.text != "")
            contactJID = contactJID + "/" + resource.text;

        otrConfigPage.imControlThread.deleteFingerprint(otrConfigPage.selectedAccountName, contactJID, contactFingerprint);
        checkIfChatVerified();
    }

    function checkIfChatVerified() {
        var isChatSecure = otrConfigPage.imControlThread.isFingerprintVerified(otrConfigPage.selectedAccountName, contactName);
        console.debug("QML: OTRChatWindow: isChatSecure: " + isChatSecure);

        if(contactFingerprint === "-") {
            // Set back to plain text as long as we don't know if key exchange worked.
            changeTextSendingMode(false);
            return;
        }

        // Do not change to encrypted if user sets plaintext
        if(chatSendEncrypted === false)
            return;

        if(!isChatSecure) {
            logoImage.source = "image://theme/icon-m-common-locked";
            changeTextSendingMode(true);
        } else {
            logoImage.source = "qrc:/images/pix/icon-m-common-locked-verified.png";
            changeTextSendingMode(true);
        }
    }

    function changeTextSendingMode(mode) {
        if(chatSendEncrypted === mode) {
            // Nothing to do
        } else {
            if(mode === false) {
                // Disable encryption for outgoing messages
                chatSendEncrypted = false;
                logoImage.source = "qrc:/images/pix/icon-m-common-locked-plaintext.png";
            } else {
                chatSendEncrypted = true;

                if(contactFingerprint === "-") {
                    // We have no key, start keyexchange first
                    var contactJID = contactName;
                    if(resource.text != "")
                        contactJID = contactJID + "/" + resource.text;
                    otrConfigPage.imControlThread.sendPlainTextMessage(otrConfigPage.selectedAccountName, contactJID, "?OTR?v");
                }

                checkIfChatVerified();
            }
        }
    }

    function uploadImage(imagePath) {
        addMessage(qsTr("Uploading file..."), false, Qt.formatTime(new Date(),"hh:mm"), true, false);
        pictureUploader.uploadFile(imagePath);
    }

    ///////////////////////////////////////////////////////////////////////////////////

    onStatusChanged: {
        if(status === DialogStatus.Open){
            showPage("CHAT_PAGE", reloadChatContent);
            checkIfChatVerified();
            reloadChatContent = true;

            if(emojiSelectionGrid.model.count === 0) {
                for(var i=0; i<otrConfigPage.imControlThread.getNumOfEmojis(); i++) {
                    var emojiData = otrConfigPage.imControlThread.getEmojiPath(i);
                    emojiSelectionGrid.model.append({ emojiPath: emojiData.split("|")[0], emojiCode: emojiData.split("|")[1] });
                }
            }
        }
    }

    PictureUploader {
        id: pictureUploader
        onUploadCompleted: {
            // Send download URL to your contact
            rumbleEffect.running = true;
            sendMessage(_downloadURL);
            chatListView.positionViewAtBeginning();
            uploadProgress.visible = false;
        }

        onErrorOccured: {
            uploadProgress.visible = false;
            addMessage("*** " + qsTr("Error uploading file: ") + _message, false, Qt.formatTime(new Date(),"hh:mm"), true, false);
        }

        onUploadStep: {
            uploadProgress.visible = true;

            if(_step > uploadProgress.value) {
                if(uploadProgress.maximumValue !== _total*1.2) uploadProgress.maximumValue = _total*1.2;
                uploadProgress.value = _step;
                console.debug("progress: " + _step + " / " + _total);
            }
        }
    }

    HapticsEffect {
        id: rumbleEffect
        attackIntensity: 1.0
        attackTime: 20
        intensity: 1.0
        duration: 30
        fadeTime: 20
        fadeIntensity: 1.0
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
                contactName = "";
                chatListView.model.clear();
                pageStack.pop();
            }
        }

        ButtonRow {
            id: buttonRow
            checkedButton: b1

            Button {
                id: b1
                iconSource: "image://theme/icon-m-toolbar-new-chat"
                onClicked: otrChatWindow.showPage("CHAT_PAGE", false)
            }

            Button {
                id: b2
                iconSource: "image://theme/icon-m-toolbar-addressbook"
                onClicked: otrChatWindow.showPage("PROPS_PAGE", false)
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

    Menu {
        id: entryContextMenu
        visualParent: otrChatWindow
        MenuLayout {
            MenuItem {
                id: menuCopyText
                text: qsTr("Copy text")
                onClicked: {
                    startPage.gpgConnector.setToClipboard(currentChatEntry);
                }
            }

            MenuItem {
                id: menuSendAgain
                text: qsTr("Send again")
                visible: false
                onClicked: {
                    sendMessage(currentChatEntry);
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

        ToolIcon {
            id: backButtonTop
            x: 0
            y: 0
            platformIconId: "toolbar-back";
            onClicked: {
                chatListView.model.clear();
                pageStack.pop();
            }
            opacity: 0.5
            height: label2.height
        }

        Label {
            id: label2
            x: 10
            y: 0
            width: parent.width - 72 - label2.height
            height: 65
            color: "#ffffff"
            text: { return getContactNameHeader(); }
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 26
            font.bold: false
            anchors.left: backButtonTop.right
        }

        Image {
            id: avatarImage
            anchors.left: label2.right
            height: {
                if(label2.height == 0)
                    return 0;
                else
                    return label2.height - 20;
            }
            width: label2.height - 20
            y: {
                if(label2.height == 0)
                    return 0;
                else
                    return 10;
            }
            source: "image://theme/icon-m-content-avatar-placeholder"
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
                    text: " ";
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22
                }

                Text {
                    id: contactNameLabel
                    height: 50
                    text: "";
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 22
                    font.bold: true

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

            Row {
                width: parent.width

                Image {
                    id: iconFinger
                    width: 50
                    height: fingerprintTextOutput.height
                    source: "qrc:/images/pix/fingerprint.png"
                    anchors.margins: 5
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }

                TextArea {
                    id: fingerprintTextOutput;
                    text: "EMPTY"
                    width: parent.width - iconFinger.width
                    readOnly: true
                    font.family: "Courier"
                    font.pixelSize: 21
                    font.bold: true
                }
            }

            GroupSeparator {
                title: qsTr("Verify Fingerprint")
            }

            Row {
                width: parent.width

                Button {
                    id: trustFP
                    width: parent.width/2
                    text: qsTr("Manually")
                    onClicked: verifyFingerprint();
                }
                Button {
                    id: trustFPSMP
                    width: parent.width/2
                    text: qsTr("SMP Verification")
                    enabled: false
                }
            }

            GroupSeparator {
                title: qsTr("Remove Fingerprint")
            }

            Button {
                id: removeFP
                width: parent.width
                text: qsTr("Remove Fingerprint")
                onClicked: deleteFingerprint();
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
            height: parent.height - rectTextInput.height - rectAdditionlInput.height
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
            id: rectTextInput
            anchors.top: flickAreaChat.bottom
            height: typeMsgField.height
            width: parent.width
            color: "white"

            Image {
                id: logoImage
                source: "image://theme/icon-m-common-locked"
                width: 48
                height: 48

                MouseArea {
                    id: encryptionModeMouseArea
                    width: parent.width
                    height: parent.height

                    onClicked: {
                        if(chatSendEncrypted)
                            dialogEncryptionMode.selectedIndex = 0;
                        else
                            dialogEncryptionMode.selectedIndex = 1;

                        dialogEncryptionMode.open();
                    }
                }
            }

            TextArea {
                id: typeMsgField
                platformStyle: myTextFieldStyle
                anchors.left: logoImage.right
                width: parent.width - logoImage.width
                height: 55
                placeholderText: qsTr("Write your message here")

                onActiveFocusChanged: {
                    if(typeMsgField.focus) {
                        rectAdditionlInput.height = 55;
                    } else {
                        rectAdditionlInput.height = 0;
                    }
                }
            }

            ProgressBar {
                id: uploadProgress
                x: typeMsgField.x
                y: typeMsgField.y
                height: 6
                width: typeMsgField.width
                minimumValue: 0
                maximumValue: 10
                value: 0
                visible: false
            }
        }

        Rectangle {
            id: rectAdditionlInput
            anchors.top: rectTextInput.bottom
            height: 0
            width: parent.width
            color: "white"
            border.width: 1
            border.color: "#eeeeee"

            Button {
                id: sendSmileyButton
                visible: true;
                text: ""
                width: 45
                height: 45
                y: 5
                x: 10
                iconSource: "image://theme/icon-s-messaging-smiley-happy"

                onClicked: {
                    if(otrConfigPage.imControlThread.getNumOfEmojis() > 0)
                        selectEmojiDialog.open();
                }
            }

            Button {
                id: sendPictureButton
                visible: true;
                text: ""
                width: 45
                height: 45
                y: 5
                x: 20 + sendSmileyButton.width
                iconSource: "image://theme/icon-s-conversation-attachment"

                onClicked: {
                    pageStack.push(pictureSelectionPage);
                }
            }

            Button {
                id: sendMsgButton
                anchors.right: parent.right
                text: qsTr("Send")
                width: parent.width / 3
                height: rectAdditionlInput.height - 10
                y: rectAdditionlInput.height / 2 - 20

                platformStyle: ButtonStyle {
                    background: "image://theme/meegotouch-button-inverted-background"
                    pressedBackground: "image://theme/color4-meegotouch-button-accent-background-pressed"
                    textColor: "white"
                }

                onClicked: {
                    rumbleEffect.running = true;
                    sendMessage(typeMsgField.text);
                    chatListView.positionViewAtBeginning();
                }
            }
        }

        TextFieldStyle {
            id: myTextFieldStyle
            backgroundSelected: ""
            background: ""
            backgroundDisabled: ""
            backgroundError: ""
        }

        Component {
            id: chatDelegate

            Item {

                ListView.onAdd: SequentialAnimation {
                    PropertyAction { target: textButton; property: "height"; value: 0 }
                    NumberAnimation { target: textButton; property: "height"; to: (systemInfo) ? (my_text.height + 10) : (my_text.height + 40); duration: animDurationIncoming; easing.type: Easing.InOutQuad }

                    onCompleted: {
                        my_text.visible = true
                    }

                    onStarted: {
                        if(animDurationIncoming > 0)
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

                        onClicked: {
                            rumbleEffect.running = true;
                            currentChatEntry = chatEntry;

                            if(remoteText)
                                menuSendAgain.visible = false;
                            else
                                menuSendAgain.visible = true;

                            console.debug("QML: Clicked message: " + currentChatEntry);
                            entryContextMenu.open();
                        }

                        platformStyle: ButtonStyle {
                            background: {
                                if(systemInfo) {
                                    if(chatEntry.indexOf("***") < 0)
                                        return "image://theme/color10-meegotouch-button-background-selected";
                                    else
                                        return "image://theme/color12-meegotouch-button-background-selected";
                                }

                                if(remoteText) {
                                    if(isEncrypted)
                                        return "qrc:/images/pix/bubble-out.png";
                                    else
                                        return "qrc:/images/pix/bubble-out-plaintext.png";
                                } else {
                                    if(isEncrypted)
                                        return "qrc:/images/pix/bubble-in.png";
                                    else
                                        return "qrc:/images/pix/bubble-in-plaintext.png";
                                }
                            }
                            pressedBackground: {
                                if(systemInfo)
                                    return "image://theme/color16-meegotouch-button-background-selected";

                                if(remoteText) {
                                    if(!isEncrypted)
                                        return "qrc:/images/pix/bubble-out.png";
                                    else
                                        return "qrc:/images/pix/bubble-out-plaintext.png";
                                } else {
                                    if(!isEncrypted)
                                        return "qrc:/images/pix/bubble-in.png";
                                    else
                                        return "qrc:/images/pix/bubble-in-plaintext.png";
                                }
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
                                var content = "<html>";

                                content += (otrConfigPage.imControlThread.makeLinksClickableInMsg(chatEntry).replace(/\n/g, "<br>") + "<br>");
                                content += "<font size='-2'>" + date + "</font></html>";

                                return content;
                            }

                            onLinkActivated: {
                                Qt.openUrlExternally(link);
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

    SelectionDialog {
        id: dialogEncryptionMode
        titleText: qsTr("Change encryption mode\n for outgoing messages:")
        selectedIndex: 0
        model: ListModel {
            ListElement { name: "OTR"; value: true }
            ListElement { name: "Plain Text"; value: false }
        }

        onAccepted: {
            changeTextSendingMode(dialogEncryptionMode.model.get(dialogEncryptionMode.selectedIndex).value);
        }
    }

    Component {
        id: contactDelegate
        Item {
            width: emojiSelectionGrid.cellWidth; height: emojiSelectionGrid.cellHeight
            Column {
                anchors.fill: parent
                Image {
                    width: parent.width; height: parent.height;
                    source: emojiPath;
                    anchors.horizontalCenter: parent.horizontalCenter

                    MouseArea {
                        id: emojiMouseArea
                        width: parent.width
                        height: parent.height
                        onClicked: {
                            console.debug("Emoji Mouse Event: " + emojiCode);
                            typeMsgField.text = typeMsgField.text + " " + emojiCode + " "
                            selectEmojiDialog.accept();
                            typeMsgField.focus = true;
                            typeMsgField.cursorPosition = typeMsgField.text.length;
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: selectEmojiDialog
        opacity: 0.8
        anchors.fill: parent

        content:
            GridView {
            id: emojiSelectionGrid
            anchors.fill: parent
            height: 500
            cellWidth: 80; cellHeight: 80

            model: ListModel {}
            delegate: contactDelegate
            //highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
            focus: true
        }

        onAccepted: {
            console.debug("Emoji: " + emojiSelectionGrid.currentIndex);
        }

    }

    Timer {
        id: loadAllMessagesLaterTimer
        interval: 460;
        running: false;
        repeat: false;
        onTriggered: getAllMessagesLater();
    }
}

