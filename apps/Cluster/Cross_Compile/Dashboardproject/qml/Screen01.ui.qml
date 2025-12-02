
/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.15
import "../"
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Rectangle {
    id: rectangle
    property var speedometerHandleCpp
    width: 1280
    height: 400
    color: "#FFE18D"

    Image {
        id: car
        x: 421
        y: 110
        source: "images/car.png"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: speedometer
        x: 158
        y: 268
        width: 68
        height: 66
        text: speedometerHandleCpp.speed
        font.pixelSize: 77
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: "Jaro"
    }

    Text {
        id: battery
        x: 1053
        y: 268
        width: 68
        height: 66
        text: qsTr("42")
        font.pixelSize: 77
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: "Jaro"
    }

    Image {
        id: line1
        x: 459
        source: "images/Line1.svg"
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: line2
        x: 701
        source: "images/Line2.svg"
        fillMode: Image.PreserveAspectFit
    }

    Text {
        id: percentage
        x: 1127
        y: 268
        width: 68
        height: 66
        text: qsTr("%")
        font.pixelSize: 55
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: "Jaro"
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
