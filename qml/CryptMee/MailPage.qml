// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import com.nokia.meego 1.1
import com.nokia.extras 1.1

import MailReader 1.0

Page {
    tools: commonTools
    id: page_Mail

    property alias prop_mailReader: myMailReader
    property int showMailsTimeWindow: 7

    ToolBarLayout {
        id: commonTools
        visible: true

        ToolIcon {
            id: backButton
            platformIconId: "toolbar-back";
            //anchors.left: (parent === undefined) ? undefined : parent.left
            enabled: true
            onClicked: {
                myMenu.close();
                pageStack.pop();
            }
        }

        ToolIcon {
            id: reloadButton
            platformIconId: "toolbar-refresh";
            enabled: true
            onClicked: {
                myMailReader.readMails(showMailsTimeWindow);
                busy01.running = true;
                busy01.visible = true;
            }
        }

        ToolIcon {
            platformIconId: "toolbar-view-menu"
            //anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }

    Menu {
        id: myMenu
        visualParent: page_Mail
        MenuLayout {
            MenuItem { text: qsTr("Info")
                onClicked: {
                    helpDialog.open();
                }
            }
            MenuItem { text: qsTr("Show GnuPG log")
                onClicked: {
                    pageStack.push(gpgHistoryPage);
                }
            }
        }
    }

    Dialog {
        id: helpDialog
        opacity: 0.8
        anchors.fill: parent

        content: Rectangle {
            id: titleField
            height: blueRect.height * 2

            Rectangle {
                id: blueRect
                width: 15
                height: 150
                color: "#0093dd"
                //anchors.right: blueText
            }

            Text {
                id: blueText
                anchors.left: blueRect.right
                anchors.margins: 15
                font.pixelSize: 25
                height: blueRect.height
                color: "white"
                text: qsTr("Received email")
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                id: greenRect
                width: 15
                height: 150
                color: "#00b804"
                //anchors.right: greenText
                anchors.top: blueRect.bottom
            }

            Text {
                id: greenText
                anchors.left: blueRect.right
                anchors.top: blueText.bottom
                anchors.margins: 15
                font.pixelSize: 25
                height: greenRect.height
                color: "white"
                text: qsTr("Sent email")
                verticalAlignment: Text.AlignVCenter
            }
        }


    }

    MailReader {
        id: myMailReader

        onReady: {
            firstUpdateTimer.stop();

            console.debug("===> onReady MailReader");
            updateModel();

            busy01.running = false;
            busy01.visible = false;
        }
    }

    Timer {
        id: firstUpdateTimer
        interval: 1500
        running: false
        repeat: false

        onTriggered: {
            console.debug("Timer triggered");
            updateModel();
        }
    }

    onStatusChanged: {
        // Load Mails
        if(status === DialogStatus.Open){
            if(listViewMails.model.count === 0) {

                // Default are 7 days old mails
                myMailReader.readMails(7);

                busy01.running = true;
                busy01.visible = true;

                firstUpdateTimer.start();
            }
        }
    }

    function updateModel() {
        var tmpFrom = "";
        var tmpSubject = "";
        var tmpDate = "";
        var tmpDirection = "REC";
        var idx = -1;

        listViewMails.model.clear();

        for(var i=0; i<myMailReader.getNumOfMails(); i++) {
            tmpFrom = myMailReader.getMail(i).split("###")[0];
            tmpSubject = myMailReader.getMail(i).split("###")[1];
            tmpDate = myMailReader.getMail(i).split("###")[2];
            tmpDirection = myMailReader.getMail(i).split("###")[3];
            idx = i;

            console.debug("Mail: " + tmpSubject + " \t[" + tmpDate + "]");

            listViewMails.model.append({ name: tmpFrom, subject: tmpSubject, date: tmpDate, direction: tmpDirection, isVisible: true, idx: idx });            
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
            text: "CryptMee <font size='-5'>" + qsTr("Encrypted eMails") + "</font>"
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 32
            font.bold: false
        }

        BusyIndicator {
            id: busy01
            x: parent.width - 40
            y: 15
            width: 30
            height: 30
            running: false
            visible: false
        }
    }

    Rectangle {
        id: filterAgeArea
        width: parent.width
        anchors.top: topDecoartion.bottom
        //anchors.bottom: allMailsArea.top
        height: 65
        color: "#dddddd"

        Label {
            id: labelFilter
            text: qsTr("Newer than: ")
            height: 65
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            x: 5
            font.pixelSize: 20
            font.bold: false
        }

        Button {
            id: buttonAge
            width: parent.width - labelFilter.width - 10
            height: 55
            text: qsTr("1 week")
            iconSource: "image://theme/icon-m-common-combobox-arrow"
            x: labelFilter.width+10
            y: 5

            onClicked: {
                dialogAge.open();
            }
        }
    }

    Rectangle {
        id: allMailsArea
        width: parent.width
        height: parent.height - topDecoartion.height - filterAgeArea.height
        anchors.top: filterAgeArea.bottom
        //anchors.bottom: parent.bottom
        //y: topDecoartion.height + filterAgeArea.height


        Flickable {
            id: flickArea
            flickableDirection: Flickable.VerticalFlick
            clip: true
            anchors.fill: parent

            ListView {
                id: listViewMails
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
                    if(direction === "SEND")
                        return "#00b804";
                    else
                        return "#0093dd";
                }

                height: textLabel.height - 3
                width: 12
                x: 2
                anchors.left: parent.left
            }

            Text {
                id: textLabel
                text: "<b>"+name+"</b><br>"+subject+"<br>"+date                
                font.pixelSize: 22
                width: parent.width - colorRect.width - 2;
                height: 120
                anchors.left: colorRect.right
                anchors.margins: 6

                MouseArea {
                    //anchors.fill: parent;
                    width: listViewMails.width;
                    height: 120

                    onClicked: {
                        parent.color = "red";
                        mailViewPage.prop_idx = idx;
                        mailViewPage.prop_eMailHeader = myMailReader.getHeader(idx);

                        console.debug("[mailPage]: push MailViewer");
                        pageStack.push(mailViewPage);
                    }
                }
            }
        }
    }

    // ----- Dialogs --------

    SelectionDialog {
        id: dialogAge
        titleText: qsTr("Show only mails\n newer than:")
        selectedIndex: 2

        model: ListModel {
            ListElement { value: "1"; name: "1 day" }
            ListElement { value: "2"; name: "2 days" }
            ListElement { value: "7"; name: "1 week" }
            ListElement { value: "14"; name: "2 weeks" }
            ListElement { value: "31"; name: "1 month" }
            ListElement { value: "182"; name: "6 months" }
            ListElement { value: "365"; name: "1 year" }
            ListElement { value: "730"; name: "2 years" }
        }

        onAccepted: {
            var age = dialogAge.model.get(dialogAge.selectedIndex).value;
            buttonAge.text = dialogAge.model.get(dialogAge.selectedIndex).name
            myMailReader.readMails(age);
            showMailsTimeWindow = age;

            busy01.running = true;
            busy01.visible = true;
        }
    }

    state: (screen.currentOrientation === Screen.Portrait) ? "portrait" : "landscape"

    states: [
        State {
            name: "landscape"
            PropertyChanges {
                target: label2;
                height: { return 45 }
            }
            PropertyChanges {
                target: busy01;
                y: { return 5 }
            }
        },
        State {
            name: "portrait"
            PropertyChanges {
                target: label2;
                height: { return 65 }
            }
            PropertyChanges {
                target: busy01;
                y: { return 15 }
            }
        }
    ]

}
