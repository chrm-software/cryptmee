import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

Page {
    tools: commonTools
    id: gpgHistoryPage

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
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }

    Menu {
        id: myMenu
        visualParent: gpgHistoryPage
        MenuLayout {
            MenuItem { text: qsTr("Toggle wrap mode")
                onClicked: {
                    if(textarea1.wrapMode == Text.NoWrap)
                        textarea1.wrapMode = Text.Wrap;
                    else
                        textarea1.wrapMode = Text.NoWrap;
                }
            }

            MenuItem { text: qsTr("Write to file")
                onClicked: {
                    if(startPage.gpgConnector.saveHistory("/home/user/MyDocs/cryptmee.log")) {
                        // OK
                        infoBanner.text = qsTr("Log saved to: MyDocs/cryptmee.log");
                        infoBanner.show();
                    } else {
                        // NOK
                        infoBanner.text = qsTr("Error: unable to save logfile!");
                        infoBanner.show();
                    }
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
            width: parent.width - 40
            height: 65
            color: "#ffffff"
            text: "CryptMee <font size='-5'>" + qsTr("Logfile view") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }
    }

    onStatusChanged: {
        if(status === DialogStatus.Open){
            textarea1.text = startPage.gpgConnector.getHistory();
        }
    }

    Rectangle {
        anchors.top: topDecoartion.bottom
        width: parent.width
        height: parent.height - topDecoartion.height
        color: "black"

        ScrollDecorator {
            flickableItem: flickAreaTxt
        }

        Flickable {
            id: flickAreaTxt
            anchors.fill: parent
            contentWidth: parent.width
            contentHeight: textarea1.height
            //flickableDirection: Flickable.VerticalFlick
            clip: true

            Text {
                id: textarea1
                width: parent.width
                color: "#38ff6d"
                text: ""
                font.family: "Courier"
                font.pixelSize: 19
                anchors.margins: 4
            }
        }
    }

}
