import QtQuick
import QtQuick3D

Node {
    id: node

    // Resources
    Texture {
        id: c___Users__DeveLion__Desktop__Targa_Diffuse_Color_png_texture
        objectName: "C:\\Users\\DeveLion\\Desktop\\Targa Diffuse Color.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        // Source texture path expected: maps/Targa Diffuse Color.png
        // Skipped property: source, reason: Failed to find texture at /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/qml/3d-assets/carro/C://Users//DeveLion//Desktop//Targa Diffuse Color.png
    }
    Texture {
        id: ruote_Diffuse_Color_png_texture
        objectName: "Ruote Diffuse Color.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        // Source texture path expected: maps/Ruote Diffuse Color.png
        // Skipped property: source, reason: Failed to find texture at /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/qml/3d-assets/carro/Ruote Diffuse Color.png
    }
    Texture {
        id: __texture
        objectName: "."
        generateMipmaps: true
        mipFilter: Texture.Linear
        source: "maps/."
    }
    Texture {
        id: parabrezza_png_texture
        objectName: "Parabrezza.png"
        generateMipmaps: true
        mipFilter: Texture.Linear
        // Source texture path expected: maps/Parabrezza.png
        // Skipped property: source, reason: Failed to find texture at /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/qml/3d-assets/carro/Parabrezza.png
    }
    PrincipledMaterial {
        id: vetroPost_material
        objectName: "VetroPost"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: body_material
        objectName: "Body"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: ruote_002_material
        objectName: "Ruote.002"
        baseColor: "#ffa3a3a3"
        baseColorMap: ruote_Diffuse_Color_png_texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: parabrezza_material
        objectName: "Parabrezza"
        baseColor: "#ffa3a3a3"
        baseColorMap: parabrezza_png_texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: finestrini_001_material
        objectName: "Finestrini.001"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: ruote_005_material
        objectName: "Ruote.005"
        baseColor: "#ffa3a3a3"
        baseColorMap: ruote_Diffuse_Color_png_texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: finestrini_material
        objectName: "Finestrini"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: ruote_material
        objectName: "Ruote"
        baseColor: "#ffa3a3a3"
        baseColorMap: ruote_Diffuse_Color_png_texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: ruote_001_material
        objectName: "Ruote.001"
        baseColor: "#ffa3a3a3"
        baseColorMap: ruote_Diffuse_Color_png_texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: fari_material
        objectName: "Fari"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: stopSX_001_material
        objectName: "StopSX.001"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: stopDX_material
        objectName: "StopDX"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: grigliaAnt_material
        objectName: "GrigliaAnt"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: grigliaPost_material
        objectName: "GrigliaPost"
        baseColor: "#ffa3a3a3"
        baseColorMap: __texture
        indexOfRefraction: 1
    }
    PrincipledMaterial {
        id: targa_material
        objectName: "Targa"
        baseColor: "#ffa3a3a3"
        baseColorMap: c___Users__DeveLion__Desktop__Targa_Diffuse_Color_png_texture
        indexOfRefraction: 1
    }

    // Nodes:
    Node {
        id: lowPolyFiatUNO_obj
        objectName: "LowPolyFiatUNO.obj"
        Model {
            id: targa_Body_Plane_003
            objectName: "Targa_Body_Plane.003"
            source: "meshes/targa_Body_Plane_003_mesh.mesh"
            materials: [
                targa_material
            ]
        }
        Model {
            id: grigliaPost_Body_Plane_002
            objectName: "GrigliaPost_Body_Plane.002"
            source: "meshes/grigliaPost_Body_Plane_002_mesh.mesh"
            materials: [
                grigliaPost_material
            ]
        }
        Model {
            id: grigliaAnt_Body_Plane_001
            objectName: "GrigliaAnt_Body_Plane.001"
            source: "meshes/grigliaAnt_Body_Plane_001_mesh.mesh"
            materials: [
                grigliaAnt_material
            ]
        }
        Model {
            id: stopDX_Body_Plane_005
            objectName: "StopDX_Body_Plane.005"
            source: "meshes/stopDX_Body_Plane_005_mesh.mesh"
            materials: [
                stopDX_material
            ]
        }
        Model {
            id: stopSX_Body_Plane_004
            objectName: "StopSX_Body_Plane.004"
            source: "meshes/stopSX_Body_Plane_004_mesh.mesh"
            materials: [
                stopSX_001_material
            ]
        }
        Model {
            id: fari_Plane_002
            objectName: "Fari_Plane.002"
            source: "meshes/fari_Plane_002_mesh.mesh"
            materials: [
                fari_material
            ]
        }
        Model {
            id: circle_001_Circle_000
            objectName: "Circle.001_Circle.000"
            source: "meshes/circle_001_Circle_000_mesh.mesh"
            materials: [
                ruote_001_material
            ]
        }
        Model {
            id: circle_002_Circle_001
            objectName: "Circle.002_Circle.001"
            source: "meshes/circle_002_Circle_001_mesh.mesh"
            materials: [
                ruote_material
            ]
        }
        Model {
            id: vetroPost_Plane_006
            objectName: "VetroPost_Plane.006"
            source: "meshes/vetroPost_Plane_006_mesh.mesh"
            materials: [
                vetroPost_material
            ]
        }
        Model {
            id: plane_001
            objectName: "Plane.001"
            source: "meshes/plane_001_mesh.mesh"
            materials: [
                body_material
            ]
        }
        Model {
            id: circle_Circle_002
            objectName: "Circle_Circle.002"
            source: "meshes/circle_Circle_002_mesh.mesh"
            materials: [
                ruote_002_material
            ]
        }
        Model {
            id: parabrezza_Plane_003
            objectName: "Parabrezza_Plane.003"
            source: "meshes/parabrezza_Plane_003_mesh.mesh"
            materials: [
                parabrezza_material
            ]
        }
        Model {
            id: finestriniAnt_Plane_004
            objectName: "FinestriniAnt_Plane.004"
            source: "meshes/finestriniAnt_Plane_004_mesh.mesh"
            materials: [
                finestrini_001_material
            ]
        }
        Model {
            id: body_Plane
            objectName: "Body_Plane"
            source: "meshes/body_Plane_mesh.mesh"
            materials: [
                body_material
            ]
        }
        Model {
            id: circle_003
            objectName: "Circle.003"
            source: "meshes/circle_003_mesh.mesh"
            materials: [
                ruote_005_material
            ]
        }
        Model {
            id: finestriniPost_Plane_005
            objectName: "FinestriniPost_Plane.005"
            source: "meshes/finestriniPost_Plane_005_mesh.mesh"
            materials: [
                finestrini_material
            ]
        }
    }

    // Animations:
}
