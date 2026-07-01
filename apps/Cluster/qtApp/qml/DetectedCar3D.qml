import QtQuick
import QtQuick3D

// Reusable Fiat UNO mesh set for detected vehicles (same geometry as ego car).
Node {
    id: root

    property color bodyColor: "#4e79a7"

    scale: Qt.vector3d(72.0, 72.0, 72.0)
    eulerRotation.y: 180
    eulerRotation.x: 0.5

    Model {
        source: "qrc:/qml/3d-assets/carro/meshes/body_Plane_mesh.mesh"
        materials: DefaultMaterial { diffuseColor: root.bodyColor }
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
        materials: DefaultMaterial { diffuseColor: root.bodyColor }
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
