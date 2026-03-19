
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

    // TEST MODE: Set to true to test without systemInfo (for Qt Design Studio preview)
    property bool previewMode: true
    property real previewLeftCarDistance: 100  // Start at closest position
    
    // Preview mode properties for generalInfo (date/time/weather)
    property string previewDate: "19/03/2026"
    property string previewTime: "14:30"
    property string previewTemperature: "22"
    property string previewWeatherInfo: "sun"
    
    // Preview mode properties for systemInfo
    property int previewSpeed: 85
    property int previewBattery: 78

    // Auto-animate in a loop
    SequentialAnimation {
        running: previewMode
        loops: Animation.Infinite

        NumberAnimation {
            target: rectangle
            property: "previewLeftCarDistance"
            from: 100
            to: 0
            duration: 4000
            easing.type: Easing.InOutQuad
        }

        PauseAnimation { duration: 500 }

        NumberAnimation {
            target: rectangle
            property: "previewLeftCarDistance"
            from: 0
            to: 100
            duration: 4000
            easing.type: Easing.InOutQuad
        }

        PauseAnimation { duration: 500 }
    }

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
                text: rectangle.previewMode ? rectangle.previewSpeed.toString() : (systemInfo ? systemInfo.speed.toString() : "0")
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
            text: rectangle.previewMode ? rectangle.previewBattery.toString() + "%" : (systemInfo ? systemInfo.battery.toString() + "%" : "0%")
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
            text: rectangle.previewMode ? rectangle.previewDate : (generalInfo ? generalInfo.currentDate.toString("dd/MM/yyyy") : "")
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
            text: rectangle.previewMode ? rectangle.previewTime : (generalInfo ? generalInfo.localTime : "")
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
            text: rectangle.previewMode ? rectangle.previewTemperature + "°C" : (generalInfo ? generalInfo.temperature + "°C" : "")
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
            source: rectangle.previewMode ? ("images/" + rectangle.previewWeatherInfo + "-256.png") : (generalInfo ? "images/" + generalInfo.weatherInfo + "-256.png" : "images/sun-256.png")
            fillMode: Image.PreserveAspectFit
        }

        Image {
            id: cruiseIndicator
            x: 389
            y: 110
            width: 80
            height: 36
            source: "images/CC.png"
            fillMode: Image.PreserveAspectFit
            visible: systemInfo ? systemInfo.cruiseActive : false
        }

        // Detected vehicles overlays (left lane)
        Image {
            id: leftLaneDetected
            property real distance: rectangle.previewMode ? rectangle.previewLeftCarDistance : (systemInfo ? systemInfo.leftCarDistance : 0)
            x: 575 - 175 * (distance / 100)
            y: 82 + 141 * (distance / 100)
            width: 46 + 137 * (distance / 100)
            height: 52 + 80 * (distance / 100)
            source: "images/leftCar.png.png"
            fillMode: Image.PreserveAspectFit
            visible: true
            opacity: 0.85

            Behavior on x { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
            Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
            Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
            Behavior on height { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
        }

        // Detected vehicles overlays (front/center lane)
        Image {
            id: frontLaneDetected
            x: 864
            y: 97
            width: 120
            height: 80
            source: "images/frontCar.png.png"
            fillMode: Image.PreserveAspectFit
            visible: true  // Test: visible to preview layout
            opacity: 0.85
        }

        // Detected vehicles overlays (right lane)
        Image {
            id: rightLaneDetected
            x: 674
            y: 150
            width: 120
            height: 80
            source: "images/rightCar.png"
            fillMode: Image.PreserveAspectFit
            visible: true  // Test: visible to preview layout
            opacity: 0.85
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
                width: rectangle.previewMode ? (rectangle.previewBattery / 100) * maxBattery.width : (systemInfo ? (systemInfo.battery / 100) * maxBattery.width : 0)
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
