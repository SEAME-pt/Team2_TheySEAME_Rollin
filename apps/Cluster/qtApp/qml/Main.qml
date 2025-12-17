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
            id: logo
            anchors.centerIn: parent
            source: "qrc:/qml/images/logo.png"
            fillMode: Image.PreserveAspectFit
            opacity: 0

            SequentialAnimation on opacity {
                running: true
                loops: 1

                NumberAnimation { from: 0; to: 1; duration: 1000 }

                PauseAnimation { duration: 2000 }

                NumberAnimation { from: 1; to: 0; duration: 1000 }

                onStopped: splash.visible = false  // Esconde o splash no final
            }
        }
        Loader {
            id: loader
            anchors.fill: parent
            active: !splash.visible
            source: "Screen01.ui.qml"
        }
    }
}
