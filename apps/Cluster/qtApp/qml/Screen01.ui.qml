
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
    width: 1280
    height: 400
    color: "#FFE18D"

    Image {
        id: cluster
        x: 0
        y: 0
        source: "images/cluster.png"
        fillMode: Image.PreserveAspectFit

        Row {
            x: 121
            y: 162
            spacing: 6

            Text {
                id: speed
                color: "#47473f"
                text: systemInfo ? systemInfo.speed.toString() : "0"
                font.pixelSize: 98
                font.family: "BaseNeueTrial-Bold"
                font.bold: true
            }

            Text {
                id: speed_unity
                color: "#47473f"
                text: qsTr("hm/h")
                font.pixelSize: 28
                font.bold: true
                font.family: "BaseNeueTrial-Bold"
                anchors.bottom: speed.bottom
            }
        }


        Text {
            id: battery_text
            x: 982
            y: 141
            width: 218
            height: 119
            color: "#47473f"
            text: systemInfo ? systemInfo.battery.toString() + "%" : "0%"
            font.pixelSize: 77
            font.family: "BaseNeueTrial-Bold"
            font.bold: true
        }

        Text {
            id: date_text
            x: 923
            y: 20
            width: 155
            height: 34
            color: "#ffffff"
            text: generalInfo ? generalInfo.currentDate.toString(
                                     "dd/MM/yyyy") : ""
            font.pixelSize: 28
            font.family: "Inter"
            font.bold: true
        }

        Text {
            id: time_text
            x: 598
            y: 20
            width: 72
            height: 34
            color: "#ffffff"
            text: generalInfo ? generalInfo.localTime : ""
            font.pixelSize: 28
            font.family: "Inter"
            font.bold: true
        }

        Text {
            id: weather
            x: 286
            y: 21
            width: 72
            height: 34
            color: "#ffffff"
            text: generalInfo ? generalInfo.temperature + "°C" : ""
            font.pixelSize: 28
            font.family: "Inter"
            font.bold: true
        }

        Image {
            id: image
            x: 241
            y: 20
            width: 47
            height: 35
            source: generalInfo ? "images/" + generalInfo.weatherInfo + "-256.png" : "images/sun-256.png"
            fillMode: Image.PreserveAspectFit
        }

        Rectangle {
            id: maxBattery
            x: 1012
            y: 290
            width: 164
            height: 12
            color: "#47473f"
            border.color: "#47473f"

            Rectangle {
                id: baterry
                x: 0
                y: 0
                width: systemInfo ? (systemInfo.battery / 100) * maxBattery.width : 0
                height: 12
                color: "#76b047"
                border.color: "#76b047"
            }
        }
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
