import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "qtApp"

    flags: Qt.FramelessWindowHint
    Screen01 {
        id: mainScreen
    }
    Item {
        id: splash
        anchors.fill: parent
        visible: true

        Rectangle {
            anchors.fill: parent
            color: "black"
        }

        Image {
            anchors.centerIn: parent
            source: "qrc:/qml/images/logo.png"
            fillMode: Image.PreserveAspectFit
        }

        Timer {
            interval: 2000
            running: true
            repeat: false
            onTriggered: splash.visible = false
        }
        Loader {
            id: loader
            anchors.fill: parent
            active: !splash.visible
            source: "Screen01.ui.qml"
        }
    }
}
