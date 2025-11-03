

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.15
import "../Dashboard"
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Rectangle {
    id: rectangle
    width: Constants.width
    height: Constants.height
    color: "#ffe9aa"

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
        text: qsTr("42")
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
        source: "images/Line 1.svg"
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: line2
        x: 701
        source: "images/Line 2.svg"
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: h
        x: 233
        y: 307
        source: "images/h.png"
        fillMode: Image.PreserveAspectFit
    }

    Image {
        id: _
        x: 1136
        y: 301
        width: 21
        height: 29
        source: "images/_.png"
        fillMode: Image.PreserveAspectFit
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
