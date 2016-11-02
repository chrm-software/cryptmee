import QtQuick 1.1
import com.nokia.meego 1.0
import QtMobility.gallery 1.1

Page {
    id: pictureSelectionPage
    anchors.fill: parent

    property string selected_picture_path: ""

    Rectangle {
        width: parent.width
        height: parent.height
        color: "black"

        GridView {
            anchors.fill: parent
            cellWidth: 160
            cellHeight: 160

            maximumFlickVelocity: 3000

            model: DocumentGalleryModel {
                rootType: DocumentGallery.Image
                properties: [ "url", "dateTaken" ]
                sortProperties: [ "-dateTaken" ]
                filter: GalleryWildcardFilter {
                    property: "fileName";
                    value: "*.jpg";
                }
                autoUpdate: true
            }

            delegate: Image {
                smooth: false
                sourceSize.width: width
                sourceSize.height: height
                width: 160
                height: 160

                asynchronous: true
                source: "file:///home/user/.thumbnails/grid/" + Qt.md5(url) + ".jpeg"

                MouseArea {
                    id: mouseAre1
                    anchors.fill: parent
                    onClicked: {
                        otrChatWindow.reloadChatContent = false;
                        selected_picture_path = url;
                        otrChatWindow.uploadImage(url);
                        pageStack.pop(); }
                }
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
                otrChatWindow.reloadChatContent = false;
                pageStack.pop();
            }
        }
    }
}
