import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 1024
    height: 600
    visible: true
    title: "Left Lane Car Distance Animation Test"
    color: "#333333"

    Rectangle {
        anchors.fill: parent
        color: "#1a1a1a"

        // Simulated cluster background
        Rectangle {
            id: clusterBg
            x: 50
            y: 50
            width: 800
            height: 400
            color: "#FFE18D"
            border.color: "#333333"
            border.width: 2

            Text {
                x: 20
                y: 20
                text: "Cluster Preview (Lane Detection Test)"
                color: "#333333"
                font.pixelSize: 24
                font.bold: true
            }

            // Distance indicator
            Text {
                x: 20
                y: 80
                text: "Distance: " + Math.round(testDistance) + "%"
                color: "#333333"
                font.pixelSize: 20
                font.bold: true
            }

            // Simulated road lanes
            Rectangle {
                x: 100
                y: 120
                width: 600
                height: 200
                color: "#222222"
                opacity: 0.3

                Text {
                    anchors.centerIn: parent
                    text: "Road Lanes"
                    color: "#666666"
                    font.pixelSize: 16
                }
            }

            // LEFT LANE CAR - Using exact interpolation formula
            Image {
                id: leftCarPreview
                x: 575 - 175 * (testDistance / 100)
                y: 82 + 141 * (testDistance / 100)
                width: 46 + 137 * (testDistance / 100)
                height: 52 + 80 * (testDistance / 100)
                source: "file:///home/hhzamith/SEA_ME/ElRepo/apps/Cluster/qtApp/qml/images/leftCar.png.png"
                fillMode: Image.PreserveAspectFit
                opacity: 0.85

                Behavior on x { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
                Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
                Behavior on height { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
            }

            // Position indicators
            Text {
                x: 20
                y: 340
                text: "Position: (" + Math.round(leftCarPreview.x) + ", " + Math.round(leftCarPreview.y) + ")"
                color: "#47473f"
                font.pixelSize: 14
            }

            Text {
                x: 20
                y: 365
                text: "Size: " + Math.round(leftCarPreview.width) + "x" + Math.round(leftCarPreview.height)
                color: "#47473f"
                font.pixelSize: 14
            }
        }

        // Control panel
        Rectangle {
            x: 50
            y: 480
            width: 800
            height: 100
            color: "#2a2a2a"
            border.color: "#555555"
            border.width: 1

            Text {
                x: 20
                y: 10
                text: "Animation Status: Cycling 0% → 100% → 0%"
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }

            Text {
                x: 20
                y: 35
                text: "Farthest position: x=575, y=82, size=46x52"
                color: "#aaaaaa"
                font.pixelSize: 12
            }

            Text {
                x: 20
                y: 55
                text: "Closest position: x=400, y=223, size=183x132"
                color: "#aaaaaa"
                font.pixelSize: 12
            }

            Text {
                x: 20
                y: 75
                text: "Watch the car slide and scale smoothly as distance changes"
                color: "#888888"
                font.pixelSize: 12
                font.italic: true
            }
        }
    }

    // Test animation
    property real testDistance: 0

    SequentialAnimation on testDistance {
        running: true
        loops: Animation.Infinite

        // 0 to 100 (farthest to closest)
        NumberAnimation {
            from: 0
            to: 100
            duration: 4000
            easing.type: Easing.InOutQuad
        }

        PauseAnimation { duration: 500 }

        // 100 to 0 (closest to farthest)
        NumberAnimation {
            from: 100
            to: 0
            duration: 4000
            easing.type: Easing.InOutQuad
        }

        PauseAnimation { duration: 500 }
    }
}
