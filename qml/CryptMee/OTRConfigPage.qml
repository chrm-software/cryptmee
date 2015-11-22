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
    //property alias allFingerprintsList: listViewFingerprints.model
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
                    isInitialized = 1;
                }                

            } else {
                switchEnableOTR.checked = false;
            }

            // Read Keys here!
            fillAccountsInDialog();

            var showMsgsAsNotification = startPage.gpgConnector.settingsGetValue("SETTINGS_OTR_SHOW_NOTIFICATIONS");
            if(showMsgsAsNotification === "1") {
                switchShowMsgNotification.checked = true;
            }
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
            chatContacts.fillContacts();
        }

        onOtrHasStopped: {
            startPage.otrIcon = "qrc:/images/pix/otr_inactive_toolbar.png"
            switchEnableOTR.checked = false;
            isInitialized = 0;
        }

        onOtrUpdateFingerprints: {
            chatContacts.fillContacts();
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
                height: 700

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
                    title: qsTr("Additional Options")
                }

                Row {
                    width: parent.width;
                    spacing: 10

                    Rectangle {
                        width: 1
                        height: labelShowMsgNotification.height
                        color: "transparent"
                    }

                    Label {
                        id: labelShowMsgNotification
                        wrapMode: Text.Wrap
                        width: parent.width - switchEnableOTR.width - 21
                        text: qsTr("Show incoming messages as system notification");
                    }

                    Switch {
                        id: switchShowMsgNotification;
                        height: labelShowMsgNotification.height
                        checked: false
                        enabled: true

                        onCheckedChanged: {
                            if(switchShowMsgNotification.checked) {
                                startPage.gpgConnector.settingsSetValue("SETTINGS_OTR_SHOW_NOTIFICATIONS", "1");
                            } else {
                                startPage.gpgConnector.settingsSetValue("SETTINGS_OTR_SHOW_NOTIFICATIONS", "0");
                            }
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
