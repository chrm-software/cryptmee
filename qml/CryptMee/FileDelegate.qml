import QtQuick 1.1

/*
  Code from https://github.com/n9dyfi/TextEditor
  */

Rectangle {
    id: fileDelegate
    property bool isDir: false

    width: parent.width
    height: fileNameView.height * 1.5
    color: ( mouseArea.pressed )
           ? "steelblue"
           : "#00AAAAAA"
    Image
    {
        id: icon
        anchors.verticalCenter: parent.verticalCenter
        x: 16
        smooth: true
        source: isDir
                ? "image://theme/icon-m-toolbar-directory-white"
                : "qrc:/images/pix/icon-s-notes-white.png";
        visible: source != ''
    }

    Text {
        id: fileNameView
        text: fileName
        font.pixelSize: 24
        anchors.verticalCenter: parent.verticalCenter
        anchors.left:icon.right
        anchors.leftMargin: 16
        color: "white"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: {
            fileNameView.color="red"
            if ( isDir ) {
                folderChanged(filePath);
            } else {
                fileOpenRequested(filePath)
            }
        }
    }
}
