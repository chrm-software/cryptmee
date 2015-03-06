import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: passwordInputPage
    anchors.fill: parent

    property string prop_content: ""
    property string prop_state: ""
    property alias prop_passwd: passwordField.text
    property string prop_private_key: ""

    signal clicked()

    onClicked: {}
    onStatusChanged: {
        if(status === DialogStatus.Open){
            passwordField.text = "";
            passwordField.forceActiveFocus();
        }
    }

    Item {
        anchors.fill: parent

        Column {
            spacing: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 40

            Label {
                id: descriptionLabel
                text: qsTr("Enter GnuPG passphrase for your private key:")
                horizontalAlignment: Text.Center
                anchors.left: parent.left
                anchors.right: parent.right
            }

            TextField {
                id: passwordField
                echoMode: TextInput.Password
                placeholderText: qsTr("Password")
                anchors.left: parent.left
                anchors.right: parent.right
            }
            Button {
                id: passwordButton
                text: qsTr("OK")
                anchors.left: parent.left
                anchors.right: parent.right
                onClicked: {
                    if(prop_passwd === "") {
                        // nothing
                    } else {
                        if(prop_state === "TXT_DECRYPT")
                            startPage.pgpDecrypt(prop_content, prop_state);
                        else if((prop_state === "SIGNKEY"))
                            keyPage.signKeyWithMine(prop_content, prop_private_key);

                        pageStack.pop();
                    }
                }
            }

            Button {
                id: passwordButtonC
                text: qsTr("Cancel")
                anchors.left: parent.left
                anchors.right: parent.right
                onClicked: {
                    prop_passwd = "EMPTY";
                    pageStack.pop();
                }

            }
        }
    }
}
