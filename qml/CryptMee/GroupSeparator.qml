import QtQuick 1.1
import com.nokia.meego 1.0

Item {
    property string title

    width: parent.width
    height: 40

        Image {
            id: sortingDivisionLine
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 16
            anchors.left: parent.left
            anchors.rightMargin: 16
            anchors.right: sortingLabel.left
            source: "image://theme/meegotouch-groupheader" + (theme.inverted ? "-inverted" : "") + "-background"
        }
        Text {
            id: sortingLabel
            text: title
            font.pixelSize: 16
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.topMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 16
            color: theme.inverted ? "#6b6d6b": "#8c8e8c"
        }

}
