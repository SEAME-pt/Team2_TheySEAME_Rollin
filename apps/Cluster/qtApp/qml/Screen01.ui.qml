
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

        Node {
            id: leadCar
            position: Qt.vector3d(0, -325, -260) // Target vehicle to overtake (spawned much farther ahead)
            scale: Qt.vector3d(72.0, 72.0, 72.0)
            eulerRotation.y: 180
            eulerRotation.x: 0.5

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/body_Plane_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#4e79a7" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_001_Circle_000_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_002_Circle_001_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_003_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/circle_Circle_002_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#1f1f1f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/fari_Plane_002_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#f3f3f3" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/finestriniAnt_Plane_004_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/finestriniPost_Plane_005_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/grigliaAnt_Body_Plane_001_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#404040" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/grigliaPost_Body_Plane_002_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#404040" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/parabrezza_Plane_003_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/plane_001_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#4e79a7" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/stopDX_Body_Plane_005_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#a72f2f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/stopSX_Body_Plane_004_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#a72f2f" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/targa_Body_Plane_003_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#d4d4d4" }
            }

            Model {
                source: "qrc:/qml/3d-assets/carro/meshes/vetroPost_Plane_006_mesh.mesh"
                materials: DefaultMaterial { diffuseColor: "#7d9fc4" }
            }
        }

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

        SequentialAnimation {
            id: fsdTestDrive // Main test scenario animation (edit values here)
            running: true
            loops: Animation.Infinite

            PauseAnimation {
                duration: 600 // Initial hold before movement starts
            }

            // Phase 1: ego car approaches lead car in center lane
            ParallelAnimation {
                NumberAnimation {
                    target: carModel
                    property: "position.z"
                    to: 400 // Keep safe following distance before lane change
                    duration: 2000 // Faster approach
                    easing.type: Easing.Linear
                }
            }

            // Phase 2: smooth diagonal lane change LEFT while continuing forward
            ParallelAnimation {
                NumberAnimation {
                    target: carModel
                    property: "position.x"
                    to: -320 // Move into left lane center using lane-guide coordinates
                    duration: 1700 // Faster lane-change speed
                    easing.type: Easing.Linear
                }
                NumberAnimation {
                    target: carModel
                    property: "position.z"
                    to: -140 // Keep stronger forward progress during lane change
                    duration: 1700
                    easing.type: Easing.Linear
                }
            }

            // Phase 3: pass the lead car while staying in left lane
            ParallelAnimation {
                NumberAnimation {
                    target: carModel
                    property: "position.z"
                    to: -700 // Advance much farther ahead of lead car
                    duration: 1800 // Faster overtake speed
                    easing.type: Easing.Linear
                }
            }

            // Phase 4: smooth diagonal return to center lane while still moving forward
            ParallelAnimation {
                NumberAnimation {
                    target: carModel
                    property: "position.x"
                    to: 0 // Center lane X
                    duration: 1500
                    easing.type: Easing.Linear
                }
                NumberAnimation {
                    target: carModel
                    property: "position.z"
                    to: -1220 // Continue much farther forward while returning
                    duration: 1500
                    easing.type: Easing.Linear
                }
            }

            PauseAnimation {
                duration: 700 // End hold before reset
            }

            // Phase 5: reset to initial pose for loop restart
            ParallelAnimation {
                NumberAnimation {
                    target: carModel
                    property: "position.x"
                    to: 0
                    duration: 1
                }
                NumberAnimation {
                    target: carModel
                    property: "position.z"
                    to: 600
                    duration: 1
                }
            }
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
}