import QtQuick.Window 2.15
import "../Dashboard"
Window {
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "Dashboard"

    flags: Qt.FramelessWindowHint
    visibility: Window.FullScreen

    Screen01 {
        id: mainScreen
    }

}

