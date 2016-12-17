import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow

    initialPage: {
        // TODO: handle arguments
        if(ARG_C === 1)
            return startPage;
        else
            return keyDialog;
    }

    /////////////////////// Init all pages ///////////////////
    MainPage {
        id: mainPage
    }

    MailPage {
        id: mailPage
    }

    MailViewer {
        id: mailViewPage
    }

    PasswordInputPage {
        id: passwordDialog
    }

    PubKeySelectionPage {
        id: keyDialog
    }

    StartPage {
        id: startPage
    }

    KeyPage {
        id: keyPage
    }

    SettingsPage {
        id: settingsPage
    }

    KeySearchResultSelectionPage {
        id: searchKeyDialog
    }

    GpgHistoryViewPage {
        id: gpgHistoryPage
    }

    OTRConfigPage {
        id: otrConfigPage
    }

    OTRChatWindow {
        id: otrChatWindow
    }

    ChatContacts {
        id: chatContacts
    }

    PictureSelectionPage {
        id: pictureSelectionPage
    }

    FileSelectionPage {
        id: fileSelectionPage
    }

    /////////////////////////////////////////////////////////////





}
