import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

Page {
    tools: commonTools
    id: mainPage

    property alias textField: textarea1.text
    property alias errorField: textarea2.text

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

            onClicked: {
                console.debug("Open Mail Client.");
                infoBanner.text = qsTr("Sending eMail...");
                infoBanner.show();

                Qt.openUrlExternally("mailto:?subject=Encrypted eMail&body="+textarea1.text);
            }
        }

        ToolIcon {
            id: clearText
            //anchors.left: sendEMail.right
            iconSource: "image://theme/icon-m-toolbar-backspace"

            onClicked: {
                textarea1.text = "";
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
                }
            }

            MenuItem { text: qsTr("Encrypt")
                onClicked: {
                    startPage.pgpEncrypt();
                }
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
        anchors.top: errorMessage.bottom
        width: parent.width
        height: parent.height - textarea2.height - showErrors.height
        color: "transparent"

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
