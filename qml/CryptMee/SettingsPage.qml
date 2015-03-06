// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

Page {
    tools: commonTools
    id: settingsPage

    property alias pGPG_PATH: gpgbin.text
    property alias pGPG_KEYSERVER: keyServer.text
    property alias pEmailDir: emailDir.text
    property alias pEmailDB: emailDB.text
    property alias pUseOwnKey: switchEncryptWithMyKey.checked

    property string textFieldColor: "green"

    function fillValues() {
        pGPG_PATH = startPage.gpgConnector.settingsGetValue("SETTINGS_GPGPATH");
        pGPG_KEYSERVER = startPage.gpgConnector.settingsGetValue("SETTINGS_GPGKEYSERVER");
        pEmailDir = startPage.gpgConnector.settingsGetValue("SETTINGS_MAILDIR");
        pEmailDB = startPage.gpgConnector.settingsGetValue("SETTINGS_MAILDB");

        var useKey = startPage.gpgConnector.settingsGetValue("SETTINGS_USEOWNKEY");
        if(useKey === "1")
            pUseOwnKey = true;
        else
            pUseOwnKey = false;

    }

    function gpgRetValVersion(_result, _content) {
        if(_result === true) {
            console.debug("Got version: " + _content);
            textFieldColor = "black";

        } else {
            console.debug("Got error: " + _content);
            textFieldColor = "red";
        }
    }

    onStatusChanged: {
        if(status === DialogStatus.Open){
            fillValues();
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
            text: "CryptMee <font size='-5'>" + qsTr("Settings") + "</font>"
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
                height: 590

                GroupSeparator {
                    title: qsTr("GnuPG binary path:")
                }

                TextField {
                    id: gpgbin;
                    text: ""
                    width: parent.width
                    style: TextFieldStyle { textColor: textFieldColor; }

                    inputMethodHints: Qt.ImhNoPredictiveText;

                    onTextChanged: {
                        startPage.getPGPVersion(gpgbin.text);
                    }
                }

                GroupSeparator {
                    title: qsTr("PGP Keyserver:")
                }

                TextField {
                    id: keyServer;
                    text: ""
                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText;
                }

                GroupSeparator {
                    title: qsTr("Email directory and DB:")
                }


                TextField {
                    id: emailDir;
                    text: ""
                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText;
                }

                TextField {
                    id: emailDB;
                    text: ""
                    width: parent.width
                    inputMethodHints: Qt.ImhNoPredictiveText;
                }

                GroupSeparator {
                    title: qsTr("Use own public key:")
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
                        width: parent.width - switchEncryptWithMyKey.width - 21
                        text: qsTr("Allwas encrypt with my own public key");
                    }

                    Switch {
                        id: switchEncryptWithMyKey;
                        height: ownKeyId.height
                        checked: {
                            var useKey = startPage.gpgConnector.settingsGetValue("SETTINGS_USEOWNKEY");
                            if(useKey === "1")
                                return true;
                            else
                                return false;
                        }
                    }
                }


                GroupSeparator {

                }

                Button {
                    id: saveSettings
                    text: qsTr("Save Settings")
                    width: parent.width
                    iconSource: "image://theme/icon-m-toolbar-done"

                    onClicked: {
                        startPage.gpgConnector.settingsSetValue("SETTINGS_GPGKEYSERVER", pGPG_KEYSERVER);
                        startPage.gpgConnector.settingsSetValue("SETTINGS_GPGPATH", pGPG_PATH);
                        startPage.gpgConnector.settingsSetValue("SETTINGS_MAILDIR", pEmailDir);
                        startPage.gpgConnector.settingsSetValue("SETTINGS_MAILDB", pEmailDB);

                        if(switchEncryptWithMyKey.checked) {
                            startPage.gpgConnector.settingsSetValue("SETTINGS_USEOWNKEY", "1");
                        } else {
                            startPage.gpgConnector.settingsSetValue("SETTINGS_USEOWNKEY", "0");
                        }

                        pageStack.pop();
                    }
                }

                Button {
                    id: resetSettings
                    text: qsTr("Reset All Values")
                    width: parent.width
                    iconSource: "image://theme/icon-m-toolbar-refresh1"

                    onClicked: {
                        startPage.gpgConnector.settingsReset();
                        fillValues();
                    }
                }

            }
        }

        ScrollDecorator {
            flickableItem: flickAreaSettings
        }
    }

}
