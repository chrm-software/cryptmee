import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1
import ImControlThread 1.0

Page {
    tools: commonTools
    id: otrConfigPage

    /////////////////////////////////
    property string selectedAccountName: ""
    property int isInitialized: 0
    property alias imControlThread: myImControlThread
    property alias allFingerprintsList: listViewFingerprints.model
    property int accountID: 0
    /////////////////////////////////

    // Init Connector
    ImControlThread {
        id: myImControlThread

        onInitReady: {

            var currentAccountNames = startPage.gpgConnector.settingsGetValue("SETTINGS_OTR_ACCOUNT1").split("|");
            selectedAccountName = currentAccountNames[0];

            if(selectedAccountName !== "") {
                buttonAccount.text = currentAccountNames[1];
                switchEnableOTR.enabled = true;

                var useKey = startPage.gpgConnector.settingsGetValue("SETTINGS_OTR_AUTORUN");
                if(useKey === "1") {
                    switchEnableOTR.checked = true;
                    startPage.otrIcon = "qrc:/images/pix/otr_active_toolbar.png"
                }

                isInitialized = 1;

            } else {
                switchEnableOTR.checked = false;
            }

            // Read Keys here!
            fillAccountsInDialog();
        }

        onOtrErrorOccured: {
            busyIndicator.running = false;
            busyIndicator.visible = false;
            switchEnableOTR.checked = false;
        }

        onOtrPrivKeyMustBeCreated: {
            askKeyGenDialog.open();
        }

        onOtrPrivateKeyGenerated: {
            busyIndicator.running = false;
            busyIndicator.visible = false;
            switchEnableOTR.checked = true;
            fingerprintTextOutput.text = myImControlThread.getFingerprintForAccount(selectedAccountName);
        }

        onOtrIsRunning: {
            initOTR();
            fillFingerprints();
        }

        onOtrHasStopped: {
            startPage.otrIcon = "qrc:/images/pix/otr_inactive_toolbar.png"
            switchEnableOTR.checked = false;
        }

        onOtrUpdateFingerprints: {
            fillFingerprints();
        }

        onOtrUpdateChatHistory: {
            console.debug("Got chat history update for: " + _contactName);
            otrChatWindow.getMessageFor(_contactName);
        }
    }

    function initOTR() {
        console.debug("QML: initOTR()");
        myImControlThread.initialize();
    }

    function fillAccountsInDialog() {
        dialogAccount.model.clear();

        var size = myImControlThread.getNumOfXMPPAccounts();
        var idx = 0;

        for(var i=0; i<size; i++) {
            var tmpAccount = myImControlThread.getXMPPAccount(i).split("|");
            dialogAccount.model.append({ name: tmpAccount[1], value: tmpAccount[0] });

            if(selectedAccountName === tmpAccount[0]) {
                idx = i;
                console.debug("QML: fillAccountsInDialog(): " + idx);
                accountID = idx;
            }
        }

        dialogAccount.selectedIndex = 0;
    }

    function fillFingerprints() {
        allFingerprintsList.clear();
        var size = myImControlThread.getNumOfFingerprints();

        for(var i=0; i<size; i++) {
            var tmpFP = myImControlThread.getFingerprint(i).split("|");
            allFingerprintsList.append({ name: tmpFP[1], fingerprint: tmpFP[0], onlineState: tmpFP[2] });
            console.debug("QML: fillFingerprints(): " + tmpFP[1]);
        }
    }

    function createPrivateKey() {
        if(selectedAccountName != "") {
            busyIndicator.running = true;
            busyIndicator.visible = true
            myImControlThread.createPrivateKey(selectedAccountName);
        } else {
            initOTR();
        }
    }

    onStatusChanged: {
        if(status === DialogStatus.Open){
            initOTR();
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
            text: "CryptMee <font size='-5'>" + qsTr("OTR Settings") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    Rectangle {
        id: allSettings
        height: parent.height - topDecoartion.height
        width: parent.width
        color: "transparent"
        border.color: "transparent"
        anchors.top: topDecoartion.bottom
        //anchors.bottom: parent.bottom


        Flickable {
            id: flickAreaSettings
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
                height: 750

                GroupSeparator {
                    title: qsTr("OTR Account Binding")
                }

                Rectangle {
                    id: filterAgeArea
                    width: parent.width
                    height: 65
                    color: "transparent"

                    Button {
                        id: buttonAccount
                        width: parent.width
                        height: 55
                        text: qsTr("Select Account")
                        iconSource: "image://theme/icon-m-common-combobox-arrow"

                        onClicked: {
                            dialogAccount.open();
                            dialogAccount.selectedIndex = accountID;
                        }
                    }
                }

                GroupSeparator {
                    title: qsTr("Enable OTR")
                }

                Row {
                    width: parent.width;
                    spacing: 10

                    Rectangle {
                        width: 1
                        height: ownKeyId.height
                        color: "transparent"
                    }

                    Label {
                        id: ownKeyId
                        wrapMode: Text.Wrap
                        width: parent.width - switchEnableOTR.width - 21
                        text: qsTr("Enable OTR.\nIt will run automatically on startup");
                    }

                    Switch {
                        id: switchEnableOTR;
                        height: ownKeyId.height
                        checked: false
                        enabled: false

                        onCheckedChanged: {
                            if(switchEnableOTR.checked) {
                                console.debug("QML: OTR switched to 1: init OTR for: " + selectedAccountName);
                                if(myImControlThread.startOTR(selectedAccountName)) {
                                    fingerprintTextOutput.text = myImControlThread.getFingerprintForAccount(selectedAccountName);
                                } else {
                                    //debugTextOutput.text = "Error: unable to start OTR.";
                                }
                            } else {
                                console.debug("QML: OTR switched to 0: stop OTR for: " + selectedAccountName);
                                myImControlThread.stopOTR();
                                busyIndicator.running = false;
                                busyIndicator.visible = false;
                                fingerprintTextOutput.text = "Inactive.";
                            }
                        }
                    }
                }

                GroupSeparator {
                    title: qsTr("My fingerprint")
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
                    title: qsTr("Known fingerprints")
                }

                Rectangle {
                    id: fingerprintsArea
                    width: parent.width
                    height: 200
                    color: "transparent"

                    Flickable {
                        id: flickArea
                        width: parent.width
                        height: 260
                        flickableDirection: Flickable.VerticalFlick
                        clip: true

                        ScrollDecorator {
                            flickableItem: listViewFingerprints
                        }

                        ListView {
                            id: listViewFingerprints
                            width: parent.width; height: parent.height
                            delegate: delegate
                            focus: true
                            model: ListModel { }
                        }
                    }
                }
            }
        }
    }

    // ----- Dialogs --------

    SelectionDialog {
        id: dialogAccount
        titleText: qsTr("Select account\nfor OTR usage:")
        selectedIndex: 0
        model: ListModel {
            ListElement { name: "empty" }
        }

        onAccepted: {
            var account = dialogAccount.model.get(dialogAccount.selectedIndex).name;
            var account_path = dialogAccount.model.get(dialogAccount.selectedIndex).value;
            buttonAccount.text = account;

            if(account_path !== selectedAccountName) {
                // User has changed account. Stop OTR and let him restart manually
                switchEnableOTR.checked = false;
            }

            console.debug("QML: Selected account: " + account_path);
            selectedAccountName = account_path;
            switchEnableOTR.enabled = true;
        }
    }

    QueryDialog  {
        id: askKeyGenDialog
        titleText: qsTr("Generate Private Key")
        message:  qsTr("A private key must be generated first for this account. It will take some time. You can reduce the time by using your phone more intense. Start a game or use the camera!");
        acceptButtonText: qsTr("Generate!")
        rejectButtonText: qsTr("Cancel")
        onAccepted: {
            createPrivateKey();
        }

        onRejected: {
            switchEnableOTR.checked = false;
        }
    }

    Component {
        id: delegate

        Item {
            height: {
                return textLabel.height;
            }

            Rectangle {
                id: colorRect
                color: "orange"
                height: textLabel.height - 3
                width: 12
                x: 2
                anchors.left: parent.left
            }

            Rectangle {
                id: colorRect2
                color: {
                    if(onlineState === "offline")
                        return "grey";
                    else
                        return "green";
                }
                height: textLabel.height - 3
                width: 12
                x: colorRect.width + 2
                anchors.left: colorRect.right
            }

            Text {
                id: textLabel
                text: "<b>"+name+"</b>"
                font.pixelSize: 26
                width: parent.width - 28;
                height: 65
                anchors.margins: 6
                anchors.left: colorRect2.right

                MouseArea {
                    width: listViewFingerprints.width;
                    height: textLabel.height

                    onClicked: {
                        parent.color = "red";
                        var contact = name;

                        // TODO: let user enter a message here!
                        otrChatWindow.contactName = contact;
                        otrChatWindow.contactFingerprint = fingerprint;
                        pageStack.push(otrChatWindow);
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
        }
    ]
}
