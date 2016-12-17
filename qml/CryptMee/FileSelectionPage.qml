import QtQuick 1.1
import com.nokia.meego 1.0
import Qt.labs.folderlistmodel 1.0

Page {
    id: fileSelectionPage
    anchors.fill: parent

    property string selected_file_path: ""
    property string action_prop: "CHAT"

    signal folderChanged(string path);
    signal fileOpenRequested(string path);

    onFolderChanged: {
            folderModel.folder = path;
    }

    onFileOpenRequested: {
        if(action_prop == "MAIL")
            mainPage.attachEncryptedFile(path);
        else
            otrChatWindow.uploadImage(path, true);

        pageStack.pop();
    }

    Rectangle {
        width: parent.width
        height: parent.height
        color: "black"

        ListView {
            id: listView
            clip: true
            height: parent.height;
            width: parent.width;

            delegate: FileDelegate {
                isDir: folderModel.isFolder(index)
            }

            model: FolderListModel {
                id: folderModel
                nameFilters: ["*"]
                showDirs: true
                showDotAndDotDot: true
            }
        }

        Button {
            id: cancelButton
            text: qsTr("Cancel")
            width: parent.width/2
            height: 60
            x: parent.width/2 - cancelButton.width/2
            y: parent.height - cancelButton.height

            platformStyle: ButtonStyle {
                background: "image://theme/meegotouch-button-inverted-background"
                pressedBackground: "image://theme/color4-meegotouch-button-accent-background-pressed"
                textColor: "white"
            }

            onClicked: {
                pageStack.pop();
            }
        }
    }
}
