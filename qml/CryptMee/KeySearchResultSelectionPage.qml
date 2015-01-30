
import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: searchKeySelectPage
    anchors.fill: parent

    property alias allKeyList: listViewKeys.model
    property string selectedKeyIDs

    onStatusChanged: {
        if(status === DialogStatus.Open){
            selectedKeyIDs = "";

        } else if(status === DialogStatus.Close) {
            listViewKeys.model.clear();
        }
    }

    function findSelected() {
        console.debug("selected keys: " + selectedKeyIDs);
        keyPage.importKeysFromServer(selectedKeyIDs);
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

                CheckBox {
                    id: checkBoxKey
                    checked: {
                        if(selected_param)
                            return true;
                        else
                            return false;
                    }

                    onCheckedChanged: {
                        if(checkBoxKey.checked)
                            selectedKeyIDs += name + "|";
                        else
                            selectedKeyIDs = selectedKeyIDs.replace(name, "");
                    }
                }

                Text {
                    id: textLabel
                    text: "<b>"+name+"</b><br>"+ids+"<br>"
                    font.pixelSize: 22
                    width: parent.width - checkBoxKey.width - 2;
                    height: {
                        var lines = ids.split("<br>");
                        return (lines.length + 2) * 30;
                    }
                    anchors.left: checkBoxKey.right
                    anchors.margins: 6
                }
            }
        }

        Rectangle {
            id: selectKey
            color: "#dddddd"
            width: parent.width; height: 64
            anchors.top: parent.top

            Button {
                id: importButton
                text: qsTr("Import from server!")
                y: 5
                width: parent.width/1.5
                onClicked: {
                    findSelected();
                    pageStack.pop();
                }
            }

            Button {
                id: cancelButton
                text: qsTr("Cancel")
                y: 5                
                anchors { left: importButton.right; right: parent.right; }
                onClicked: {
                    pageStack.pop();
                }
            }
         }

        ScrollDecorator {
            flickableItem: flickArea
        }

        Flickable {
            id: flickArea
            width: parent.width
            height: parent.height
            anchors.top: selectKey.bottom
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


    }
}
