
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: keyInputPage
    anchors.fill: parent

    property alias allKeyList: listViewKeys.model
    property alias filterKeys: textfield1.text
    property string selectedKeyID
    property string stateAfterExit

    signal clicked()

    onClicked: {}
    onStatusChanged: {
        if(status === DialogStatus.Open){
            startPage.updatePublicKeys();
            selectedKeyID = "-1";

        } else if(status === DialogStatus.Close) {
            listViewKeys.model.clear();
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height


        Component {
            id: delegate

            Item {
                visible: isVisible
                height: {
                    if(isVisible)
                        return textLabel.height;
                    else
                        return 0;
                }

                Rectangle {
                    id: colorRect
                    color: {
                        if(trust == "u")
                            return "green";
                        if(trust == "m" || trust == "f")
                            return "orange";
                        if(trust == "n" || trust == "-" || trust == "q" || trust == "i")
                            return "red";
                        return "grey";
                    }

                    height: textLabel.height-3
                    width: 12
                    x: 2
                    anchors.left: parent.left
                }

                Text {
                    id: textLabel
                    text: "<b>"+name+"</b><br>"+ids+"<br>"
                    font.pixelSize: 22
                    width: parent.width - colorRect.width - 2;
                    height: {
                        var lines = ids.split("<br>");
                        return (lines.length + 2) * 30;
                    }
                    anchors.left: colorRect.right
                    anchors.margins: 6

                    MouseArea {
                        //anchors.fill: parent;
                        width: listViewKeys.width;
                        height: textLabel.height

                        onClicked: {
                            parent.color = "red";                            
                            selectedKeyID = name;

                            if(stateAfterExit == "TXT_ENCRYPT")
                                startPage.pgpEncrypt(name, stateAfterExit);
                            else if(stateAfterExit == "SHOWKEY")
                                keyPage.showOneKey();

                            pageStack.pop()
                        }
                    }
                }
            }
        }

        Rectangle {
            id: searchOperation
            color: "#dddddd"
            width: parent.width; height: 64
            anchors.top: parent.top

            TextField {
                id: textfield1
                width: parent.width/1.5
                y: 5
                text: ""
                placeholderText: qsTr("Search...")
                platformStyle: TextFieldStyle { paddingRight: clearText.width }
                inputMethodHints: Qt.ImhNoPredictiveText;

                onAccepted: {
                    textfield1.selectAll();
                    startPage.updatePublicKeys();
                }

                onTextChanged: {
                    startPage.updatePublicKeys();
                }

                Image {
                     id: clearText

                     anchors {
                         top: parent.top;
                         right: parent.right;
                         margins: 3;
                     }

                     fillMode: Image.PreserveAspectFit
                     smooth: true; visible: textfield1.text
                     source: "image://theme/icon-m-input-clear"
                     height: 42
                     width: 42

                     MouseArea {
                         id: clear
                         anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                         height: textfield1.height; width: textfield1.height
                         onClicked: {
                             textfield1.text = ""
                         }
                     }
                 }
            }


            Button {
                id: cancelButton
                text: qsTr("Cancel")
                y: 5
                anchors { left: textfield1.right; right: parent.right; }
                onClicked: {
                    selectedKeyID = "-1";
                    pageStack.pop();
                }
            }
         }

        Flickable {
            id: flickArea
            width: parent.width
            height: parent.height
            anchors.top: searchOperation.bottom
            anchors.bottom: parent.bottom
            flickableDirection: Flickable.VerticalFlick
            clip: true

            ListView {
                id: listViewKeys
                width: parent.width; height: parent.height
                delegate: delegate
                focus: true
                model: ListModel { }
            }
        }

        ScrollDecorator {
            flickableItem: flickArea
        }
    }
}
