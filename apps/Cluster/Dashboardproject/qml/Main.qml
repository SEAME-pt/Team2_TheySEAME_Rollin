import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "Dashboard"

    flags: Qt.FramelessWindowHint
    Screen01 {
        id: mainScreen
    }
}
