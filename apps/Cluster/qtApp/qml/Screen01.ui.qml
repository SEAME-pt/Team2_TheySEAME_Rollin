
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
import QtQuick3D

Rectangle {
    id: rectangle
    width: 1280
    height: 400
    color: "#FFE18D"

    // TEST MODE: Set to true to test without systemInfo (for Qt Design Studio preview)
    property bool previewMode: false

    // 3D lane layout (matches lane guide spacing)
    readonly property real detectedCarY: -325
    readonly property real laneCenterX: 0
    readonly property real laneLeftX: -320
    readonly property real laneRightX: 320

    // Preview mode properties for generalInfo (date/time/weather)
    property string previewDate: "19/03/2026"
    property string previewTime: "14:30"
    property string previewTemperature: "22"
    property string previewWeatherInfo: "sun"

    // Preview mode properties for systemInfo
    property int previewSpeed: 85
    property int previewBattery: 78
    property bool previewCruiseActive: true
    property int previewTargetSpeed: 130

    Image {
        id: cluster
        x: 0
        y: 0
        source: "qrc:/qml/images/cluster.png"
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
            source: generalInfo ? "qrc:/qml/images/" + generalInfo.weatherInfo + "-256.png" : "qrc:/qml/images/sun-256.png"
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: targetSpeedDisplay
            x: 389
            y: 155
            width: 120
            color: "#47473f"
            text: rectangle.previewMode ? (rectangle.previewCruiseActive ? rectangle._targetSpeed.toString() + " hm/h" : "---") : (systemInfo ? systemInfo.targetSpeedDisplay : "---")
            font.pixelSize: 16
            font.family: "BaseNeueTrial-Bold"
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
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


    View3D {
        id: cars3DView
        x: (rectangle.width - width) / 2 // 2D screen X position of the 3D viewport (auto-centered)
        y: 100 // 2D screen Y position of the 3D viewport (bottom-aligned area)
        width: 560 // 3D viewport width on UI (~75% of screen)
        height: 300 // 3D viewport height on UI (~75% of screen)
        z: 60 // Layer order (keep above cluster background/UI)
        camera: camera

        environment: SceneEnvironment {
            clearColor: "transparent" // View clear color
            backgroundMode: SceneEnvironment.Transparent // Let cars3DBackground show through
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(carModel.position.x, carModel.position.y + 675, carModel.position.z + 650) // Camera follows ego car
            eulerRotation.x: -28 // Camera tilt: negative looks downward (steeper FSD-like angle)
        }

        DirectionalLight {
            eulerRotation.x: -35 // Main light vertical angle
            eulerRotation.y: -25 // Main light horizontal angle
            brightness: 1.0 // Main light intensity
        }

        DirectionalLight {
            eulerRotation.x: -20 // Fill light vertical angle
            eulerRotation.y: 35 // Fill light horizontal angle (opposite side)
            brightness: 0.35 // Fill light intensity (kept lower than main)
        }

        Node {
            id: laneGuides
            position: Qt.vector3d(0, -360, 120)

            Model {
                source: "#Cube"
                position: Qt.vector3d(-480, 0, 0)
                scale: Qt.vector3d(0.1, 0.001, 3000)
                materials: DefaultMaterial { diffuseColor: "#9b9b9b" }
            }

            Model {
                source: "#Cube"
                position: Qt.vector3d(-160, 0, 0)
                scale: Qt.vector3d(0.1, 0.001, 3000)
                materials: DefaultMaterial { diffuseColor: "#9b9b9b" }
            }

            Model {
                source: "#Cube"
                position: Qt.vector3d(160, 0, 0)
                scale: Qt.vector3d(0.1, 0.001, 3000)
                materials: DefaultMaterial { diffuseColor: "#9b9b9b" }
            }

            Model {
                source: "#Cube"
                position: Qt.vector3d(480, 0, 0)
                scale: Qt.vector3d(0.1, 0.001, 3000)
                materials: DefaultMaterial { diffuseColor: "#9b9b9b" }
            }
        }

        // Ego vehicle — fixed pose, gray body distinguishes from detected cars
        Node {
            id: carModel
            position: Qt.vector3d(0, -325, 600) // Car world position: X shift, Y height, Z depth (lower = closer to bottom)
            scale: Qt.vector3d(75.0, 75.0, 75.0) // Uniform car size (increase/decrease all 3 equally)
            eulerRotation.y: 180 // Car yaw: turn left/right (180 = rear facing camera)
            eulerRotation.x: 0.5 // Car pitch: nose up/down

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/body_Plane_mesh.mesh" // Main body shell
                materials: DefaultMaterial { diffuseColor: "#b6b6b6" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_001_Circle_000_mesh.mesh" // Wheel/tire part 1
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_002_Circle_001_mesh.mesh" // Wheel/tire part 2
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_003_mesh.mesh" // Wheel/tire part 3
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_Circle_002_mesh.mesh" // Wheel/tire part 4
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/fari_Plane_002_mesh.mesh" // Front light geometry
                materials: DefaultMaterial { diffuseColor: "#f3f3f3" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/finestriniAnt_Plane_004_mesh.mesh" // Front side windows
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/finestriniPost_Plane_005_mesh.mesh" // Rear side windows
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/grigliaAnt_Body_Plane_001_mesh.mesh" // Front grille
                materials: DefaultMaterial { diffuseColor: "#404040" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/grigliaPost_Body_Plane_002_mesh.mesh" // Rear grille/vent
                materials: DefaultMaterial { diffuseColor: "#404040" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/parabrezza_Plane_003_mesh.mesh" // Windshield
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/plane_001_mesh.mesh" // Additional body panel
                materials: DefaultMaterial { diffuseColor: "#b6b6b6" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/stopDX_Body_Plane_005_mesh.mesh" // Right tail light
                materials: DefaultMaterial { diffuseColor: "#a72f2f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/stopSX_Body_Plane_004_mesh.mesh" // Left tail light
                materials: DefaultMaterial { diffuseColor: "#a72f2f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/targa_Body_Plane_003_mesh.mesh" // License plate area
                materials: DefaultMaterial { diffuseColor: "#d4d4d4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/vetroPost_Plane_006_mesh.mesh" // Rear window glass
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }
        }

        // Detected center-lane vehicle
        DetectedCar3D {
            id: detectedFrontCar
            visible: systemInfo && systemInfo.frontCarVisible
            x: rectangle.laneCenterX
            y: rectangle.detectedCarY
            z: systemInfo ? (-100 - (100 - systemInfo.frontCarDistance) * 4) : -500
            bodyColor: "#4e79a7"

            Behavior on z { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
        }

        // Detected left-lane vehicle
        DetectedCar3D {
            id: detectedLeftCar
            visible: systemInfo && systemInfo.leftCarVisible
            x: rectangle.laneLeftX
            y: rectangle.detectedCarY
            z: systemInfo ? (-100 - (100 - systemInfo.leftCarDistance) * 4) : -500
            bodyColor: "#4e79a7"

            Behavior on z { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
        }

        // Detected right-lane vehicle
        DetectedCar3D {
            id: detectedRightCar
            visible: systemInfo && systemInfo.rightCarVisible
            x: rectangle.laneRightX
            y: rectangle.detectedCarY
            z: systemInfo ? (-100 - (100 - systemInfo.rightCarDistance) * 4) : -500
            bodyColor: "#4e79a7"

            Behavior on z { NumberAnimation { duration: 300; easing.type: Easing.InOutQuad } }
        }
    }

//    Rectangle {
//        id: cars3DFrameDebug
//        x: cars3DView.x // Debug frame follows 3D view X
//        y: cars3DView.y // Debug frame follows 3D view Y
//        width: cars3DView.width // Debug frame follows 3D view width
//        height: cars3DView.height // Debug frame follows 3D view height
//        z: 200 // Keep above all dashboard content for visibility
//        color: "transparent" // No tint inside debug frame
//        border.color: "#ff0000" // High-contrast debug border color
//        border.width: 3 // Thicker border for easier visibility
//    }

    Rectangle {
        id: speedLimitPanel
        x: 760
        y: 120
        width: 70
        height: 70
        color: "transparent"
        z: 1000
        parent: rectangle

        property var speedImages: {
            "30": "speed_30",
            "50": "speed_50",
            "60": "speed_60",
            "80": "speed_80",
            "100": "speed_100",
            "120": "speed_120"
        }

        Image {
            anchors.centerIn: parent
            width: 60
            height: 60
            fillMode: Image.PreserveAspectFit
            source: {
                var limit = generalInfo ? generalInfo.trafficSignSpeedLimit.toString() : "0"
                var img = speedLimitPanel.speedImages[limit]
                if (!img) return ""
                return "qrc:/qml/images/" + img + ".png"
            }
            visible: source !== ""
        }
    }

    Rectangle {
        id: trafficSignPanel
        x: 840
        y: 120
        width: 70
        height: 70
        color: "transparent"
        z: 1000
        parent: rectangle

        property var signImages: [
            "",            // 0 UNKNOWN
            "stop",        // 1 STOP
            "",            // 2 SPEED_LIMIT_30 (shown in speedLimitPanel)
            "",            // 3 SPEED_LIMIT_50 (shown in speedLimitPanel)
            "",            // 4 SPEED_LIMIT_80 (shown in speedLimitPanel)
            "",            // 5 SPEED_LIMIT_100 (shown in speedLimitPanel)
            "",            // 6 SPEED_LIMIT_120 (shown in speedLimitPanel)
            "yield",       // 7 YIELD
            "no_entry",    // 8 NO_ENTRY
            "turn_left",   // 9 TURN_LEFT
            "turn_right",  // 10 TURN_RIGHT
            "pedestrian",  // 11 PEDESTRIAN
            "traffic_light", // 12 TRAFFIC_LIGHT
            "one_way",     // 13 ONE_WAY
            "no_parking",  // 14 NO_PARKING
            "no_overtaking" // 15 NO_OVERTAKING
        ]

        Image {
            anchors.centerIn: parent
            width: 60
            height: 60
            fillMode: Image.PreserveAspectFit
            source: {
                var idx = generalInfo ? generalInfo.trafficSignInfo : 0
                if (idx < 0 || idx >= trafficSignPanel.signImages.length) return ""
                return "qrc:/qml/images/" + trafficSignPanel.signImages[idx] + ".png"
            }
        }
    }

    Rectangle {
        id: cruiseControl
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: -12
        anchors.bottomMargin: 0
        width: 160
        height: 60
        color: "transparent"
        border.color: "transparent"
        z: 30

        Row {
            anchors.centerIn: parent
            spacing: 8
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: ccImage
                source: systemInfo && systemInfo.cruiseActive ? "qrc:/qml/images/CC_enabled_green.png" : "qrc:/qml/images/CC_disabled.png"
                fillMode: Image.PreserveAspectFit
                width: 40
                height: 40
            }

            Text {
                text: systemInfo ? systemInfo.targetSpeedDisplay : "---"
                width: 70
                font.pixelSize: 18
                font.family: "BaseNeueTrial-Bold"
                font.bold: true
                color: "#000000"
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
    states: [
        State {
            name: "clicked"
        }
    ]

    Item {
        id: adasWarningNotification
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 24
        width: 440
        height: contentColumn.implicitHeight + 28
        z: 2100
        visible: opacity > 0.01
        opacity: systemInfo && systemInfo.adasWarningVisible ? 1 : 0
        scale: systemInfo && systemInfo.adasWarningVisible ? 1 : 0.96
        y: systemInfo && systemInfo.adasWarningVisible ? 0 : -20

        Behavior on opacity {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }
        Behavior on scale {
            NumberAnimation { duration: 320; easing.type: Easing.OutCubic }
        }
        Behavior on y {
            NumberAnimation { duration: 320; easing.type: Easing.OutCubic }
        }

        property real countdownProgress: 1.0

        function restartCountdown() {
            if (!systemInfo || !systemInfo.adasWarningTimed)
                return
            countdownProgress = 1.0
            countdownAnim.stop()
            countdownAnim.duration = Math.max(systemInfo.adasWarningRemainingMs, 250)
            countdownAnim.from = 1.0
            countdownAnim.to = 0.0
            countdownAnim.start()
        }

        NumberAnimation on countdownProgress {
            id: countdownAnim
            duration: 12000
            easing.type: Easing.Linear
        }

        Connections {
            target: systemInfo
            function onAdasWarningUpdated() {
                if (systemInfo.adasWarningVisible)
                    adasWarningNotification.restartCountdown()
            }
        }

        onOpacityChanged: {
            if (opacity > 0.99)
                restartCountdown()
        }

        Rectangle {
            anchors.fill: parent
            radius: 12
            color: "#FFE18D"
            border.color: "#47473f"
            border.width: 2
        }

        Column {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 14
            spacing: 10

            Row {
                width: parent.width
                spacing: 14

                Image {
                    source: "qrc:/qml/images/warning.png"
                    fillMode: Image.PreserveAspectFit
                    width: 44
                    height: 44
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    width: parent.width - 58
                    text: systemInfo ? systemInfo.adasWarningMessage : ""
                    font.pixelSize: 22
                    font.family: "BaseNeueTrial-Bold"
                    font.bold: true
                    color: "#47473f"
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Row {
                width: parent.width
                spacing: 10
                visible: systemInfo && systemInfo.adasWarningTimed

                Rectangle {
                    width: parent.width - 52
                    height: 10
                    radius: 2
                    color: "#47473f"
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * adasWarningNotification.countdownProgress
                        radius: 2
                        color: "#76b047"
                    }
                }

                Text {
                    width: 42
                    horizontalAlignment: Text.AlignRight
                    text: {
                        if (!systemInfo || !systemInfo.adasWarningTimed)
                            return ""
                        return Math.max(1, Math.ceil(systemInfo.adasWarningRemainingMs / 1000)) + "s"
                    }
                    font.pixelSize: 14
                    font.family: "Inter"
                    font.bold: true
                    color: "#47473f"
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}